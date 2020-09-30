// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020 NXP
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
};

int plat_scmi_id2clk(u32 scmi_clk_id, u32 *clk_id)
{
	if (!clk_id)
		return -EINVAL;

	if (INDEX(scmi_clk_id) >= ARRAY_SIZE(s32r45_scmi_ids))
		return -EINVAL;

	*clk_id = s32r45_scmi_ids[INDEX(scmi_clk_id)];
	if (!*clk_id) {
		pr_err("Unhandled clock: %u\n", scmi_clk_id);
		return -EINVAL;
	}

	return 0;
}

int plat_compound_clk_get(struct clk *clk)
{
	return -EINVAL;
}

int plat_compound_clk_set_parents(struct clk *clk)
{
	return -EINVAL;
}

int plat_compound_clk_enable(struct clk *clk)
{
	return -EINVAL;
}

ulong plat_compound_clk_set_rate(struct clk *clk, ulong rate)
{
	return 0;
}

ulong plat_compound_clk_get_rate(struct clk *clk)
{
	return 0;
}

