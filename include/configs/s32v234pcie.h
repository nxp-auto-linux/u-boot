/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 *
 * Configuration settings for the Freescale S32V234PCIE board.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#undef CONFIG_RUN_FROM_IRAM_ONLY

/* S32V234PCIE has LPDDR2 */
#define CONFIG_S32V234_LPDDR2

#define CONFIG_RUN_FROM_DDR1
#undef CONFIG_RUN_FROM_DDR0

/* PCIE board has 2x256 MB DDR chips, DDR0 and DDR1, u-boot is using just one */
#define DDR_SIZE		(256 * 1024 * 1024)

/* Enable DDR handshake at functional reset event */
#define CONFIG_DDR_HANDSHAKE_AT_RESET

#define CONFIG_MACH_TYPE		4146

/* Config CACHE */
#define CONFIG_CMD_CACHE

/* Enable DCU QoS fix */
#define CONFIG_DCU_QOS_FIX

#define CONFIG_ARCH_EARLY_INIT_R

/* System Timer */
#define CONFIG_SYS_GENERIC_TIMER
/* #define CONFIG_SYS_PIT_TIMER */

#define LINFLEXUART_BASE		LINFLEXD1_BASE_ADDR

#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC_BASE_ADDR

/* #define CONFIG_CMD_EXT2 EXT2 Support */

#define CONFIG_FEC_XCV_TYPE     RGMII
#define CONFIG_PHYLIB

/* CONFIG_PHY_RGMII_DIRECT_CONNECTED should be enabled when
 * BCM switch is configured.
 */
/* #define CONFIG_PHY_RGMII_DIRECT_CONNECTED */
#ifdef CONFIG_PHY_RGMII_DIRECT_CONNECTED
#define CONFIG_BCM_SPEED    SPEED_1000
#else
#define CONFIG_FEC_MXC_PHYADDR  1
#define CONFIG_PHY_BROADCOM
#endif

#define CONFIG_LOADADDR		0xC307FFC0

#define CONFIG_EXTRA_ENV_SETTINGS \
	"script=boot.scr\0" \
	"uimage=uImage\0" \
	"ramdisk=rootfs.uimg\0"\
	"console=ttyLF1\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file=s32v234-pcie.dtb\0" \
	"fdt_addr=" __stringify(FDT_ADDR) "\0" \
	"kernel_addr=0xC307FFC0\0" \
	"ramdisk_addr=0xC4000000\0" \
	"boot_fdt=try\0" \
	"ip_dyn=yes\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=" __stringify(CONFIG_MMC_PART) "\0" \
	"mmcroot=/dev/mmcblk0p2 rootwait rw\0" \
	"update_sd_firmware_filename=u-boot.s32\0" \
	"update_sd_firmware=" \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"if mmc dev ${mmcdev}; then "	\
			"if ${get_cmd} ${update_sd_firmware_filename}; then " \
				"setexpr fw_sz ${filesize} / 0x200; " \
				"setexpr fw_sz ${fw_sz} + 1; "	\
				"mmc write ${loadaddr} 0x2 ${fw_sz}; " \
			"fi; "	\
		"fi\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loaduimage=fatload mmc ${mmcdev}:${mmcpart} ${kernel_addr} ${uimage}\0" \
	"loadramdisk=fatload mmc ${mmcdev}:${mmcpart} ${ramdisk_addr} ${ramdisk}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"jtagboot=echo Booting using jtag...; " \
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0" \
	"jtagsdboot=echo Booting loading Linux with ramdisk from SD...; " \
		"run loaduimage; run loadramdisk; run loadfdt;"\
		"bootm ${kernel_addr} ${ramdisk_addr} ${fdt_addr}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"bootm ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootm; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootm; " \
		"fi;\0" \
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs " \
	"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
		"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${uimage}; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"bootm ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"bootm; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"bootm; " \
		"fi;\0"

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE - CONFIG_SYS_TEXT_OFFSET)

/* #define CONFIG_CMD_PCI */
#ifdef CONFIG_CMD_PCI
#define CONFIG_PCIE_EP_MODE
#define CONFIG_GICSUPPORT
#define CONFIG_USE_IRQ
#define CONFIG_CMD_IRQ
#endif

/* we include this file here because it depends on the above definitions */
#include <configs/s32v234_common.h>

#endif
