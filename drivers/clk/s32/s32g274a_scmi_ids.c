// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020 NXP
 */
#include <dt-bindings/clock/s32g274a-clock.h>
#include <dt-bindings/clock/s32g274a-scmi-clock.h>
#include <errno.h>
#include <linux/kernel.h>
#include <s32gen1_clk_funcs.h>
#include <s32gen1_scmi_clk.h>

#define INDEX(X)	((X) - S32GEN1_SCMI_PLAT_CLK_BASE_ID)

static u32 s32g274a_scmi_ids[] = {
	[INDEX(S32G274A_SCMI_CLK_USB_MEM)] = S32GEN1_CLK_XBAR_DIV4,
	[INDEX(S32G274A_SCMI_CLK_USB_LOW)] = S32GEN1_CLK_SIRC,
	[INDEX(S32G274A_SCMI_CLK_PFE_AXI)] = S32G274A_CLK_PFE_SYS,
	[INDEX(S32G274A_SCMI_CLK_PFE_APB)] = S32G274A_CLK_PFE_SYS,
	[INDEX(S32G274A_SCMI_CLK_PFE_TS)] = S32GEN1_CLK_GMAC0_TS,
	/* PFE0 */
	[INDEX(S32G274A_SCMI_CLK_PFE0_RX_SGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32G274A_SCMI_CLK_PFE0_TX_SGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32G274A_SCMI_CLK_PFE0_RX_RGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32G274A_SCMI_CLK_PFE0_TX_RGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	/* PFE1 */
	[INDEX(S32G274A_SCMI_CLK_PFE1_RX_SGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32G274A_SCMI_CLK_PFE1_TX_SGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32G274A_SCMI_CLK_PFE1_RX_RGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32G274A_SCMI_CLK_PFE1_TX_RGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	/* PFE2 */
	[INDEX(S32G274A_SCMI_CLK_PFE2_RX_SGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32G274A_SCMI_CLK_PFE2_TX_SGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32G274A_SCMI_CLK_PFE2_RX_RGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32G274A_SCMI_CLK_PFE2_TX_RGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
};

static int s32g_compound2clkid(u32 scmi_clk_id, u32 *clk_id)
{
	switch (scmi_clk_id) {
	case S32G274A_SCMI_CLK_PFE0_RX_SGMII:
	case S32G274A_SCMI_CLK_PFE0_RX_RGMII:
		if (clk_id)
			*clk_id = S32G274A_CLK_PFE_MAC0_RX;
		break;
	case S32G274A_SCMI_CLK_PFE0_TX_SGMII:
	case S32G274A_SCMI_CLK_PFE0_TX_RGMII:
		if (clk_id)
			*clk_id = S32G274A_CLK_PFE_MAC0_TX_DIV;
		break;
	case S32G274A_SCMI_CLK_PFE1_RX_SGMII:
	case S32G274A_SCMI_CLK_PFE1_RX_RGMII:
		if (clk_id)
			*clk_id = S32G274A_CLK_PFE_MAC1_RX;
		break;
	case S32G274A_SCMI_CLK_PFE1_TX_SGMII:
	case S32G274A_SCMI_CLK_PFE1_TX_RGMII:
		if (clk_id)
			*clk_id = S32G274A_CLK_PFE_MAC1_TX;
		break;
	case S32G274A_SCMI_CLK_PFE2_RX_SGMII:
	case S32G274A_SCMI_CLK_PFE2_RX_RGMII:
		if (clk_id)
			*clk_id = S32G274A_CLK_PFE_MAC2_RX;
		break;
	case S32G274A_SCMI_CLK_PFE2_TX_SGMII:
	case S32G274A_SCMI_CLK_PFE2_TX_RGMII:
		if (clk_id)
			*clk_id = S32G274A_CLK_PFE_MAC2_TX;
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

	if (INDEX(scmi_clk_id) >= ARRAY_SIZE(s32g274a_scmi_ids))
		return -EINVAL;

	*clk_id = s32g274a_scmi_ids[INDEX(scmi_clk_id)];
	if (!*clk_id) {
		pr_err("Unhandled clock: %u\n", scmi_clk_id);
		return -EINVAL;
	}

	return 0;
}

int plat_compound_clk_get(struct clk *clk)
{
	u32 scmi_clk_id = clk->id;

	if (INDEX(scmi_clk_id) >= ARRAY_SIZE(s32g274a_scmi_ids))
		return -EINVAL;

	if (s32g_compound2clkid(scmi_clk_id, NULL)) {
		pr_err("Invalid compound clock : %u\n", scmi_clk_id);
		return -EINVAL;
	}

	return 0;
}

static int set_mac0_rx_parent(struct clk *clk)
{
	u32 rx_id;
	u32 clk_id = clk->id;

	if (clk_id == S32G274A_SCMI_CLK_PFE0_RX_SGMII) {
		rx_id = S32G274A_CLK_SERDES1_LANE0_CDR;
	} else if (clk_id == S32G274A_SCMI_CLK_PFE0_RX_RGMII) {
		rx_id = S32G274A_CLK_PFE_MAC0_EXT_RX;
	} else {
		pr_err("Invalid PFE0 RX mode\n");
		return -EINVAL;
	}

	return cc_set_mux_parent(clk, S32G274A_CLK_MC_CGM2_MUX4, rx_id);
}

static int set_mac0_tx_parent(struct clk *clk)
{
	u32 tx_id;
	u32 clk_id = clk->id;

	if (clk_id == S32G274A_SCMI_CLK_PFE0_TX_SGMII) {
		tx_id = S32G274A_CLK_SERDES1_LANE0_TX;
	} else if (clk_id == S32G274A_SCMI_CLK_PFE0_TX_RGMII) {
		tx_id = S32GEN1_CLK_PERIPH_PLL_PHI5;
	} else {
		pr_err("Invalid PFE0 TX mode\n");
		return -EINVAL;
	}

	return cc_set_mux_parent(clk, S32G274A_CLK_MC_CGM2_MUX1, tx_id);
}

static int set_mac1_rx_parent(struct clk *clk)
{
	u32 rx_id;
	u32 clk_id = clk->id;

	if (clk_id == S32G274A_SCMI_CLK_PFE1_RX_SGMII) {
		rx_id = S32G274A_CLK_SERDES1_LANE1_CDR;
	} else if (clk_id == S32G274A_SCMI_CLK_PFE1_RX_RGMII) {
		rx_id = S32G274A_CLK_PFE_MAC1_EXT_RX;
	} else {
		pr_err("Invalid PFE1 RX mode\n");
		return -EINVAL;
	}

	return cc_set_mux_parent(clk, S32G274A_CLK_MC_CGM2_MUX5, rx_id);
}

static int set_mac1_tx_parent(struct clk *clk)
{
	u32 tx_id;
	u32 clk_id = clk->id;

	if (clk_id == S32G274A_SCMI_CLK_PFE1_TX_SGMII) {
		tx_id = S32G274A_CLK_SERDES1_LANE1_TX;
	} else if (clk_id == S32G274A_SCMI_CLK_PFE1_TX_RGMII) {
		tx_id = S32GEN1_CLK_PERIPH_PLL_PHI5;
	} else {
		pr_err("Invalid PFE1 TX mode\n");
		return -EINVAL;
	}

	return cc_set_mux_parent(clk, S32G274A_CLK_MC_CGM2_MUX2, tx_id);
}

static int set_mac2_rx_parent(struct clk *clk)
{
	u32 rx_id;
	u32 clk_id = clk->id;

	if (clk_id == S32G274A_SCMI_CLK_PFE2_RX_SGMII) {
		rx_id = S32G274A_CLK_SERDES0_LANE1_CDR;
	} else if (clk_id == S32G274A_SCMI_CLK_PFE2_RX_RGMII) {
		rx_id = S32G274A_CLK_PFE_MAC2_EXT_RX;
	} else {
		pr_err("Invalid PFE2 RX mode\n");
		return -EINVAL;
	}

	return cc_set_mux_parent(clk, S32G274A_CLK_MC_CGM2_MUX6, rx_id);
}

static int set_mac2_tx_parent(struct clk *clk)
{
	u32 tx_id;
	u32 clk_id = clk->id;

	if (clk_id == S32G274A_SCMI_CLK_PFE2_TX_SGMII) {
		tx_id = S32G274A_CLK_SERDES0_LANE1_TX;
	} else if (clk_id == S32G274A_SCMI_CLK_PFE2_TX_RGMII) {
		tx_id = S32GEN1_CLK_PERIPH_PLL_PHI5;
	} else {
		pr_err("Invalid PFE2 TX mode\n");
		return -EINVAL;
	}

	return cc_set_mux_parent(clk, S32G274A_CLK_MC_CGM2_MUX3, tx_id);
}

int plat_compound_clk_set_parents(struct clk *clk)
{
	u32 clk_id = clk->id;
	u32 id;

	if (s32g_compound2clkid(clk_id, &id)) {
		pr_err("Invalid compound clock : %u\n", clk_id);
		return -EINVAL;
	}

	switch (id) {
	case S32G274A_CLK_PFE_MAC0_TX_DIV:
		return set_mac0_tx_parent(clk);
	case S32G274A_CLK_PFE_MAC0_RX:
		return set_mac0_rx_parent(clk);
	case S32G274A_CLK_PFE_MAC1_TX:
		return set_mac1_tx_parent(clk);
	case S32G274A_CLK_PFE_MAC1_RX:
		return set_mac1_rx_parent(clk);
	case S32G274A_CLK_PFE_MAC2_TX:
		return set_mac2_tx_parent(clk);
	case S32G274A_CLK_PFE_MAC2_RX:
		return set_mac2_rx_parent(clk);
	default:
		pr_err("%s: Invalid clock %d\n", __func__, id);
		return -EINVAL;
	}
}

int plat_compound_clk_enable(struct clk *clk)
{
	struct clk sclock = *clk;
	u32 clk_id = clk->id;
	u32 id;
	int ret;

	if (s32g_compound2clkid(clk_id, &id)) {
		pr_err("Invalid s32g274a compound clock : %u\n", clk_id);
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
		pr_err("Failed to enable %u clock\n", clk_id);
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

	if (INDEX(scmi_clk_id) >= ARRAY_SIZE(s32g274a_scmi_ids))
		return -EINVAL;

	ret = plat_compound_clk_set_parents(clk);
	if (ret) {
		pr_err("Failed to set parents for %u\n", scmi_clk_id);
		return -EINVAL;
	}

	if (s32g_compound2clkid(scmi_clk_id, &id)) {
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

