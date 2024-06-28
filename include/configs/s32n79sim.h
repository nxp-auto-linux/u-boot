/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2024 NXP
 */
#ifndef __S32N79SIM_H
#define __S32N79SIM_H

#include <configs/s32n79.h>

#define FDT_FILE		"s32n79-simulator.dtb"

#define MMC_PART_FAT			1
#define MMC_ROOT			"/dev/mmcblk0p2"

#define EXTRA_BOOT_ARGS	\
	" maxcpus=2 arm64.nopauth"

#define FDT_FIXUP \
	"run fdt_init; run fixup_initrd_size;"

#define S32N_ENV_SETTINGS \
	S32_ENV_SETTINGS \
	"console=ttyAMA0\0" \
	"fdt_init=fdt addr ${fdt_addr}; fdt resize;\0" \
	"bootargs=console=${console},${baudrate} root=/dev/ram rw " \
		LINUX_EARLY_CONSOLE LINUX_LOG_DISABLE \
		EXTRA_BOOT_ARGS "\0" \
	"fixup_initrd_size=setexpr ramdisk_size_addr ${ramdisk_addr} - 0x4; " \
		  "setexpr.l initrd-end ${ramdisk_addr} + " \
		  "*${ramdisk_size_addr}; fdt set /chosen linux,initrd-start " \
		  "<${ramdisk_addr}>; fdt set /chosen linux,initrd-end " \
		  "<0x${initrd-end}>;\0" \

#define BOOTENV

#define CONFIG_BOOTCOMMAND \
	"run fdt_fixups; ${boot_mtd} ${loadaddr} - ${fdt_addr}"

#define CONFIG_EXTRA_ENV_SETTINGS S32N_ENV_SETTINGS

#endif
