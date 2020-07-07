// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2019-2020 NXP */

#include <image.h>
#include <generated/autoconf.h>
#include <config.h>
#include "imagetool.h"
#include "s32_common.h"
#include "s32v234image.h"
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/mc_cgm_regs.h>

static struct program_image image_layout = {
	.ivt = {
		/* The offset is actually 0x1000, but we do not
		 * want to integrate it in the generated image.
		 * This allows writing the image at 0x1000 on
		 * sdcard/qspi, which avoids overwriting the
		 * partition table.
		 */
		.offset = 0x0,
		.size = sizeof(struct ivt),
	},
	.boot_data = {
		.offset = S32_AUTO_OFFSET,
		.alignment = 0x8U,
		.size = sizeof(struct boot_data),
	},
	.dcd = {
		.offset = S32_AUTO_OFFSET,
		.alignment = 0x8U,
		.size = DCD_MAXIMUM_SIZE,
	},
};

static uint32_t dcd_data[] = {
	DCD_HEADER,
	DCD_WRITE_HEADER(4, PARAMS_BYTES(4)),
	DCD_ADDR(FXOSC_CTL), DCD_MASK(FXOSC_CTL_FASTBOOT_VALUE),

#ifdef CONFIG_S32V234_FAST_BOOT
	DCD_ADDR(MC_ME_DRUN_MC),
	DCD_MASK(DRUN_MC_RESETVAL | MC_ME_RUNMODE_MC_XOSCON |
		 MC_ME_RUNMODE_MC_PLL(ARM_PLL) |
		 MC_ME_RUNMODE_MC_PLL(ENET_PLL) |
		 MC_ME_RUNMODE_MC_SYSCLK(SYSCLK_ARM_PLL_DFS_1)),
#else
	DCD_ADDR(MC_ME_DRUN_MC),
	DCD_MASK(DRUN_MC_RESETVAL | MC_ME_RUNMODE_MC_XOSCON |
		 MC_ME_RUNMODE_MC_SYSCLK(SYSCLK_FXOSC)),
#endif

	DCD_ADDR(MC_ME_MCTL),
	DCD_MASK(MC_ME_MCTL_KEY | MC_ME_MCTL_DRUN),
	DCD_ADDR(MC_ME_MCTL),
	DCD_MASK(MC_ME_MCTL_INVERTEDKEY | MC_ME_MCTL_DRUN),
};

static struct ivt *get_ivt(struct program_image *image)
{
	return (struct ivt *)image->ivt.data;
}

static uint8_t *get_dcd(struct program_image *image)
{
	return image->dcd.data;
}

static struct boot_data *get_boot_data(struct program_image *image)
{
	return (struct boot_data *)image->boot_data.data;
}

static void set_data_pointers(struct program_image *layout, void *header)
{
	uint8_t *data = (uint8_t *)header;

	layout->ivt.data = data + layout->ivt.offset;
	layout->boot_data.data = data + layout->boot_data.offset;
	layout->dcd.data = data + layout->dcd.offset;
}

static void s32gen1_set_header(void *header, struct stat *sbuf, int unused,
			       struct image_tool_params *tool_params)
{
	uint8_t *dcd;
	struct ivt *ivt;
	struct boot_data *boot_data;

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
	ivt->tag = IVT_TAG;
	ivt->length = cpu_to_be16(sizeof(struct ivt));
	ivt->version = IVT_VERSION;
	ivt->entry = CONFIG_SYS_TEXT_BASE;
	ivt->self = ivt->entry - S32V234_INITLOAD_SIZE + S32V234_IVT_OFFSET;
	ivt->dcd_pointer = ivt->self + image_layout.dcd.offset;
	ivt->boot_data_pointer = ivt->self +  image_layout.boot_data.offset;

	boot_data = get_boot_data(&image_layout);
	boot_data->start = ivt->entry - S32V234_INITLOAD_SIZE;
	boot_data->length = ROUND(sbuf->st_size + S32V234_INITLOAD_SIZE,
				  0x1000);
}

static int s32gen1_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_S32V234IMAGE)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

static int s32g2xx_build_layout(struct program_image *program_image,
				size_t *header_size, void **image)
{
	uint8_t *image_layout;
	struct image_comp *parts[] = {&program_image->ivt,
		&program_image->boot_data,
		&program_image->dcd,
	};
	size_t last_comp = ARRAY_SIZE(parts) - 1;

	program_image->dcd.size = sizeof(dcd_data);

	qsort(&parts[0], ARRAY_SIZE(parts), sizeof(parts[0]), image_parts_comp);

	/* Compute auto-offsets */
	s32_compute_dyn_offsets(parts, ARRAY_SIZE(parts));

	*header_size = S32V234_HEADER_SIZE;
	if (parts[last_comp]->offset + parts[last_comp]->size > *header_size) {
		perror("S32V234 Header is too large");
		exit(EXIT_FAILURE);
	}

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

U_BOOT_IMAGE_TYPE(
	s32v2image,
	"NXP S32V234 Boot Image",
	0,
	NULL,
	NULL,
	NULL,
	s32_print_header,
	s32gen1_set_header,
	NULL,
	s32gen1_check_image_type,
	NULL,
	s32gen1_vrec_header
);
