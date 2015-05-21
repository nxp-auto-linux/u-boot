/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __ARCH_ARM_MACH_S32V234_MCCGM_REGS_H__
#define __ARCH_ARM_MACH_S32V234_MCCGM_REGS_H__

#ifndef __ASSEMBLY__

/* MC_CGM registers definitions */
/* MC_CGM_SC_SS */
#define CGM_SC_SS(cgm_addr)		( ((cgm_addr) + 0x000007E4) )
#define MC_CGM_SC_SEL_FIRC				(0x0)
#define MC_CGM_SC_SEL_XOSC				(0x1)
#define MC_CGM_SC_SEL_ARMPLL			(0x2)
#define MC_CGM_SC_SEL_CLKDISABLE		(0xF)

/* MC_CGM_SC_DCn */
#define CGM_SC_DCn(cgm_addr,dc)		( ((cgm_addr) + 0x000007E8) + ((dc) * 0x4) )
#define MC_CGM_SC_DCn_PREDIV(val)	(MC_CGM_SC_DCn_PREDIV_MASK & ((val) << MC_CGM_SC_DCn_PREDIV_OFFSET))
#define MC_CGM_SC_DCn_PREDIV_MASK	(0x00070000)
#define MC_CGM_SC_DCn_PREDIV_OFFSET	(16)
#define MC_CGM_SC_DCn_DE			(1 << 31)
#define MC_CGM_SC_SEL_MASK			(0x0F000000)
#define MC_CGM_SC_SEL_OFFSET		(24)

/* MC_CGM_ACn_DCm */
#define CGM_ACn_DCm(cgm_addr,ac,dc)		( ((cgm_addr) + 0x00000808) + ((ac) * 0x20) + ((dc) * 0x4) )
#define MC_CGM_ACn_DCm_PREDIV(val)		(MC_CGM_ACn_DCm_PREDIV_MASK & ((val) << MC_CGM_ACn_DCm_PREDIV_OFFSET))

/*
 * MC_CGM_ACn_DCm_PREDIV_MASK is on 5 bits because practical test has shown
 * that the 5th bit is always ignored during writes if the current
 * MC_CGM_ACn_DCm_PREDIV field has only 4 bits
 *
 * The manual states only selectors 1, 5 and 15 have DC0_PREDIV on 5 bits
 *
 * This should be changed if any problems occur.
 */
#define MC_CGM_ACn_DCm_PREDIV_MASK		(0x001F0000)
#define MC_CGM_ACn_DCm_PREDIV_OFFSET	(16)
#define MC_CGM_ACn_DCm_DE				(1 << 31)

/*
 *	MC_CGM_ACn_SC/MC_CGM_ACn_SS
*/
#define CGM_ACn_SC(cgm_addr,ac)			((cgm_addr + 0x00000800) + ((ac) * 0x20))
#define CGM_ACn_SS(cgm_addr,ac)			((cgm_addr + 0x00000800) + ((ac) * 0x24))
#define MC_CGM_ACn_SEL_MASK				(0x07000000)
#define MC_CGM_ACn_SEL_SET(source)		(MC_CGM_ACn_SEL_MASK & (((source) & 0x7) << MC_CGM_ACn_SEL_OFFSET))
#define MC_CGM_ACn_SEL_OFFSET			(24)

#define MC_CGM_ACn_SEL_FIRC				(0x0)
#define MC_CGM_ACn_SEL_XOSC				(0x1)
#define MC_CGM_ACn_SEL_ARMPLL			(0x2)
/*
 * According to the manual some PLL can be divided by X (X={1,3,5}):
 * PERPLLDIVX, VIDEOPLLDIVX.
 */
#define MC_CGM_ACn_SEL_PERPLLDIVX		(0x3)
#define MC_CGM_ACn_SEL_ENETPLL			(0x4)
#define MC_CGM_ACn_SEL_DDRPLL			(0x5)
#define MC_CGM_ACn_SEL_EXTSRCPAD		(0x7)
#define MC_CGM_ACn_SEL_SYSCLK			(0x8)
#define MC_CGM_ACn_SEL_VIDEOPLLDIVX		(0x9)
#define MC_CGM_ACn_SEL_PERCLK			(0xA)


/* PLLDIG PLL Divider Register (PLLDIG_PLLDV) */
#define PLLDIG_PLLDV(pll)				((MC_CGM0_BASE_ADDR + 0x00000028) + ((pll) * 0x80))
#define PLLDIG_PLLDV_MFD(div)			(PLLDIG_PLLDV_MFD_MASK & (div))
#define PLLDIG_PLLDV_MFD_MASK			(0x000000FF)

/*	PLLDIG_PLLDV_RFDPHIB has a different format for /32 according to 
	the reference manual. This other value respect the formula 2^[RFDPHIBY+1]
*/
#define PLLDIG_PLLDV_RFDPHI_SET(val)	(PLLDIG_PLLDV_RFDPHI_MASK & (((val) & PLLDIG_PLLDV_RFDPHI_MAXVALUE) << PLLDIG_PLLDV_RFDPHI_OFFSET))
#define PLLDIG_PLLDV_RFDPHI_MASK		(0x003F0000)
#define PLLDIG_PLLDV_RFDPHI_MAXVALUE	(0x3F)
#define PLLDIG_PLLDV_RFDPHI_OFFSET		(16)

#define PLLDIG_PLLDV_RFDPHI1_SET(val)	(PLLDIG_PLLDV_RFDPHI1_MASK & (((val) & PLLDIG_PLLDV_RFDPHI1_MAXVALUE) << PLLDIG_PLLDV_RFDPHI1_OFFSET))
#define PLLDIG_PLLDV_RFDPHI1_MASK		(0x7E000000)
#define PLLDIG_PLLDV_RFDPHI1_MAXVALUE	(0x3F)
#define PLLDIG_PLLDV_RFDPHI1_OFFSET		(25)

#define PLLDIG_PLLDV_PREDIV_SET(val)	(PLLDIG_PLLDV_PREDIV_MASK & (((val) & PLLDIG_PLLDV_PREDIV_MAXVALUE) << PLLDIG_PLLDV_PREDIV_OFFSET))
#define PLLDIG_PLLDV_PREDIV_MASK		(0x00007000)
#define PLLDIG_PLLDV_PREDIV_MAXVALUE	(0x7)
#define PLLDIG_PLLDV_PREDIV_OFFSET		(12)


/* PLLDIG PLL Fractional  Divide Register (PLLDIG_PLLFD) */
#define PLLDIG_PLLFD(pll)				((MC_CGM0_BASE_ADDR + 0x00000030) + ((pll) * 0x80))
#define PLLDIG_PLLFD_MFN_SET(val)		(PLLDIG_PLLFD_MFN_MASK & (val))
#define PLLDIG_PLLFD_MFN_MASK			(0x00007FFF)

/* PLL Calibration Register 1 (PLLDIG_PLLCAL1) */
#define PLLDIG_PLLCAL1(pll)				((MC_CGM0_BASE_ADDR + 0x00000038) + ((pll) * 0x80))
#define PLLDIG_PLLCAL1_NDAC1_SET(val)	(PLLDIG_PLLCAL1_NDAC1_MASK & ((val) << PLLDIG_PLLCAL1_NDAC1_OFFSET))
#define PLLDIG_PLLCAL1_NDAC1_OFFSET		(24)
#define PLLDIG_PLLCAL1_NDAC1_MASK		(0x7F000000)


/* Digital Frequency Synthesizer (DFS) */
/* According to the manual there are 3 DFS module only for ARM_PLL, DDR_PLL, ENET_PLL */
#define DFS0_BASE_ADDR					(MC_CGM0_BASE_ADDR + 0x00000040)

/* DFS Control Register (DFS_CTRL) */
#define DFS_CTRL(pll)					(DFS0_BASE_ADDR + 0x00000018 + ((pll) * 0x80))
#define DFS_CTRL_DLL_LOLIE				(1 << 0)
#define DFS_CTRL_DLL_RESET				(1 << 1)

/* DFS Port Reset Register (DFS_PORTRESET) */
#define DFS_PORTRESET(pll)					(DFS0_BASE_ADDR + 0x00000014 + ((pll) * 0x80))
#define DFS_PORTRESET_PORTRESET_SET(val)	(DFS_PORTRESET_PORTRESET_MASK | (((val) & DFS_PORTRESET_PORTRESET_MAXVAL) << DFS_PORTRESET_PORTRESET_OFFSET))
#define DFS_PORTRESET_PORTRESET_MAXVAL		(0xF)
#define DFS_PORTRESET_PORTRESET_MASK		(0x0FFFFFFF)
#define DFS_PORTRESET_PORTRESET_OFFSET		(28)

/* DFS Divide Register Portn (DFS_DVPORTn) */

#define DFS_DVPORTn(pll,n)				(DFS0_BASE_ADDR + ((pll) * 0x80) + (0x0000001C + ((n) * 0x4)))
#define DFS_DVPORTn_MFI_SET(val)		(DFS_DVPORTn_MFI_MASK & (((val) & DFS_DVPORTn_MFI_MAXVAL) << DFS_DVPORTn_MFI_OFFSET) )
#define DFS_DVPORTn_MFN_SET(val)		(DFS_DVPORTn_MFN_MASK & (((val) & DFS_DVPORTn_MFN_MAXVAL) << DFS_DVPORTn_MFN_OFFSET) )
#define DFS_DVPORTn_MFI_MASK			(0x0000FF00)
#define DFS_DVPORTn_MFN_MASK			(0x000000FF)
#define DFS_DVPORTn_MFI_MAXVAL			(0xFF)
#define DFS_DVPORTn_MFN_MAXVAL			(0xFF)
#define DFS_DVPORTn_MFI_OFFSET			(8)
#define DFS_DVPORTn_MFN_OFFSET			(0)
#define DFS_MAXNUMBER					(4)

#endif
/* Frequencies are in Hz */
#define FIRC_CLK_FREQ					(48000000)
#define XOSC_CLK_FREQ					(40000000)

#define PLL_MIN_FREQ					(650000000)
#define PLL_MAX_FREQ					(1300000000)

#define ARM_PLL_PHI0_FREQ				(1000000000)
#define ARM_PLL_PHI1_FREQ				(1000000000)
#define ARM_PLL_PHI1_DFS1_FREQ			(266000000)
#define ARM_PLL_PHI1_DFS2_FREQ			(600000000)
#define ARM_PLL_PHI1_DFS3_FREQ			(600000000)
#define ARM_PLL_PHI1_DFS_Nr				(3)
#define ARM_PLL_PLLDV_PREDIV			(2)
#define ARM_PLL_PLLDV_MFD				(50)
#define ARM_PLL_PLLDV_MFN				(0)

#define PERIPH_PLL_PHI0_FREQ			(400000000)
#define PERIPH_PLL_PHI1_FREQ			(100000000)
#define PERIPH_PLL_PHI1_DFS_Nr			(0)
#define PERIPH_PLL_PLLDV_PREDIV			(1)
#define PERIPH_PLL_PLLDV_MFD			(30)
#define PERIPH_PLL_PLLDV_MFN			(0)

#define ENET_PLL_PHI0_FREQ				(500000000)
#define ENET_PLL_PHI1_FREQ				(1000000000)
#define ENET_PLL_PHI1_DFS1_FREQ			(350000000)
#define ENET_PLL_PHI1_DFS2_FREQ			(350000000)
#define ENET_PLL_PHI1_DFS3_FREQ			(400000000)
#define ENET_PLL_PHI1_DFS4_FREQ			(104000000)
#define ENET_PLL_PHI1_DFS_Nr			(4)
#define ENET_PLL_PLLDV_PREDIV			(2)
#define ENET_PLL_PLLDV_MFD				(50)
#define ENET_PLL_PLLDV_MFN				(0)

#define DDR_PLL_PHI0_FREQ				(533000000)
#define DDR_PLL_PHI1_FREQ				(1066000000)
#define DDR_PLL_PHI1_DFS1_FREQ			(500000000)
#define DDR_PLL_PHI1_DFS2_FREQ			(500000000)
#define DDR_PLL_PHI1_DFS3_FREQ			(350000000)
#define DDR_PLL_PHI1_DFS_Nr				(3)
#define DDR_PLL_PLLDV_PREDIV			(2)
#define DDR_PLL_PLLDV_MFD				(53)
#define DDR_PLL_PLLDV_MFN				(0)

#define VIDEO_PLL_PHI0_FREQ				(600000000)
#define VIDEO_PLL_PHI1_FREQ				(0)
#define VIDEO_PLL_PHI1_DFS_Nr			(0)
#define VIDEO_PLL_PLLDV_PREDIV			(1)
#define VIDEO_PLL_PLLDV_MFD				(30)
#define VIDEO_PLL_PLLDV_MFN				(0)

#endif /*__ARCH_ARM_MACH_S32V234_MCCGM_REGS_H__ */
