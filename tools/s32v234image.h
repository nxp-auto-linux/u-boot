/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2019-2020 NXP */

#ifndef S32V234IMAGE_H
#define S32V234IMAGE_H

#include <asm/types.h>
#include <generated/autoconf.h>

#define DCD_HEADER			(0x500000d2)
#define IVT_VERSION			(0x50)

#define S32V234_IVT_OFFSET		(0x1000U)
#define S32V234_HEADER_SIZE		(0x1000U)
#define S32V234_INITLOAD_SIZE		(0x2000U)

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
	struct image_comp boot_data;
	struct image_comp dcd;
	__u8 *header;
};

#endif /* S32V234IMAGE_H */
