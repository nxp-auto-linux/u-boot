/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2024-2025 NXP
 */
#ifndef __S32N79_H
#define __S32N79_H

#include <configs/s32.h>

#define S32N_IEPRC_PHYS_ADDR 0x240000000ULL
#define S32N_IEPRC_PHYS_SIZE 0x2000000ULL

/* Highest valid peripheral page address. */
#define HIGHEST_PER_ADDR	(S32N_IEPRC_PHYS_ADDR + \
				 S32N_IEPRC_PHYS_SIZE - PAGE_SIZE)

#define FDT_FILE		CONFIG_DTB_FILE
#define IMAGE_FILE		"/BOOT/linux/Image"
#define BOOT_SCRIPT_FILE_SD	"/boot/boot.scr-sdcard"
#define BOOT_SCRIPT_FILE_UFS	"/boot/boot.scr-ufs"

#define MMC_PART_FAT			3
#define MMC_PART_EXT			4
#define MMC_ROOT			"/dev/mmcblk0p4"

#define EXTRA_BOOT_ARGS	\
	" arm64.nopauth clk_ignore_unused "

#define SCSI_LUN0_ID			"0"
#define SCSI_LUN1_ID			"1"
#define LUN0_IMAGE_NAME			"lun0.ufs"
#define LUN1_IMAGE_NAME			"lun1.ufs"
#define SCSI_PART_FDT			"0:3"
#define SCSI_PART_IMAGE			"1:1"
#define SCSI_PART_ROOTFS		"1:2"
#define SCSI_ROOT			"/dev/sdb2"

#define FDT_FIXUP ";"

#if defined(CONFIG_XEN_SUPPORT)
#  define XEN_COMMON_ENV \
	"script_addr=0x80000000\0" \
	"mmcpart_ext=" __stringify(MMC_PART_EXT) "\0" \

#  if defined(CONFIG_SD_BOOT)
#    define XEN_SCRIPT_FILE	BOOT_SCRIPT_FILE_SD
#    define XEN_BOOTCMD \
	"ext4load mmc ${mmcdev}:${mmcpart_ext} ${script_addr} ${script}; " \
	"source ${script_addr}; booti ${xen_addr} - ${fdt_addr};"
#  else /* UFS */
#    define XEN_SCRIPT_FILE	BOOT_SCRIPT_FILE_UFS
#    define XEN_BOOTCMD \
	"ext4load scsi ${scsi_part_rootfs} ${script_addr} ${script}; " \
	"source ${script_addr}; booti ${xen_addr} - ${fdt_addr};"
#  endif

#  define XEN_ENV_SETTINGS \
	XEN_COMMON_ENV \
	"script=" __stringify(XEN_SCRIPT_FILE) "\0"
#else
#  define XEN_ENV_SETTINGS      ""
#  define XEN_BOOTCMD           ""
#endif

#define S32N_ENV_SETTINGS \
	S32_ENV_SETTINGS \
	"console=ttyAMA" __stringify(CONFIG_CONS_INDEX) "\0" \
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
	"scsi_lun0_id=" SCSI_LUN0_ID "\0" \
	"scsi_lun1_id=" SCSI_LUN1_ID "\0" \
	"lun0_image_name=" LUN0_IMAGE_NAME "\0" \
	"lun1_image_name=" LUN1_IMAGE_NAME "\0" \
	"load_lun0_image=setenv image ${lun0_image_name}; run loadimage; setexpr n_blocks ${filesize} / 0x1000; setexpr n_blocks ${n_blocks} + 1\0" \
	"load_lun1_image=setenv image ${lun1_image_name}; run loadimage; setexpr n_blocks ${filesize} / 0x1000; setexpr n_blocks ${n_blocks} + 1\0" \
	"write_lun0=scsi scan; run load_lun0_image; scsi device ${scsi_lun0_id}; scsi write ${loadaddr} 0 ${n_blocks}\0" \
	"write_lun1=scsi scan; run load_lun1_image; scsi device ${scsi_lun1_id}; scsi write ${loadaddr} 0 ${n_blocks}\0" \
	"scsi_part_fdt=" SCSI_PART_FDT "\0" \
	"scsi_part_image=" SCSI_PART_IMAGE "\0" \
	"scsi_part_rootfs=" SCSI_PART_ROOTFS "\0" \
	"scsiroot=" SCSI_ROOT " rootwait rw\0" \
	XEN_ENV_SETTINGS \

#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif

#if defined(CONFIG_SD_BOOT)
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

#if defined(CONFIG_XEN_SUPPORT)
#  undef CONFIG_BOOTCOMMAND
#  define CONFIG_BOOTCOMMAND XEN_BOOTCMD
#endif

#define CFG_EXTRA_ENV_SETTINGS S32N_ENV_SETTINGS

#endif
