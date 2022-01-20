/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2019-2022 NXP */

#include <ctype.h>
#include <image.h>
#include <imagetool.h>
#include <inttypes.h>
#include <linux/kernel.h>
#include <s32gen1image.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define UNSPECIFIED	-1

#define MBR_OFFSET			0x0
#define MBR_SIZE			0x200

#define S32GEN1_QSPI_IVT_OFFSET		0x0
#define S32GEN1_SD_IVT_OFFSET		0x1000
#define IVT_VERSION			(0x60)
#define IVT_TAG				(0xd1)

#define S32GEN1_QSPI_PARAMS_SIZE	(0x200U)
#define S32GEN1_QSPI_PARAMS_OFFSET	(0x200U)

#define S32GEN1_SECBOOT_HSE_RES_SIZE 0x80000ul

#define APPLICATION_BOOT_CODE_TAG	(0xd5)
#define APPLICATION_BOOT_CODE_VERSION	(0x60)

#define S32_AUTO_OFFSET ((size_t)(-1))

#define DCD_HEADER			(0x600000d2)
#define DCD_MAXIMUM_SIZE		(8192)
#define DCD_HEADER_LENGTH_OFFSET	(1)

#define DCD_COMMAND_HEADER(tag, len, params) ((tag) | \
					      (cpu_to_be16((len)) << 8) | \
					      (params) << 24)
#define DCD_WRITE_TAG	(0xcc)
#define DCD_CHECK_TAG	(0xcf)
#define DCD_NOP_TAG	(0xc0)

#define DCD_PARAMS(SET, MASK, LEN) \
	(((LEN) & 0x7) | \
	((SET) ? (1 << 4) : 0x0) | \
	((MASK) ? (1 << 3) : 0x0))

#define DCD_WRITE_HEADER(n, params)	DCD_COMMAND_HEADER(DCD_WRITE_TAG, \
							   4 + (n) * 8, \
							   (params))
#define DCD_CHECK_HEADER(params)	DCD_COMMAND_HEADER(DCD_CHECK_TAG, \
							   16, \
							   (params))
#define DCD_CHECK_HEADER_NO_COUNT(params) \
					DCD_COMMAND_HEADER(DCD_CHECK_TAG, \
							   12, \
							   (params))
#define DCD_NOP_HEADER			DCD_COMMAND_HEADER(DCD_NOP_TAG, 4, 0)

#define DCD_ADDR(x)	cpu_to_be32((x))
#define DCD_MASK(x)	cpu_to_be32((x))
#define DCD_COUNT(x)	cpu_to_be32((x))

typedef int (*parser_handler_t)(char *);

struct image_config {
	struct {
		uint32_t *data;
		size_t size;
		size_t allocated_size;
	} dcd;
	struct {
		uint8_t *data;
		size_t size;
	} qspi_params;
	bool flash_boot;
	bool secboot;
};

struct fip_image_data {
	__u32		toc_header_name;
	__u32		dont_care1;
	__u64		dont_care2;
	__u8		uuid[16];
	__u64		offset;
	__u64		size;
	__u8		dont_care3[0];
};

struct line_parser {
	const char *tag;
	parser_handler_t parse;
};

enum boot_type {
	QSPI_BOOT,
	SD_BOOT,
	EMMC_BOOT,
	INVALID_BOOT
};

enum dcd_cmd {
	WRITE,
	CLEAR_MASK,
	SET_MASK,
	CHECK_MASK_CLEAR,
	CHECK_MASK_SET,
	CHECK_NOT_MASK,
	CHECK_NOT_CLEAR,
	INVALID_DCD_CMD,
};

struct dcd_args {
	enum dcd_cmd cmd;
	uint32_t addr;
	union {
		uint32_t mask;
		uint32_t value;
	};
	uint32_t nbytes;
	uint32_t count;
};

static const char * const dcd_cmds[] = {
	[WRITE] = "WRITE",
	[CLEAR_MASK] = "CLEAR_MASK",
	[SET_MASK] = "SET_MASK",
	[CHECK_MASK_CLEAR] = "CHECK_MASK_CLEAR",
	[CHECK_MASK_SET] = "CHECK_MASK_SET",
	[CHECK_NOT_MASK] = "CHECK_NOT_MASK",
	[CHECK_NOT_CLEAR] = "CHECK_NOT_CLEAR",
};

static struct image_config iconfig;

static struct program_image image_layout = {
	.mbr_reserved = {
		.offset = MBR_OFFSET,
		.size = MBR_SIZE,
	},
	.ivt = {
		.size = sizeof(struct ivt),
	},
	.qspi_params = {
		.offset = S32GEN1_QSPI_PARAMS_OFFSET,
		.size = S32GEN1_QSPI_PARAMS_SIZE,
	},
	.dcd = {
		.offset = S32_AUTO_OFFSET,
		.alignment = 0x200U,
		.size = DCD_MAXIMUM_SIZE,
	},
	.hse_reserved = {
		.offset = S32_AUTO_OFFSET,
		.alignment = 0x2000U,
		.size = S32GEN1_SECBOOT_HSE_RES_SIZE,
	},
	.app_code = {
		.offset = S32_AUTO_OFFSET,
		.alignment = 0x200U,
		.size = sizeof(struct application_boot_code),
	},
	.code = {
		.offset = S32_AUTO_OFFSET,
		.alignment = 0x200U,
		.size = 0,
	},
};

static const uint32_t default_dcd_data[] = {
	DCD_HEADER,
	DCD_NOP_HEADER,
	DCD_NOP_HEADER,
	DCD_NOP_HEADER,
	DCD_NOP_HEADER,
};

static const uint32_t *get_dcd_data(size_t *size)
{
	if (iconfig.dcd.data) {
		*size = iconfig.dcd.size;
		return iconfig.dcd.data;
	}

	*size = sizeof(default_dcd_data);
	return &default_dcd_data[0];
}

static struct ivt *get_ivt(struct program_image *image)
{
	return (struct ivt *)image->ivt.data;
}

static uint8_t *get_dcd(struct program_image *image)
{
	return image->dcd.data;
}

static struct application_boot_code *get_app_code(struct program_image *image)
{
	return (struct application_boot_code *)image->app_code.data;
}

/* Areas of SRAM reserved by BootROM according to the
 * Reset and Boot: Boot: Program Image section of the Reference Manual,
 * while taking into account the fact that SRAM is mirrored at 0x3800_0000.
 */

struct reserved_range {
	void *start;
	void *end;
};

static struct reserved_range reserved_sram[] = {
	{.start = (void *)0x34008000, .end = (void *)0x34078000},
	{.start = (void *)0x38008000, .end = (void *)0x38078000},
	{.start = (void *)0x343ff000, .end = (void *)0x34400000},
	{.start = (void *)0x383ff000, .end = (void *)0x38400000},
};

static void enforce_reserved_ranges(void *image_start, int image_length)
{
	void *image_end = (void*)((__u8*)image_start + image_length);
	int i;

	for (i = 0; i < ARRAY_SIZE(reserved_sram); i++)
		if (image_start < reserved_sram[i].end &&
		    image_end > reserved_sram[i].start) {
			fprintf(stderr, "Loading data of size 0x%x at 0x%p "
				"forbidden.", image_length, image_start);
			fprintf(stderr, " Range 0x%p --- 0x%p is reserved!\n",
				reserved_sram[i].start, reserved_sram[i].end);
			exit(EXIT_FAILURE);
		}
}

static int image_parts_comp(const void *p1, const void *p2)
{
	const struct image_comp **part1 = (typeof(part1))p1;
	const struct image_comp **part2 = (typeof(part2))p2;

	if (!*part1 && !*part2)
		return 0;

	if (!*part1)
		return -1;

	if (!*part2)
		return 1;

	if ((*part2)->offset > (*part1)->offset)
		return -1;

	if ((*part2)->offset < (*part1)->offset)
		return 1;

	return 0;
}

static void check_overlap(struct image_comp *comp1,
			  struct image_comp *comp2)
{
	size_t end1 = comp1->offset + comp1->size;
	size_t end2 = comp2->offset + comp2->size;

	if (end1 > comp2->offset && end2 > comp1->offset) {
		fprintf(stderr,
			"Detected overlap between 0x%zx@0x%zx and 0x%zx@0x%zx\n",
			comp1->size, comp1->offset,
			comp2->size, comp2->offset);
		exit(EXIT_FAILURE);
	}
}

static void s32_compute_dyn_offsets(struct image_comp **parts, size_t n_parts)
{
	size_t i;
	size_t start_index = 0U, previous;

	/* Skip empty entries */
	while (!parts[start_index])
		start_index++;

	previous = start_index;
	for (i = start_index; i < n_parts; i++) {
		if (!parts[i])
			continue;

		if (parts[i]->offset == S32_AUTO_OFFSET) {
			if (i == start_index) {
				parts[i]->offset = 0U;
				continue;
			}

			parts[i]->offset = parts[previous]->offset +
			    parts[previous]->size;
		}

		/* Apply alignment constraints */
		if (parts[i]->alignment != 0U)
			parts[i]->offset = ROUND(parts[i]->offset,
						 parts[i]->alignment);

		if (i != start_index)
			check_overlap(parts[previous], parts[i]);

		previous = i;
	}
}

static struct qspi_params *get_qspi_params(struct program_image *image)
{
	return (struct qspi_params *)image->qspi_params.data;
}

static void s32gen1_set_qspi_params(struct qspi_params *qspi_params)
{
	memcpy(qspi_params, iconfig.qspi_params.data,
	       iconfig.qspi_params.size);
}

static void set_data_pointers(struct program_image *layout, void *header)
{
	uint8_t *data = (uint8_t *)header;

	layout->ivt.data = data + layout->ivt.offset;

	if (iconfig.flash_boot)
		layout->qspi_params.data = data + layout->qspi_params.offset;

	layout->dcd.data = data + layout->dcd.offset;
	layout->app_code.data = data + layout->app_code.offset;
}

static int read_fip_image(struct image_tool_params *tool_params,
			  struct fip_image_data *fip_data)
{
	FILE *f;
	uint8_t bl2_uuid[] = FIP_BL2_UUID;

	f = fopen(tool_params->datafile, "rb");
	if (!f) {
		fprintf(stderr, "Cannot open %s\n", tool_params->datafile);
		exit(EXIT_FAILURE);
	}

	if (fread(fip_data, sizeof(struct fip_image_data), 1, f) != 1) {
		fprintf(stderr, "Cannot read from %s\n", tool_params->datafile);
		exit(EXIT_FAILURE);
	}

	if (fip_data->toc_header_name != FIP_TOC_HEADER_NAME) {
		fclose(f);
		return -EINVAL;
	}

	if (fip_data->offset != FIP_BL2_OFFSET ||
	    memcmp(&fip_data->uuid[0], &bl2_uuid[0], sizeof(bl2_uuid))) {
		fprintf(stderr,
			"s32gen1image: FIP image does not have a BL2"
			" at offset %x\n",
		FIP_BL2_OFFSET);
		exit(EXIT_FAILURE);
	}

	fclose(f);
	return 0;
}

static void s32gen1_set_header(void *header, struct stat *sbuf, int unused,
			       struct image_tool_params *tool_params)
{
	size_t code_length;
	size_t pre_code_padding;
	size_t dcd_data_size;
	uint8_t *dcd;
	struct ivt *ivt;
	struct application_boot_code *app_code;
	struct fip_image_data fip_data;
	const uint32_t *dcd_data;

	set_data_pointers(&image_layout, header);
	dcd_data = get_dcd_data(&dcd_data_size);

	dcd = get_dcd(&image_layout);
	if (dcd_data_size > DCD_MAXIMUM_SIZE) {
		fprintf(stderr, "DCD exceeds the maximum size\n");
		exit(EXIT_FAILURE);
	}
	memcpy(dcd, &dcd_data[0], dcd_data_size);
	*(uint16_t *)(dcd + DCD_HEADER_LENGTH_OFFSET) =
						cpu_to_be16(dcd_data_size);

	ivt = get_ivt(&image_layout);
	app_code = get_app_code(&image_layout);

	ivt->tag = IVT_TAG;
	ivt->length = cpu_to_be16(sizeof(struct ivt));
	ivt->version = IVT_VERSION;

	ivt->dcd_pointer = image_layout.dcd.offset;
	ivt->boot_configuration_word = BCW_BOOT_TARGET_A53_0;
	ivt->application_boot_code_pointer = image_layout.app_code.offset;

	if (iconfig.secboot)
		ivt->hse_h_firmware_pointer = image_layout.hse_reserved.offset;

	app_code->tag = APPLICATION_BOOT_CODE_TAG;
	app_code->version = APPLICATION_BOOT_CODE_VERSION;

	pre_code_padding = image_layout.code.offset
				- image_layout.app_code.offset
				- image_layout.app_code.size;

	if (tool_params->ep < tool_params->addr) {
		fprintf(stderr,
			"The entrypoint is higher than the load address (0x%x < 0x%x)\n",
			tool_params->ep, tool_params->addr);
		exit(1);
	}

	if (read_fip_image(tool_params, &fip_data)) {
		code_length = sbuf->st_size
				- image_layout.app_code.offset
				- offsetof(struct application_boot_code, code);
		code_length += pre_code_padding;
		app_code->code_length = code_length;

		app_code->ram_start_pointer = tool_params->addr
							- pre_code_padding;
		app_code->ram_entry_pointer = tool_params->ep;
	} else {
		printf("mkimage: s32gen1image: %s is a FIP image\n",
		       tool_params->datafile);
		app_code->ram_start_pointer =
					tool_params->addr
					- FIP_BL2_OFFSET
					- pre_code_padding;
		app_code->ram_entry_pointer = tool_params->ep;
		app_code->code_length = fip_data.size
					+ FIP_BL2_OFFSET
					+ pre_code_padding;
	}
	/* The ' code_length', like plenty of entries in the Program
	 * Image structure, must be 512 bytes aligned.
	 */
	app_code->code_length = ROUND(app_code->code_length, 512);

	if (!iconfig.flash_boot) {
		enforce_reserved_ranges((void *)(__u64)
					app_code->ram_start_pointer,
					app_code->code_length);
	} else {
		s32gen1_set_qspi_params(get_qspi_params(&image_layout));
	}

	image_layout.code.size = sbuf->st_size - image_layout.app_code.offset -
		image_layout.app_code.size;

	if (iconfig.dcd.data)
		free(iconfig.dcd.data);

	if (iconfig.qspi_params.data)
		munmap(iconfig.qspi_params.data, iconfig.qspi_params.size);
}

static int s32gen1_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_S32GEN1IMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static struct image_comp *
get_image_qspi_params(struct program_image *program_image)
{
	if (iconfig.flash_boot)
		return &program_image->qspi_params;

	return NULL;
}

static struct image_comp *
get_image_hse_params(struct program_image *program_image)
{
	if (iconfig.secboot)
		return &program_image->hse_reserved;

	return NULL;
}

static struct image_comp *
get_image_mbr(struct program_image *program_image)
{
	/* Available on SD only */
	if (!iconfig.flash_boot)
		return &program_image->mbr_reserved;

	return NULL;
}

static int s32g2xx_build_layout(struct program_image *program_image,
				size_t *header_size, void **image)
{
	uint8_t *image_layout;
	struct image_comp *parts[] = {
		get_image_mbr(program_image),
		get_image_qspi_params(program_image),
		&program_image->ivt,
		&program_image->dcd,
		get_image_hse_params(program_image),
		&program_image->app_code,
		&program_image->code,
	};
	size_t last_comp = ARRAY_SIZE(parts) - 1;

	/* Compute auto-offsets */
	s32_compute_dyn_offsets(parts, ARRAY_SIZE(parts));

	qsort(&parts[0], ARRAY_SIZE(parts), sizeof(parts[0]), image_parts_comp);

	*header_size = parts[last_comp]->offset + parts[last_comp]->size;

	image_layout = calloc(*header_size, sizeof(*image_layout));
	if (!image_layout) {
		perror("Call to calloc() failed");
		return -ENOMEM;
	}

	*image = image_layout;
	return 0;
}

static bool is_ivt_header(const void *ptr)
{
	struct ivt *ivt = (struct ivt *)ptr;

	if (ivt->tag != IVT_TAG)
		return false;

	if (ivt->version != IVT_VERSION)
		return false;

	if (ivt->length != cpu_to_be16(sizeof(struct ivt)))
		return false;

	return true;
}

static struct ivt *get_ivt_from_raw_blob(const unsigned char *ptr, int size,
					 bool *qspi_ivt)
{
	uint32_t offset, min_size;

	offset = S32GEN1_QSPI_IVT_OFFSET;
	min_size = offset + sizeof(struct ivt);

	if (size < min_size)
		return NULL;

	if (is_ivt_header(ptr)) {
		if (qspi_ivt)
			*qspi_ivt = true;
		return (struct ivt *)ptr;
	}

	offset = S32GEN1_SD_IVT_OFFSET;
	min_size = offset + sizeof(struct ivt);

	if (size < min_size)
		return NULL;

	ptr += offset;
	if (is_ivt_header(ptr)) {
		if (qspi_ivt)
			*qspi_ivt = false;
		return (struct ivt *)ptr;
	}

	return NULL;
}

static void s32gen1_print_header(const void *data)
{
	const struct ivt *ivt;
	const uint8_t *data8 = data;
	const uint16_t *dcd_len;
	const struct application_boot_code *app;
	bool qspi_boot;
	int min_size = S32GEN1_SD_IVT_OFFSET + sizeof(struct ivt);

	ivt = get_ivt_from_raw_blob(data, min_size, &qspi_boot);
	if (!ivt)
		return;

	app = (data + ivt->application_boot_code_pointer);
	dcd_len = (uint16_t *)(data8 + ivt->dcd_pointer
			      + DCD_HEADER_LENGTH_OFFSET);

	fprintf(stderr, "\nIVT:\t\t\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)((void *)ivt - data),
		(unsigned int)sizeof(struct ivt));

	if (qspi_boot)
		fprintf(stderr,
			"QSPI Parameters:\tOffset: 0x%x\t\tSize: 0x%x\n",
			(unsigned int)S32GEN1_QSPI_PARAMS_OFFSET,
			(unsigned int)S32GEN1_QSPI_PARAMS_SIZE);

	fprintf(stderr, "DCD:\t\t\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)ivt->dcd_pointer,
		(unsigned int)be16_to_cpu(*dcd_len));

	if (ivt->hse_h_firmware_pointer)
		fprintf(stderr, "HSE Reserved:\t\tOffset: 0x%x\t\tSize: 0x%x\n",
			(unsigned int)ivt->hse_h_firmware_pointer,
			(unsigned int)S32GEN1_SECBOOT_HSE_RES_SIZE);

	fprintf(stderr, "AppBootCode Header:\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)ivt->application_boot_code_pointer,
		(unsigned int)sizeof(*app));

	fprintf(stderr, "U-Boot/FIP:\t\tOffset: 0x%x\t\tSize: 0x%x\n\n",
		(unsigned int)((uint8_t *)app->code - data8),
		(unsigned int)app->code_length);

	fprintf(stderr, "Load address:\t0x%x\nEntry point:\t0x%x\n",
		(unsigned int)app->ram_start_pointer,
		(unsigned int)app->ram_entry_pointer);

	fprintf(stderr, "\n");
}

static char *ltrim(char *s)
{
	while (isspace((int)*s))
		s++;

	return s;
}

static int parse_boot_cmd(char *line)
{
	size_t i, len;
	enum boot_type boot = INVALID_BOOT;
	static const char * const bsources[] = {
		[QSPI_BOOT] = "qspi",
		[SD_BOOT] = "sd",
		[EMMC_BOOT] = "emmc",
	};

	for (i = 0; i < ARRAY_SIZE(bsources); i++) {
		len = strlen(bsources[i]);
		if (strncmp(bsources[i], line, len))
			continue;

		boot = (enum boot_type)i;
		break;
	}

	if (boot == INVALID_BOOT) {
		fprintf(stderr, "Invalid boot type: %s", line);
		return -EINVAL;
	}

	if (boot == QSPI_BOOT) {
		iconfig.flash_boot = true;
		image_layout.ivt.offset = S32GEN1_QSPI_IVT_OFFSET;
	} else {
		image_layout.ivt.offset = S32GEN1_SD_IVT_OFFSET;
	}

	return 0;
}

static int parse_secboot_cmd(char *line)
{
	iconfig.secboot = true;

	return 0;
}

static int get_dcd_cmd_args(char *line, struct dcd_args *dcd)
{
	int ret;
	size_t i;
	char cmd[sizeof(dcd_cmds) / 2];

	/**
	 * Info:
	 * %[^ \t]  - Read everything up to space or tab character
	 * %*[^0] - ignore everything up to character '0'
	 */
	ret = sscanf(line,
		     "%[^ \t]%*[^0]0x%" PRIx32
		     "%*[^0]0x%" PRIx32
		     "%*[^0]0x%" PRIx32
		     "%*[^0]0x%" PRIx32,
		     cmd, &dcd->nbytes, &dcd->addr, &dcd->value, &dcd->count);
	/* 4 or 5 tokens are expected */
	if (ret < 4) {
		fprintf(stderr, "Failed to interpret DCD line: %s\n", line);
		return -EINVAL;
	}

	if (ret == 4)
		dcd->count = 0;

	dcd->cmd = INVALID_DCD_CMD;
	for (i = 0; i < ARRAY_SIZE(dcd_cmds); i++) {
		if (strcmp(cmd, dcd_cmds[i]))
			continue;

		dcd->cmd = (enum dcd_cmd)i;
	}

	if (dcd->cmd == INVALID_DCD_CMD) {
		fprintf(stderr, "Invalid DCD command: %s\n", cmd);
		return -EINVAL;
	}

	if (dcd->nbytes != 1 && dcd->nbytes != 2 && dcd->nbytes != 4) {
		fprintf(stderr, "Unsupported DCD address length: 0x%x\n",
			dcd->nbytes);
		return -EINVAL;
	}

	return 0;
}

static int push_to_dcd_array(uint32_t data)
{
	size_t elem;
	size_t elem_size = sizeof(iconfig.dcd.data[0]);
	void *backup;

	if (iconfig.dcd.allocated_size <= iconfig.dcd.size + elem_size) {
		if (!iconfig.dcd.allocated_size)
			iconfig.dcd.allocated_size = 2 * elem_size;
		else
			iconfig.dcd.allocated_size *= 2;

		backup = iconfig.dcd.data;
		iconfig.dcd.data = realloc(backup, iconfig.dcd.allocated_size);
		if (!iconfig.dcd.data) {
			free(backup);
			fprintf(stderr,
				"Failed to allocate %zu bytes for DCD array\n",
				iconfig.dcd.allocated_size);
			iconfig.dcd.allocated_size = 0;
			return -ENOMEM;
		}
	}

	elem = iconfig.dcd.size / elem_size;
	iconfig.dcd.data[elem] = data;

	iconfig.dcd.size += elem_size;

	return 0;
}

static int add_to_dcd(struct dcd_args *dcd_arg)
{
	int ret;
	uint8_t set, mask;
	bool write = false;
	uint32_t params, header;

	if (!iconfig.dcd.data) {
		ret = push_to_dcd_array(DCD_HEADER);
		if (ret)
			return ret;
	}

	switch (dcd_arg->cmd) {
	case WRITE:
		write = true;
	case CHECK_MASK_CLEAR:
		mask = 0;
		set = 0;
		break;
	case CHECK_MASK_SET:
		mask = 0;
		set = 1;
		break;
	case CLEAR_MASK:
		write = true;
	case CHECK_NOT_MASK:
		mask = 1;
		set = 0;
		break;
	case SET_MASK:
		write = true;
	case CHECK_NOT_CLEAR:
		mask = 1;
		set = 1;
		break;
	default:
		fprintf(stderr, "%s: Received an invalid DCD command %d\n",
			__func__, dcd_arg->cmd);
		return -EINVAL;
	}

	params = DCD_PARAMS(set, mask, dcd_arg->nbytes);
	if (write) {
		header = DCD_WRITE_HEADER(1, params);
	} else {
		if (dcd_arg->count)
			header = DCD_CHECK_HEADER(params);
		else
			header = DCD_CHECK_HEADER_NO_COUNT(params);
	}

	ret = push_to_dcd_array(header);
	if (ret)
		return ret;
	ret = push_to_dcd_array(DCD_ADDR(dcd_arg->addr));
	if (ret)
		return ret;
	ret = push_to_dcd_array(DCD_MASK(dcd_arg->value));
	if (ret)
		return ret;

	if (!write && dcd_arg->count) {
		ret = push_to_dcd_array(DCD_COUNT(dcd_arg->count));
		if (ret)
			return ret;
	}

	return 0;
}

static int parse_dcd_cmd(char *line)
{
	int ret;
	struct dcd_args dcd;

	ret = get_dcd_cmd_args(line, &dcd);
	if (ret)
		return ret;

	ret = add_to_dcd(&dcd);
	if (ret)
		return ret;

	return 0;
}

static int parse_qspi_cmd(char *line)
{
	int fd, ret;
	struct stat stat;
	char *path = line;
	uint8_t *data;
	size_t len;

	fd = open(path, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Failed to open '%s'\n", path);
		perror("open error");
		return -errno;
	}

	ret = fstat(fd, &stat);
	if (ret) {
		close(fd);
		fprintf(stderr, "Failed to stat '%s'\n", path);
		perror("stat error");
		return -errno;
	}

	len = stat.st_size;

	data = mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (data == MAP_FAILED) {
		fprintf(stderr, "Failed to map '%s'\n", path);
		perror("mmap error");
		return -errno;
	}

	iconfig.qspi_params.data = data;
	iconfig.qspi_params.size = len;

	return 0;
}

static const struct line_parser parsers[] = {
	{
		.tag = "BOOT_FROM",
		.parse = parse_boot_cmd,
	},
	{
		.tag = "SECBOOT",
		.parse = parse_secboot_cmd,
	},
	{
		.tag = "DCD",
		.parse = parse_dcd_cmd,
	},
	{
		.tag = "QSPI_PARAMS_FILE",
		.parse = parse_qspi_cmd,
	},
};

static int parse_config_line(char *line)
{
	size_t i, len;
	const struct line_parser *parser;

	for (i = 0; i < ARRAY_SIZE(parsers); i++) {
		parser = &parsers[i];

		len = strlen(parser->tag);
		if (strncmp(parser->tag, line, len))
			continue;

		return parser->parse(ltrim(line + len));
	}

	fprintf(stderr, "Failed to parse line: %s", line);
	return -EINVAL;
}

static int build_conf(FILE *fconf)
{
	int ret;
	char *line;
	char buffer[256];
	size_t len;

	while (!feof(fconf)) {
		memset(buffer, 0, sizeof(buffer));
		line = fgets(buffer, sizeof(buffer), fconf);
		if (!line)
			break;

		line = ltrim(line);
		len = strlen(line);

		if (line[0] == '#' || !line[0])
			continue;

		if (line[len - 1] == '\n')
			line[len - 1] = 0;

		ret = parse_config_line(line);
		if (ret)
			return ret;
	}

	return 0;
}

static int s32gen1_parse_config(struct image_tool_params *mparams)
{
	FILE *fconf;
	int ret;

	fconf = fopen(mparams->imagename, "r");
	if (!fconf) {
		fprintf(stderr, "Could not open input file %s\n",
			mparams->imagename);
		exit(EXIT_FAILURE);
	}

	ret = build_conf(fconf);

	fclose(fconf);
	return ret;
}

static int s32gen1_vrec_header(struct image_tool_params *tool_params,
			       struct image_type_params *type_params)
{
	size_t header_size;
	void *image = NULL;
	int ret;

	ret = s32gen1_parse_config(tool_params);
	if (ret)
		return ret;

	s32g2xx_build_layout(&image_layout, &header_size, &image);
	type_params->header_size = header_size;
	type_params->hdr = image;

	return 0;
}

static int s32gen1_check_params(struct image_tool_params *params)
{
	if (!params)
		return -EINVAL;

	if (!strlen(params->imagename)) {
		fprintf(stderr,
			"Error: %s - Configuration file not specified, it is needed for s32gen1image generation\n",
			params->cmdname);
		return -EINVAL;
	}

	/*
	 * Check parameters:
	 * XIP is not allowed and verify that incompatible
	 * parameters are not sent at the same time
	 * For example, if list is required a data image must not be provided
	 */
	return	(params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag)) ||
		(params->xflag) || !(strlen(params->imagename));
}

static int s32gen1_verify_header(unsigned char *ptr, int image_size,
				 struct image_tool_params *params)
{
	if (!get_ivt_from_raw_blob(ptr, image_size, NULL))
		return -FDT_ERR_BADSTRUCTURE;

	return 0;
}

U_BOOT_IMAGE_TYPE(
	s32gen1_image,
	"NXP S32GEN1 Boot Image",
	0,
	NULL,
	s32gen1_check_params,
	s32gen1_verify_header,
	s32gen1_print_header,
	s32gen1_set_header,
	NULL,
	s32gen1_check_image_type,
	NULL,
	s32gen1_vrec_header
);
