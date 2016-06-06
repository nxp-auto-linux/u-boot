/*
 * (C) Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MACH_MAC57D5XH_MCCGM_REGS_H__
#define __ARCH_ARM_MACH_MAC57D5XH_MCCGM_REGS_H__

#ifndef __ASSEMBLY__

/* Clock Generation Module (CGM) */
struct mc_cgm_reg {
    u32 reserved_0x0[6];
    u32 pll0_pllcal3;       /* PLL Calibration Register 3                       */
    u32 reserved_0x1c[1];
    u32 pll0_pllcr;         /* PLLDIG PLL Control Register                      */
    u32 pll0_pllsr;         /* PLLDIG PLL Status Register                       */
    u32 pll0_plldv;         /* PLLDIG PLL Divider Register                      */
    u32 pll0_pllfm;         /* PLLDIG PLL Frequency Modulation Register         */
    u32 pll0_pllfd;         /* PLLDIG PLL Fractional Divide Register            */
    u32 reserved_0x34[243];
    u32 mc_cgm_sc_dc6;      /* System Clock Divider 6 Configuration Register    */
    u32 reserved_0x404[191];
    u8  reserved_0x700[3];
    u8  mc_cgm_pcs_sdur;    /* PCS Switch Duration Register                        */
    u32 mc_cgm_pcs_divc1;   /* PCS Divider Change Register 1                       */
    u32 mc_cgm_pcs_dive1;   /* PCS Divider End Register 1                          */
    u32 mc_cgm_pcs_divs1;   /* PCS Divider Start Register 1                        */
    u32 reserved_0x710[6];
    u32 mc_cgm_pcs_divc4;   /* PCS Divider Change Register 4                       */
    u32 mc_cgm_pcs_dive4;   /* PCS Divider End Register 4                          */
    u32 mc_cgm_pcs_divs4;   /* PCS Divider Start Register 4                        */
    u32 mc_cgm_pcs_divc5;   /* PCS Divider Change Register 5                       */
    u32 mc_cgm_pcs_dive5;   /* PCS Divider End Register 5                          */
    u32 mc_cgm_pcs_divs5;   /* PCS Divider Start Register 5                        */
    u32 reserved_0x740[36];
    u32 mc_cgm_sc_div_rc;    /* System Clock Divider Ratio Change Register          */
    u32 mc_cgm_div_upd_type; /* Divider Update Type                                 */
    u32 mc_cgm_div_upd_trig; /* Divider Update Trigger                              */
    u32 mc_cgm_div_upd_stat; /* Divider Update Status                               */
    u32 reserved_0x7e0[1];
    u32 mc_cgm_sc_ss;        /* System Clock Select Status Register                 */
    u32 mc_cgm_sc_dc0;       /* System Clock Divider 0 Configuration Register       */
    u32 mc_cgm_sc_dc1;       /* System Clock Divider 1 Configuration Register       */
    u32 mc_cgm_sc_dc2;       /* System Clock Divider 2 Configuration Register       */
    u32 mc_cgm_sc_dc3;       /* System Clock Divider 3 Configuration Register       */
    u32 mc_cgm_sc_dc4;       /* System Clock Divider 4 Configuration Register       */
    u32 mc_cgm_sc_dc5;       /* System Clock Divider 5 Configuration Register       */
    u32 mc_cgm_ac0_sc;       /* Auxiliary Clock 0 Select Control Register           */
    u32 mc_cgm_ac0_ss;       /* Auxiliary Clock 0 Select Status Register            */
    u32 reserved_0x808[6];
    u32 mc_cgm_ac1_sc;       /* Auxiliary Clock 1 Select Control Register           */
    u32 mc_cgm_ac1_ss;       /* Auxiliary Clock 1 Select Status Register            */
    u32 mc_cgm_ac1_dc0;      /* Auxiliary Clock 1 Divider 0 Configuration Register  */
    u32 reserved_0x82c[5];
    u32 mc_cgm_ac2_sc;       /* Auxiliary Clock 2 Select Control Register           */
    u32 mc_cgm_ac2_ss;       /* Auxiliary Clock 2 Select Status Register            */
    u32 mc_cgm_ac2_dc0;      /* Auxiliary Clock 2 Divider 0 Configuration Register  */
    u32 mc_cgm_ac2_dc1;      /* Auxiliary Clock 2 Divider 1 Configuration Register  */
    u32 reserved_0x850[4];
    u32 mc_cgm_ac3_sc;       /* Auxiliary Clock 3 Select Control Register           */
    u32 mc_cgm_ac3_ss;       /* Auxiliary Clock 3 Select Status Register            */
    u32 mc_cgm_ac3_dc0;      /* Auxiliary Clock 3 Divider 0 Configuration Register  */
    u32 reserved_0x86c[5];
    u32 mc_cgm_ac4_sc;       /* Auxiliary Clock 4 Select Control Register           */
    u32 mc_cgm_ac4_ss;       /* Auxiliary Clock 4 Select Status Register            */
    u32 mc_cgm_ac4_dc0;      /* Auxiliary Clock 4 Divider 0 Configuration Register  */
    u32 reserved_0x88C[5];
    u32 mc_cgm_ac5_sc;       /* Auxiliary Clock 5 Select Control Register           */
    u32 mc_cgm_ac5_ss;       /* Auxiliary Clock 5 Select Status Register            */
    u32 mc_cgm_ac5_dc0;      /* Auxiliary Clock 5 Divider 0 Configuration Register  */
    u32 mc_cgm_ac5_dc1;      /* Auxiliary Clock 5 Divider 1 Configuration Register  */
    u32 reserved_0x8b0[4];
    u32 mc_cgm_ac6_sc;       /* Auxiliary Clock 6 Select Control Register           */
    u32 mc_cgm_ac6_ss;       /* Auxiliary Clock 6 Select Status Register            */
    u32 mc_cgm_ac6_dc0;      /* Auxiliary Clock 6 Divider 0 Configuration Register  */
    u32 mc_cgm_ac6_dc1;      /* Auxiliary Clock 6 Divider 1 Configuration Register  */
    u32 reserved_0x8d0[4];
    u32 mc_cgm_ac7_sc;       /* Auxiliary Clock 7 Select Control Register           */
    u32 mc_cgm_ac7_ss;       /* Auxiliary Clock 7 Select Status Register            */
    u32 mc_cgm_ac7_dc0;      /* Auxiliary Clock 7 Divider 0 Configuration Register  */
    u32 mc_cgm_ac7_dc1;      /* Auxiliary Clock 7 Divider 1 Configuration Register  */
    u32 reserved_0x8f0[4];
    u32 mc_cgm_ac8_sc;       /* Auxiliary Clock 8 Select Control Register           */
    u32 mc_cgm_ac8_ss;       /* Auxiliary Clock 8 Select Status Register            */
    u32 mc_cgm_ac8_dc0;      /* Auxiliary Clock 8 Divider 0 Configuration Register  */
    u32 reserved_0x90c[5];
    u32 mc_cgm_ac9_sc;       /* Auxiliary Clock 9 Select Control Register           */
    u32 mc_cgm_ac9_ss;       /* Auxiliary Clock 9 Select Status Register            */
    u32 mc_cgm_ac9_dc0;      /* Auxiliary Clock 9 Divider 0 Configuration Register  */
    u32 reserved_0x92c[5];
    u32 mc_cgm_ac10_sc;      /* Auxiliary Clock 10 Select Control Register          */
    u32 mc_cgm_ac10_ss;      /* Auxiliary Clock 10 Select Status Register           */
    u32 mc_cgm_ac10_dc0;     /* Auxiliary Clock 10 Divider 0 Configuration Register */
    u32 reserved_0x94c[5];
    u32 mc_cgm_ac11_sc;      /* Auxiliary Clock 11 Select Control Register          */
    u32 mc_cgm_ac11_ss;      /* Auxiliary Clock 11 Select Status Register           */
    u32 mc_cgm_ac11_dc0;     /* Auxiliary Clock 11 Divider 0 Configuration Register */
    u32 reserved_0x96c[5];
    u32 mc_cgm_ac12_sc;      /* Auxiliary Clock 12 Select Control Register          */
    u32 mc_cgm_ac12_ss;      /* Auxiliary Clock 12 Select Status Register           */
    u32 mc_cgm_ac12_dc0;     /* Auxiliary Clock 12 Divider 0 Configuration Register */
    u32 reserved_0x98c[5];
    u32 mc_cgm_ac13_sc;      /* Auxiliary Clock 13 Select Control Register          */
    u32 mc_cgm_ac13_ss;      /* Auxiliary Clock 13 Select Status Register           */
    u32 mc_cgm_ac13_dc0;     /* Auxiliary Clock 13 Divider 0 Configuration Register */
    u32 reserved_0x9ac[5];
    u32 mc_cgm_ac14_sc;      /* Auxiliary Clock 14 Select Control Register          */
    u32 mc_cgm_ac14_ss;      /* Auxiliary Clock 14 Select Status Register           */
    u32 mc_cgm_ac14_dc0;     /* Auxiliary Clock 14 Divider 0 Configuration Register */
    u32 reserved_0x9cc[5];
    u32 mc_cgm_ac15_sc;      /* Auxiliary Clock 15 Select Control Register          */
    u32 mc_cgm_ac15_ss;      /* Auxiliary Clock 14 Select Status Register           */
    u32 mc_cgm_ac15_dc0;     /* Auxiliary Clock 15 Divider 0 Configuration Register */
    u32 mc_cgm_ac15_dc1;     /* Auxiliary Clock 15 Divider 1 Configuration Register */
};

/* MC_CGM registers definitions */

/* MC_CGM_SC_DCm */
#define MC_CGM_SC_DCn_PREDIV(val)   (MC_CGM_SC_DCn_PREDIV_MASK & ((val) << 16))
#define MC_CGM_SC_DCn_PREDIV_MASK   (0x001F0000)
#define MC_CGM_SC_DCn_PREDIV_OFFSET (16)
#define MC_CGM_SC_DCn_DE            (1 << 31)
#define MC_CGM_SC_SEL_MASK          (0x07000000)
#define MC_CGM_SC_SEL_OFFSET        (24)


/* MC_CGM_ACn_DCn */
#define MC_CGM_ACn_DCn_PREDIV(val)    (MC_CGM_ACn_DCn_PREDIV_MASK & ((val) << MC_CGM_ACn_DCn_PREDIV_OFFSET))

/*
 * MC_CGM_ACn_DCn_PREDIV_MASK is on 5 bits because practical test has shown
 * that the 5th bit is always ignored during writes if the current
 * MC_CGM_ACn_DCn_PREDIV field has only 4 bits
 *
 * The manual states only selectors 1, 5 and 15 have DC0_PREDIV on 5 bits
 *
 * This should be changed if any problems occur.
 */
#define MC_CGM_ACn_DCn_PREDIV_MASK      (0x001F0000)
#define MC_CGM_ACn_DCn_PREDIV_OFFSET    (16)
#define MC_CGM_ACn_DCn_DE               (1 << 31)

/*
 *  MC_CGM_ACn_SC/MC_CGM_ACn_SS
*/
#define MC_CGM_ACn_SEL_MASK             (0x07000000)
#define MC_CGM_ACn_SEL_SET(source)      (MC_CGM_ACn_SEL_MASK & (((source) & 0x7) << MC_CGM_ACn_SEL_OFFSET))
#define MC_CGM_ACn_SEL_OFFSET           (24)
#define MC_CGM_ACn_SEL_FIRC             (0x0)
#define MC_CGM_ACn_SEL_FXOSC            (0x1)
#define MC_CGM_ACn_SEL_SIRC             (0x2)
#define MC_CGM_ACn_SEL_PLL0             (0x4)
#define MC_CGM_ACn_SEL_PLL1             (0x5)
#define MC_CGM_ACn_SEL_PLL2             (0x6)
#define MC_CGM_ACn_SEL_PLL3             (0x7)
#define MC_CGM_ACn_SEL_SYSCLK           (0x8)

/* PLLDIG PLL Divider Register (PLLDIG_PLLDV) */
#define PLLDIG_PLLDV_MFD(div)           (0x000000FF & (div))

/*  PLLDIG_PLLDV_RFDPHIB has a different format for /32 according to 
    the reference manual. This other value respect the formula 2^[RFDPHIBY+1]
*/
#define PLLDIG_PLLDV_RFDPHIBY_32        (0x4)
#define PLLDIG_PLLDV_RFDPHI_SET(val)    (PLLDIG_PLLDV_RFDPHI_MASK & (((val) & PLLDIG_PLLDV_RFDPHI_MAXVALUE) << PLLDIG_PLLDV_RFDPHI_OFFSET))
#define PLLDIG_PLLDV_RFDPHI_MASK        (0x003F0000)
#define PLLDIG_PLLDV_RFDPHI_MAXVALUE    (0x7)
#define PLLDIG_PLLDV_RFDPHI_OFFSET      (16)


#define PLLDIG_PLLDV_MFD_MASK           (0x000000FF)

#define PLLDIG_PLLDV_PREDIV_SET(val)    (PLLDIG_PLLDV_PREDIV_MASK & (((val) & PLLDIG_PLLDV_PREDIV_MAXVALUE) << PLLDIG_PLLDV_PREDIV_OFFSET))
#define PLLDIG_PLLDV_PREDIV_MASK        (0x00007000)
#define PLLDIG_PLLDV_PREDIV_MAXVALUE    (0x7)
#define PLLDIG_PLLDV_PREDIV_OFFSET      (12)


/* PLLDIG PLL Fractional  Divide Register (PLLDIG_PLLFD) */
#define PLLDIG_PLLFD_MFN_SET(val)       (PLLDIG_PLLFD_MFN_MASK & (val))
#define PLLDIG_PLLFD_MFN_MASK           (0x00007FFF)


/* PLL Calibration Register 3 (PLLDIG_PLLCAL3) */
#define PLLDIG_PLLCAL3_MFDEN_SET(val)   (PLLDIG_PLLCAL3_MFDEN_MASK & ((val) << PLLDIG_PLLCAL3_MFDEN_OFFSET))
#define PLLDIG_PLLCAL3_MFDEN_OFFSET     (14)
#define PLLDIG_PLLCAL3_MFDEN_MASK       (0x3FFFFC000)

#endif
/* Frequencies are in MHz */
#define FIRC_CLK_FREQ                   16000
#define FXOSC_CLK_FREQ                  40000

#endif /*__ARCH_ARM_MACH_MAC57D5XH_MCCGM_REGS_H__ */
