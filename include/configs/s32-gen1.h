/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2020 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Configuration settings for the Freescale/NXP S32-GEN1.
 */

#ifndef __S32_GEN1_H
#define __S32_GEN1_H

/* The configurations of this board depend on the definitions in this file and
* the ones in the header included at the end, configs/s32.h */

/* Init CSE3 from u-boot */
/* #define CONFIG_CSE3		1 */

#undef CONFIG_RUN_FROM_IRAM_ONLY

/* u-boot uses just DDR0 */
#define CONFIG_RUN_FROM_DDR0
#undef CONFIG_RUN_FROM_DDR1

#define CONFIG_MACH_TYPE		4146

/* Config CACHE */
#define CONFIG_CMD_CACHE

/* Flat device tree definitions */
#define CONFIG_OF_FDT
#define CONFIG_OF_BOARD_SETUP

/* System Timer */
/* #define CONFIG_SYS_PIT_TIMER */

#define CONFIG_FSL_USDHC
#define CONFIG_SYS_FSL_ESDHC_ADDR	USDHC_BASE_ADDR

/* #define CONFIG_CMD_EXT2 EXT2 Support */

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

#if defined(CONFIG_TARGET_S32G274AEVB)
#define FDT_FILE fsl-s32g274a-evb.dtb
#elif defined(CONFIG_TARGET_S32G274ARDB)
#define FDT_FILE fsl-s32g274a-rdb.dtb
#elif defined(CONFIG_TARGET_S32R45XEVB)
#define FDT_FILE fsl-s32r45x-evb.dtb
#endif

#define CONFIG_LOADADDR		LOADADDR

#define CONFIG_SYS_INIT_SP_OFFSET	(CONFIG_UBOOT_SRAM_FOOTPRINT - \
			GENERATED_GBL_DATA_SIZE - CONFIG_SYS_TEXT_OFFSET)

#ifdef CONFIG_CMD_IRQ
#define CONFIG_GICSUPPORT
#define CONFIG_USE_IRQ
#endif

#if CONFIG_FSL_LINFLEX_MODULE == 0
#define LINFLEXUART_BASE	LINFLEXD0_BASE_ADDR
#elif CONFIG_FSL_LINFLEX_MODULE == 1
#define LINFLEXUART_BASE	LINFLEXD1_BASE_ADDR
#else
#define LINFLEXUART_BASE	LINFLEXD2_BASE_ADDR
#endif

/* memory mapped external flash */
#define CONFIG_SYS_FSL_FLASH0_BASE      0x0
#define CONFIG_SYS_FSL_FLASH0_SIZE      0x20000000
#define QSPI_BASE_ADDR		        0x40134000
/* Flash related definitions */
#define CONFIG_S32_GEN_1_USES_FLASH

/* we include this file here because it depends on the above definitions */
#include <configs/s32.h>

#define IMX_FEC_BASE            ENET0_BASE_ADDR

#if defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
/* In secure boot scenarios, the Trusted Firmware runs at EL3, while U-Boot runs
 * in the non-secure world. This produces errors while U-Boot attempts to
 * configure the secure GIC registers. As a result, GICv3 initialization on S32G
 * is done by the Trusted Firmware - unless we run U-Boot at EL3.
 */
#define CONFIG_GICV3
#define GIC_BASE	0x50800000
#define GICD_BASE	GIC_BASE
#define GICR_BASE	(GIC_BASE + 0x80000)
#endif

#ifdef CONFIG_RUN_FROM_DDR0
#define DDR_BASE_ADDR		0x80000000
#else
#define DDR_BASE_ADDR		0xA0000000
#endif

#define CONFIG_SYS_TEXT_OFFSET      0x00020000

#define IRAM_BASE_ADDR  CONFIG_SYS_DATA_BASE
#define IRAM_SIZE		CONFIG_SYS_MEM_SIZE

#ifndef CONFIG_SYS_DDR_SIZE
#define CONFIG_SYS_DDR_SIZE 0x20000000
#endif

#define IS_ADDR_IN_DDR(addr) \
	((addr) >= (DDR_BASE_ADDR) && \
	(addr) <= (DDR_BASE_ADDR) + (CONFIG_SYS_DDR_SIZE))

#if defined(CONFIG_SPI_FLASH) && defined(CONFIG_FSL_QSPI)
#define CONFIG_SYS_FSL_QSPI_AHB

#undef FSL_QSPI_FLASH_SIZE
#define FSL_QSPI_FLASH_SIZE            SZ_64M
#endif



#endif
