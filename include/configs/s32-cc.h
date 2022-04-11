/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 NXP
 */
#ifndef __S32CC_H__
#define __S32CC_H__

#include <linux/sizes.h>
#include <generated/autoconf.h>

/* memory mapped external flash */
#define CONFIG_SYS_FLASH_BASE		0x0UL
#define CONFIG_SYS_FLASH_SIZE		(SZ_512M)

#define PHYS_SDRAM_1			0x80000000UL
#define PHYS_SDRAM_1_SIZE		(SZ_2G)
#define PHYS_SDRAM_2			0x880000000UL
#define PHYS_SDRAM_2_SIZE		(SZ_2G)

/* DDR images layout */
#define S32CC_FDT_ADDR			0x83E00000
#define S32CC_FDT_HIGH_ADDR		0xa0000000

#define CONFIG_SYS_INIT_SP_OFFSET	(SZ_16K)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_DATA_BASE + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_MMC_ENV_DEV		0
#define MMC_PART_FAT			1

#define S32CC_ENV_SETTINGS \
	"boot_mtd=booti\0" \
	"console=ttyLF0\0" \
	"fdt_addr=" __stringify(S32CC_FDT_ADDR) "\0" \
	"fdt_file=" FDT_FILE "\0" \
	"fdt_high=" __stringify(S32CC_FDT_HIGH_ADDR) "\0" \
	"fdt_override=;\0" \
	"image=Image\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}; " \
		 "run fdt_override;\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate}" \
		" root=${mmcroot} earlycon " EXTRA_BOOT_ARGS "\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if run loadfdt; then " \
			"${boot_mtd} ${loadaddr} - ${fdt_addr}; " \
		"else " \
			"echo WARN: Cannot load the DT; " \
		"fi;\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=" __stringify(MMC_PART_FAT) "\0" \
	"mmcroot=/dev/mmcblk0p2 rootwait rw\0" \

#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif
#define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; " \
	"if mmc rescan; " \
	"then " \
		"if run loadimage; "\
		"then " \
			"run mmcboot; " \
		"fi; " \
	"fi"

#ifdef CONFIG_SYS_I2C_MXC
#define I2C_QUIRK_REG
#endif

#endif
