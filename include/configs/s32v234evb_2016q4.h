/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Configuration settings for the Freescale S32V234 boards,
 * 2016, quarter 4 versions.
 */

#ifndef __S32V234EVB_2016Q4_CONFIG_H
#define __S32V234EVB_2016Q4_CONFIG_H

/* The configurations of this board depend on the definitions in this file and
* the ones in the header included at the end, configs/s32v234_common.h */

#define CONFIG_DDR_INIT_DELAY 100

#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC_BASE_ADDR

/* HDMI configs */
#define CONFIG_FSL_DCU_SII9022A
#define CONFIG_SYS_I2C_MXC_I2C2         /* enable I2C bus 2 */
#define CONFIG_SYS_I2C_DVI_BUS_NUM      1
#define CONFIG_SYS_I2C_DVI_ADDR         0x39

/* Ethernet config */

#define CONFIG_FEC_XCV_TYPE     RGMII
#define CONFIG_PHYLIB

#define CONFIG_FEC_MXC_PHYADDR  3
#define CONFIG_PHY_MICREL
#define CONFIG_PHY_MICREL_KSZ9031
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
