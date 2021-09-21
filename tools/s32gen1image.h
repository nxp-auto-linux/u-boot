/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2019-2021 NXP */

#ifndef S32GEN1IMAGE_H
#define S32GEN1IMAGE_H

#include <asm/types.h>
#include <generated/autoconf.h>

#define FIP_TOC_HEADER_NAME		(0xaa640001)
#define FIP_BL2_UUID			{0x5f, 0xf9, 0xec, 0x0b, \
					0x4d, 0x22, 0x3e, 0x4d, \
					0xa5, 0x44, 0xc3, 0x9d, \
					0x81, 0xc7, 0x3f, 0x0a}
#define FIP_BL2_OFFSET			(0x200)

struct fip_image_data {
	__u32		toc_header_name;
	__u32		dont_care1;
	__u64		dont_care2;
	__u8		uuid[16];
	__u64		offset;
	__u64		size;
	__u8		dont_care3[0];
};

#define BCW_BOOT_SEQ			(1 << 3)
#define BCW_SWT				(1 << 2)
#define BCW_BOOT_TARGET_M7_0		(0)
#define BCW_BOOT_TARGET_A53_0		(1 << 0)

#define LCCW_IN_FIELD			(1 << 1)
#define LCCW_OEM_PROD			(1 << 0)

#define DCD_HEADER            (0x600000d2)
#define MSCR25_SET_GPIO25_SRC (0x21c000)
#define GPDO25_HIGH           (0x1)

#define IVT_VERSION			(0x60)
#define APPLICATION_BOOT_CODE_TAG	(0xd5)
#define APPLICATION_BOOT_CODE_VERSION	(0x60)

#define S32GEN1_QSPI_PARAMS_SIZE	(0x200)

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

#ifdef CONFIG_FLASH_BOOT
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
#endif

struct program_image {
	struct image_comp ivt;
#ifdef CONFIG_FLASH_BOOT
	struct image_comp qspi_params;
#endif
	struct image_comp ivt_duplicate;
	struct image_comp dcd;
#ifdef CONFIG_HSE_SECBOOT
	struct image_comp hse_reserved;
#endif
	struct image_comp app_code;
	struct image_comp code;
	__u8 *header;
};

struct qspi_params *get_s32g2xx_qspi_conf(void);

#endif /* S32GEN1IMAGE_H */
