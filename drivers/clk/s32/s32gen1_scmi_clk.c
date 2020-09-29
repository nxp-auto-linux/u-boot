// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020 NXP
 */
#include <dt-bindings/clock/s32gen1-clock.h>
#include <dt-bindings/clock/s32gen1-scmi-clock.h>
#include <linux/printk.h>
#include <s32gen1_clk_funcs.h>
#include <s32gen1_scmi_clk.h>

static bool is_scmi_clk(uint32_t id)
{
	if (id >= S32GEN1_SCMI_CLK_BASE_ID && id < S32GEN1_CLK_ID_BASE)
		return true;

	return false;
}

int s32gen1_scmi_request(struct clk *clk)
{
	int ret;
	u32 clk_id;

	if (is_scmi_clk(clk->id)) {
		ret = cc_scmi_id2clk(clk->id, &clk_id);
		if (ret) {
			pr_err("Clock with ID %ld isn't covered by this driver\n",
			       clk->id);
			return -EINVAL;
		}

		if (clk_id == S32GEN1_SCMI_COMPLEX_CLK)
			return cc_compound_clk_get(clk);

		clk->id = clk_id;
	} else {
		clk_id = clk->id;
	}

	if (!get_clock(clk_id)) {
		pr_err("Clock %ld is not part of the clock tree\n", clk->id);
		return -EINVAL;
	}

	return 0;
}

ulong s32gen1_scmi_get_rate(struct clk *clk)
{
	int ret;
	u32 clk_id;

	if (is_scmi_clk(clk->id)) {
		ret = cc_scmi_id2clk(clk->id, &clk_id);
		if (ret) {
			pr_err("Clock with ID %ld isn't covered by this driver\n",
			       clk->id);
			return 0;
		}

		if (clk_id == S32GEN1_SCMI_COMPLEX_CLK)
			return cc_compound_clk_get_rate(clk);

		clk->id = clk_id;
	}

	return s32gen1_get_rate(clk);
}

ulong s32gen1_scmi_set_rate(struct clk *clk, ulong rate)
{
	int ret;
	u32 clk_id;

	if (is_scmi_clk(clk->id)) {
		ret = cc_scmi_id2clk(clk->id, &clk_id);
		if (ret) {
			pr_err("Clock with ID %ld isn't covered by this driver\n",
			       clk->id);
			return 0;
		}

		if (clk_id == S32GEN1_SCMI_COMPLEX_CLK)
			return cc_compound_clk_set_rate(clk, rate);

		clk->id = clk_id;
	}

	return s32gen1_set_rate(clk, rate);
}

int s32gen1_scmi_set_parent(struct clk *clk, struct clk *parent)
{
	if (is_scmi_clk(clk->id)) {
		pr_err("Is not allowed to set parents for SCMI clocks\n");
		return -EINVAL;
	}

	return s32gen1_set_parent(clk, parent);
}

int s32gen1_scmi_enable(struct clk *clk)
{
	int ret;
	u32 clk_id;

	if (is_scmi_clk(clk->id)) {
		ret = cc_scmi_id2clk(clk->id, &clk_id);
		if (ret) {
			pr_err("Clock with ID %ld isn't covered by this driver\n",
			       clk->id);
			return -1;
		}

		if (clk_id == S32GEN1_SCMI_COMPLEX_CLK)
			return cc_compound_clk_enable(clk);

		clk->id = clk_id;
	}

	return s32gen1_enable(clk);
}

int s32gen1_scmi_disable(struct clk *clk)
{
	/* Not implemented yet */
	return -EINVAL;
}
