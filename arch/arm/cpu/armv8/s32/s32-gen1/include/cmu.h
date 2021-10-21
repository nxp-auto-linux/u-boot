/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021 NXP
 */

#ifndef __S32_CMU_H
#define __S32_CMU_H

#include <common.h>

#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

#define FIRC_VARIATION			6.0f
#define FXOSC_VARIATION			0.5f
#define PERIPH_VARIATION		0.5f

#define FIRC_FREQ			((double)48)
#define FXOSC_FREQ			((double)40)
#define SIRC_FREQ			((double)0.032)

#define CMU(ID, REF_CLK, MON_CLK, REF_FRQ, MON_FRQ, REF_VAR, MON_VAR, FC) \
{\
	.addr = CMU_BASE_ADDR + 0x20 * (ID),\
	.ref_clk = (REF_CLK),\
	.mon_clk = (MON_CLK),\
	.ref_name = STR(REF_CLK),\
	.mon_name = STR(MON_CLK),\
	.ref_freq = (REF_FRQ),\
	.mon_freq = (MON_FRQ),\
	.ref_var = (REF_VAR),\
	.mon_var = (MON_VAR),\
	.fc = (FC),\
}

#define FXOSC_PERIPH_CMU_FC(ID, MON, MON_FRQ) \
	CMU(ID, FXOSC_CLK, MON, FXOSC_FREQ, MON_FRQ, \
			FXOSC_VARIATION, PERIPH_VARIATION, true)

#define FIRC_PERIPH_CMU_FC(ID, MON, MON_FRQ) \
	CMU(ID, FIRC_CLK, MON, FIRC_FREQ, MON_FRQ, \
			FIRC_VARIATION, PERIPH_VARIATION, true)

#define FXOSC_PERIPH_CMU_FM(ID, MON, MON_FRQ) \
	CMU(ID, FXOSC_CLK, MON, FXOSC_FREQ, MON_FRQ, \
			0, 0, false)

#define FIRC_PERIPH_CMU_FM(ID, MON, MON_FRQ) \
	CMU(ID, FIRC_CLK, MON, FIRC_FREQ, MON_FRQ, \
			0, 0, false)

enum cmu_fc_clk {
	FIRC_CLK,
	FXOSC_CLK,
	SIRC_CLK,
	XBAR_CLK_M7_0,
	XBAR_CLK_M7_1,
	XBAR_CLK_M7_2,
	XBAR_CLK_M7_3,
	XBAR_DIV3_CLK,
	SERDES_REF_CLK,
	PER_CLK,
	CAN_PE_CLK,
	LIN_CLK,
	QSPI_1X_CLK,
	SDHC_CLK,
	DDR_CLK,
	SPI_CLK,
	A53_CORE_CLK,
	ACCEL3_CLK,
	ACCEL4_CLK_0,
	ACCEL4_CLK_1,
	MIPICSI2_0,
	MIPICSI2_2,
	GMAC_TS_CLK,
	GMAC_0_TX_CLK,
	GMAC_0_RX_CLK,
	GMAC_1_TX_CLK,
	GMAC_1_RX_CLK,
	PFE_SYS_CLK,
	PFE_MAC_0_TX_CLK,
	PFE_MAC_0_RX_CLK,
	PFE_MAC_1_TX_CLK,
	PFE_MAC_1_RX_CLK,
	PFE_MAC_2_TX_CLK,
	PFE_MAC_2_RX_CLK,
	FTM_0_REF_CLK,
	FTM_1_REF_CLK,
	FLEXRAY_PE_CLK,
};

struct cmu {
	uintptr_t addr;
	enum cmu_fc_clk ref_clk;
	enum cmu_fc_clk mon_clk;
	const char *ref_name;
	const char *mon_name;
	double ref_freq;
	double mon_freq;
	double ref_var;
	double mon_var;
	bool fc;
};

struct cmu *get_cmu_block(int index);
int get_cmu_blocks_number(void);
#endif
