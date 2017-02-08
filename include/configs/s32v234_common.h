/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Configuration settings for all the Freescale S32V234 boards.
 */

#ifndef __S32V234_COMMON_H
#define __S32V234_COMMON_H

#include <configs/s32v.h>
#include <asm/arch/imx-regs.h>

#define CONFIG_S32V234

/* Config DCU */
#ifdef CONFIG_FSL_DCU_FB
#define CONFIG_SYS_DCU_ADDR             0x40028000
#define DCU_LAYER_MAX_NUM               8
#define DCU_CTRL_DESC_LAYER_NUM		10
#define CONFIG_SYS_FSL_DCU_LE

/* HDMI configs */
#define CONFIG_FSL_DCU_SII9022A
#define CONFIG_SYS_I2C_MXC_I2C2         /* enable I2C bus 2 */
#define CONFIG_SYS_I2C_DVI_BUS_NUM      1
#define CONFIG_SYS_I2C_DVI_ADDR         0x39
#endif

/* Config GIC */
#define CONFIG_GICV2
#define GICD_BASE		0x7D001000
#define GICC_BASE		0x7D002000

#define CONFIG_REMAKE_ELF

/* Run by default from DDR1 */
#ifdef CONFIG_RUN_FROM_DDR0
#define DDR_BASE_ADDR		0x80000000
#else
#define DDR_BASE_ADDR		0xC0000000
#endif

#define CONFIG_SYS_TEXT_BASE        0x3E820000 /* SDRAM */
#define CONFIG_SYS_TEXT_OFFSET      0x00020000

#define IMX_FEC_BASE            ENET_BASE_ADDR

#define CONFIG_S32V234_FLASH

/* memory mapped external flash */
#define CONFIG_SYS_FSL_FLASH0_BASE      0x20000000
#define CONFIG_SYS_FSL_FLASH0_SIZE      0x10000000
#define CONFIG_SYS_FSL_FLASH1_BASE      0x60000000
#define CONFIG_SYS_FSL_FLASH1_SIZE      0x10000000

/* QSPI/hyperflash configs */
#ifdef CONFIG_S32V234_FLASH

/* debug stuff for qspi/hyperflash */
#undef CONFIG_DEBUG_S32V234_QSPI_QSPI

/* flash comand disabled until implemented */
#undef CONFIG_CMD_FLASH

#define QSPI_BASE_ADDR		0x400A6000
#define FLASH_BASE_ADR		CONFIG_SYS_FSL_FLASH0_BASE
#define FLASH_BASE_ADR2		0x24000000

#endif


#endif
