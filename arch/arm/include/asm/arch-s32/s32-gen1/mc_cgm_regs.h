/*
 * (C) Copyright 2018-2020 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MACH_S32GEN1_MCCGM_REGS_H__
#define __ARCH_ARM_MACH_S32GEN1_MCCGM_REGS_H__

#include <config.h>
#include "s32-gen1-regs.h"

#ifndef __ASSEMBLY__

/* FXOSC registers. */
#define FXOSC_CTRL(FXOSC)		(UPTR(FXOSC) + 0x0)
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

#define FXOSC_STAT(FXOSC)		(UPTR(FXOSC) + 0x4)
#define FXOSC_STAT_OSC_STAT		(1 << 31)

/* MC_CGM registers definitions */
/* MC_CGM_MUX_n_CSC */
#define CGM_MUXn_CSC(cgm_addr, mux)	((UPTR(cgm_addr) + 0x300 + \
					 (mux) * 0x40))
#define MC_CGM_MUXn_CSC_SELCTL(val)	(MC_CGM_MUXn_CSC_SELCTL_MASK & ((val) \
					 << MC_CGM_MUXn_CSC_SELCTL_OFFSET))
#define MC_CGM_MUXn_CSC_SELCTL_MASK	(0x3F000000)
#define MC_CGM_MUXn_CSC_SELCTL_OFFSET	(24)

#define MC_CGM_MUXn_CSC_CLK_SW		(1 << 2)

/* MC_CGM_MUX_n_CSS */
#define CGM_MUXn_CSS(cgm_addr, mux)	((UPTR(cgm_addr) + 0x304 + \
					 (mux) * 0x40))
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
#define CGM_MUXn_DCm(cgm_addr, mux, dc)	((UPTR(cgm_addr) + 0x308) + \
					 ((mux) * 0x40))
#define MC_CGM_MUXn_DCm_DIV(val)	(MC_CGM_MUXn_DCm_DIV_MASK & ((val) \
					 << MC_CGM_MUXn_DCm_DIV_OFFSET))
#define MC_CGM_MUXn_DCm_DIV_VAL(val)	((MC_CGM_MUXn_DCm_DIV_MASK & (val)) \
					 >> MC_CGM_MUXn_DCm_DIV_OFFSET)
#define MC_CGM_MUXn_DCm_DIV_MASK	(0x00FF0000)
#define MC_CGM_MUXn_DCm_DIV_OFFSET	(16)
#define MC_CGM_MUXn_DCm_DE		(1 << 31)
#define MC_CGM_MUXn_CSC_SEL_MASK	(0x0F000000)
#define MC_CGM_MUXn_CSC_SEL_OFFSET	(24)

/* DIV_UPD_STAT */
#define CGM_MUXn_DIV_UPD_STAT(cgm_addr, mux)	((UPTR(cgm_addr) + 0x33C + \
						 (mux) * 0x40))
#define MC_CGM_MUXn_DIV_UPD_STAT_DIVSTAT(css)	((MC_CGM_MUXn_DIV_UPD_STAT_DIVSTAT_MASK \
						  & (css)) \
						  >> MC_CGM_MUXn_DIV_UPD_STAT_DIVSTAT_OFFSET)
#define MC_CGM_MUXn_DIV_UPD_STAT_DIVSTAT_MASK	(0x00000001)
#define MC_CGM_MUXn_DIV_UPD_STAT_DIVSTAT_OFFSET	(0)

#define pll_addr(pll)			UPTR(pll)
#define dfs_addr(pll)			UPTR(pll)

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
#define PLLDIG_PLLDV_RDIV(val)		(((val) & PLLDIG_PLLDV_RDIV_MASK) >> \
					 PLLDIG_PLLDV_RDIV_OFFSET)

/* PLL Frequency Modulation (PLLFM) */
#define PLLDIG_PLLFM(pll)		((pll_addr(pll)) + 0x0000000C)
#define PLLDIG_PLLFM_SSCGBYP_OFFSET	(30)
#define PLLDIG_PLLFM_SSCGBYP_MASK	(0x40000000)
#define PLLDIG_PLLFM_SSCGBYP(val)	(((val) & PLLDIG_PLLFM_SSCGBYP_MASK) >>\
					 PLLDIG_PLLFM_SSCGBYP_OFFSET)

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
#define PLLDIG_PLLCLKMUX(pll)			(UPTR(pll) + 0x00000020)
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
#define PLLDIG_PLLODIV_DIV(val)		(((val) & PLLDIG_PLLODIV_DIV_MASK) >> \
					 PLLDIG_PLLODIV_DIV_OFFSET)

#define PLLDIG_PLLODIV_DE		(1 << 31)

/* Digital Frequency Synthesizer (DFS) */
/* According to the manual there are DFS modules for ARM_PLL, PERIPH_PLL */

/* DFS Control Register (DFS_CTL) */
#define DFS_CTL(dfs)			((dfs_addr(dfs)) + 0x00000018)
#define DFS_CTL_RESET			(1 << 1)

#define PLL2DFS(pll)			(pll_addr(pll) + 0x1C000)

/* DFS Port Status Register (DFS_PORTSR) */
#define DFS_PORTSR(dfs)			((dfs_addr(dfs)) + 0x0000000C)
/* DFS Port Reset Register (DFS_PORTRESET) */
#define DFS_PORTRESET(dfs)			((dfs_addr(dfs)) + 0x00000014)
#define DFS_PORTRESET_PORTRESET_SET(val)	\
			(((val) & DFS_PORTRESET_PORTRESET_MASK) \
			<< DFS_PORTRESET_PORTRESET_OFFSET)
#define DFS_PORTRESET_PORTRESET_MAXVAL		(0x0000003F)
#define DFS_PORTRESET_PORTRESET_MASK		(0x0000003F)
#define DFS_PORTRESET_PORTRESET_OFFSET		(0)

/* DFS Divide Register Portn (DFS_DVPORTn) */
#define DFS_DVPORTn(dfs, n)			((dfs_addr(dfs)) + \
						 (0x1C + ((n) * 0x4)))

/* Port Loss of Lock Status (PORTLOLSR) */
#define DFS_PORTOLSR(dfs)			((dfs_addr(dfs)) + 0x00000010)
#define DFS_PORTOLSR_LOL(n)			(BIT(n) & 0x3FU)

/*
 * The mathematical formula for fdfs_clockout is the following:
 * fdfs_clckout = fdfs_clkin / (2 * (DFS_DVPORTn[MFI] + (DFS_DVPORTn[MFN]/36)))
 */
#define DFS_DVPORTn_MFI_SET(val)	(DFS_DVPORTn_MFI_MASK & \
		(((val) & DFS_DVPORTn_MFI_MAXVAL) << DFS_DVPORTn_MFI_OFFSET))
#define DFS_DVPORTn_MFN_SET(val)	(DFS_DVPORTn_MFN_MASK & \
		(((val) & DFS_DVPORTn_MFN_MAXVAL) << DFS_DVPORTn_MFN_OFFSET))
#define DFS_DVPORTn_MFI(val)		(((val) & DFS_DVPORTn_MFI_MASK) >> \
					 DFS_DVPORTn_MFI_OFFSET)
#define DFS_DVPORTn_MFN(val)		(((val) & DFS_DVPORTn_MFN_MASK) >> \
					 DFS_DVPORTn_MFN_OFFSET)
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
#endif

#endif /*__ARCH_ARM_MACH_S32GEN1_MCCGM_REGS_H__ */
