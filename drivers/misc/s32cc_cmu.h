/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2021-2023 NXP
 */

#ifndef __S32_CMU_H
#define __S32_CMU_H

#include <linux/stringify.h>

#define FIRC_VARIATION			(50ULL)	// 5.0%
#define FXOSC_VARIATION			(5ULL)	// 0.5%
#define PERIPH_VARIATION		(5ULL)	// 0.5%

#define FIRC_FREQ			(48000000ULL)
#define FIRC_MIN_FREQ			(45600000ULL)
#define FIRC_MAX_FREQ			(50400000ULL)
#define FXOSC_FREQ			(40000000ULL)
#define SIRC_FREQ			(32000ULL)

#define CMU(ID, REF_CLK, MON_CLK, REF_FRQ, EXP_FRQ, REF_VAR, MON_VAR, FC) \
{\
	.offset = 0x20 * (ID),\
	.ref_clk = (REF_CLK),\
	.mon_clk = (MON_CLK),\
	.ref_name = __stringify(REF_CLK),\
	.mon_name = __stringify(MON_CLK),\
	.ref_freq = (REF_FRQ),\
	.has_exp_range = false,\
	.exp_freq = (EXP_FRQ),\
	.ref_var = (REF_VAR),\
	.mon_var = (MON_VAR),\
	.fc = (FC),\
}

#define CMU_EXP_RANGE(ID, REF_CLK, MON_CLK, REF_FRQ, EXP_MIN, EXP_MAX,\
		      REF_VAR, MON_VAR, FC) \
{\
	.offset = 0x20 * (ID),\
	.ref_clk = (REF_CLK),\
	.mon_clk = (MON_CLK),\
	.ref_name = __stringify(REF_CLK),\
	.mon_name = __stringify(MON_CLK),\
	.ref_freq = (REF_FRQ),\
	.has_exp_range = true,\
	.exp_range = { .min = (EXP_MIN), .max = (EXP_MAX) },\
	.ref_var = (REF_VAR),\
	.mon_var = (MON_VAR),\
	.fc = (FC),\
}

#define FXOSC_PERIPH_CMU_FC(ID, MON, MON_FRQ) \
	CMU(ID, FXOSC, MON, FXOSC_FREQ, MON_FRQ, \
			FXOSC_VARIATION, PERIPH_VARIATION, true)

#define FXOSC_PERIPH_CMU_FC_RANGE(ID, MON, MIN_FRQ, MAX_FRQ) \
	CMU_EXP_RANGE(ID, FXOSC, MON, FXOSC_FREQ, MIN_FRQ, MAX_FRQ, \
			FXOSC_VARIATION, PERIPH_VARIATION, true)

#define FIRC_PERIPH_CMU_FC(ID, MON, MON_FRQ) \
	CMU(ID, FIRC, MON, FIRC_FREQ, MON_FRQ, \
			FIRC_VARIATION, PERIPH_VARIATION, true)

#define FXOSC_PERIPH_CMU_FM(ID, MON, MON_FRQ) \
	CMU(ID, FXOSC, MON, FXOSC_FREQ, MON_FRQ, \
			0, 0, false)

#define FXOSC_PERIPH_CMU_FM_RANGE(ID, MON, MIN_FRQ, MAX_FRQ) \
	CMU_EXP_RANGE(ID, FXOSC, MON, FXOSC_FREQ, MIN_FRQ, MAX_FRQ, \
			FXOSC_VARIATION, PERIPH_VARIATION, false)

#define FIRC_PERIPH_CMU_FM(ID, MON, MON_FRQ) \
	CMU(ID, FIRC, MON, FIRC_FREQ, MON_FRQ, \
			0, 0, false)

enum cmu_fc_clk {
	FIRC,
	FXOSC,
	SIRC,
	XBAR_M7_0,
	XBAR_M7_1,
	XBAR_M7_2,
	XBAR_M7_3,
	XBAR_DIV3,
	SERDES_REF,
	PER,
	CAN_PE,
	LIN,
	QSPI_1X,
	SDHC,
	DDR,
	SPI,
	A53_CORE,
	SPT,
	LAX0,
	LAX1,
	MIPICSI2_0,
	MIPICSI2_2,
	GMAC_TS,
	GMAC_0_TX,
	GMAC_0_RX,
	GMAC_1_TX,
	GMAC_1_RX,
	PFE_SYS,
	PFE_MAC_0_TX,
	PFE_MAC_0_RX,
	PFE_MAC_1_TX,
	PFE_MAC_1_RX,
	PFE_MAC_2_TX,
	PFE_MAC_2_RX,
	FTM_0_REF,
	FTM_1_REF,
	FLEXRAY_PE,
};

struct cmu {
	uintptr_t offset;
	enum cmu_fc_clk ref_clk;
	enum cmu_fc_clk mon_clk;
	const char *ref_name;
	const char *mon_name;
	u64 ref_freq;
	bool has_exp_range;
	union {
		u64 exp_freq;
		struct {
			u64 min;
			u64 max;
		} exp_range;
	};
	u64 ref_var;
	u64 mon_var;
	bool fc;
};
#endif
