/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2019-2020 NXP */

#ifndef S32V234IMAGE_H
#define S32V234IMAGE_H

#include <asm/types.h>
#include <generated/autoconf.h>

#define DCD_HEADER			(0x500000d2)
#define IVT_VERSION			(0x50)

#define S32V234_IVT_OFFSET		(0x1000U)
#define S32V234_INITLOAD_SIZE		(0x2000U)

#ifdef CONFIG_FLASH_BOOT
#  define S32V234_COMMAND_SEQ_FILL_OFF 20
#endif

#ifdef CONFIG_FLASH_BOOT
#  define S32V234_QSPI_PARAMS_OFFSET	0x200U
#  define S32V234_QSPI_PARAMS_SIZE	0x200
#endif

#ifdef CONFIG_FLASH_BOOT
#  define S32V234_HEADER_SIZE	0x2000U
#else
#  define S32V234_HEADER_SIZE	0x1000U
#endif

struct ivt {
	__u8		tag;
	__u16		length;
	__u8		version;
	__u32		entry;
	__u32		reserved1;
	__u32		dcd_pointer;
	__u32		boot_data_pointer;
	__u32		self;
	__u32		reserved2;
	__u32		self_test;
	__u32		reserved3;
	__u32		reserved4;
} __attribute((packed));

struct boot_data {
	__u32		start;
	__u32		length;
	__u8		reserved2[4];
} __packed;

struct program_image {
	struct image_comp ivt;
#ifdef CONFIG_FLASH_BOOT
	struct image_comp qspi_params;
#endif
	struct image_comp boot_data;
	struct image_comp dcd;
	__u8 *header;
};

#ifdef CONFIG_FLASH_BOOT
struct qspi_params {
	__u32 dqs;
	__u8 hold_delay;
	__u8 half_speed_phase_sel;
	__u8 half_speed_delay_sel;
	__u8 reserved1;
	__u32 clock_conf;
	__u32 soc_conf;
	__u32 reserved2;
	__u32 cs_hold;
	__u32 cs_setup;
	__u32 flash_a1_size;
	__u32 flash_a2_size;
	__u32 flash_b1_size;
	__u32 flash_b2_size;
	__u32 clock_freq;
	__u32 reserved3;
	__u8 mode;
	__u8 flash_b_sel;
	__u8 ddr_mode;
	__u8 dss;
	__u8 parallel_mode_en;
	__u8 cs1_port_a;
	__u8 cs1_port_b;
	__u8 full_speed_phase_sel;
	__u8 full_speed_delay_sel;
	__u8 ddr_sampling_point;
	__u8 luts[256];
};

static const struct qspi_params s32v234_qspi_params = {
	.hold_delay = 0x1,
	.flash_a1_size = 0x40000000,
	.clock_freq = 0x3,
	.ddr_mode = 0x1,
	.dss = 0x1,
	.luts = {
		/*Flash specific LUT */
		0xA0, 0x47, 0x18, 0x2B, 0x10, 0x4F, 0x0F, 0x0F, 0x80,
		/* 128 bytes*/
		0x3B, 0x00, 0x03,
		/*STOP - 8pads*/
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	}
};
#endif //CONFIG_FLASH_BOOT

#endif /* S32V234IMAGE_H */
