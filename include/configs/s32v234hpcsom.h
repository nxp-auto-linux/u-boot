/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 NXP
 *
 * Configuration settings for the NXP HPC SOM board, S32V SoC.
 * Schematics 50519.
 */

#ifndef __S32V234HPCSOM_H
#define __S32V234HPCSOM_H

/* The configurations of this board depend on the definitions in this file and
 * the ones in the header included at the end, configs/s32v234_common.h
 */

#define	FDT_FILE fsl-s32v234-hpcsom.dtb

#define CONFIG_SYS_FSL_DRAM_SIZE1	0x20000000
#define CONFIG_SYS_FSL_DRAM_SIZE2	0x20000000

#define CONFIG_FSL_DCU_FB

#define CONFIG_DDR_INIT_DELAY 100

#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC_BASE_ADDR

/* Ethernet config */

#define CONFIG_FEC_FIXED_SPEED  1000
#define CONFIG_FEC_XCV_TYPE     RGMII
#define CONFIG_FEC_MXC_PHYADDR	0

#define CONFIG_PCIE_EP_MODE

#ifdef CONFIG_CMD_PCI
#define CONFIG_GICSUPPORT
#define CONFIG_CMD_IRQ
#endif

#define PCIE_EXTRA_ENV_SETTINGS "hwconfig=pcie:mode=ep,clock=ext"

/* we include this file here because it depends on the above definitions */
#include <configs/s32v234_common.h>

#endif
