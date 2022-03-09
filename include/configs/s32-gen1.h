/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2022 NXP
 *
 */

/*
 * Configuration settings for the Freescale/NXP S32-GEN1.
 */

#ifndef __NXP_S32_CC_H
#define __NXP_S32_CC_H

#include <linux/sizes.h>

#define PHYS_SDRAM_1       0x80000000
#ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
#define PHYS_SDRAM_1_SIZE       0x40000000
#else
#define PHYS_SDRAM_1_SIZE       0x80000000
#define PHYS_SDRAM_2       0x880000000
#define PHYS_SDRAM_2_SIZE       0x80000000
#endif

#ifdef CONFIG_CMD_IRQ
#define CONFIG_GICSUPPORT
#define CONFIG_USE_IRQ
#endif

/* memory mapped external flash */
#define CONFIG_SYS_FLASH_BASE		0x0
#define CONFIG_SYS_FLASH_SIZE		0x20000000

#define QSPI_BASE_ADDR		        0x40134000

#if defined(CONFIG_TARGET_S32G274ABLUEBOX3)
#define CONFIG_BOARD_USE_RAMFS_IN_NFSBOOT
#endif

/* we include this file here because it depends on the above definitions */
#include <configs/s32.h>

#define S32_SRAM_BASE		0x34000000

#ifdef CONFIG_ARCH_S32G3
#define S32_SRAM_SIZE		(20 * SZ_1M)
#else
#define S32_SRAM_SIZE		(8 * SZ_1M)
#endif

#if defined(CONFIG_SPI_FLASH) && defined(CONFIG_FSL_QSPI)
#define CONFIG_SYS_FSL_QSPI_AHB

#undef FSL_QSPI_FLASH_SIZE
#define FSL_QSPI_FLASH_SIZE            SZ_64M
#endif

#endif
