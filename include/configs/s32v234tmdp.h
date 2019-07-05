/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Configuration settings for the Freescale S32V234 TMDP board.
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* The configurations of this board depend on the definitions in this file and
* the ones in the header included at the end, configs/s32v234_common.h */

#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC_BASE_ADDR

/* #define CONFIG_CMD_EXT2 EXT2 Support */

/* HDMI configs */
#define CONFIG_FSL_DCU_SII9022A
#define CONFIG_SYS_I2C_MXC_I2C2         /* enable I2C bus 2 */
#define CONFIG_SYS_I2C_DVI_BUS_NUM      1
#define CONFIG_SYS_I2C_DVI_ADDR         0x39

/* Ethernet config */

#define CONFIG_FEC_XCV_TYPE     RGMII
#define CONFIG_PHYLIB

/* CONFIG_PHY_RGMII_DIRECT_CONNECTED should be enabled when
 * BCM switch is configured.
 */
#define CONFIG_BCM_SPEED	SPEED_100

#define FDT_FILE		fsl-s32v234-tmdp.dtb


#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE - CONFIG_SYS_TEXT_OFFSET)

/* #define CONFIG_CMD_PCI */

/* we include this file here because it depends on the above definitions */
#include <configs/s32v234_common.h>

#endif
