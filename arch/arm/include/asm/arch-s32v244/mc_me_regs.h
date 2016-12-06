/*
 * (C) Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MACH_S32V234_MCME_REGS_H__
#define __ARCH_ARM_MACH_S32V234_MCME_REGS_H__

#ifndef __ASSEMBLY__

/* MC_ME registers definitions */

/* MC_ME_GS */
#define MC_ME_GS						(MC_ME_BASE_ADDR + 0x00000000)

#define MC_ME_GS_S_SYSCLK_FIRC			(0x0 << 0)
#define MC_ME_GS_S_SYSCLK_FXOSC			(0x1 << 0)
#define MC_ME_GS_S_SYSCLK_ARMPLL		(0x2 << 0)
#define MC_ME_GS_S_STSCLK_DISABLE		(0xF << 0)
#define MC_ME_GS_S_FIRC					(1 << 4)
#define MC_ME_GS_S_XOSC					(1 << 5)
#define MC_ME_GS_S_ARMPLL				(1 << 6)
#define MC_ME_GS_S_PERPLL				(1 << 7)
#define MC_ME_GS_S_ENETPLL				(1 << 8)
#define MC_ME_GS_S_DDRPLL				(1 << 9)
#define MC_ME_GS_S_VIDEOPLL				(1 << 10)
#define MC_ME_GS_S_MVR					(1 << 20)
#define MC_ME_GS_S_PDO					(1 << 23)
#define MC_ME_GS_S_MTRANS				(1 << 27)
#define MC_ME_GS_S_CRT_MODE_RESET		(0x0 << 28)
#define MC_ME_GS_S_CRT_MODE_TEST		(0x1 << 28)
#define MC_ME_GS_S_CRT_MODE_DRUN		(0x3 << 28)
#define MC_ME_GS_S_CRT_MODE_RUN0		(0x4 << 28)
#define MC_ME_GS_S_CRT_MODE_RUN1		(0x5 << 28)
#define MC_ME_GS_S_CRT_MODE_RUN2		(0x6 << 28)
#define MC_ME_GS_S_CRT_MODE_RUN3		(0x7 << 28)

/* MC_ME_MCTL */
#define MC_ME_MCTL						(MC_ME_BASE_ADDR + 0x00000004)

#define MC_ME_MCTL_KEY					(0x00005AF0)
#define MC_ME_MCTL_INVERTEDKEY				(0x0000A50F)
#define MC_ME_MCTL_RESET				(0x0 << 28)
#define MC_ME_MCTL_TEST					(0x1 << 28)
#define MC_ME_MCTL_DRUN					(0x3 << 28)
#define MC_ME_MCTL_RUN0					(0x4 << 28)
#define MC_ME_MCTL_RUN1					(0x5 << 28)
#define MC_ME_MCTL_RUN2					(0x6 << 28)
#define MC_ME_MCTL_RUN3					(0x7 << 28)


/* MC_ME_ME */
#define MC_ME_ME						(MC_ME_BASE_ADDR + 0x00000008)

#define MC_ME_ME_RESET_FUNC				(1 << 0)
#define MC_ME_ME_TEST					(1 << 1)
#define MC_ME_ME_DRUN					(1 << 3)
#define MC_ME_ME_RUN0					(1 << 4)
#define MC_ME_ME_RUN1					(1 << 5)
#define MC_ME_ME_RUN2					(1 << 6)
#define MC_ME_ME_RUN3					(1 << 7)

/* MC_ME_RUN_PCn */
#define MC_ME_RUN_PCn(n)				(MC_ME_BASE_ADDR + 0x00000080 + 0x4 * (n))

#define MC_ME_RUN_PCn_RESET				(1 << 0)
#define MC_ME_RUN_PCn_TEST				(1 << 1)
#define MC_ME_RUN_PCn_DRUN				(1 << 3)
#define MC_ME_RUN_PCn_RUN0				(1 << 4)
#define MC_ME_RUN_PCn_RUN1				(1 << 5)
#define MC_ME_RUN_PCn_RUN2				(1 << 6)
#define MC_ME_RUN_PCn_RUN3				(1 << 7)

/*
 * MC_ME_RESET_MC/MC_ME_TEST_MC
 * MC_ME_DRUN_MC
 * MC_ME_RUNn_MC
 */
#define MC_ME_RESET_MC						(MC_ME_BASE_ADDR + 0x00000020)
#define MC_ME_TEST_MC						(MC_ME_BASE_ADDR + 0x00000024)
#define MC_ME_DRUN_MC						(MC_ME_BASE_ADDR + 0x0000002C)
#define MC_ME_RUNn_MC(n)					(MC_ME_BASE_ADDR + 0x00000030 + 0x4 * (n))

#define MC_ME_RUNMODE_MC_SYSCLK(val)	(MC_ME_RUNMODE_MC_SYSCLK_MASK & (val))
#define MC_ME_RUNMODE_MC_SYSCLK_MASK	(0x0000000F)
#define MC_ME_RUNMODE_MC_FIRCON			(1 << 4)
#define MC_ME_RUNMODE_MC_XOSCON			(1 << 5)
#define MC_ME_RUNMODE_MC_PLL(pll)		(1 << (6 + (pll)))
#define MC_ME_RUNMODE_MC_MVRON			(1 << 20)
#define MC_ME_RUNMODE_MC_PDO			(1 << 23)
#define MC_ME_RUNMODE_MC_PWRLVL0		(1 << 28)
#define MC_ME_RUNMODE_MC_PWRLVL1		(1 << 29)
#define MC_ME_RUNMODE_MC_PWRLVL2		(1 << 30)

/* MC_ME_DRUN_SEC_CC_I */
#define MC_ME_DRUN_SEC_CC_I					(MC_ME_BASE_ADDR + 0x260)
/* MC_ME_RUNn_SEC_CC_I */
#define MC_ME_RUNn_SEC_CC_I(n)				(MC_ME_BASE_ADDR + 0x270 + (n) * 0x10)
#define MC_ME_RUNMODE_SEC_CC_I_SYSCLK(val,offset)	((MC_ME_RUNMODE_SEC_CC_I_SYSCLK_MASK & (val)) << offset)
#define MC_ME_RUNMODE_SEC_CC_I_SYSCLK1_OFFSET	(4)
#define MC_ME_RUNMODE_SEC_CC_I_SYSCLK2_OFFSET	(8)
#define MC_ME_RUNMODE_SEC_CC_I_SYSCLK3_OFFSET	(12)
#define MC_ME_RUNMODE_SEC_CC_I_SYSCLK_MASK		(0x3)

/*
 * ME_PCTLn
 * Please note that these registers are 8 bits width, so
 * the operations over them should be done using 8 bits operations.
 */
#define MC_ME_PCTLn_RUNPCm(n)			( (n) & MC_ME_PCTLn_RUNPCm_MASK )
#define MC_ME_PCTLn_RUNPCm_MASK			(0x7)

/* MC_ME MIPI-CSI0 */
#define MC_ME_PCTL15		(MC_ME_BASE_ADDR + 0xCC)
/* MC_ME 2D-ACE */
#define MC_ME_PCTL14		(MC_ME_BASE_ADDR + 0xCD)
/* MC_ME USDHC */
#define MC_ME_PCTL18		(MC_ME_BASE_ADDR + 0xD1)
/* MC_ME ENET0 */
#define MC_ME_PCTL17		(MC_ME_BASE_ADDR + 0xD2)
/* MC_ME DMAMUX0 */
#define MC_ME_PCTL16		(MC_ME_BASE_ADDR + 0xD3)
/* MC_ME FLEXCAN0 */
#define MC_ME_PCTL23		(MC_ME_BASE_ADDR + 0xD4)
/* MC_ME PIT0 */
#define MC_ME_PCTL22		(MC_ME_BASE_ADDR + 0xD5)
/* MC_ME DRAM Controller 0 */
#define MC_ME_PCTL21		(MC_ME_BASE_ADDR + 0xD6)
/* MC_ME FLEXRAY */
#define MC_ME_PCTL20		(MC_ME_BASE_ADDR + 0xD7)
/* MC_ME SPI4 */
#define MC_ME_PCTL27		(MC_ME_BASE_ADDR + 0xD8)
/* MC_ME SPI0 */
#define MC_ME_PCTL26		(MC_ME_BASE_ADDR + 0xD9)
/* MC_ME I2C0 */
#define MC_ME_PCTL25		(MC_ME_BASE_ADDR + 0xDA)
/* MC_ME FLEXCAN4 */
#define MC_ME_PCTL24		(MC_ME_BASE_ADDR + 0xDB)
/* MC_ME CRC0 */
#define MC_ME_PCTL28		(MC_ME_BASE_ADDR + 0xDF)
/* MC_ME SAR-ADC */
#define MC_ME_PCTL39		(MC_ME_BASE_ADDR + 0xE4)
/* MC_ME FLEXCAN1 */
#define MC_ME_PCTL43		(MC_ME_BASE_ADDR + 0xE8)
/* MC_ME LINFLEXD0 */
#define MC_ME_PCTL42		(MC_ME_BASE_ADDR + 0xE9)
/* MC_ME I2C1 */
#define MC_ME_PCTL41		(MC_ME_BASE_ADDR + 0xEA)
/* MC_ME FTM0 FlexTIMER */
#define MC_ME_PCTL40		(MC_ME_BASE_ADDR + 0xEB)
/* MC_ME CRC1 */
#define MC_ME_PCTL45		(MC_ME_BASE_ADDR + 0xEE)
/* MC_ME SPI1 */
#define MC_ME_PCTL44		(MC_ME_BASE_ADDR + 0xEF)
/* MC_ME LFAST */
#define MC_ME_PCTL50		(MC_ME_BASE_ADDR + 0xF1)
/* MC_ME SIPI */
#define MC_ME_PCTL49		(MC_ME_BASE_ADDR + 0xF2)
/* MC_ME MIPI-CSI2 */
#define MC_ME_PCTL48		(MC_ME_BASE_ADDR + 0xF3)
/* MC_ME DRAM PHY 0 */
#define MC_ME_PCTL54		(MC_ME_BASE_ADDR + 0xF5)
/* MC_ME DRAM Controller 1 */
#define MC_ME_PCTL83		(MC_ME_BASE_ADDR + 0x110)
/* MC_ME DMAMUX1 */
#define MC_ME_PCTL82		(MC_ME_BASE_ADDR + 0x111)
/* MC_ME MIPI-CSI1 */
#define MC_ME_PCTL81		(MC_ME_BASE_ADDR + 0x112)
/* MC_ME FTM1 FlexTIMER */
#define MC_ME_PCTL87		(MC_ME_BASE_ADDR + 0x114)
/* MC_ME PIT1 */
#define MC_ME_PCTL85		(MC_ME_BASE_ADDR + 0x116)
/* MC_ME QUADSPI */
#define MC_ME_PCTL84		(MC_ME_BASE_ADDR + 0x117)
/* MC_ME FLEXCAN2 */
#define MC_ME_PCTL91		(MC_ME_BASE_ADDR + 0x118)
/* MC_ME SIUL2 */
#define MC_ME_PCTL90		(MC_ME_BASE_ADDR + 0x119)
/* MC_ME LINFLEXD1 */
#define MC_ME_PCTL89		(MC_ME_BASE_ADDR + 0x11A)
/* MC_ME I2C2 */
#define MC_ME_PCTL88		(MC_ME_BASE_ADDR + 0x11B)
/* MC_ME SPI2 */
#define MC_ME_PCTL92		(MC_ME_BASE_ADDR + 0x11F)
/* MC_ME LG PF */
#define MC_ME_PCTL107		(MC_ME_BASE_ADDR + 0x128)
/* MC_ME LG KLT */
#define MC_ME_PCTL106		(MC_ME_BASE_ADDR + 0x129)
/* MC_ME TMU Temperature sensor */
#define MC_ME_PCTL105		(MC_ME_BASE_ADDR + 0x12A)
/* MC_ME H264_ENC video encoder */
#define MC_ME_PCTL109		(MC_ME_BASE_ADDR + 0x12E)
/* MC_ME LG SGM */
#define MC_ME_PCTL108		(MC_ME_BASE_ADDR + 0x12F)
/* MC_ME MIPI-CSI3 */
#define MC_ME_PCTL115		(MC_ME_BASE_ADDR + 0x130)
/* MC_ME MBIST controller, MCT and REP */
#define MC_ME_PCTL114		(MC_ME_BASE_ADDR + 0x131)
/* MC_ME ENET1 */
#define MC_ME_PCTL113		(MC_ME_BASE_ADDR + 0x132)
/* MC_ME SPI3 */
#define MC_ME_PCTL119		(MC_ME_BASE_ADDR + 0x134)
/* MC_ME I2C3 */
#define MC_ME_PCTL118		(MC_ME_BASE_ADDR + 0x135)
/* MC_ME FLEXCAN3 */
#define MC_ME_PCTL117		(MC_ME_BASE_ADDR + 0x136)
/* MC_ME DRAM PHY 1 */
#define MC_ME_PCTL116		(MC_ME_BASE_ADDR + 0x137)
/* Core Status */
#define MC_ME_CS		(MC_ME_BASE_ADDR + 0x1C0)
/* Cortex-A53_CORE0 */
#define MC_ME_CCTL1		(MC_ME_BASE_ADDR + 0x1C4)
/* Cortex-M7 Core */
#define MC_ME_CCTL0		(MC_ME_BASE_ADDR + 0x1C6)
/* Cortex-A53_CORE2 */
#define MC_ME_CCTL3		(MC_ME_BASE_ADDR + 0x1C8)
/* Cortex-A53_CORE1 */
#define MC_ME_CCTL2		(MC_ME_BASE_ADDR + 0x1CA)
/* Cortex-A53_CORE3 */
#define MC_ME_CCTL4		(MC_ME_BASE_ADDR + 0x1CE)
/* Cortex-M7 Core Address */
#define MC_ME_CADDR0		(MC_ME_BASE_ADDR + 0x1E0)
/* Cortex-A53_CORE0 Core Address */
#define MC_ME_CADDR1		(MC_ME_BASE_ADDR + 0x1E4)
/* Cortex-A53_CORE1 Core Address */
#define MC_ME_CADDR2		(MC_ME_BASE_ADDR + 0x1E8)
/* Cortex-A53_CORE2 Core Address */
#define MC_ME_CADDR3		(MC_ME_BASE_ADDR + 0x1EC)
/* Cortex-A53_CORE3 Core Address */
#define MC_ME_CADDR4		(MC_ME_BASE_ADDR + 0x1F0)
/* DRUN Secondary Clock Configuration */
#define MC_ME_DRUN_SEC_CC_I	(MC_ME_BASE_ADDR + 0x260)
/* DRUN Secondary Clock Configuration II */
#define MC_ME_DRUN_SEC_CC_II	(MC_ME_BASE_ADDR + 0x264)
/* RUN0 Secondary Clock Configuration I */
#define MC_ME_RUN0_SEC_CC_I	(MC_ME_BASE_ADDR + 0x270)
/* RUN0 Secondary Clock Configuration II */
#define MC_ME_RUN0_SEC_CC_II	(MC_ME_BASE_ADDR + 0x274)
/* RUN1 Secondary Clock Configuration I */
#define MC_ME_RUN1_SEC_CC_I	(MC_ME_BASE_ADDR + 0x280)
/* RUN1 Secondary Clock Configuration II */
#define MC_ME_RUN1_SEC_CC_II	(MC_ME_BASE_ADDR + 0x284)
/* RUN2 Secondary Clock Configuration I */
#define MC_ME_RUN2_SEC_CC_I	(MC_ME_BASE_ADDR + 0x290)
/* RUN1 Secondary Clock Configuration II */
#define MC_ME_RUN2_SEC_CC_II	(MC_ME_BASE_ADDR + 0x294)
/* RUN3 Secondary Clock Configuration I */
#define MC_ME_RUN3_SEC_CC_I	(MC_ME_BASE_ADDR + 0x2A0)
/* RUN3 Secondary Clock Configuration II */
#define MC_ME_RUN3_SEC_CC_II	(MC_ME_BASE_ADDR + 0x2A4)
/* Secondary Clock Status */
#define MC_ME_SEC_CS	(MC_ME_BASE_ADDR + 0x2D0)


/* Cortex-A53 cores */
#define MC_ME_CCTL_DEASSERT_CORE       (0xFA)

#define MC_ME_CADDRn_ADDR_EN	(1 << 0)

#endif

#endif /*__ARCH_ARM_MACH_S32V234_MCME_REGS_H__ */
