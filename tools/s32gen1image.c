/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2019-2022 NXP */

#include <image.h>
#include <generated/autoconf.h>
#include <config.h>
#include <asm/arch-s32/siul-s32-gen1.h>
#include <asm/arch-s32/s32-gen1/s32-gen1-regs.h>
#include "imagetool.h"
#include "s32_common.h"
#include "s32gen1image.h"

#define UNSPECIFIED	-1

#define S32GEN1_SECBOOT_HSE_RES_SIZE 0x80000ul

#define S32GEN1_QSPI_PARAMS_OFFSET 0x200U

#define S32GEN1_IVT_OFFSET_0		0x0
#define S32GEN1_IVT_OFFSET_1000		0x1000

struct image_config {
	struct {
		uint32_t offset;
		uint32_t size;
	} env;
	uint32_t entrypoint;
	uint32_t dtb_addr;
	bool flash_boot;
	bool secboot;
	bool is_rdb2;
	bool is_emu;
};

static struct image_config iconfig;

static struct program_image image_layout = {
	.ivt = {
		.offset = S32GEN1_IVT_OFFSET_0,
		.size = sizeof(struct ivt),
	},
	.qspi_params = {
		.offset = S32GEN1_QSPI_PARAMS_OFFSET,
		.size = S32GEN1_QSPI_PARAMS_SIZE,
	},
	.ivt_duplicate = {
		.offset = S32GEN1_IVT_OFFSET_1000,
		.size = sizeof(struct ivt),
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

static uint32_t default_dcd_data[] = {
	DCD_HEADER,
	DCD_NOP_HEADER,
	DCD_NOP_HEADER,
	DCD_NOP_HEADER,
	DCD_NOP_HEADER,
};

static uint32_t s32g274ardb2_dcd_data[] = {
	DCD_HEADER,
	/*
	 * Enable VDD_EFUSE, so that HSE can read SYS_IMG.
	 * VDD_EFUSE is disabled by default on s32g274ardb2
	 */
	DCD_WRITE_HEADER(1, PARAMS_BYTES(4)),
	DCD_ADDR(SIUL2_0_MSCRn(25)), DCD_MASK(MSCR25_SET_GPIO25_SRC),
	DCD_WRITE_HEADER(1, PARAMS_BYTES(1)),
	DCD_ADDR(SIUL2_PDO_N(25)), DCD_MASK(GPDO25_HIGH),
};

static uint32_t *get_dcd_data(size_t *size)
{
	if (iconfig.is_rdb2) {
		*size = sizeof(s32g274ardb2_dcd_data);
		return &s32g274ardb2_dcd_data[0];
	}

	*size = sizeof(default_dcd_data);
	return &default_dcd_data[0];
}

static struct ivt *get_ivt(struct program_image *image)
{
	return (struct ivt *)image->ivt.data;
}

static struct ivt *get_ivt_duplicate(struct program_image *image)
{
	return (struct ivt *)image->ivt_duplicate.data;
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

static struct qspi_params *get_qspi_params(struct program_image *image)
{
	return (struct qspi_params *)image->qspi_params.data;
}

static void s32gen1_set_qspi_params(struct qspi_params *qspi_params)
{
	struct qspi_params *s32g2xx_qspi_conf = get_s32g2xx_qspi_conf();

	memcpy(qspi_params, s32g2xx_qspi_conf, sizeof(*qspi_params));
}

static void set_data_pointers(struct program_image *layout, void *header)
{
	uint8_t *data = (uint8_t *)header;

	layout->ivt.data = data + layout->ivt.offset;

	if (iconfig.flash_boot)
		layout->qspi_params.data = data + layout->qspi_params.offset;

	layout->ivt_duplicate.data = data + layout->ivt_duplicate.offset;
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
	struct ivt *ivt_duplicate;
	struct application_boot_code *app_code;
	struct fip_image_data fip_data;
	uint32_t *dcd_data;

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

	ivt_duplicate = get_ivt_duplicate(&image_layout);
	memcpy(ivt_duplicate, ivt, sizeof(struct ivt));

	app_code->tag = APPLICATION_BOOT_CODE_TAG;
	app_code->version = APPLICATION_BOOT_CODE_VERSION;

	pre_code_padding = image_layout.code.offset
				- image_layout.app_code.offset
				- image_layout.app_code.size;

	if (read_fip_image(tool_params, &fip_data)) {
		code_length = sbuf->st_size
				- image_layout.app_code.offset
				- offsetof(struct application_boot_code, code);
		code_length += pre_code_padding;
		app_code->code_length = code_length;

		app_code->ram_start_pointer = iconfig.dtb_addr
							- pre_code_padding;
		app_code->ram_entry_pointer = iconfig.entrypoint;
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
		if (!iconfig.is_emu)
			s32gen1_set_qspi_params(get_qspi_params(&image_layout));
	}

	image_layout.code.size = sbuf->st_size - image_layout.app_code.offset -
		image_layout.app_code.size;
	s32_check_env_overlap(sbuf->st_size);
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

static int s32g2xx_build_layout(struct program_image *program_image,
				size_t *header_size, void **image)
{
	uint8_t *image_layout;
	struct image_comp *parts[] = {&program_image->ivt,
		&program_image->ivt_duplicate,
		&program_image->dcd,
		&program_image->app_code,
		&program_image->code,
		get_image_qspi_params(program_image),
		get_image_hse_params(program_image),
	};
	size_t last_comp = ARRAY_SIZE(parts) - 1;

	qsort(&parts[0], ARRAY_SIZE(parts), sizeof(parts[0]), image_parts_comp);

	/* Compute auto-offsets */
	s32_compute_dyn_offsets(parts, ARRAY_SIZE(parts));

	*header_size = parts[last_comp]->offset + parts[last_comp]->size;

	image_layout = calloc(*header_size, sizeof(*image_layout));
	if (!image_layout) {
		perror("Call to calloc() failed");
		return -ENOMEM;
	}

	*image = image_layout;
	return 0;
}

static void s32gen1_init_iconfig(void)
{
#ifdef CONFIG_FLASH_BOOT
	iconfig.flash_boot = true;
#endif
#ifdef CONFIG_HSE_SECBOOT
	iconfig.secboot = true;
#endif
#ifdef CONFIG_S32G274ARDB2
	iconfig.is_rdb2 = true;
#endif
#ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
	iconfig.is_emu = true;
#endif
	iconfig.entrypoint = CONFIG_SYS_TEXT_BASE;
	iconfig.dtb_addr = CONFIG_DTB_SRAM_ADDR;

	if (iconfig.entrypoint < iconfig.dtb_addr) {
		fprintf(stderr,
			"The entrypoint is higher than the DTB base (0x%x < 0x%x)\n",
			iconfig.entrypoint, iconfig.dtb_addr);
		exit(1);
	}

#if defined(CONFIG_ENV_OFFSET) && defined(CONFIG_ENV_SIZE)
	iconfig.env.offset = CONFIG_ENV_OFFSET;
	iconfig.env.size = CONFIG_ENV_SIZE;
#endif
}

static int s32gen1_vrec_header(struct image_tool_params *tool_params,
			       struct image_type_params *type_params)
{
	size_t header_size;
	void *image = NULL;

	s32gen1_init_iconfig();
	s32g2xx_build_layout(&image_layout, &header_size, &image);
	type_params->header_size = header_size;
	type_params->hdr = image;

	return 0;
}

static void s32gen1_print_header(const void *header)
{
	fprintf(stderr, "\nIVT:\t\t\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)image_layout.ivt.offset,
		(unsigned int)image_layout.ivt.size);

	if (iconfig.flash_boot)
		fprintf(stderr,
			"QSPI Parameters:\tOffset: 0x%x\t\tSize: 0x%x\n",
			(unsigned int)image_layout.qspi_params.offset,
			(unsigned int)image_layout.qspi_params.size);

	fprintf(stderr, "IVT (duplicate):\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)image_layout.ivt_duplicate.offset,
		(unsigned int)image_layout.ivt_duplicate.size);
	fprintf(stderr, "DCD:\t\t\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)image_layout.dcd.offset,
		(unsigned int)image_layout.dcd.size);

	if (iconfig.secboot)
		fprintf(stderr, "HSE Reserved:\t\tOffset: 0x%x\t\tSize: 0x%x\n",
			(unsigned int)image_layout.hse_reserved.offset,
			(unsigned int)image_layout.hse_reserved.size);

	fprintf(stderr, "AppBootCode Header:\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)image_layout.app_code.offset,
		(unsigned int)image_layout.app_code.size);
	fprintf(stderr, "U-Boot/FIP:\t\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)image_layout.code.offset,
		(unsigned int)image_layout.code.size);

	if (iconfig.env.offset && iconfig.env.size)
		fprintf(stderr,
			"U-Boot Environment:\tOffset: 0x%x\tSize: 0x%x\n",
			(unsigned int)iconfig.env.offset,
			(unsigned int)iconfig.env.size);

	fprintf(stderr, "\n");
}

U_BOOT_IMAGE_TYPE(
	s32gen1_image,
	"NXP S32GEN1 Boot Image",
	0,
	NULL,
	NULL,
	NULL,
	s32gen1_print_header,
	s32gen1_set_header,
	NULL,
	s32gen1_check_image_type,
	NULL,
	s32gen1_vrec_header
);
