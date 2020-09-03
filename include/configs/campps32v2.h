/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
 *
 * Configuration settings for the NXP X-CAMPPS32V2 board,
 * Schematics 46300.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* The configurations of this board depend on the definitions in this file and
 * the ones in the header included at the end, configs/s32v234_common.h
 */

#define	FDT_FILE fsl-s32v234-campp-primary.dtb
#define FDT_FILE_SEC fsl-s32v234-campp-secondary.dtb

#define CONFIG_SYS_FSL_DRAM_SIZE1	0x20000000
#define CONFIG_SYS_FSL_DRAM_SIZE2	0x20000000

/* #define CONFIG_CMD_PCI */

#define CONFIG_BOARD_USE_RAMFS_IN_NFSBOOT
#define CONFIG_BOARD_EXTRA_ENV_SETTINGS \
	"setphy=mii write 3 d 2; mii write 3 e 2; mii write 3 d 4002; " \
	"mii write 3 e 8000; mii write 3 0 8000;\0"

#define CONFIG_DSPI_CS_SCK_DELAY 100
#define CONFIG_DSPI_SCK_CS_DELAY 100

#define CONFIG_DDR_INIT_DELAY 100

#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC_BASE_ADDR

/* Ethernet config */

#define CONFIG_FEC_FIXED_SPEED  1000
#define CONFIG_FEC_MXC_PHYADDR  3
#define CONFIG_FEC_XCV_TYPE     RGMII

#define CONFIG_PCIE_EP_MODE

#ifdef CONFIG_CMD_PCI
#define CONFIG_GICSUPPORT
#define CONFIG_CMD_IRQ
#endif

#define PCIE_EXTRA_ENV_SETTINGS "hwconfig=pcie:mode=ep,clock=ext"

#ifdef CONFIG_SJA1105
#define SJA_1_BUS	0
#define SJA_1_CS	0
#define SJA_2_BUS	0
#define SJA_2_CS	1
#endif /* CONFIG_SJA1105 */

/* we include this file here because it depends on the above definitions */
#include <configs/s32v234_common.h>

#endif
