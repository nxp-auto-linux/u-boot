/*
 * (C) Copyright 2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MACH_S32GEN1_MCCGM_REGS_H__
#define __ARCH_ARM_MACH_S32GEN1_MCCGM_REGS_H__

#include <config.h>
#include "s32-gen1-regs.h"

#ifndef __ASSEMBLY__

/* FXOSC registers. */
#define FXOSC_CTRL			(XOSC_BASE_ADDR)
#define FXOSC_CTRL_OSC_BYP		(1 << 31)
#define FXOSC_CTRL_COMP_EN		(1 << 24)

#define FXOSC_CTRL_EOCV(val)		(FXOSC_CTRL_EOCV_MASK & ((val) << \
					 FXOSC_CTRL_EOCV_OFFSET))
#define FXOSC_CTRL_EOCV_MASK		(0x00FF0000)
#define FXOSC_CTRL_EOCV_OFFSET		(16)

#define FXOSC_CTRL_GM_SEL(val)		(FXOSC_CTRL_GM_SEL_MASK & ((val) << \
					 FXOSC_CTRL_GM_SEL_OFFSET))
#define FXOSC_CTRL_GM_SEL_MASK		(0x000000F0)
#define FXOSC_CTRL_GM_SEL_OFFSET	(4)

#define FXOSC_CTRL_OSCON		(1 << 0)

#define FXOSC_STAT			(XOSC_BASE_ADDR + 0x4)
#define FXOSC_STAT_OSC_STAT		(1 << 31)

/* MC_CGM registers definitions */
/* MC_CGM_MUX_n_CSC */
#define CGM_MUXn_CSC(cgm_addr, mux)	(((cgm_addr) + 0x300 + (mux) * 0x40))
#define MC_CGM_MUXn_CSC_SELCTL(val)	(MC_CGM_MUXn_CSC_SELCTL_MASK & ((val) \
					 << MC_CGM_MUXn_CSC_SELCTL_OFFSET))
#define MC_CGM_MUXn_CSC_SELCTL_MASK	(0x3F000000)
#define MC_CGM_MUXn_CSC_SELCTL_OFFSET	(24)

#define MC_CGM_MUXn_CSC_CLK_SW		(1 << 2)

/* MC_CGM_MUX_n_CSS */
#define CGM_MUXn_CSS(cgm_addr, mux)	(((cgm_addr) + 0x304 + (mux) * 0x40))
#define MC_CGM_MUXn_CSS_SELSTAT(css)	((MC_CGM_MUXn_CSS_SELSTAT_MASK & (css))\
					 >> MC_CGM_MUXn_CSS_SELSTAT_OFFSET)
#define MC_CGM_MUXn_CSS_SELSTAT_MASK	(0x3F000000)
#define MC_CGM_MUXn_CSS_SELSTAT_OFFSET	(24)

#define MC_CGM_MUXn_CSS_SWIP		(1 << 16)
#define MC_CGM_MUXn_CSS_SWTRG(css)	((MC_CGM_MUXn_CSS_SWTRG_MASK & (css)) \
					 >> MC_CGM_MUXn_CSS_SWTRG_OFFSET)
#define MC_CGM_MUXn_CSS_SWTRG_MASK	(0x000E0000)
#define MC_CGM_MUXn_CSS_SWTRG_OFFSET	(17)
#define MC_CGM_MUXn_CSS_SWTRG_SUCCESS	(0x1)

/* MC_CGM_SC_DCn */
#define CGM_MUXn_DCm(cgm_addr, mux, dc)	(((cgm_addr) + 0x308) + ((mux) * 0x40))
#define MC_CGM_MUXn_DCm_DIV(val)	(MC_CGM_MUXn_DCm_DIV_MASK & ((val) \
					 << MC_CGM_MUXn_DCm_DIV_OFFSET))
#define MC_CGM_MUXn_DCm_DIV_MASK	(0x00070000)
#define MC_CGM_MUXn_DCm_DIV_OFFSET	(16)
#define MC_CGM_MUXn_DCm_DE		(1 << 31)
#define MC_CGM_MUXn_CSC_SEL_MASK	(0x0F000000)
#define MC_CGM_MUXn_CSC_SEL_OFFSET	(24)



#define pll_addr(pll)			(ARM_PLL_BASE_ADDR + (pll) * 0x4000)
#define dfs_addr(pll)			(ARM_DFS_BASE_ADDR + (pll) * 0x4000)

/* PLLDIG PLL Control Register (PLLDIG_PLLCR) */
#define PLLDIG_PLLCR(pll)		(pll_addr(pll))
#define PLLDIG_PLLCR_PLLPD		(1 << 31)

/* PLLDIG PLL Status Register (PLLDIG_PLLSR) */
#define PLLDIG_PLLSR(pll)		((pll_addr(pll)) + 0x00000004)
#define PLLDIG_PLLSR_LOCK		(1 << 2)

/* PLLDIG PLL Divider Register (PLLDIG_PLLDV) */
#define PLLDIG_PLLDV(pll)		((pll_addr(pll)) + 0x00000008)
#define PLLDIG_PLLDV_MFI(div)		(PLLDIG_PLLDV_MFI_MASK & (div))
#define PLLDIG_PLLDV_MFI_MASK		(0x000000FF)

#define PLLDIG_PLLDV_ODIV1_SET(val)	(PLLDIG_PLLDV_ODIV_MASK & \
					 (((val) & PLLDIG_PLLDV_ODIV_MAXVALUE) \
					  << PLLDIG_PLLDV_ODIV_OFFSET))
#define PLLDIG_PLLDV_ODIV1_MASK		(0x003F0000)
#define PLLDIG_PLLDV_ODIV1_MAXVALUE	(0x3F)
#define PLLDIG_PLLDV_ODIV1_OFFSET	(16)

#define PLLDIG_PLLDV_ODIV2_SET(val)	(PLLDIG_PLLDV_ODIV1_MASK & \
					 (((val) & PLLDIG_PLLDV_ODIV1_MAXVALUE)\
					  << PLLDIG_PLLDV_ODIV1_OFFSET))
#define PLLDIG_PLLDV_ODIV2_MASK		(0x7E000000)
#define PLLDIG_PLLDV_ODIV2_MAXVALUE	(0x3F)
#define PLLDIG_PLLDV_ODIV2_OFFSET	(25)

#define PLLDIG_PLLDV_RDIV_SET(val)	(PLLDIG_PLLDV_RDIV_MASK & \
					 (((val) & PLLDIG_PLLDV_RDIV_MAXVALUE) \
					  << PLLDIG_PLLDV_RDIV_OFFSET))
#define PLLDIG_PLLDV_RDIV_MASK		(0x00007000)
#define PLLDIG_PLLDV_RDIV_MAXVALUE	(0x7)
#define PLLDIG_PLLDV_RDIV_OFFSET	(12)

/* PLL Frequency Modulation (PLLFM) */
#define PLLDIG_PLLFM(pll)		((pll_addr(pll)) + 0x0000000C)

/* PLLDIG PLL Fractional  Divide Register (PLLDIG_PLLFD) */
#define PLLDIG_PLLFD(pll)		((pll_addr(pll)) + 0x00000010)
#define PLLDIG_PLLFD_MFN_SET(val)	(PLLDIG_PLLFD_MFN_MASK & (val))
#define PLLDIG_PLLFD_MFN_MASK		(0x00007FFF)
#define PLLDIG_PLLFD_SMDEN		(1 << 30)

/* PLL Calibration Register 1 (PLLDIG_PLLCAL1) */
#define PLLDIG_PLLCAL1(pll)		((pll_addr(pll)) + 0x00000014)
#define PLLDIG_PLLCAL1_NDAC1_SET(val)	(PLLDIG_PLLCAL1_NDAC1_MASK & \
					 ((val) << PLLDIG_PLLCAL1_NDAC1_OFFSET))
#define PLLDIG_PLLCAL1_NDAC1_OFFSET	(24)
#define PLLDIG_PLLCAL1_NDAC1_MASK	(0x7F000000)

/* PLL Calibration Register 2 (PLLDIG_PLLCAL2) */
#define PLLDIG_PLLCAL2(pll)		((pll_addr(pll)) + 0x00000018)

/* PLL Clock Mux (PLLCLKMUX) */
#define PLLDIG_PLLCLKMUX(pll)			((pll_addr(pll)) + 0x00000020)
#define PLLDIG_PLLCLKMUX_REFCLKSEL_SET(val)	((val) & \
						PLLDIG_PLLCLKMUX_REFCLKSEL_MASK)
#define PLLDIG_PLLCLKMUX_REFCLKSEL_MASK		(0x3)

#define PLLDIG_PLLCLKMUX_REFCLKSEL_SET_FIRC	(0x0)
#define PLLDIG_PLLCLKMUX_REFCLKSEL_SET_XOSC	(0x1)

/* PLL Output Divider (PLLODIV0 - PLLODIV7) */
#define PLLDIG_PLLODIV(pll, n)		((pll_addr(pll)) + 0x00000080 + n * 0x4)
#define PLLDIG_PLLODIV_DIV_SET(val)	(PLLDIG_PLLODIV_DIV_MASK & \
					 ((val) << PLLDIG_PLLODIV_DIV_OFFSET))
#define PLLDIG_PLLODIV_DIV_MASK		(0x00FF0000)
#define PLLDIG_PLLODIV_DIV_OFFSET	(16)

#define PLLDIG_PLLODIV_DE		(1 << 31)

/* Digital Frequency Synthesizer (DFS) */
/* According to the manual there are DFS modules for ARM_PLL, PERIPH_PLL */

/* DFS Control Register (DFS_CTL) */
#define DFS_CTL(pll)			((dfs_addr(pll)) + 0x00000018)
#define DFS_CTL_RESET			(1 << 1)

/* DFS Port Status Register (DFS_PORTSR) */
#define DFS_PORTSR(pll)			((dfs_addr(pll)) + 0x0000000C)
/* DFS Port Reset Register (DFS_PORTRESET) */
#define DFS_PORTRESET(pll)			((dfs_addr(pll)) + 0x00000014)
#define DFS_PORTRESET_PORTRESET_SET(val)	(DFS_PORTRESET_PORTRESET_MASK |\
		(((val) & DFS_PORTRESET_PORTRESET_MAXVAL) \
		 << DFS_PORTRESET_PORTRESET_OFFSET))
#define DFS_PORTRESET_PORTRESET_MAXVAL		(0x0000003F)
#define DFS_PORTRESET_PORTRESET_MASK		(0x0000003F)
#define DFS_PORTRESET_PORTRESET_OFFSET		(0)

/* DFS Divide Register Portn (DFS_DVPORTn) */
#define DFS_DVPORTn(pll, n)			((dfs_addr(pll)) + \
						 (0x1C + ((n) * 0x4)))

/*
 * The mathematical formula for fdfs_clockout is the following:
 * fdfs_clckout = fdfs_clkin / (2 * (DFS_DVPORTn[MFI] + (DFS_DVPORTn[MFN]/36)))
 */
#define DFS_DVPORTn_MFI_SET(val)	(DFS_DVPORTn_MFI_MASK & \
		(((val) & DFS_DVPORTn_MFI_MAXVAL) << DFS_DVPORTn_MFI_OFFSET))
#define DFS_DVPORTn_MFN_SET(val)	(DFS_DVPORTn_MFN_MASK & \
		(((val) & DFS_DVPORTn_MFN_MAXVAL) << DFS_DVPORTn_MFN_OFFSET))
#define DFS_DVPORTn_MFI_MASK		(0x0000FF00)
#define DFS_DVPORTn_MFN_MASK		(0x000000FF)
#define DFS_DVPORTn_MFI_MAXVAL		(0xFF)
#define DFS_DVPORTn_MFN_MAXVAL		(0xFF)
#define DFS_DVPORTn_MFI_OFFSET		(8)
#define DFS_DVPORTn_MFN_OFFSET		(0)
#define DFS_MAXNUMBER			(6)

#define DFS_PARAMS_Nr			(5)

#define PHI_MAXNUMBER			(8)

/* Frequencies are in Hz */
#define FIRC_CLK_FREQ			(48000000)
#define XOSC_CLK_FREQ			(40000000)

#define PLL_MIN_FREQ			(1300000000)
#define PLL_MAX_FREQ			(5000000000)

#define ARM_PLL_PHI0_FREQ		(1000000000)
#define ARM_PLL_PHI1_FREQ		(500000000)
#define ARM_PLL_PHI_Nr			(2)

/* ARM_PLL_DFS1_FREQ - 800 Mhz */
#define ARM_PLL_DFS1_EN			(1)
#define ARM_PLL_DFS1_MFI		(1)
#define ARM_PLL_DFS1_MFN		(9)
/* ARM_PLL_DFS2_REQ - 800 Mhz */
#define ARM_PLL_DFS2_EN		(1)
#define ARM_PLL_DFS2_MFI		(1)
#define ARM_PLL_DFS2_MFN		(9)
/* ARM_PLL_DFS3_freq - 500 mhz */
#define ARM_PLL_DFS3_EN		(1)
#define ARM_PLL_DFS3_MFI		(2)
#define ARM_PLL_DFS3_MFN		(0)
/* ARM_PLL_DFS4_freq - 300 mhz */
#define ARM_PLL_DFS4_EN		(1)
#define ARM_PLL_DFS4_MFI		(3)
#define ARM_PLL_DFS4_MFN		(12)
/* ARM_PLL_DFS5_freq - 600 mhz */
#define ARM_PLL_DFS5_EN		(1)
#define ARM_PLL_DFS5_MFI		(1)
#define ARM_PLL_DFS5_MFN		(24)
/* ARM_PLL_DFS6_freq - 600 mhz */
#define ARM_PLL_DFS6_EN		(1)
#define ARM_PLL_DFS6_MFI		(1)
#define ARM_PLL_DFS6_MFN		(24)

#define ARM_PLL_DFS_Nr			(6)
#define ARM_PLL_PLLDV_RDIV		(2)
#define ARM_PLL_PLLDV_MFI		(100)
#define ARM_PLL_PLLDV_MFN		(0)

#define PERIPH_PLL_PHI0_FREQ		(125000000)
#define PERIPH_PLL_PHI1_FREQ		(80000000)
#define PERIPH_PLL_PHI2_FREQ		(80000000)
#define PERIPH_PLL_PHI3_FREQ		(133000000)
#define PERIPH_PLL_PHI4_FREQ		(200000000)
#define PERIPH_PLL_PHI5_FREQ		(125000000)
#define PERIPH_PLL_PHI6_FREQ		(100000000)
#define PERIPH_PLL_PHI7_FREQ		(100000000)
#define PERIPH_PLL_PHI_Nr		(8)

/* PERIPH_PLL_DFS1_freq - 800 mhz */
#define PERIPH_PLL_DFS1_EN		(1)
#define PERIPH_PLL_DFS1_MFI		(1)
#define PERIPH_PLL_DFS1_MFN		(9)
/* PERIPH_PLL_DFS2_freq - 960 mhz */
#define PERIPH_PLL_DFS2_EN		(1)
#define PERIPH_PLL_DFS2_MFI		(1)
#define PERIPH_PLL_DFS2_MFN		(2)
/* PERIPH_PLL_DFS3_freq - 800 mhz */
#define PERIPH_PLL_DFS3_EN		(1)
#define PERIPH_PLL_DFS3_MFI		(1)
#define PERIPH_PLL_DFS3_MFN		(9)
/* PERIPH_PLL_DFS4_freq - 600 mhz */
#define PERIPH_PLL_DFS4_EN		(1)
#define PERIPH_PLL_DFS4_MFI		(1)
#define PERIPH_PLL_DFS4_MFN		(24)
/* PERIPH_PLL_DFS5_freq - 330 mhz */
#define PERIPH_PLL_DFS5_EN		(1)
#define PERIPH_PLL_DFS5_MFI		(3)
#define PERIPH_PLL_DFS5_MFN		(1)
/* PERIPH_PLL_DFS6_freq - 500 mhz */
#define PERIPH_PLL_DFS6_EN		(1)
#define PERIPH_PLL_DFS6_MFI		(2)
#define PERIPH_PLL_DFS6_MFN		(0)

#define PERIPH_PLL_DFS_Nr		(6)
#define PERIPH_PLL_PLLDV_RDIV		(2)
#define PERIPH_PLL_PLLDV_MFI		(100)
#define PERIPH_PLL_PLLDV_MFN		(0)

#define DDR_PLL_PHI0_FREQ		(800000000)
#define DDR_PLL_PHI_Nr			(1)
#define DDR_PLL_DFS_Nr			(0)
#define DDR_PLL_PLLDV_RDIV		(1)
#define DDR_PLL_PLLDV_MFI		(40)
#define DDR_PLL_PLLDV_MFN		(0)

#define ACCEL_PLL_PHI0_FREQ		(80000000)
#define ACCEL_PLL_PHI1_FREQ		(80000000)
#define ACCEL_PLL_PHI_Nr		(2)
#define ACCEL_PLL_DFS_Nr		(0)
#define ACCEL_PLL_PLLDV_RDIV		(1)
#define ACCEL_PLL_PLLDV_MFI		(60)
#define ACCEL_PLL_PLLDV_MFN		(0)

/* Clock source mapping on MC_CGM clock selectors. */
/* Clock source / Clock selector index */
#define MC_CGM_MUXn_CSC_SEL_FIRC			0
#define MC_CGM_MUXn_CSC_SEL_SIRC			1
#define MC_CGM_MUXn_CSC_SEL_FXOSC			2
#define MC_CGM_MUXn_CSC_SEL_SXOSC			3
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_PHI0		4
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_PHI1		5
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_PHI2		6
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_PHI3		7
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_PHI4		8
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_PHI5		9
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_PHI6		10
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_PHI7		11
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_DFS1		12
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_DFS2		13
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_DFS3		14
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_DFS4		15
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_DFS5		16
#define MC_CGM_MUXn_CSC_SEL_ARM_PLL_DFS6		17
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI0		18
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI1		19
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI2		20
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI3		21
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI4		22
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI5		23
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI6		24
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI7		25
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_DFS1		26
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_DFS2		27
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_DFS3		28
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_DFS4		29
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_DFS5		30
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_DFS6		31
#define MC_CGM_MUXn_CSC_SEL_ACCEL_PLL_PHI0		32
#define MC_CGM_MUXn_CSC_SEL_ACCEL_PLL_PHI1		33
#define MC_CGM_MUXn_CSC_SEL_FTM0_EXT_REF		34
#define MC_CGM_MUXn_CSC_SEL_FTM1_EXT_REF		35
#define MC_CGM_MUXn_CSC_SEL_DDR_PLL_PHI0		36
#define MC_CGM_MUXn_CSC_SEL_GMAC_TX_CLK			37
#define MC_CGM_MUXn_CSC_SEL_GMAC_RX_CLK			38
#define MC_CGM_MUXn_CSC_SEL_GMAC_REF_CLK		39
#define MC_CGM_MUXn_CSC_SEL_SERDES_TX_CLK		40
#define MC_CGM_MUXn_CSC_SEL_SERDES_CDR_CLK		41
#define MC_CGM_MUXn_CSC_SEL_LFAST_EXT_REF		42
#define MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI8		43
#define MC_CGM_MUXn_CSC_SEL_GMAC_TS_CLK			44
#define MC_CGM_MUXn_CSC_SEL_GMAC_0_REF_DIV_CLK		45

#endif


#endif /*__ARCH_ARM_MACH_S32GEN1_MCCGM_REGS_H__ */
