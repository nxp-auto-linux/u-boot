/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2024 NXP
 */
#ifndef __S32N79SIM_H
#define __S32N79SIM_H

#include <configs/s32n79.h>

#define S32N_FDT_ADDR			0x93000000
#define S32N_FDT_HIGH_ADDR		0xffffffffffffffff
#define S32N_INITRD_HIGH_ADDR		0xffffffffffffffff
#define S32N_RAMDISK_ADDR		0xc0000000

#define FDT_FILE		"s32n79-simulator.dts"

#define S32N_ENV_SETTINGS \
	"boot_mtd=booti\0" \
	"console=ttyAMA0\0" \
	"fdt_addr=" __stringify(S32N_FDT_ADDR) "\0" \
	"fdt_file=" FDT_FILE "\0" \
	"fdt_high=" __stringify(S32N_FDT_HIGH_ADDR) "\0" \
	"fdt_override=;\0" \
	"image=Image\0" \
	"initrd_high=" __stringify(S32N_INITRD_HIGH_ADDR) "\0" \
	"ramdisk_addr=" __stringify(S32N_RAMDISK_ADDR) "\0" \
	"bootargs=earlycon maxcpus=1\0" \

#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif

#define CONFIG_BOOTCOMMAND "${boot_mtd} ${loadaddr} - ${fdt_addr}"

#define CONFIG_EXTRA_ENV_SETTINGS S32N_ENV_SETTINGS

#endif
