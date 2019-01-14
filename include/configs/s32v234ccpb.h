/*
 * (C) Copyright 2018 NXP
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


#define	FDT_FILE s32v234-ccpb.dtb

#define CONFIG_BOARD_USE_RAMFS_IN_NFSBOOT

#define CONFIG_DSPI_CS_SCK_DELAY 100
#define CONFIG_DSPI_SCK_CS_DELAY 100

#define CONFIG_DDR_INIT_DELAY 100

#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC_BASE_ADDR

/* HDMI configs */
#define CONFIG_FSL_DCU_SII9022A
#define CONFIG_SYS_I2C_MXC_I2C1		/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_DVI_BUS_NUM      0
#define CONFIG_SYS_I2C_DVI_ADDR         0x39

/* Ethernet config */
#define CONFIG_FEC_XCV_TYPE     RGMII

#define CONFIG_FEC_MXC_PHYADDR  3
#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE - CONFIG_SYS_TEXT_OFFSET)

#define CONFIG_CMD_PCI

#ifdef CONFIG_CMD_PCI
#define CONFIG_GICSUPPORT
#define CONFIG_USE_IRQ
#define CONFIG_CMD_IRQ
#endif

#ifdef CONFIG_SJA1105
#define SJA_1_BUS	0
#define SJA_1_CS	0
#endif /* CONFIG_SJA1105 */

/* we include this file here because it depends on the above definitions */
#include <configs/s32v234_common.h>

#endif
