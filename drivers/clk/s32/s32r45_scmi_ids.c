// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020-2021 NXP
 */
#include <dt-bindings/clock/s32r45-clock.h>
#include <dt-bindings/clock/s32r45-scmi-clock.h>
#include <errno.h>
#include <linux/kernel.h>
#include <s32gen1_clk_funcs.h>
#include <s32gen1_scmi_clk.h>

#define INDEX(X)	((X) - S32GEN1_SCMI_PLAT_CLK_BASE_ID)

static u32 s32r45_scmi_ids[] = {
	/* LAX */
	[INDEX(S32R45_SCMI_CLK_LAX_MODULE)] = S32R45_CLK_ACCEL4,
	/* SPT */
	[INDEX(S32R45_SCMI_CLK_SPT_SPT)] = S32R45_CLK_ACCEL3,
	[INDEX(S32R45_SCMI_CLK_SPT_AXI)] = S32R45_CLK_ACCEL3,
	[INDEX(S32R45_SCMI_CLK_SPT_MODULE)] = S32R45_CLK_ACCEL3_DIV3,
	/* GMAC1 */
	[INDEX(S32R45_SCMI_CLK_GMAC1_RX_SGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32R45_SCMI_CLK_GMAC1_TX_SGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32R45_SCMI_CLK_GMAC1_TS)] = S32GEN1_SCMI_COMPLEX_CLK,
};

static int s32r_compound2clkid(u32 scmi_clk_id, u32 *clk_id)
{
	switch (scmi_clk_id) {
	case S32R45_SCMI_CLK_GMAC1_RX_SGMII:
	case S32R45_SCMI_CLK_GMAC1_RX_RGMII:
		if (clk_id)
			*clk_id = S32R45_CLK_GMAC1_RX;
		break;
	case S32R45_SCMI_CLK_GMAC1_TX_SGMII:
	case S32R45_SCMI_CLK_GMAC1_TX_RGMII:
		if (clk_id)
			*clk_id = S32R45_CLK_GMAC1_TX;
		break;
	case S32R45_SCMI_CLK_GMAC1_TS:
		if (clk_id)
			*clk_id = S32GEN1_CLK_GMAC0_TS;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int plat_scmi_id2clk(u32 scmi_clk_id, u32 *clk_id)
{
	if (!clk_id)
		return -EINVAL;

	if (INDEX(scmi_clk_id) >= ARRAY_SIZE(s32r45_scmi_ids)) {
		pr_err("Unhandled clock (index out of bounds): %u\n",
		       scmi_clk_id);
		return -EINVAL;
	}

	*clk_id = s32r45_scmi_ids[INDEX(scmi_clk_id)];
	if (!*clk_id) {
		pr_err("Unhandled clock (no clock id): %u\n", scmi_clk_id);
		return -EINVAL;
	}

	return 0;
}

int plat_compound_clk_get(struct clk *clk)
{
	u32 scmi_clk_id = clk->id;

	if (INDEX(scmi_clk_id) >= ARRAY_SIZE(s32r45_scmi_ids))
		return -EINVAL;

	if (s32r_compound2clkid(scmi_clk_id, NULL)) {
		pr_err("Invalid compound clock : %u\n", scmi_clk_id);
		return -EINVAL;
	}

	return 0;
}

static int set_gmac1_rx_parent(struct clk *clk)
{
	u32 rx_id;
	u32 clk_id = clk->id;

	if (clk_id == S32R45_SCMI_CLK_GMAC1_RX_SGMII) {
		rx_id = S32R45_CLK_SERDES1_LANE0_CDR;
	} else {
		pr_err("Invalid GMAC RX mode\n");
		return -EINVAL;
	}

	return cc_set_mux_parent(clk, S32R45_CLK_MC_CGM2_MUX4, rx_id);
}

static int set_gmac1_tx_parent(struct clk *clk)
{
	u32 tx_id;
	u32 clk_id = clk->id;

	if (clk_id == S32R45_SCMI_CLK_GMAC1_TS) {
		tx_id = S32GEN1_CLK_PERIPH_PLL_PHI5;
	} else if (clk_id == S32R45_SCMI_CLK_GMAC1_TX_SGMII) {
		tx_id = S32R45_CLK_SERDES1_LANE0_TX;
	} else {
		pr_err("Invalid GMAC TX mode\n");
		return -EINVAL;
	}

	return cc_set_mux_parent(clk, S32R45_CLK_MC_CGM2_MUX2, tx_id);
}

static int set_gmac_ts_parent(struct clk *clk)
{
	u32 ts_id;
	u32 clk_id = clk->id;

	if (clk_id == S32R45_SCMI_CLK_GMAC1_TS) {
		ts_id = S32GEN1_CLK_PERIPH_PLL_PHI4;
	} else {
		pr_err("Invalid GMAC TS mode\n");
		return -EINVAL;
	}

	return cc_set_mux_parent(clk, S32GEN1_CLK_MC_CGM0_MUX9, ts_id);
}

int plat_compound_clk_set_parents(struct clk *clk)
{
	u32 clk_id = clk->id;
	u32 id;

	if (s32r_compound2clkid(clk_id, &id)) {
		pr_err("Invalid compound clock : %u\n", clk_id);
		return -EINVAL;
	}

	switch (id) {
	case S32R45_CLK_GMAC1_RX:
		return set_gmac1_rx_parent(clk);
	case S32R45_CLK_GMAC1_TX:
		return set_gmac1_tx_parent(clk);
	case S32GEN1_CLK_GMAC0_TS:
		return set_gmac_ts_parent(clk);
	default:
		return plat_compound_clk_set_parents(clk);
	}

}

int plat_compound_clk_enable(struct clk *clk)
{
	struct clk sclock = *clk;
	u32 clk_id = clk->id;
	u32 id;
	int ret;

	if (s32r_compound2clkid(clk_id, &id)) {
		pr_err("Invalid s3245 compound clock : %u\n", clk_id);
		return -EINVAL;
	}

	ret = plat_compound_clk_set_parents(clk);
	if (ret) {
		pr_err("Failed to set parents for %u\n", clk_id);
		return -EINVAL;
	}

	sclock.id = id;
	ret = s32gen1_enable(&sclock);
	if (ret) {
		pr_err("%s Failed to enable %u clock\n", __func__, clk_id);
		return ret;
	}

	return 0;

}

ulong plat_compound_clk_set_rate(struct clk *clk, ulong rate)
{
	struct clk sclock = *clk;
	u32 scmi_clk_id = clk->id;
	u32 id;
	int ret;

	if (INDEX(scmi_clk_id) >= ARRAY_SIZE(s32r45_scmi_ids))
		return -EINVAL;

	ret = plat_compound_clk_set_parents(clk);
	if (ret) {
		pr_err("Failed to set parents for %u\n", scmi_clk_id);
		return -EINVAL;
	}

	if (s32r_compound2clkid(scmi_clk_id, &id)) {
		pr_err("Invalid compound clock : %u\n", scmi_clk_id);
		return -EINVAL;
	}

	sclock.id = id;
	return s32gen1_set_rate(&sclock, rate);

}

ulong plat_compound_clk_get_rate(struct clk *clk)
{
	return 0;
}

