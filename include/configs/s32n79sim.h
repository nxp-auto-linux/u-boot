/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2024 NXP
 */
#ifndef __S32N79SIM_H
#define __S32N79SIM_H

#include <configs/s32n79.h>

#define FDT_FILE		"s32n79-simulator.dtb"

#define MMC_PART_FAT			3
#define MMC_ROOT			"/dev/mmcblk0p4"

#define EXTRA_BOOT_ARGS	\
	" maxcpus=2 arm64.nopauth"

#define SCSI_PART_FDT			"0:3"
#define SCSI_PART_IMAGE			"1:1"
#define SCSI_ROOT			"/dev/sdb2"

/* Boot from memory. Can be used in Simulator/Emulator case. */
#if defined(CONFIG_MEM_BOOT)
#  define FDT_FIXUP \
	"run fdt_init; run fixup_initrd_size;"
#else
#  define FDT_FIXUP ";"
#endif

#define S32N_ENV_SETTINGS \
	S32_ENV_SETTINGS \
	"console=ttyAMA0\0" \
	"fdt_init=fdt addr ${fdt_addr}; fdt resize;\0" \
	"loadufsfdt=fatload scsi ${scsi_part_fdt} ${fdt_addr} " \
		 " ${fdt_file}; run fdt_override;\0" \
	"loadufsimage=fatload scsi ${scsi_part_image} ${loadaddr} ${image}\0" \
	"ufsargs=setenv bootargs console=${console},${baudrate}" \
		" root=${scsiroot} " LINUX_EARLY_CONSOLE LINUX_LOG_DISABLE \
		EXTRA_BOOT_ARGS "\0" \
	"ufsboot=echo Booting from ufs ...; " \
		"run ufsargs; " \
		"if run loadufsfdt; then " \
			"run fdt_fixups; " \
			"${boot_mtd} ${loadaddr} - ${fdt_addr}; " \
		"fi;\0" \
	"scsi_part_fdt=" SCSI_PART_FDT "\0" \
	"scsi_part_image=" SCSI_PART_IMAGE "\0" \
	"scsiroot=" SCSI_ROOT " rootwait rw\0" \

#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif

#if defined(CONFIG_MEM_BOOT) /* Boot from DDR. */
#  define BOOTENV \
	"bootargs=console=${console},${baudrate} root=/dev/ram rw " \
		LINUX_EARLY_CONSOLE LINUX_LOG_DISABLE \
		EXTRA_BOOT_ARGS "\0" \
	"fixup_initrd_size=setexpr ramdisk_size_addr ${ramdisk_addr} - 0x4; " \
		  "setexpr.l initrd-end ${ramdisk_addr} + " \
		  "*${ramdisk_size_addr}; fdt set /chosen linux,initrd-start " \
		  "<${ramdisk_addr}>; fdt set /chosen linux,initrd-end " \
		  "<0x${initrd-end}>;\0" \

#  define CONFIG_BOOTCOMMAND \
	"run fdt_fixups; ${boot_mtd} ${loadaddr} - ${fdt_addr}"

#elif defined(CONFIG_SD_BOOT)
#  define BOOTENV
#  define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; " \
	"if mmc rescan; " \
	"then " \
		"if run loadimage; "\
		"then " \
			"run mmcboot; " \
		"fi; " \
	"fi"

#else /* Boot from UFS. */
#  define BOOTENV
#  define CONFIG_BOOTCOMMAND \
	"if scsi scan; " \
	"then " \
		"if run loadufsimage; "\
		"then " \
			"run ufsboot; " \
		"fi; " \
	"fi"
#endif

#define CONFIG_EXTRA_ENV_SETTINGS S32N_ENV_SETTINGS

#endif
