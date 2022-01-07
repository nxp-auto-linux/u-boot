/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2019-2022 NXP */

#ifndef S32GEN1IMAGE_H
#define S32GEN1IMAGE_H

#include <asm/types.h>
#include <stddef.h>

#define FIP_TOC_HEADER_NAME		(0xaa640001)
#define FIP_BL2_UUID			{0x5f, 0xf9, 0xec, 0x0b, \
					0x4d, 0x22, 0x3e, 0x4d, \
					0xa5, 0x44, 0xc3, 0x9d, \
					0x81, 0xc7, 0x3f, 0x0a}
#define FIP_BL2_OFFSET			(0x200)

#define BCW_BOOT_SEQ			(1 << 3)
#define BCW_SWT				(1 << 2)
#define BCW_BOOT_TARGET_M7_0		(0)
#define BCW_BOOT_TARGET_A53_0		(1 << 0)

#define LCCW_IN_FIELD			(1 << 1)
#define LCCW_OEM_PROD			(1 << 0)

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
} __attribute__((packed));

struct flash_write {
	union {
		struct {
			__u32 opcode:8;
			__u32 reserved:8;
			__u32 pad:2;
			__u32 addr_size:6;
			__u32 cdata_size:7;

			__u32 valid_addr:1;
		} config;
		__u32 _config;
	};
	__u32 addr;
	__u32 data;
} __attribute__((packed));

struct qspi_params {
	__u32 header;
	__u32 mcr;
	__u32 flshcr;
	__u32 bufgencr;
	__u32 dllcr;
	__u32 paritycr;
	__u32 sfacr;
	__u32 smpr;
	__u32 dlcr;
	__u32 sflash_1_size;
	__u32 sflash_2_size;
	__u32 dlpr;
	__u32 sfar;
	__u32 ipcr;
	__u32 tbdr;
	__u8 dll_bypass_en;
	__u8 dll_slv_upd_en;
	__u8 dll_auto_upd_en;
	__u8 ipcr_trigger_en;
	__u8 sflash_clk_freq;
	__u8 reserved[3];
	__u32 command_seq[80];
	struct flash_write writes[10];
};

struct image_comp {
	size_t offset;
	size_t size;
	size_t alignment;
	__u8 *data;
};

struct program_image {
	struct image_comp ivt;
	struct image_comp qspi_params;
	struct image_comp dcd;
	struct image_comp hse_reserved;
	struct image_comp app_code;
	struct image_comp code;
	__u8 *header;
};

struct qspi_params *get_macronix_qspi_conf(void);
struct qspi_params *get_micron_qspi_conf(void);

#endif /* S32GEN1IMAGE_H */
