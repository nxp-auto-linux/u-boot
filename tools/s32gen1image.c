/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2019-2021 NXP */

#include <image.h>
#include <generated/autoconf.h>
#include <config.h>
#include "imagetool.h"
#include "s32_common.h"
#include "s32gen1image.h"

#define UNSPECIFIED	-1

#ifdef CONFIG_HSE_SECBOOT
#  define S32GEN1_SECBOOT_HSE_RES_SIZE 0x80000ul
#endif

#ifdef CONFIG_FLASH_BOOT
#  define S32G2XX_COMMAND_SEQ_FILL_OFF 20
#endif

#ifdef CONFIG_FLASH_BOOT
#  define S32GEN1_QSPI_PARAMS_OFFSET 0x200U
#endif

#define S32GEN1_IVT_OFFSET_0		0x0
#define S32GEN1_IVT_OFFSET_1000		0x1000

static struct program_image image_layout = {
	.ivt = {
		.offset = S32GEN1_IVT_OFFSET_0,
		.size = sizeof(struct ivt),
	},
#ifdef CONFIG_FLASH_BOOT
	.qspi_params = {
		.offset = S32GEN1_QSPI_PARAMS_OFFSET,
		.size = S32GEN1_QSPI_PARAMS_SIZE,
	},
#endif
	.ivt_duplicate = {
		.offset = S32GEN1_IVT_OFFSET_1000,
		.size = sizeof(struct ivt),
	},
	.dcd = {
		.offset = S32_AUTO_OFFSET,
		.alignment = 0x200U,
		.size = DCD_MAXIMUM_SIZE,
	},
#ifdef CONFIG_HSE_SECBOOT
	.hse_reserved = {
		.offset = S32_AUTO_OFFSET,
		.alignment = 0x2000U,
		.size = S32GEN1_SECBOOT_HSE_RES_SIZE,
	},
#endif
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

static uint32_t dcd_data[] = {
	DCD_HEADER,
	DCD_NOP_HEADER,
	DCD_NOP_HEADER,
	DCD_NOP_HEADER,
	DCD_NOP_HEADER,
};

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

#ifndef CONFIG_FLASH_BOOT

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

#else
#ifndef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
/* Load U-Boot using DTR octal mode @ 133 MHz*/
static struct qspi_params s32g2xx_qspi_conf = {
	.header   = 0x5a5a5a5a,
	.mcr      = 0x030f00cc,
	.flshcr   = 0x00010303,
	.bufgencr = 0x00000000,
	.dllcr    = 0xc280000c,
	.paritycr = 0x00000000,
	.sfacr    = 0x00020000,
	.smpr     = 0x44000000,
	.dlcr     = 0x40ff40ff,
	.sflash_1_size = 0x20000000,
	.sflash_2_size = 0x20000000,
	.dlpr = 0xaa553443,
	.sfar = 0x00000000,
	.ipcr = 0x00000000,
	.tbdr = 0x00000000,
	.dll_bypass_en   = 0x00,
	.dll_slv_upd_en  = 0x00,
	.dll_auto_upd_en = 0x01,
	.ipcr_trigger_en = 0x00,
	.sflash_clk_freq = 200,
	.reserved = {0x00, 0x00, 0x00},
	/* Macronix read - 8DTRD */
	.command_seq = {0x471147ee,
			0x0f142b20,
			0x00003b10},
	.writes = {
		{
			/* Write enable */
			.config = {
				.valid_addr = 0,
				.cdata_size = 0,
				.addr_size = 0,
				.pad = 0,
				.reserved = 0,
				.opcode = 6,
			},
			.addr = 0,
			.data = 0,
		},
		{
			/* WRCR2 - DTR OPI */
			.config = {
				.valid_addr = 1,
				.cdata_size = 1,
				.addr_size = 32,
				.pad = 0,
				.reserved = 0,
				.opcode = 0x72,
			},
			.addr = 0x0,
			.data = 0x2,
		},
	},
};

#if defined(CONFIG_S32G274ARDB) || defined(CONFIG_TARGET_S32R45EVB) || \
	defined(CONFIG_TARGET_S32G274AEVB)
static void adjust_qspi_params(struct qspi_params *qspi_params)
{
	qspi_params->dllcr = 0x8280000c;
	qspi_params->dll_slv_upd_en = 0x01;
	qspi_params->sflash_clk_freq = 133;
}
#endif

static struct qspi_params *get_qspi_params(struct program_image *image)
{
	return (struct qspi_params *)image->qspi_params.data;
}

static void s32gen1_set_qspi_params(struct qspi_params *qspi_params)
{
	memcpy(qspi_params, &s32g2xx_qspi_conf, sizeof(*qspi_params));
#if defined(CONFIG_S32G274ARDB) || defined(CONFIG_TARGET_S32R45EVB) || \
	defined(CONFIG_TARGET_S32G274AEVB)
	adjust_qspi_params(qspi_params);
#endif
}
#endif /* CONFIG_TARGET_TYPE_S32GEN1_EMULATOR */
#endif

static void set_data_pointers(struct program_image *layout, void *header)
{
	uint8_t *data = (uint8_t *)header;

	layout->ivt.data = data + layout->ivt.offset;
#ifdef CONFIG_FLASH_BOOT
	layout->qspi_params.data = data + layout->qspi_params.offset;
#endif
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
	uint8_t *dcd;
	struct ivt *ivt;
	struct ivt *ivt_duplicate;
	struct application_boot_code *app_code;
	struct fip_image_data fip_data;

	set_data_pointers(&image_layout, header);

	dcd = get_dcd(&image_layout);
	if (sizeof(dcd_data) > DCD_MAXIMUM_SIZE) {
		fprintf(stderr, "DCD exceeds the maximum size\n");
		exit(EXIT_FAILURE);
	}
	memcpy(dcd, &dcd_data[0], sizeof(dcd_data));
	*(uint16_t *)(dcd + DCD_HEADER_LENGTH_OFFSET) =
						cpu_to_be16(sizeof(dcd_data));

	ivt = get_ivt(&image_layout);
	app_code = get_app_code(&image_layout);

	ivt->tag = IVT_TAG;
	ivt->length = cpu_to_be16(sizeof(struct ivt));
	ivt->version = IVT_VERSION;

	ivt->dcd_pointer = image_layout.dcd.offset;
	ivt->boot_configuration_word = BCW_BOOT_TARGET_A53_0;
	ivt->application_boot_code_pointer = image_layout.app_code.offset;

#ifdef CONFIG_HSE_SECBOOT
	ivt->hse_h_firmware_pointer = image_layout.hse_reserved.offset;
#endif

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

#if CONFIG_SYS_TEXT_BASE < CONFIG_DTB_SRAM_ADDR
#error "mkimage: CONFIG_DTB_SRAM_ADDR is higher than CONFIG_SYS_TEXT_BASE"
#endif

		app_code->ram_start_pointer = CONFIG_DTB_SRAM_ADDR
							- pre_code_padding;
		app_code->ram_entry_pointer = CONFIG_SYS_TEXT_BASE;
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

#ifndef CONFIG_FLASH_BOOT
	enforce_reserved_ranges((void *)(__u64)
				app_code->ram_start_pointer,
				app_code->code_length);
#else
#ifndef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
	s32gen1_set_qspi_params(get_qspi_params(&image_layout));
#endif
#endif

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

static int s32g2xx_build_layout(struct program_image *program_image,
				size_t *header_size, void **image)
{
	uint8_t *image_layout;
	struct image_comp *parts[] = {&program_image->ivt,
#ifdef CONFIG_FLASH_BOOT
		&program_image->qspi_params,
#endif
		&program_image->ivt_duplicate,
		&program_image->dcd,
#ifdef CONFIG_HSE_SECBOOT
		&program_image->hse_reserved,
#endif
		&program_image->app_code,
		&program_image->code,
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

static int s32gen1_vrec_header(struct image_tool_params *tool_params,
			       struct image_type_params *type_params)
{
	size_t header_size;
	void *image = NULL;

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
#ifdef CONFIG_FLASH_BOOT
	fprintf(stderr, "QSPI Parameters:\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)image_layout.qspi_params.offset,
		(unsigned int)image_layout.qspi_params.size);
#endif
	fprintf(stderr, "IVT (duplicate):\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)image_layout.ivt_duplicate.offset,
		(unsigned int)image_layout.ivt_duplicate.size);
	fprintf(stderr, "DCD:\t\t\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)image_layout.dcd.offset,
		(unsigned int)image_layout.dcd.size);
#ifdef CONFIG_HSE_SECBOOT
	fprintf(stderr, "HSE Reserved:\t\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)image_layout.hse_reserved.offset,
		(unsigned int)image_layout.hse_reserved.size);
#endif
	fprintf(stderr, "AppBootCode Header:\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)image_layout.app_code.offset,
		(unsigned int)image_layout.app_code.size);
	fprintf(stderr, "U-Boot/FIP:\t\tOffset: 0x%x\t\tSize: 0x%x\n",
		(unsigned int)image_layout.code.offset,
		(unsigned int)image_layout.code.size);
#if defined(CONFIG_ENV_OFFSET) && defined(CONFIG_ENV_SIZE)
	fprintf(stderr, "U-Boot Environment:\tOffset: 0x%x\tSize: 0x%x\n",
		(unsigned int)CONFIG_ENV_OFFSET,
		(unsigned int)CONFIG_ENV_SIZE);
#endif
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
