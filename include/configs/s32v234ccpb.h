/*
 * (C) Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Configuration settings for the X-S32GRV-PLAT board, Schematics 30081,
 * using the X-S32V234TPROC board, Schematics 30094, as processor board.
 *
 */

#ifndef __CONFIG_H
#define __CONFIG_H

#define	FDT_FILE fsl-s32v234-ccpb.dtb

/* 512 MB x 2 banks */
#define CONFIG_SYS_FSL_DRAM_SIZE1       0x20000000
#define CONFIG_SYS_FSL_DRAM_SIZE2       0x20000000

#define CONFIG_BOARD_USE_RAMFS_IN_NFSBOOT

#define CONFIG_DSPI_CS_SCK_DELAY 100
#define CONFIG_DSPI_SCK_CS_DELAY 100

#define CONFIG_DDR_INIT_DELAY 100

/* HDMI configs */
#define CONFIG_FSL_DCU_SII9022A
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_DVI_BUS_NUM      0
#define CONFIG_SYS_I2C_DVI_ADDR         0x39

/* Ethernet config */
#define CONFIG_FEC_XCV_TYPE     RGMII

#define CONFIG_FEC_MXC_PHYADDR  3

#ifdef CONFIG_SJA1105
#define SJA_1_BUS	0
#define SJA_1_CS	0
#endif /* CONFIG_SJA1105 */

/* we include this file here because it depends on the above definitions */
#include <configs/s32v234_common.h>

#endif
