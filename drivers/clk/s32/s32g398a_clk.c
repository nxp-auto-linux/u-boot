// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2021 NXP
 */
#include <linux/types.h>
#include <s32gen1_scmi_clk.h>

ulong s32gen1_plat_set_rate(struct clk *c, ulong rate)
{
	if (s32gen1_scmi_request(c))
		return 0;

	return s32gen1_scmi_set_rate(c, rate);
}
