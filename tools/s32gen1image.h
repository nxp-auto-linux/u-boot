/* Copyright 2019 NXP */
/* SPDX-License-Identifier: GPL-2.0+ */

#ifndef _S32G2XXIMAGE_H_
#define _S32G2XXIMAGE_H_

#include <asm/types.h>
#include <generated/autoconf.h>

#define BCW_BOOT_SEQ			(1 << 3)
#define BCW_SWT				(1 << 2)
#define BCW_BOOT_TARGET_M7_0		(0)
#define BCW_BOOT_TARGET_A53_0		(1 << 0)

#define LCCW_IN_FIELD			(1 << 1)
#define LCCW_OEM_PROD			(1 << 0)

#define DCD_WRITE_COMMAND_TAG		0xcc
#define DCD_CHECK_COMMAND_TAG		0xcf
#define DCD_NOP_COMMAND_TAG		0xc0

#define DCD_COMMAND_PARAMS_DATA_SET	(1 << 4)
#define DCD_COMMAND_PARAMS_DATA_MASK	(1 << 3)

#define IVT_TAG				0xd1
#define IVT_VERSION			0x60
#define DCD_TAG				0xd2
#define DCD_VERSION			0x60
#define APPLICATION_BOOT_CODE_TAG	0xd5
#define APPLICATION_BOOT_CODE_VERSION	0x60

#define SRAM_RESERVED_0_START		0x34008050
#define SRAM_RESERVED_0_END		0x34008200
#define SRAM_RESERVED_1_START		0x38008050
#define SRAM_RESERVED_1_END		0x38008200

#define DCD_MAXIMUM_SIZE		8192

enum dcd_command_type {
	INVALID = -1,
	WRITE_DATA,
	WRITE_SET_BITMASK,
	WRITE_CLEAR_BITMASK,
	CHECK_BITS_ARE_SET,
	CHECK_BITS_ARE_CLEAR,
	CHECK_ANY_BIT_IS_SET,
	CHECK_ANY_BIT_IS_CLEAR,
	NOP,
};

struct dcd_command {
	__u8	tag;
	__u16	length;
	__u8	params;
	union specific {
		struct check_command_specific {
			__u32	address;
			__u32	mask;
			__u32	count;
		} check;
		struct write_command_specific {
			__u32	addr;
			__u32	data;
		} write[0];
	} s;
} __attribute__((packed));

struct dcd {
	__u8	tag;
	__u16	length;
	__u8	version;
	__u8	commands[DCD_MAXIMUM_SIZE - 4 - 16];
	__u8	gmac[16];
} __attribute__((packed, aligned(512)));

struct ivt {
	__u8		tag;
	__u16		length;
	__u8		version;
	__u8		reserved1[4];
	__u32		self_test_dcd_pointer;
	__u32		self_test_dcd_pointer_backup;
	__u32		dcd_pointer;
	__u32		dcd_pointer_backup;
	__u32		hse_h_firmware_pointer;
	__u32		hse_h_firmware_pointer_backup;
	__u32		application_boot_code_pointer;
	__u32		application_boot_code_pointer_backup;
	__u32		boot_configuration_word;
	__u32		lifecycle_configuration_word;
	__u8		reserved2[4];
	__u8		reserved_for_hse_h_fw[32];
	__u8		reserved3[156];
	__u32		gmac[4];
} __attribute__((packed));

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
} __attribute__((packed, aligned(512)));

struct program_image {
	struct ivt ivt;
	/* padding required for not overelapping with the MBR */
	__u8 padding[512 - sizeof(struct ivt)];
#ifdef CONFIG_FLASH_BOOT
	__u8 qspi_params[512];
#endif
	struct dcd dcd;
	struct application_boot_code application_boot_code;
};

#endif /* _S32G2XXIMAGE_H_ */
