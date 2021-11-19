// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020-2021 NXP
 */
#include <dt-bindings/clock/s32g-clock.h>
#include <dt-bindings/clock/s32gen1-clock.h>
#include <dt-bindings/clock/s32gen1-scmi-clock.h>
#include <errno.h>
#include <linux/kernel.h>
#include <s32gen1_clk_funcs.h>
#include <s32gen1_clk_utils.h>
#include <s32gen1_scmi_clk.h>

#define INDEX(X)	((X) - S32GEN1_SCMI_CLK_BASE_ID)

static u32 s32gen1_scmi_ids[] = {
	[INDEX(S32GEN1_SCMI_CLK_A53)] = S32GEN1_CLK_A53_CORE,
	[INDEX(S32GEN1_SCMI_CLK_SERDES_AXI)] = S32GEN1_CLK_XBAR,
	[INDEX(S32GEN1_SCMI_CLK_SERDES_AUX)] = S32GEN1_CLK_FIRC,
	[INDEX(S32GEN1_SCMI_CLK_SERDES_APB)] = S32GEN1_CLK_XBAR_DIV3,
	[INDEX(S32GEN1_SCMI_CLK_SERDES_REF)] = S32GEN1_CLK_SERDES_REF,
	[INDEX(S32GEN1_SCMI_CLK_FTM0_SYS)] = S32GEN1_CLK_PER,
	[INDEX(S32GEN1_SCMI_CLK_FTM0_EXT)] = S32GEN1_CLK_FTM0_REF,
	[INDEX(S32GEN1_SCMI_CLK_FTM1_SYS)] = S32GEN1_CLK_PER,
	[INDEX(S32GEN1_SCMI_CLK_FTM1_EXT)] = S32GEN1_CLK_FTM1_REF,
	[INDEX(S32GEN1_SCMI_CLK_FLEXCAN_REG)] = S32GEN1_CLK_XBAR_DIV3,
	[INDEX(S32GEN1_SCMI_CLK_FLEXCAN_SYS)] = S32GEN1_CLK_XBAR_DIV3,
	[INDEX(S32GEN1_SCMI_CLK_FLEXCAN_CAN)] = S32GEN1_CLK_CAN_PE,
	[INDEX(S32GEN1_SCMI_CLK_FLEXCAN_TS)] = S32GEN1_CLK_XBAR_DIV2,
	[INDEX(S32GEN1_SCMI_CLK_LINFLEX_XBAR)] = S32GEN1_CLK_LINFLEXD,
	[INDEX(S32GEN1_SCMI_CLK_LINFLEX_LIN)] = S32GEN1_CLK_LIN_BAUD,
	[INDEX(S32GEN1_SCMI_CLK_GMAC0_RX_SGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32GEN1_SCMI_CLK_GMAC0_TX_SGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32GEN1_SCMI_CLK_GMAC0_RX_RGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32GEN1_SCMI_CLK_GMAC0_TX_RGMII)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32GEN1_SCMI_CLK_GMAC0_TS)] = S32GEN1_SCMI_COMPLEX_CLK,
	[INDEX(S32GEN1_SCMI_CLK_GMAC0_AXI)] = S32GEN1_CLK_XBAR,
	[INDEX(S32GEN1_SCMI_CLK_SPI_REG)] = S32GEN1_CLK_SPI,
	[INDEX(S32GEN1_SCMI_CLK_SPI_MODULE)] = S32GEN1_CLK_SPI,
	[INDEX(S32GEN1_SCMI_CLK_QSPI_REG)] = S32GEN1_CLK_XBAR_DIV3,
	[INDEX(S32GEN1_SCMI_CLK_QSPI_AHB)] = S32GEN1_CLK_XBAR_DIV3,
	[INDEX(S32GEN1_SCMI_CLK_QSPI_FLASH2X)] = S32GEN1_CLK_QSPI_2X,
	[INDEX(S32GEN1_SCMI_CLK_QSPI_FLASH1X)] = S32GEN1_CLK_QSPI,
	[INDEX(S32GEN1_SCMI_CLK_USDHC_AHB)] = S32GEN1_CLK_XBAR,
	[INDEX(S32GEN1_SCMI_CLK_USDHC_MODULE)] = S32GEN1_CLK_XBAR_DIV3,
	[INDEX(S32GEN1_SCMI_CLK_USDHC_CORE)] = S32GEN1_CLK_SDHC,
	[INDEX(S32GEN1_SCMI_CLK_USDHC_MOD32K)] = S32GEN1_CLK_SIRC,
	[INDEX(S32GEN1_SCMI_CLK_DDR_REG)] = S32GEN1_CLK_XBAR_DIV3,
	[INDEX(S32GEN1_SCMI_CLK_DDR_PLL_REF)] = S32GEN1_CLK_DDR,
	[INDEX(S32GEN1_SCMI_CLK_DDR_AXI)] = S32GEN1_CLK_DDR,
	[INDEX(S32GEN1_SCMI_CLK_SRAM_AXI)] = S32GEN1_CLK_XBAR,
	[INDEX(S32GEN1_SCMI_CLK_SRAM_REG)] = S32GEN1_CLK_XBAR_DIV3,
	[INDEX(S32GEN1_SCMI_CLK_I2C_REG)] = S32GEN1_CLK_XBAR_DIV3,
	[INDEX(S32GEN1_SCMI_CLK_I2C_MODULE)] = S32GEN1_CLK_XBAR_DIV3,
	[INDEX(S32GEN1_SCMI_CLK_RTC_REG)] = S32GEN1_CLK_XBAR_DIV6,
	[INDEX(S32GEN1_SCMI_CLK_RTC_SIRC)] = S32GEN1_CLK_SIRC,
	[INDEX(S32GEN1_SCMI_CLK_RTC_FIRC)] = S32GEN1_CLK_FIRC,
	[INDEX(S32GEN1_SCMI_CLK_SIUL2_REG)] = S32GEN1_CLK_XBAR_DIV6,
	[INDEX(S32GEN1_SCMI_CLK_SIUL2_FILTER)] = S32GEN1_CLK_FIRC,
	[INDEX(S32GEN1_SCMI_CLK_CRC_REG)] = S32GEN1_CLK_XBAR_DIV3,
	[INDEX(S32GEN1_SCMI_CLK_CRC_MODULE)] = S32GEN1_CLK_XBAR_DIV3,
	[INDEX(S32GEN1_SCMI_CLK_EIM0_REG)] = S32GEN1_CLK_A53_CORE_DIV10,
	[INDEX(S32GEN1_SCMI_CLK_EIM0_MODULE)] = S32GEN1_CLK_A53_CORE_DIV10,
	[INDEX(S32GEN1_SCMI_CLK_EIM123_REG)] = S32GEN1_CLK_XBAR_DIV6,
	[INDEX(S32GEN1_SCMI_CLK_EIM123_MODULE)] = S32GEN1_CLK_XBAR_DIV6,
	[INDEX(S32GEN1_SCMI_CLK_EIM_REG)] = S32GEN1_CLK_XBAR_DIV6,
	[INDEX(S32GEN1_SCMI_CLK_EIM_MODULE)] = S32GEN1_CLK_XBAR_DIV6,
	[INDEX(S32GEN1_SCMI_CLK_FCCU_MODULE)] = S32GEN1_CLK_XBAR_DIV6,
	[INDEX(S32GEN1_SCMI_CLK_FCCU_SAFE)] = S32GEN1_CLK_FIRC,
};

static int compound2clkid(u32 scmi_clk_id, u32 *clk_id)
{
	switch (scmi_clk_id) {
	case S32GEN1_SCMI_CLK_GMAC0_RX_SGMII:
	case S32GEN1_SCMI_CLK_GMAC0_RX_RGMII:
		if (clk_id)
			*clk_id = S32GEN1_CLK_GMAC0_RX;
		break;
	case S32GEN1_SCMI_CLK_GMAC0_TX_SGMII:
	case S32GEN1_SCMI_CLK_GMAC0_TX_RGMII:
		if (clk_id)
			*clk_id = S32GEN1_CLK_GMAC0_TX;
		break;
	case S32GEN1_SCMI_CLK_GMAC0_TS:
		if (clk_id)
			*clk_id = S32GEN1_CLK_GMAC0_TS;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int cc_set_mux_parent(struct clk *clk, u32 mux_id, u32 mux_source)
{
	struct clk source = *clk;
	struct clk mux = *clk;
	int ret;

	source.id = mux_source;
	mux.id = mux_id;

	ret = s32gen1_set_parent(&mux, &source);
	if (ret) {
		pr_err("Failed to set cgm0_mux11 source\n");
		return -EINVAL;
	}

	return 0;
}

static int set_gmac_rx_parent(struct clk *clk)
{
	u32 rx_id, parent_id;
	u32 clk_id = clk->id;

	if (clk_id == S32GEN1_SCMI_CLK_GMAC0_RX_SGMII) {
		rx_id = S32GEN1_CLK_SERDES0_LANE0_CDR;
	} else if (clk_id == S32GEN1_SCMI_CLK_GMAC0_RX_RGMII) {
		rx_id = S32GEN1_CLK_GMAC0_EXT_RX;
	} else {
		pr_err("Invalid GMAC RX mode\n");
		return -EINVAL;
	}

	if (cc_compound_clk_get_pid(clk_id, &parent_id))
		return -EINVAL;

	return cc_set_mux_parent(clk, parent_id, rx_id);
}

static int set_gmac_tx_parent(struct clk *clk)
{
	u32 tx_id, parent_id;
	u32 clk_id = clk->id;

	if (clk_id == S32GEN1_SCMI_CLK_GMAC0_TX_RGMII) {
		tx_id = S32GEN1_CLK_PERIPH_PLL_PHI5;
	} else if (clk_id == S32GEN1_SCMI_CLK_GMAC0_TX_SGMII) {
		tx_id = S32GEN1_CLK_SERDES0_LANE0_TX;
	} else {
		pr_err("Invalid GMAC TX mode\n");
		return -EINVAL;
	}

	if (cc_compound_clk_get_pid(clk_id, &parent_id))
		return -EINVAL;

	return cc_set_mux_parent(clk, parent_id, tx_id);
}

static int set_gmac_ts_parent(struct clk *clk)
{
	u32 ts_id, parent_id;
	u32 clk_id = clk->id;

	if (clk_id == S32GEN1_SCMI_CLK_GMAC0_TS) {
		ts_id = S32GEN1_CLK_PERIPH_PLL_PHI4;
	} else {
		pr_err("Invalid GMAC TS mode\n");
		return -EINVAL;
	}

	if (cc_compound_clk_get_pid(clk_id, &parent_id))
		return -EINVAL;

	return cc_set_mux_parent(clk, parent_id, ts_id);
}

static int cc_compound_clk_set_parents(struct clk *clk)
{
	u32 clk_id = clk->id;
	u32 id;

	if (compound2clkid(clk_id, &id)) {
		pr_err("Invalid compound clock : %u\n", clk_id);
		return -EINVAL;
	}

	switch (id) {
	case S32GEN1_CLK_GMAC0_RX:
		return set_gmac_rx_parent(clk);
	case S32GEN1_CLK_GMAC0_TX:
		return set_gmac_tx_parent(clk);
	case S32GEN1_CLK_GMAC0_TS:
		return set_gmac_ts_parent(clk);
	default:
		return plat_compound_clk_set_parents(clk);
	}
}

int cc_scmi_id2clk(u32 scmi_clk_id, u32 *clk_id)
{
	if (!clk_id)
		return -EINVAL;

	if (scmi_clk_id >= S32GEN1_SCMI_PLAT_CLK_BASE_ID)
		return plat_scmi_id2clk(scmi_clk_id, clk_id);

	if (INDEX(scmi_clk_id) >= ARRAY_SIZE(s32gen1_scmi_ids))
		return -EINVAL;

	*clk_id = s32gen1_scmi_ids[INDEX(scmi_clk_id)];
	if (!*clk_id) {
		pr_err("Unhandled clock: %u\n", scmi_clk_id);
		return -EINVAL;
	}
	return 0;
}

int cc_compound_clk_get(struct clk *clk)
{
	u32 scmi_clk_id = clk->id;

	if (scmi_clk_id >= S32GEN1_SCMI_PLAT_CLK_BASE_ID)
		return plat_compound_clk_get(clk);

	if (INDEX(scmi_clk_id) >= ARRAY_SIZE(s32gen1_scmi_ids))
		return -EINVAL;

	if (compound2clkid(scmi_clk_id, NULL)) {
		pr_err("Invalid compound clock : %u\n", scmi_clk_id);
		return -EINVAL;
	}

	return 0;
}

ulong cc_compound_clk_set_rate(struct clk *clk, ulong rate)
{
	struct clk sclock = *clk;
	u32 scmi_clk_id = clk->id;
	u32 id;
	int ret;

	if (scmi_clk_id >= S32GEN1_SCMI_PLAT_CLK_BASE_ID)
		return plat_compound_clk_set_rate(clk, rate);

	if (INDEX(scmi_clk_id) >= ARRAY_SIZE(s32gen1_scmi_ids))
		return -EINVAL;

	ret = cc_compound_clk_set_parents(clk);
	if (ret) {
		pr_err("Failed to set parents for %u\n", scmi_clk_id);
		return -EINVAL;
	}

	if (compound2clkid(scmi_clk_id, &id)) {
		pr_err("Invalid compound clock : %u\n", scmi_clk_id);
		return -EINVAL;
	}

	sclock.id = id;
	return s32gen1_set_rate(&sclock, rate);
}

int cc_compound_clk_enable(struct clk *clk)
{
	struct clk sclock = *clk;
	u32 clk_id = clk->id;
	u32 id;
	int ret;

	if (clk_id >= S32GEN1_SCMI_PLAT_CLK_BASE_ID)
		return plat_compound_clk_enable(clk);

	if (compound2clkid(clk_id, &id)) {
		pr_err("Invalid compound clock : %u\n", clk_id);
		return -EINVAL;
	}

	ret = cc_compound_clk_set_parents(clk);
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

ulong cc_compound_clk_get_rate(struct clk *clk)
{
	printf("TODO: %s\n", __func__);
	return 0;
}

