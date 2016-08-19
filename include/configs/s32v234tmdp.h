/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Configuration settings for the Freescale S32V234 TMDP board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#undef CONFIG_RUN_FROM_IRAM_ONLY

#define CONFIG_RUN_FROM_DDR0
#undef CONFIG_RUN_FROM_DDR1

/* TMDP board has 2x256 MB DDR chips, DDR0 and DDR1, u-boot is using just one */
#define DDR_SIZE		(256 * 1024 * 1024)

#define CONFIG_MACH_TYPE		4146

/* Config CACHE */
#define CONFIG_CMD_CACHE

/* Enable DCU QoS fix */
#define CONFIG_DCU_QOS_FIX

/* System Timer */
#define CONFIG_SYS_GENERIC_TIMER
/* #define CONFIG_SYS_PIT_TIMER */

#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC_BASE_ADDR

/* #define CONFIG_CMD_EXT2 EXT2 Support */

#define CONFIG_FEC_XCV_TYPE     RGMII
#define CONFIG_PHYLIB

/* CONFIG_PHY_RGMII_DIRECT_CONNECTED should be enabled when
 * BCM switch is configured.
 */
#define CONFIG_BCM_SPEED	SPEED_100

#define FDT_FILE		s32v234-tmdp.dtb

#define CONFIG_LOADADDR		LOADADDR

#define CONFIG_BOARD_EXTRA_ENV_SETTINGS \
	"ipaddr=10.0.0.100\0" \
	"serverip=10.0.0.1\0" \
	"netmask=255.255.255.0\0" \
	"nfsbootargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs rw " \
		"ip=${ipaddr}:${serverip}::${netmask}::eth0:off " \
		"nfsroot=${serverip}:/tftpboot/rfs,nolock \0" \
	"loadtftpimage=tftp ${loadaddr} ${image};\0" \
	"loadtftpfdt=tftp ${fdt_addr} ${fdt_file};\0" \
	"nfsboot=echo Booting from net using tftp and nfs...; " \
		"run nfsbootargs;"\
		"run loadtftpimage; run loadtftpfdt;"\
		"${boot_mtd} ${loadaddr} - ${fdt_addr};\0"\

#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE - CONFIG_SYS_TEXT_OFFSET)

/* #define CONFIG_CMD_PCI */

/* we include this file here because it depends on the above definitions */
#include <configs/s32v234_common.h>

#endif
