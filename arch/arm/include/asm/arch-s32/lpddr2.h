/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017, 2020-2021 NXP
 */

#ifndef __ARCH_ARM_MACH_S32_LPDDR2_H__
#define __ARCH_ARM_MACH_S32_LPDDR2_H__

/* definitions for LPDDR2 PAD values */
#define LPDDR2_CLK0_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |\
	 SIUL2_MSCR_DSE_48ohm | SIUL2_MSCR_PUS_100K_UP | SIUL2_MSCR_CRPOINT_TRIM_1 |			\
	 SIUL2_MSCR_DCYCLE_TRIM_NONE)
#define LPDDR2_CKEn_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |\
	 SIUL2_MSCR_PUS_100K_UP | SIUL2_MSCR_DSE_48ohm)
#define LPDDR2_CS_Bn_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |\
	 SIUL2_MSCR_PUS_100K_UP | SIUL2_MSCR_DSE_48ohm)
#define LPDDR2_DMn_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |\
	 SIUL2_MSCR_PUS_100K_UP | SIUL2_MSCR_DSE_48ohm)
#define LPDDR2_DQSn_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |	\
	 SIUL2_MSCR_DSE_48ohm | SIUL2_MSCR_PUE_EN | SIUL2_MSCR_PUS_100K_DOWN |						\
	 SIUL2_MSCR_PKE_EN | SIUL2_MSCR_CRPOINT_TRIM_1 | SIUL2_MSCR_DCYCLE_TRIM_NONE)
#define LPDDR2_An_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |	\
	 SIUL2_MSCR_DSE_48ohm | SIUL2_MSCR_DDR_DO_TRIM_50PS | SIUL2_MSCR_DCYCLE_TRIM_LEFT		|	\
	 SIUL2_MSCR_PUS_100K_UP)
#define LPDDR2_Dn_PAD	\
	(SIUL2_MSCR_DDR_SEL_LPDDR2 | SIUL2_MSCR_DDR_INPUT_DIFF_DDR | SIUL2_MSCR_DDR_ODT_120ohm |	\
	 SIUL2_MSCR_DSE_48ohm | SIUL2_MSCR_DDR_DO_TRIM_50PS | SIUL2_MSCR_DCYCLE_TRIM_LEFT		|	\
	 SIUL2_MSCR_PUS_100K_UP)

/* Set MDSCR[CON_REQ] (configuration request) */
#define MMDC_MDSCR_CFG_VALUE		0x00008000
/* Precharge-all command CS0 */
#define MMDC_MDSCR_CS0_VALUE		0x00008010
/* Precharge-all command CS1 */
#define MMDC_MDSCR_CS1_VALUE		0x00008018
/* tAOFPD=n/a,tAONPD=n/a,tANPD=n/a,tAXPD=n/a,tODTLon=n/a,tODT_idle_off=n/a */
#define MMDC_MDOTC_VALUE		0x00000000
/* tXPR=n/a , SDE_to_RST=n/a, RST_to_CKE=14 */
#define MMDC_MDOR_VALUE			0x00000010
/* Force delay line initialisation */
#define MMDC_MPMUR0_VALUE		0x00000800
/* Reset command CS0 */
#define MMDC_MDSCR_RST_VALUE		0x003F8030
/* ZQ_LP2_HW_ZQCS=0x1B (90ns spec), ZQ_LP2_HW_ZQCL=0x5F (160ns spec),
 * ZQ_LP2_HW_ZQINIT=0x109 (1us spec)
 */
#define MMDC_MPZQLP2CTL_VALUE		0x1B5F0109
/* ZQ_EARLY_COMPARATOR_EN_TIMER=0x14, TZQ_CS=n/a, TZQ_OPER=n/a,
 * TZQ_INIT=n/a, ZQ_HW_FOR=1, ZQ_HW_PER=0, ZQ_MODE=3
 */
#define MMDC_MPZQHWCTRL_VALUE		0xA0010003
/* Configure MR1: BL 4, burst type interleaved,
 * wrap control no wrap, tWR cycles 8
 */
#define MMDC_MDSCR_MR1_VALUE		0xC2018030
/* Configure MR10: Calibration at init */
#define MMDC_MDSCR_MR10_VALUE		0xFF0A8030
/* Read/write command delay - default used */
#define MMDC_MDRWD_VALUE		0x0F9F26D2
/* Power down control */
#define MMDC_MDPDC_VALUE		0x00020024
/* Refresh control */
#define MMDC_MDREF_VALUE		0x30B01800
/* No ODT */
#define MMDC_MPODTCTRL_VALUE		0x00000000
/* Deassert the configuration request */
#define MMDC_MDSCR_DEASSERT_VALUE	0x00000000
/* DVFS and LPMD request */
#define MMDC_MAPSR_EN_SLF_REF		0x00300000

struct lpddr2_config {
	u32 mdasp_module0;
	u32 mdasp_module1;
	u32 mdcfg0;
	u32 mdcfg1;
	u32 mdcfg2;
	u32 mdcfg3lp;
	u32 mdctl;
	u32 mdmisc;
	u32 mdscr_mr2;
	u32 mdscr_mr3;
	u32 mprddlctl_module0;
	u32 mprddlctl_module1;
	u32 mpwrdlctl_module0;
	u32 mpwrdlctl_module1;
	u32 mpdgctrl0_module0;
	u32 mpdgctrl1_module0;
	u32 mpdgctrl0_module1;
	u32 mpdgctrl1_module1;
	u32 frequency;
};

const struct lpddr2_config *s32_get_lpddr2_config(void);

/* set I/O pads for DDR */
void ddr_config_iomux(uint8_t module);
void config_mmdc(uint8_t module);

#endif
