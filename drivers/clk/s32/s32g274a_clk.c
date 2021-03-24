// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020-2021 NXP
 */
#include <asm/arch/siul.h>
#include <dt-bindings/clock/s32g-clock.h>
#include <dt-bindings/clock/s32gen1-scmi-clock.h>
#include <s32gen1_clk_funcs.h>
#include <s32gen1_scmi_clk.h>

ulong s32gen1_plat_set_rate(struct clk *c, ulong rate)
{
	ulong qspi_max_rate;

	if (s32gen1_scmi_request(c))
		return 0;

	if (is_qspi_clk(c->id)) {
		if (is_qspi2x_clk(c->id))
			qspi_max_rate = S32G274A_REV1_QSPI_MAX_FREQ * 2;
		else
			qspi_max_rate = S32G274A_REV1_QSPI_MAX_FREQ;

		if (is_s32gen1_soc_rev1() && rate > qspi_max_rate)
			rate = qspi_max_rate;
	}

	return s32gen1_scmi_set_rate(c, rate);
}
