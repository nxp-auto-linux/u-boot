/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2024 NXP
 */
#ifndef __S32_H__
#define __S32_H__

#include <linux/sizes.h>
#include <generated/autoconf.h>

/* Disable Ramdisk & FDT relocation*/
#define S32_INITRD_HIGH_ADDR		0xffffffffffffffff
#define S32_FDT_HIGH_ADDR		0xffffffffffffffff

#define CONFIG_SYS_INIT_SP_OFFSET	(SZ_16K)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_DATA_BASE + \
					 CONFIG_SYS_MALLOC_F_LEN + \
					 GENERATED_GBL_DATA_SIZE + \
					 CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_MMC_ENV_DEV		0

/* Increase max gunzip size */
#define CONFIG_SYS_BOOTM_LEN	(SZ_64M)

#if defined(CONFIG_NO_LINUX_EARLY_CONSOLE)
#  define LINUX_EARLY_CONSOLE ""
#else
#  define LINUX_EARLY_CONSOLE \
	" earlycon"
#endif

#if defined(CONFIG_LINUX_LOG_DISABLE)
#  define LINUX_LOG_DISABLE \
	" quiet"
#else
#  define LINUX_LOG_DISABLE ""
#endif

#define S32_ENV_SETTINGS \
	BOOTENV \
	"boot_mtd=booti\0" \
	"console=ttyLF0\0" \
	"fdt_addr=" __stringify(CONFIG_DTB_ADDR) "\0" \
	"fdt_file=" FDT_FILE "\0" \
	"fdt_fixups=" FDT_FIXUP "\0" \
	"fdt_high=" __stringify(S32_FDT_HIGH_ADDR) "\0" \
	"fdt_override=;\0" \
	"image=Image\0" \
	"initrd_high=" __stringify(S32_INITRD_HIGH_ADDR) "\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}; " \
		 "run fdt_override;\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate}" \
		" root=${mmcroot}" LINUX_EARLY_CONSOLE LINUX_LOG_DISABLE \
		EXTRA_BOOT_ARGS "\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if run loadfdt; then " \
			"run fdt_fixups; " \
			"${boot_mtd} ${loadaddr} - ${fdt_addr}; " \
		"fi;\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=" __stringify(MMC_PART_FAT) "\0" \
	"mmcroot=" __stringify(MMC_ROOT) " rootwait rw\0" \
	"ramdisk_addr=" __stringify(CONFIG_RAMDISK_ADDR) "\0" \

#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif

#define CONFIG_SYS_CBSIZE		(SZ_512)
#endif
