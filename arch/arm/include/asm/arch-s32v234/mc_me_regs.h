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

#ifndef __ARCH_ARM_MACH_S32V234_MCME_REGS_H__
#define __ARCH_ARM_MACH_S32V234_MCME_REGS_H__

#ifndef __ASSEMBLY__

/* Mode Entry Module (MC_ME) */
struct mc_me_reg {
	u32 mc_me_gs;			/* Global Status Register						*/
	u32 mc_me_mctl;			/* Mode Control Register						*/
	u32 mc_me_me;			/* Mode Enable Register							*/
	u32 mc_me_is;			/* Interrupt Status Register					*/
	u32 mc_me_im;			/* Interrupt Mask Register						*/
	u32 mc_me_imts;			/* Invalid Mode Transition Status Register		*/
	u32 mc_me_dmts;			/* Debug Mode Transition Status Register		*/
	u32 reserved_0x1c[1];
	u32 mc_me_reset_mc;		/* RESET Mode Configuration Register			*/
	u32 mc_me_test_mc;		/* TEST Mode Configuration Register				*/
	u32 reserved_0x28[1];
	u32 mc_me_drun_mc;		/* DRUN Mode Configuration Register				*/
	u32 mc_me_run0_mc;		/* RUN0 Mode Configuration Register				*/
	u32 mc_me_run1_mc;		/* RUN1 Mode Configuration Register				*/
	u32 mc_me_run2_mc;		/* RUN2 Mode Configuration Register				*/
	u32 mc_me_run3_mc;		/* RUN3 Mode Configuration Register				*/
	u32 reserved_0x40[9];
	u32 mc_me_ps1;			/* Peripheral Status Register 1					*/
	u32 mc_me_ps2;			/* Peripheral Status Register 2					*/
	u32 mc_me_ps3;			/* Peripheral Status Register 3					*/
	u32 reserved_0x70[1];
	u32 mc_me_ps5;			/* Peripheral Status Register 5					*/
	u32 mc_me_ps6;			/* Peripheral Status Register 6					*/
	u32 mc_me_ps7;			/* Peripheral Status Register 7					*/
	u32 mc_me_run_pc0;		/* Run Peripheral Configuration Register		*/
	u32 mc_me_run_pc1;		/* Run Peripheral Configuration Register		*/
	u32 mc_me_run_pc2;		/* Run Peripheral Configuration Register		*/
	u32 mc_me_run_pc3;		/* Run Peripheral Configuration Register		*/
	u32 mc_me_run_pc4;		/* Run Peripheral Configuration Register		*/
	u32 mc_me_run_pc5;		/* Run Peripheral Configuration Register		*/
	u32 mc_me_run_pc6;		/* Run Peripheral Configuration Register		*/
	u32 mc_me_run_pc7;		/* Run Peripheral Configuration Register		*/
	u8	mc_me_pctl39;		/* DEC200 Peripheral Control Register		*/
	u8	reserved_0xe5[6];
	u8	mc_me_pctl40;		/* 2D-ACE Peripheral Control Register		*/
	u8	reserved_ec[5];
	u8	mc_me_pctl50;		/* ENET Peripheral Control Register			*/
	u8	mc_me_pctl49;		/* DMACHMUX0 Peripheral Control Register	*/
	u8	mc_me_pctl4;		/* CSI0 Peripheral Control Register			*/
	u8	reserved_0xf4[1];
	u8	mc_me_pctl54;		/* MMDC0 Peripheral Control Register		*/
	u8	reserved_0xf6[1];
	u8	mc_me_pctl52;		/* FRAY Peripheral Control Register			*/
	u8	reserved_0xf8[1];
	u8	mc_me_pctl58;		/* PIT0 Peripheral Control Register			*/
	u8	reserved_0xfa[18];
	u8	mc_me_pctl79;		/* FlexTIMER0 Peripheral Control Register	*/
	u8	reserved_0x10d[1];
	u8	mc_me_pctl77;		/* SARADC0 Peripheral Control Register		*/
	u8	reserved_0x10f[1];
	u8	mc_me_pctl83;		/* LINFLEX0 Peripheral Control Register		*/
	u8	reserved_0x111[1];
	u8	mc_me_pctl81;		/* IIC0 Peripheral Control Register			*/
	u8	reserved_0x113[1];
	u8	mc_me_pctl87;		/* DSPI0 Peripheral Control Register		*/
	u8	reserved_0x115[1];
	u8	mc_me_pctl85;		/* CANFD0 Peripheral Control Register		*/
	u8	reserved_0x117[1];
	u8	mc_me_pctl91;		/* CRC0 Peripheral Control Register			*/
	u8	reserved_0x119[1];
	u8	mc_me_pctl89;		/* DSPI2 Peripheral Control Register		*/
	u8	reserved_0x11b[3];
	u8	mc_me_pctl93;		/* SDHC Peripheral Control Register			*/
	u8	reserved_0x11f[8];
	u8	mc_me_pctl100;		/* VIU0 Peripheral Control Register			*/
	u8	reserved_0x128[3];
	u8	mc_me_pctl104;		/* HPSMI Peripheral Control Register		*/
	u8	reserved_0x12c[11];
	u8	mc_me_pctl116;		/* SIPI Peripheral Control Register			*/
	u8	reserved_0x138[3];
	u8	mc_me_pctl120;		/* LFAST Peripheral Control Register		*/
	u8	reserved_0x13c[37];
	u8	mc_me_pctl162;		/* MMDC1 Peripheral Control Register		*/
	u8	mc_me_pctl161;		/* DMACHMUX1 Peripheral Control Register	*/
	u8	mc_me_pctl160;		/* CSI1 Peripheral Control Register			*/
	u8	reserved_0x164[1];
	u8	mc_me_pctl166;		/* QUADSPI0 Peripheral Control Register		*/
	u8	reserved_0x166[3];
	u8	mc_me_pctl170;		/* PIT1 Peripheral Control Register			*/
	u8	reserved_0x16a[11];
	u8	mc_me_pctl182;		/* FlexTIMER1 Peripheral Control Register	*/
	u8	reserved_0x176[3];
	u8	mc_me_pctl186;		/* IIC2 Peripheral Control Register			*/
	u8	reserved_0x17a[1];
	u8	mc_me_pctl184;		/* IIC1 Peripheral Control Register			*/
	u8	reserved_0x17c[1];
	u8	mc_me_pctl190;		/* CANFD1 Peripheral Control Register		*/
	u8	reserved_0x17e[1];
	u8	mc_me_pctl188;		/* LINFLEX1 Peripheral Control Register		*/
	u8	reserved_0x180[1];
	u8	mc_me_pctl194;		/* DSPI3 Peripheral Control Register		*/
	u8	reserved_0x182[1];
	u8	mc_me_pctl192;		/* DSPI1 Peripheral Control Register		*/
	u8	reserved_0x184[9];
	u8	mc_me_pctl206;		/* TSENS Peripheral Control Register		*/
	u8	reserved_0x18e[1];
	u8	mc_me_pctl204;		/* CRC1 Peripheral Control Register			*/
	u8	reserved_0x190[3];
	u8	mc_me_pctl208;		/* VIU1 Peripheral Control Register			*/
	u8	reserved_0x194[3];
	u8	mc_me_pctl212;		/* JPEG Peripheral Control Register			*/
	u8	reserved_0x198[4];
	u8	mc_me_pctl216;		/* H264_DEC Peripheral Control Register		*/
	u8	reserved_0x19c[4];
	u8	mc_me_pctl220;		/* H264_ENC Peripheral Control Register		*/
	u8	reserved_0x1a0[9];
	u8	mc_me_pctl236;		/* MBIST Peripheral Control Register		*/
	u8	reserved_0x1aa[22];
	u32 mc_me_cs;			/* Core Status Register						*/
	u16 mc_me_cctl1;		/* CORE0 Control Register					*/
	u16 mc_me_cctl0;		/* CM4 Core Control Register				*/
	u16 mc_me_cctl3;		/* CORE2 Control Register					*/
	u16 mc_me_cctl2;		/* CORE1 Control Register					*/
	u16 reserved_0x1cc[1];
	u16 mc_me_cctl4;		/* CORE3 Control Register					*/
	u16 reserved_0x1d0[8];
	u32 mc_me_caddr0;		/* CM4 Core Address Register				*/
	u32 mc_me_caddr1;		/* CORE0 Core Address Register				*/
	u32 mc_me_caddr2;		/* CORE1 Core Address Register				*/
	u32 mc_me_caddr3;		/* CORE2 Core Address Register				*/
	u32 mc_me_caddr4;		/* CORE3 Core Address Register				*/
	u32 reserved_0x1f4[19];
	u32 mc_me_test_sec_cc_i;	/* TEST Secondary Clock Configuration Register */
	u32 reserved_0x244[7];
	u32 mc_me_drun_sec_cc_i;	/* DRUN Secondary Clock Configuration Register */
	u32 reserved_0x264[7];
	u32 mc_me_run0_sec_cc_i;	/* RUN0 Secondary Clock Configuration Register */
	u32 reserved_0x274[7];
	u32 mc_me_run1_sec_cc_i;	/* RUN1 Secondary Clock Configuration Register */
	u32 reserved_0x284[7];
	u32 mc_me_run2_sec_cc_i;	/* RUN2 Secondary Clock Configuration Register */
	u32 reserved_0x2a0[11];
	u32 mc_me_run3_sec_cc_i;	/* RUN3 Secondary Clock Configuration Register */
};

/* MC_ME registers definitions */

/* MC_ME_ME */
#define MC_ME_ME_RESET_FUNC				(1 << 0)
#define MC_ME_ME_TEST					(1 << 1)
#define MC_ME_ME_DRUN					(1 << 3)
#define MC_ME_ME_RUN0					(1 << 4)
#define MC_ME_ME_RUN1					(1 << 5)
#define MC_ME_ME_RUN2					(1 << 6)
#define MC_ME_ME_RUN3					(1 << 7)

/* MC_ME_RUN_PCn */
#define MC_ME_RUN_PCn_RESET				(1 << 0)
#define MC_ME_RUN_PCn_TEST				(1 << 1)
#define MC_ME_RUN_PCn_DRUN				(1 << 3)
#define MC_ME_RUN_PCn_RUN0				(1 << 4)
#define MC_ME_RUN_PCn_RUN1				(1 << 5)
#define MC_ME_RUN_PCn_RUN2				(1 << 6)
#define MC_ME_RUN_PCn_RUN3				(1 << 7)

/*
 * MC_ME_DRUN_MC/ MC_ME_RUN0_MC/ MC_ME_RUN1_MC
 * MC_ME_RUN2_MC/ MC_ME_RUN3_MC
 * MC_ME_STANDBY0_MC/MC_ME_STOP0_MC
 */
#define MC_ME_RUNMODE_MC_SYSCLK(val)	(0x0000000F & (val))
#define MC_ME_RUNMODE_MC_FIRCON			(1 << 4)
#define MC_ME_RUNMODE_MC_XOSCON			(1 << 5)
#define MC_ME_RUNMODE_MC_ARMPLL			(1 << 6)
#define MC_ME_RUNMODE_MC_PERPLL			(1 << 7)
#define MC_ME_RUNMODE_MC_ENETPLL		(1 << 8)
#define MC_ME_RUNMODE_MC_DDRPLL			(1 << 9)
#define MC_ME_RUNMODE_MC_VIDEO			(1 << 10)
#define MC_ME_RUNMODE_MC_MVRON			(1 << 20)
#define MC_ME_RUNMODE_MC_PDO			(1 << 23)
#define MC_ME_RUNMODE_MC_PWRLVL0		(1 << 28)
#define MC_ME_RUNMODE_MC_PWRLVL1		(1 << 29)
#define MC_ME_RUNMODE_MC_PWRLVL2		(1 << 30)

/* MC_ME_MCTL */
#define MC_ME_MCTL_KEY					(0x00005AF0)
#define MC_ME_MCTL_INVERTEDKEY			(0x0000A50F)
#define MC_ME_MCTL_RESET				(0x0 << 28)
#define MC_ME_MCTL_TEST					(0x1 << 28)
#define MC_ME_MCTL_DRUN					(0x3 << 28)
#define MC_ME_MCTL_RUN0					(0x4 << 28)
#define MC_ME_MCTL_RUN1					(0x5 << 28)
#define MC_ME_MCTL_RUN2					(0x6 << 28)
#define MC_ME_MCTL_RUN3					(0x7 << 28)

/* MC_ME_GS */
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

#endif

#endif /*__ARCH_ARM_MACH_S32V234_MCME_REGS_H__ */
