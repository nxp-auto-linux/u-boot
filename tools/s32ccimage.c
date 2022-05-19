// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2019-2022 NXP */
#include <ctype.h>
#include <image.h>
#include <imagetool.h>
#include <inttypes.h>
#include <s32cc_image_params.h>
#include <stddef.h>
#include <unistd.h>
#include <linux/sizes.h>
#include <sys/stat.h>
#include <sys/types.h>

#define UNSPECIFIED			-1

#define BCW_BOOT_TARGET_A53_0		(1)

#define MBR_OFFSET			0x0
#define MBR_SIZE			0x200

#define QSPI_IVT_OFFSET			0x0
#define SD_IVT_OFFSET			0x1000
#define IVT_VERSION			(0x60)
#define IVT_TAG				(0xd1)

#define QSPI_PARAMS_SIZE		(0x200U)
#define QSPI_PARAMS_OFFSET		(0x200U)

#define HSE_FW_MAX_SIZE			(0x80000ul)
#define HSE_SYS_IMG_MAX_SIZE		(0xc000ul)

#define APPLICATION_BOOT_CODE_TAG	(0xd5)
#define APPLICATION_BOOT_CODE_VERSION	(0x60)

#define AUTO_OFFSET			((size_t)(-1))

#define DCD_HEADER			(0x600000d2)
#define DCD_MAXIMUM_SIZE		(8192)
#define DCD_HEADER_LENGTH_OFFSET	(1)

#define BOOTROM_QSPI_ALIGNMENT		(0x8)
#define BOOTROM_SD_ALIGNMENT		(0x200)

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

struct ivt {
	__u8		tag;
	__u16		length;
	__u8		version;
	__u8		reserved1[4];
	__u32		self_test_dcd_pointer;
	__u32		self_test_dcd_pointer_backup;
	__u32		dcd_pointer;
	__u32		dcd_pointer_backup;
	__u32		hse_firmware_pointer;
	__u32		hse_firmware_pointer_backup;
	__u32		application_boot_code_pointer;
	__u32		application_boot_code_pointer_backup;
	__u32		boot_configuration_word;
	__u32		lifecycle_configuration_word;
	__u8		reserved2[4];
	__u32		hse_sys_img_pointer;
	__u8		reserved_for_hse_h_fw[28];
	__u8		reserved3[156];
	__u32		gmac[4];
} __packed;

struct application_boot_code {
	__u8		tag;
	__u8		reserved1[2];
	__u8		version;
	__u32		ram_start_pointer;
	__u32		ram_entry_pointer;
	__u32		code_length;
	__u32		auth_mode;
	__u8		reserved2[44];
	__u8		code[0];
} __packed;

struct image_comp {
	size_t offset;
	size_t size;
	size_t padding;
	size_t alignment;
	__u8 *data;
};

struct program_image {
	struct image_comp mbr_reserved;
	struct image_comp ivt;
	struct image_comp qspi_params;
	struct image_comp dcd;
	struct image_comp hse_fw;
	struct image_comp hse_sys_img;
	struct image_comp app_code;
	struct image_comp code;
	__u8 *header;
};

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
	struct {
		uint32_t size;
	} data_file;
	struct {
		bool enable;
		uint8_t *fw_data;
		size_t fw_size;
	} secboot;
	bool flash_boot;
	bool err051257;
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

enum data_file_cmd {
	SET_SIZE,
	INVALID_DATA_FILE_CMD
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

static const char * const data_file_cmds[] = {
	[SET_SIZE] = "SIZE",
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
		.offset = QSPI_PARAMS_OFFSET,
		.size = QSPI_PARAMS_SIZE,
	},
	.dcd = {
		.offset = AUTO_OFFSET,
		.size = DCD_MAXIMUM_SIZE,
	},
	.hse_fw = {
		.offset = AUTO_OFFSET,
		.size = HSE_FW_MAX_SIZE,
	},
	.hse_sys_img = {
		.offset = AUTO_OFFSET,
		.size = HSE_SYS_IMG_MAX_SIZE,
	},
	.app_code = {
		.offset = AUTO_OFFSET,
		.size = sizeof(struct application_boot_code),
	},
	.code = {
		.offset = AUTO_OFFSET,
		.alignment = 0x8U,
		.size = 0,
	},
};

static inline bool apply_err051257_workaround(void)
{
	return iconfig.err051257 && iconfig.flash_boot;
}

static inline size_t get_padding(void)
{
	return apply_err051257_workaround() ? SZ_1K : 0;
}

static const uint32_t *get_dcd_data(size_t *size)
{
	if (iconfig.dcd.data) {
		*size = iconfig.dcd.size;
		return iconfig.dcd.data;
	}

	*size = 0;
	return NULL;
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
	void *image_end = (void *)((__u8 *)image_start + image_length);
	int i;

	for (i = 0; i < ARRAY_SIZE(reserved_sram); i++)
		if (image_start < reserved_sram[i].end &&
		    image_end > reserved_sram[i].start) {
			fprintf(stderr,
				"Loading data of size 0x%x at %p forbidden.",
				image_length, image_start);
			fprintf(stderr, " Range %p --- %p is reserved!\n",
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

static bool is_in_overlap(struct image_comp *comp1,
			  struct image_comp *comp2)
{
	size_t end1 = comp1->offset + comp1->size + comp1->padding;
	size_t end2 = comp2->offset + comp2->size + comp2->padding;

	if (end1 > comp2->offset && end2 > comp1->offset)
		return true;

	return false;
}

static void place_after(struct image_comp *first, struct image_comp *second)
{
	second->offset = first->offset + first->size + first->padding;

	/* Apply alignment constraints */
	if (second->alignment != 0U)
		second->offset = ROUND(second->offset, second->alignment);
}

static void s32_compute_dyn_offsets(struct image_comp **parts, size_t n_parts)
{
	size_t i;
	size_t start_index = 0U, aindex;

	qsort(&parts[0], n_parts, sizeof(parts[0]), image_parts_comp);

	/* Skip empty entries */
	while (start_index < n_parts && !*parts) {
		start_index++;
		parts++;
	}
	n_parts -= start_index;
	start_index = 0;

	/* First image with auto offset */
	while ((start_index < n_parts) &&
	       (parts[start_index]->offset != AUTO_OFFSET))
		start_index++;

	for (aindex = start_index; aindex < n_parts; aindex++) {
		if (aindex == 0) {
			parts[aindex]->offset = 0U;
			continue;
		}

		/* Look for an empty spot between existing allocations */
		for (i = 1; i < aindex; i++) {
			place_after(parts[i - 1], parts[aindex]);

			/* Does it fit between i - 1 and i ? */
			if (is_in_overlap(parts[aindex], parts[i])) {
				parts[aindex]->offset = AUTO_OFFSET;
				continue;
			}

			if (is_in_overlap(parts[aindex], parts[i - 1])) {
				parts[aindex]->offset = AUTO_OFFSET;
				continue;
			}

			break;
		}

		if (parts[aindex]->offset != AUTO_OFFSET) {
			/* Move freshly allocated aindex */
			qsort(&parts[0], aindex + 1, sizeof(parts[0]),
			      image_parts_comp);
			continue;
		}

		place_after(parts[aindex - 1], parts[aindex]);
	}
}

static void set_padding(struct image_comp **parts, size_t n_parts)
{
	int i;

	for (i = 0; i < n_parts; i++)
		if (parts[i])
			parts[i]->padding = get_padding();
}

static struct qspi_params *get_qspi_params(struct program_image *image)
{
	return (struct qspi_params *)image->qspi_params.data;
}

static void s32cc_set_qspi_params(struct qspi_params *qspi_params)
{
	memcpy(qspi_params, iconfig.qspi_params.data,
	       iconfig.qspi_params.size);
}

static void s32cc_set_hse_fw(struct program_image *image)
{
	uint8_t *data = iconfig.secboot.fw_data;
	size_t size = iconfig.secboot.fw_size;

	memcpy(image->hse_fw.data, data, size);
}

static void set_data_pointers(struct program_image *layout, void *header)
{
	uint8_t *data = (uint8_t *)header;

	layout->ivt.data = data + layout->ivt.offset;

	if (iconfig.flash_boot)
		layout->qspi_params.data = data + layout->qspi_params.offset;

	layout->dcd.data = data + layout->dcd.offset;
	layout->app_code.data = data + layout->app_code.offset;
	layout->hse_fw.data = data + layout->hse_fw.offset;
}

static void s32cc_set_header(void *header, struct stat *sbuf, int unused,
			     struct image_tool_params *tool_params)
{
	size_t dcd_data_size;
	uint8_t *dcd;
	struct ivt *ivt;
	struct application_boot_code *app_code;
	const uint32_t *dcd_data;

	set_data_pointers(&image_layout, header);
	dcd_data = get_dcd_data(&dcd_data_size);

	dcd = get_dcd(&image_layout);
	if (dcd_data_size > DCD_MAXIMUM_SIZE) {
		fprintf(stderr, "DCD exceeds the maximum size\n");
		exit(EXIT_FAILURE);
	}
	if (dcd_data) {
		memcpy(dcd, &dcd_data[0], dcd_data_size);
		*(uint16_t *)(dcd + DCD_HEADER_LENGTH_OFFSET) =
		    cpu_to_be16(dcd_data_size);
	}

	ivt = get_ivt(&image_layout);
	app_code = get_app_code(&image_layout);

	ivt->tag = IVT_TAG;
	ivt->length = cpu_to_be16(sizeof(struct ivt));
	ivt->version = IVT_VERSION;

	if (dcd_data)
		ivt->dcd_pointer = image_layout.dcd.offset;
	ivt->boot_configuration_word = BCW_BOOT_TARGET_A53_0;
	ivt->application_boot_code_pointer = image_layout.app_code.offset;

	if (iconfig.secboot.enable) {
		ivt->hse_firmware_pointer = image_layout.hse_fw.offset;
		ivt->hse_sys_img_pointer = image_layout.hse_sys_img.offset;
	}

	app_code->tag = APPLICATION_BOOT_CODE_TAG;
	app_code->version = APPLICATION_BOOT_CODE_VERSION;

	if (tool_params->ep < tool_params->addr) {
		fprintf(stderr,
			"The entrypoint is higher than the load address (0x%x < 0x%x)\n",
			tool_params->ep, tool_params->addr);
		exit(1);
	}

	if (iconfig.data_file.size)
		app_code->code_length = iconfig.data_file.size;
	else
		app_code->code_length = sbuf->st_size;

	/* The 'code_length', like plenty of entries in the Program
	 * Image structure, must be 512 bytes aligned.
	 */
	app_code->code_length = ROUND(app_code->code_length, 512);

	app_code->ram_start_pointer = tool_params->addr;
	app_code->ram_entry_pointer = tool_params->ep;

	if (!iconfig.flash_boot) {
		enforce_reserved_ranges((void *)(uintptr_t)
					app_code->ram_start_pointer,
					app_code->code_length);
	} else {
		s32cc_set_qspi_params(get_qspi_params(&image_layout));
	}

	if (iconfig.secboot.enable)
		s32cc_set_hse_fw(&image_layout);

	image_layout.code.size = sbuf->st_size - image_layout.app_code.offset -
		image_layout.app_code.size;

	if (iconfig.dcd.data)
		free(iconfig.dcd.data);

	if (iconfig.qspi_params.data)
		munmap(iconfig.qspi_params.data, iconfig.qspi_params.size);
}

static int s32cc_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_S32CCIMAGE)
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
get_image_hse_fw(struct program_image *program_image)
{
	if (iconfig.secboot.enable)
		return &program_image->hse_fw;

	return NULL;
}

static struct image_comp *
get_image_hse_sys_img(struct program_image *program_image)
{
	if (iconfig.secboot.enable)
		return &program_image->hse_sys_img;

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

static struct image_comp *
get_image_dcd(struct program_image *program_image)
{
	if (!iconfig.dcd.size)
		return NULL;

	return &program_image->dcd;
}

static void set_headers_size(struct program_image *program_image)
{
	program_image->dcd.size = iconfig.dcd.size;
	if (program_image->dcd.size > DCD_MAXIMUM_SIZE) {
		fprintf(stderr,
			"DCD area exceeds the maximum allowed size (0x%x\n)",
			DCD_MAXIMUM_SIZE);
		exit(EXIT_FAILURE);
	}

	program_image->hse_fw.size = iconfig.secboot.fw_size;
	if (program_image->hse_fw.size > HSE_FW_MAX_SIZE) {
		fprintf(stderr,
			"HSE FW area exceeds the maximum allowed size (0x%lx\n)",
			HSE_FW_MAX_SIZE);
		exit(EXIT_FAILURE);
	}
}

static void set_headers_alignment(struct program_image *program_image)
{
	size_t alignment;

	if (iconfig.flash_boot)
		alignment = BOOTROM_QSPI_ALIGNMENT;
	else
		alignment = BOOTROM_SD_ALIGNMENT;

	program_image->dcd.alignment = alignment;
	program_image->hse_fw.alignment = alignment;
	program_image->hse_sys_img.alignment = alignment;
	program_image->app_code.alignment = alignment;
}

static int s32g2xx_build_layout(struct program_image *program_image,
				size_t *header_size, void **image)
{
	uint8_t *image_layout;
	struct image_comp *parts[] = {
		get_image_mbr(program_image),
		get_image_qspi_params(program_image),
		&program_image->ivt,
		get_image_dcd(program_image),
		get_image_hse_fw(program_image),
		get_image_hse_sys_img(program_image),
	};
	size_t last_comp = ARRAY_SIZE(parts) - 1;

	set_headers_size(program_image);
	set_headers_alignment(program_image);

	set_padding(parts, ARRAY_SIZE(parts));

	/* Compute auto-offsets */
	s32_compute_dyn_offsets(parts, ARRAY_SIZE(parts));

	/**
	 * Place APP header at the end of the image header as the APP code
	 * will be glued to it.
	 */
	place_after(parts[last_comp], &program_image->app_code);
	place_after(&program_image->app_code, &program_image->code);

	*header_size = program_image->app_code.offset +
	    program_image->app_code.size;

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

	offset = QSPI_IVT_OFFSET;
	min_size = offset + sizeof(struct ivt);

	if (size < min_size)
		return NULL;

	if (is_ivt_header(ptr)) {
		if (qspi_ivt)
			*qspi_ivt = true;
		return (struct ivt *)ptr;
	}

	offset = SD_IVT_OFFSET;
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

struct layout_comp {
	uint32_t offset;
	uint32_t size;
	const char *line_desc;
};

static int layout_comparator(const void *p1, const void *p2)
{
	const struct layout_comp **part1 = (typeof(part1))p1;
	const struct layout_comp **part2 = (typeof(part2))p2;

	return (*part1)->offset - (*part2)->offset;
}

static void print_layout(struct layout_comp **comps, size_t n)
{
	size_t i;

	fprintf(stderr, "\nImage Layout\n");
	for (i = 0u; i < n; i++) {
		if (!comps[i]->offset && !comps[i]->size)
			continue;

		fprintf(stderr, "\t%sOffset: 0x%x\t\t",
			comps[i]->line_desc, comps[i]->offset);

		if (comps[i]->size)
			fprintf(stderr, "Size: 0x%x", comps[i]->size);

		fprintf(stderr, "\n");
	}

	fprintf(stderr, "\n");
}

static void s32cc_print_header(const void *data)
{
	const struct ivt *ivt;
	const uint8_t *data8 = data;
	const uint16_t *dcd_len;
	const struct application_boot_code *app;
	bool qspi_boot;
	size_t i;
	int min_size = SD_IVT_OFFSET + sizeof(struct ivt);
	struct layout_comp ivt_comp, qspi, dcd, hse_fw,
			   hse_img, app_hdr, app_comp;
	struct layout_comp *comps[] = { &ivt_comp, &qspi, &dcd, &hse_fw,
		&hse_img, &app_hdr, &app_comp };

	for (i = 0; i < ARRAY_SIZE(comps); i++)
		memset(comps[i], 0, sizeof(*comps[i]));

	ivt = get_ivt_from_raw_blob(data, min_size, &qspi_boot);
	if (!ivt)
		return;

	ivt_comp.offset = (uint32_t)((void *)ivt - data);
	ivt_comp.size = (uint32_t)sizeof(struct ivt);
	ivt_comp.line_desc = "IVT:\t\t\t";

	if (qspi_boot) {
		qspi.offset = QSPI_PARAMS_OFFSET;
		qspi.size = QSPI_PARAMS_SIZE;
		qspi.line_desc = "QSPI Parameters:\t";
	}

	if (ivt->dcd_pointer) {
		dcd_len = (uint16_t *)(data8 + ivt->dcd_pointer
				       + DCD_HEADER_LENGTH_OFFSET);
		dcd.offset = ivt->dcd_pointer;
		dcd.size = (uint32_t)be16_to_cpu(*dcd_len);
		dcd.line_desc = "DCD:\t\t\t";
	}

	if (ivt->hse_firmware_pointer) {
		hse_fw.offset = ivt->hse_firmware_pointer;
		hse_fw.line_desc = "HSE Firmware:\t\t";
	}

	if (ivt->hse_sys_img_pointer) {
		hse_img.offset = ivt->hse_sys_img_pointer;
		hse_img.size = HSE_SYS_IMG_MAX_SIZE;
		hse_img.line_desc = "HSE SYS Image:\t\t";
	}

	app = (data + ivt->application_boot_code_pointer);

	app_hdr.offset = ivt->application_boot_code_pointer;
	app_hdr.size = (uint32_t)sizeof(*app);
	app_hdr.line_desc = "AppBootCode Header:\t";

	app_comp.offset = (uint32_t)((uint8_t *)app->code - data8);
	app_comp.size = app->code_length;
	app_comp.line_desc = "Application:\t\t";

	qsort(comps, ARRAY_SIZE(comps), sizeof(comps[0]), layout_comparator);
	print_layout(comps, ARRAY_SIZE(comps));

	fprintf(stderr, "IVT Location:\t");
	if (qspi_boot)
		fprintf(stderr, "QSPI\n");
	else
		fprintf(stderr, "SD/eMMC\n");

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
		image_layout.ivt.offset = QSPI_IVT_OFFSET;
	} else {
		image_layout.ivt.offset = SD_IVT_OFFSET;
	}

	return 0;
}

static int map_file(const char *path, uint8_t **data, size_t *size)
{
	int fd, ret;
	struct stat stat;

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

	*size = stat.st_size;

	*data = mmap(NULL, *size, PROT_READ, MAP_PRIVATE, fd, 0);
	close(fd);
	if (*data == MAP_FAILED) {
		fprintf(stderr, "Failed to map '%s'\n", path);
		perror("mmap error");
		return -errno;
	}

	return 0;
}

static int parse_secboot_cmd(char *line)
{
	int ret;
	char *path;

	path = calloc(strnlen(line, 256), sizeof(*path));
	if (!path)
		return -ENOMEM;

	ret = sscanf(line, "\"%[^\"]\"", path);
	if (ret != 1) {
		ret = -EINVAL;
		goto free_mem;
	}

	iconfig.secboot.enable = true;
	ret = map_file(path, &iconfig.secboot.fw_data,
		       &iconfig.secboot.fw_size);
free_mem:
	free(path);
	return ret;
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
	int ret;
	char *path = line;

	ret = map_file(path, &iconfig.qspi_params.data,
		       &iconfig.qspi_params.size);
	if (ret)
		return ret;

	return 0;
}

static int parse_data_file_cmd(char *line)
{
	int ret;
	size_t i;
	char cmd_str[sizeof(data_file_cmds)];
	uint32_t addr;
	enum data_file_cmd cmd = INVALID_DATA_FILE_CMD;

	ret = sscanf(line, "%[^ \t]%*[^0]0x%" PRIx32, cmd_str, &addr);
	/* 2 tokens are expected */
	if (ret < 2) {
		fprintf(stderr, "Failed to interpret DATA_FILE line: %s\n",
			line);
		return -EINVAL;
	}

	for (i = 0; i < ARRAY_SIZE(data_file_cmds); i++) {
		if (strcmp(cmd_str, data_file_cmds[i]))
			continue;

		cmd = (enum data_file_cmd)i;
	}

	if (cmd == INVALID_DATA_FILE_CMD) {
		fprintf(stderr, "Failed to interpret DATA_FILE command: %s\n",
			cmd_str);
		return -EINVAL;
	}

	switch (cmd) {
	case SET_SIZE:
		iconfig.data_file.size = addr;
		break;
	default:
		fprintf(stderr, "Failed to interpret DATA_FILE command: %s\n",
			cmd_str);
		return -EINVAL;
	}

	return 0;
}

static int parse_err051257(__attribute__((unused)) char *line)
{
	iconfig.err051257 = true;

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
	{
		.tag = "DATA_FILE",
		.parse = parse_data_file_cmd,
	},
	{
		.tag = "ERR051257_WORKAROUND",
		.parse = parse_err051257,
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

static int s32cc_parse_config(struct image_tool_params *mparams)
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

static int s32cc_vrec_header(struct image_tool_params *tool_params,
			     struct image_type_params *type_params)
{
	size_t header_size;
	void *image = NULL;
	int ret;

	ret = s32cc_parse_config(tool_params);
	if (ret)
		exit(ret);

	s32g2xx_build_layout(&image_layout, &header_size, &image);
	type_params->header_size = header_size;
	type_params->hdr = image;

	return 0;
}

static int s32cc_check_params(struct image_tool_params *params)
{
	if (!params)
		return -EINVAL;

	if (!strlen(params->imagename)) {
		fprintf(stderr,
			"Error: %s - Configuration file not specified, it is needed for s32ccimage generation\n",
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

static int s32cc_verify_header(unsigned char *ptr, int image_size,
			       struct image_tool_params *params)
{
	if (!get_ivt_from_raw_blob(ptr, image_size, NULL))
		return -FDT_ERR_BADSTRUCTURE;

	return 0;
}

U_BOOT_IMAGE_TYPE(
	s32ccimage,
	"NXP S32 Common Chassis Boot Image",
	0,
	NULL,
	s32cc_check_params,
	s32cc_verify_header,
	s32cc_print_header,
	s32cc_set_header,
	NULL,
	s32cc_check_image_type,
	NULL,
	s32cc_vrec_header
);
