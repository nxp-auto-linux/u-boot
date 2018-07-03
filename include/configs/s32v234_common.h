/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2016-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Configuration settings for all the Freescale S32V234 boards.
 */

#ifndef __S32V234_COMMON_H
#define __S32V234_COMMON_H

#define CONFIG_S32V234

#if CONFIG_FSL_LINFLEX_MODULE == 0
#define LINFLEXUART_BASE	LINFLEXD0_BASE_ADDR
#else
#define LINFLEXUART_BASE	LINFLEXD1_BASE_ADDR
#endif

/* memory mapped external flash */
#define CONFIG_SYS_FSL_FLASH0_BASE      0x20000000
#define CONFIG_SYS_FSL_FLASH0_SIZE      0x10000000
#define CONFIG_SYS_FSL_FLASH1_BASE      0x60000000
#define CONFIG_SYS_FSL_FLASH1_SIZE      0x10000000
#define QSPI_BASE_ADDR		        0x400A6000
/* Flash related definitions */
#define CONFIG_S32V234_USES_FLASH

#include <configs/s32.h>

/* Config DCU */
#ifdef CONFIG_FSL_DCU_FB
#define CONFIG_SYS_DCU_ADDR             0x40028000
#define DCU_LAYER_MAX_NUM               8
#define DCU_CTRL_DESC_LAYER_NUM		10
#define CONFIG_SYS_FSL_DCU_LE
#endif

/* Config GIC */
#define CONFIG_GICV2
#define GICD_BASE		0x7D001000
#define GICC_BASE		0x7D002000

/* Run by default from DDR1 */
#ifdef CONFIG_RUN_FROM_DDR0
#define DDR_BASE_ADDR			0x80000000
#define CONFIG_STANDALONE_LOAD_ADDR	0x80100000
#else
#define DDR_BASE_ADDR			0xC0000000
#define CONFIG_STANDALONE_LOAD_ADDR	0xC0100000
#endif

#define CONFIG_SYS_TEXT_BASE        0x3E820000 /* SDRAM */
#define CONFIG_SYS_TEXT_OFFSET      0x00020000

#define IMX_FEC_BASE            ENET_BASE_ADDR

#define MMAP_DSPI                       SPI0_BASE_ADDR


#endif
