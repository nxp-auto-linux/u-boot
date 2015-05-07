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

/* Clock Generation Module (CGM) */
struct mc_cgm_reg {
	u32 reserved_0x0[8];
	u32 armpll_pllcr;		/* ARM PLL Control Register						 	*/
	u32 armpll_pllsr;		/* ARM PLL Status Register						 	*/
	u32 armpll_plldv;		/* ARM Divider Register							 	*/
	u32 armpll_pllfm;		/* ARM PLL Frequency Modulation Register		 	*/
	u32 armpll_pllfd;		/* ARM PLL Fractional Divide Register			 	*/
	u32 reserved_0x34[1];
	u32 perpll_pllcal1;		/* Peripheral PLL Calibration Register 1		 	*/
	u32 reserved_0x3c[25];
	u32 perpll_pllcr;		/* Peripheral PLL Control Register				 	*/
	u32 perpll_pllsr;		/* Peripheral PLL Status Register				 	*/
	u32 perpll_plldv;		/* Peripheral PLL Divider Register				 	*/
	u32 perpll_pllfm;		/* Peripheral PLL Frequency Modulation Register	 	*/
	u32 perpll_pllfd;		/* Peripheral PLL Fractional Divide Register	 	*/
	u32 reserved_0xb4[1];
	u32 perpll_pllcal3;		/* Peripheral PLL Calibration Register 1		 	*/
	u32 reserved_0xbc[25];
	u32 enetpll_pllcr;		/* ENET PLL Control Register					 	*/
	u32 enetpll_pllsr;		/* ENET PLL Status Register						 	*/
	u32 enetpll_plldv;		/* ENET PLL Divider Register					 	*/
	u32 enetpll_pllfm;		/* ENET PLL Frequency Modulation Register		 	*/
	u32 enetpll_pllfd;		/* ENET PLL Fractional Divide Register			 	*/
	u32 reserved_0x134[1];
	u32 enetpll_pllcal1;	/* ENET PLL Calibration Register 1					*/
	u32 reserved_0x13c[25];
	u32 ddrpll_pllcr;		/* DDR PLL Control Register							*/
	u32 ddrpll_pllsr;		/* DDR PLL Status Register							*/
	u32 ddrtpll_plldv;		/* DDR PLL Divider Register							*/
	u32 ddrtpll_pllfm;		/* DDR PLL Frequency Modulation Register		 	*/
	u32 ddrtpll_pllfd;		/* DDR PLL Fractional Divide Register			 	*/
	u32 reserved_0x1b4[1];
	u32 ddrpll_pllcal1;		/* DDR PLL Calibration Register 1				 	*/
	u32 reserved_0x1bc[25];
	u32 videopll_pllcr;		/* VIDEO PLL Control Register					  	*/
	u32 videopll_pllsr;		/* VIDEO PLL Status Register					  	*/
	u32 videopll_plldv;		/* VIDEO PLL Divider Register					  	*/
	u32 videopll_pllfm;		/* VIDEO PLL Frequency Modulation Register		  	*/
	u32 videopll_pllfd;		/* VIDEO PLL Fractional Divide Register				*/
	u32 reserved_0x234[1];
	u32 videopll_pllcal3;	/* VIDEO PLL Calibration Register 1					*/
	u32 reserved_0x23c[305];
	u8	mc_cgm_pcs_sdur;	 /* PCS Switch Duration Register					*/
	u8	reserved_0x701[3];
	u32 mc_cgm_pcs_divc1;	 /* PCS Divider Change Register 1					*/
	u32 mc_cgm_pcs_dive1;	 /* PCS Divider End Register 1						*/
	u32 mc_cgm_pcs_divs1;	 /* PCS Divider Start Register 1					*/
	u32 mc_cgm_pcs_divc2;	 /* PCS Divider Change Register 2					*/
	u32 mc_cgm_pcs_dive2;	 /* PCS Divider End Register 2						*/
	u32 mc_cgm_pcs_divs2;	 /* PCS Divider Start Register 2					*/
	u32 reserved_0x71c[46];
	u32 mc_cgm_div_upd_type; /* Divider Update Type								*/
	u32 mc_cgm_div_upd_trig; /* Divider Update Trigger							*/
	u32 mc_cgm_div_upd_stat; /* Divider Update Status							*/
	u32 reserved_0x7e0[1];
	u32 mc_cgm_sc_ss;		 /* System Clock Select Status Register				*/
	u32 mc_cgm_sc_dc0;		 /* System Clock Divider 0 Configuration Register	*/
	u32 mc_cgm_sc_dc1;		 /* System Clock Divider 1 Configuration Register	*/
	u32 mc_cgm_sc_dc2;		 /* System Clock Divider 2 Configuration Register	*/
	u32 reserved_0x7f4[3];
	u32 mc_cgm_ac0_sc;		 /* Auxiliary Clock 0 Select Control Register		*/
	u32 mc_cgm_ac0_ss;		 /* Auxiliary Clock 0 Select Status Register		*/
	u32 mc_cgm_ac0_dc0;		  /* Auxiliary Clock 0 Select Status Register		*/
	u32 reserved_0x80c[5];
	u32 mc_cgm_ac1_sc;		 /* Auxiliary Clock 1 Select Control Register			*/
	u32 mc_cgm_ac1_ss;		 /* Auxiliary Clock 1 Select Status Register			*/
	u32 mc_cgm_ac1_dc0;		 /* Auxiliary Clock 1 Divider 0 Configuration Register	*/
	u32 reserved_0x82c[5];
	u32 mc_cgm_ac2_sc;		 /* Auxiliary Clock 2 Select Control Register			*/
	u32 mc_cgm_ac2_ss;		 /* Auxiliary Clock 2 Select Status Register			*/
	u32 mc_cgm_ac2_dc0;		 /* Auxiliary Clock 2 Divider 0 Configuration Register	*/
	u32 reserved_0x84c[5];
	u32 mc_cgm_ac3_sc;		 /* Auxiliary Clock 3 Select Control Register			*/
	u32 mc_cgm_ac3_ss;		 /* Auxiliary Clock 3 Select Status Register			*/
	u32 mc_cgm_ac3_dc0;		 /* Auxiliary Clock 3 Divider 0 Configuration Register	*/
	u32 reserved_0x86c[5];
	u32 mc_cgm_ac4_sc;		 /* Auxiliary Clock 4 Select Control Register			*/
	u32 mc_cgm_ac4_ss;		 /* Auxiliary Clock 4 Select Status Register			*/
	u32 mc_cgm_ac4_dc0;		 /* Auxiliary Clock 4 Divider 0 Configuration Register	*/
	u32 reserved_0x88C[5];
	u32 mc_cgm_ac5_sc;		 /* Auxiliary Clock 5 Select Control Register			*/
	u32 mc_cgm_ac5_ss;		 /* Auxiliary Clock 5 Select Status Register			*/
	u32 mc_cgm_ac5_dc0;		 /* Auxiliary Clock 5 Divider 0 Configuration Register	*/
	u32 mc_cgm_ac5_dc1;		 /* Auxiliary Clock 5 Divider 1 Configuration Register	*/
	u32 reserved_0x8b0[4];
	u32 mc_cgm_ac6_sc;		 /* Auxiliary Clock 6 Select Control Register			*/
	u32 mc_cgm_ac6_ss;		 /* Auxiliary Clock 6 Select Status Register			*/
	u32 mc_cgm_ac6_dc0;		 /* Auxiliary Clock 6 Divider 0 Configuration Register	*/
	u32 mc_cgm_ac6_dc1;		 /* Auxiliary Clock 6 Divider 1 Configuration Register	*/
	u32 reserved_0x0x8cc[5];
	u32 mc_cgm_ac7_sc;		 /* Auxiliary Clock 7 Select Control Register			*/
	u32 mc_cgm_ac7_ss;		 /* Auxiliary Clock 7 Select Status Register			*/
	u32 reserved_0x8e8[1];
	u32 mc_cgm_ac7_dc1;		 /* Auxiliary Clock 7 Divider 1 Configuration Register	*/
	u32 reserved_0x8f0[4];
	u32 mc_cgm_ac8_sc;		 /* Auxiliary Clock 8 Select Control Register			*/
	u32 mc_cgm_ac8_ss;		 /* Auxiliary Clock 8 Select Status Register			*/
	u32 mc_cgm_ac8_dc0;		 /* Auxiliary Clock 8 Divider 0 Configuration Register	*/
	u32 mc_cgm_ac8_dc1;		 /* Auxiliary Clock 8 Divider 1 Configuration Register	*/
	u32 reserved_0x910[4];
	u32 mc_cgm_ac9_sc;		 /* Auxiliary Clock 9 Select Control Register			*/
	u32 mc_cgm_ac9_ss;		 /* Auxiliary Clock 9 Select Status Register			*/
	u32 mc_cgm_ac9_dc0;		 /* Auxiliary Clock 9 Divider 0 Configuration Register	*/
	u32 mc_cgm_ac9_dc1;		 /* Auxiliary Clock 9 Divider 1 Configuration Register	*/
	u32 reserved_0x930[4];
	u32 mc_cgm_ac10_sc;		 /* Auxiliary Clock 10 Select Control Register			*/
	u32 mc_cgm_ac10_ss;		 /* Auxiliary Clock 10 Select Status Register			*/
	u32 mc_cgm_ac10_dc0;	 /* Auxiliary Clock 10 Divider 0 Configuration Register */
	u32 mc_cgm_ac10_dc1;	 /* Auxiliary Clock 10 Divider 1 Configuration Register */
	u32 reserved_0x950[4];
	u32 mc_cgm_ac11_sc;		 /* Auxiliary Clock 11 Select Control Register			*/
	u32 mc_cgm_ac11_ss;		 /* Auxiliary Clock 11 Select Status Register			*/
	u32 mc_cgm_ac11_dc0;	 /* Auxiliary Clock 11 Divider 0 Configuration Register */
	u32 reserved_0x96c[5];
	u32 mc_cgm_ac12_sc;		 /* Auxiliary Clock 12 Select Control Register			*/
	u32 mc_cgm_ac12_ss;		 /* Auxiliary Clock 12 Select Status Register			*/
	u32 mc_cgm_ac12_dc0;	 /* Auxiliary Clock 12 Divider 0 Configuration Register */
	u32 reserved_0x98c[5];
	u32 mc_cgm_ac13_sc;		 /* Auxiliary Clock 13 Select Control Register			*/
	u32 mc_cgm_ac13_ss;		 /* Auxiliary Clock 13 Select Status Register			*/
	u32 mc_cgm_ac13_dc0;	 /* Auxiliary Clock 13 Divider 0 Configuration Register */
	u32 reserved_0x9ac[5];
	u32 mc_cgm_ac14_sc;		 /* Auxiliary Clock 14 Select Control Register			*/
	u32 mc_cgm_ac14_ss;		 /* Auxiliary Clock 14 Select Status Register			*/
	u32 mc_cgm_ac14_dc0;	 /* Auxiliary Clock 14 Divider 0 Configuration Register */
	u32 reserved_0x9cc[5];
	u32 mc_cgm_ac15_sc;		 /* Auxiliary Clock 15 Select Control Register			*/
	u32 mc_cgm_ac15_ss;		 /* Auxiliary Clock 15 Select Status Register			*/
	u32 mc_cgm_ac15_dc0;	 /* Auxiliary Clock 15 Divider 0 Configuration Register */
};

/* MC_CGM registers definitions */

/* MC_CGM_SC_DCm */
#define MC_CGM_SC_DCn_PREDIV(val)	(MC_CGM_SC_DCn_PREDIV_MASK & ((val) << MC_CGM_SC_DCn_PREDIV_OFFSET))
#define MC_CGM_SC_DCn_PREDIV_MASK	(0x00070000)
#define MC_CGM_SC_DCn_PREDIV_OFFSET (16)
#define MC_CGM_SC_DCn_DE			(1 << 31)
#define MC_CGM_SC_SEL_MASK			(0x0F000000)
#define MC_CGM_SC_SEL_OFFSET		(24)


/* MC_CGM_ACn_DCn */
#define MC_CGM_ACn_DCn_PREDIV(val)	  (MC_CGM_ACn_DCn_PREDIV_MASK & ((val) << MC_CGM_ACn_DCn_PREDIV_OFFSET))

/*
 * MC_CGM_ACn_DCn_PREDIV_MASK is on 5 bits because practical test has shown
 * that the 5th bit is always ignored during writes if the current
 * MC_CGM_ACn_DCn_PREDIV field has only 4 bits
 *
 * The manual states only selectors 1, 5 and 15 have DC0_PREDIV on 5 bits
 *
 * This should be changed if any problems occur.
 */
#define MC_CGM_ACn_DCn_PREDIV_MASK		(0x001F0000)
#define MC_CGM_ACn_DCn_PREDIV_OFFSET	(16)
#define MC_CGM_ACn_DCn_DE				(1 << 31)

/*
 *	MC_CGM_ACn_SC/MC_CGM_ACn_SS
*/
#define MC_CGM_ACn_SEL_MASK				(0x07000000)
#define MC_CGM_ACn_SEL_SET(source)		(MC_CGM_ACn_SEL_MASK & (((source) & 0x7) << MC_CGM_ACn_SEL_OFFSET))
#define MC_CGM_ACn_SEL_OFFSET			(24)
#define MC_CGM_ACn_SEL_FIRC				(0x0)
#define MC_CGM_ACn_SEL_FXOSC			(0x1)
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
#define PLLDIG_PLLDV_MFD(div)			(0x000000FF & (div))

/*	PLLDIG_PLLDV_RFDPHIB has a different format for /32 according to 
	the reference manual. This other value respect the formula 2^[RFDPHIBY+1]
*/
#define PLLDIG_PLLDV_RFDPHIBY_32		(0x4)
#define PLLDIG_PLLDV_RFDPHI_SET(val)	(PLLDIG_PLLDV_RFDPHI_MASK & (((val) & PLLDIG_PLLDV_RFDPHI_MAXVALUE) << PLLDIG_PLLDV_RFDPHI_OFFSET))
#define PLLDIG_PLLDV_RFDPHI_MASK		(0x003F0000)
#define PLLDIG_PLLDV_RFDPHI_MAXVALUE	(0x7)
#define PLLDIG_PLLDV_RFDPHI_OFFSET		(16)


#define PLLDIG_PLLDV_MFD_MASK			(0x000000FF)

#define PLLDIG_PLLDV_PREDIV_SET(val)	(PLLDIG_PLLDV_PREDIV_MASK & (((val) & PLLDIG_PLLDV_PREDIV_MAXVALUE) << PLLDIG_PLLDV_PREDIV_OFFSET))
#define PLLDIG_PLLDV_PREDIV_MASK		(0x00007000)
#define PLLDIG_PLLDV_PREDIV_MAXVALUE	(0x7)
#define PLLDIG_PLLDV_PREDIV_OFFSET		(12)


/* PLLDIG PLL Fractional  Divide Register (PLLDIG_PLLFD) */
#define PLLDIG_PLLFD_MFN_SET(val)		(PLLDIG_PLLFD_MFN_MASK & (val))
#define PLLDIG_PLLFD_MFN_MASK			(0x00007FFF)

#endif
/* Frequencies are in MHz */
#define FIRC_CLK_FREQ					16000
#define FXOSC_CLK_FREQ					40000

#endif /*__ARCH_ARM_MACH_S32V234_MCCGM_REGS_H__ */
