/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2020 NXP
 */

/*
 * Configuration settings for the Freescale S32V234 EVB board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* The configurations of this board depend on the definitions in this file and
* the ones in the header included at the end, configs/s32v234_common.h */

/* HDMI configs */
#define CONFIG_FSL_DCU_SII9022A
#define CONFIG_SYS_I2C_MXC_I2C2         /* enable I2C bus 2 */
#define CONFIG_SYS_I2C_DVI_BUS_NUM      1
#define CONFIG_SYS_I2C_DVI_ADDR         0x39

/* 256 MB x 2 banks */
#define CONFIG_SYS_FSL_DRAM_SIZE1       0x10000000
#define CONFIG_SYS_FSL_DRAM_SIZE2       0x10000000

/* Ethernet config */
#define CONFIG_FEC_XCV_TYPE     RGMII

/* CONFIG_PHY_RGMII_DIRECT_CONNECTED should be enabled when
 * BCM switch is configured.
 */
#ifdef CONFIG_PHY_RGMII_DIRECT_CONNECTED
#define CONFIG_BCM_SPEED	SPEED_1000
#else
#define CONFIG_FEC_MXC_PHYADDR  7
#endif

#ifdef CONFIG_PHY_RGMII_DIRECT_CONNECTED
	#define FDT_FILE fsl-s32v234-evbbcm.dtb
#else
	#define	FDT_FILE fsl-s32v234-evb28899.dtb
#endif

/* we include this file here because it depends on the above definitions */
#include <configs/s32v234_common.h>

#endif
