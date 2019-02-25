/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017-2019 NXP
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

#define VIRTUAL_PLATFORM
#define CONFIG_S32_GEN1

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

#ifdef VIRTUAL_PLATFORM
/* The Phy is emulated */
#define CONFIG_PHY_RGMII_DIRECT_CONNECTED
#endif

/* CONFIG_PHY_RGMII_DIRECT_CONNECTED should be enabled when
 * BCM switch is configured.
 */
#ifdef CONFIG_PHY_RGMII_DIRECT_CONNECTED
#define CONFIG_BCM_SPEED	SPEED_1000
#else
#define CONFIG_FEC_MXC_PHYADDR  7
#endif

#ifdef CONFIG_PHY_RGMII_DIRECT_CONNECTED
	#define FDT_FILE s32v234-evbbcm.dtb
#else
	#define	FDT_FILE s32v234-evb.dtb
#endif

#define CONFIG_LOADADDR		LOADADDR


#define CONFIG_SYS_INIT_SP_OFFSET \
	(CONFIG_SYS_INIT_RAM_SIZE - GENERATED_GBL_DATA_SIZE - CONFIG_SYS_TEXT_OFFSET)

/* #define CONFIG_CMD_PCI */
#ifdef CONFIG_CMD_PCI
#define CONFIG_GICSUPPORT
#define CONFIG_USE_IRQ
#define CONFIG_CMD_IRQ
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
#define QSPI_BASE_ADDR		        0x4032D000
/* Flash related definitions */
#define CONFIG_S32_GEN_1_USES_FLASH

/* we include this file here because it depends on the above definitions */
#include <configs/s32.h>

#define IMX_FEC_BASE            ENET0_BASE_ADDR

#if defined(CONFIG_RUN_AT_EL3)
/* In secure boot scenarios such as on S32G275, the Trusted Firmware runs at
 * EL3, while U-Boot runs at EL2. This produces errors while U-Boot attempts to
 * configure the secure GIC registers. As a result, GICv3 initialization on S32G
 * is done by the Trusted Firmware - or we keep running U-Boot at EL3.
 */
#define CONFIG_GICV3
#define GIC_BASE	0x50800000
#define GICD_BASE	GIC_BASE
#define GICR_BASE	(GIC_BASE + 0x80000)
#endif

#ifdef VIRTUAL_PLATFORM
#ifdef CONFIG_SYS_FSL_ERRATUM_A008585
#undef CONFIG_SYS_FSL_ERRATUM_A008585
#endif
#endif

#ifdef CONFIG_RUN_FROM_DDR0
#define DDR_BASE_ADDR		0x80000000
#else
#define DDR_BASE_ADDR		0xA0000000
#endif

#define CONFIG_SYS_TEXT_BASE        0x38020000 /* SDRAM */
#define CONFIG_SYS_TEXT_OFFSET      0x00020000

#endif
