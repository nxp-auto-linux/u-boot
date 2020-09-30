/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020 NXP
 */
#ifndef S32GEN1_SCMI_CLK_H
#define S32GEN1_SCMI_CLK_H

#include <clk.h>

int cc_scmi_id2clk(u32 scmi_clk_id, u32 *clk_id);
int cc_compound_clk_get(struct clk *clk);
ulong cc_compound_clk_get_rate(struct clk *clk);
ulong cc_compound_clk_set_rate(struct clk *clk, ulong rate);
int cc_compound_clk_enable(struct clk *clk);
int cc_set_mux_parent(struct clk *clk, u32 mux_id, u32 mux_source);

int plat_scmi_id2clk(u32 scmi_clk_id, u32 *clk_id);
int plat_compound_clk_get(struct clk *clk);
ulong plat_compound_clk_get_rate(struct clk *clk);
ulong plat_compound_clk_set_rate(struct clk *clk, ulong rate);
int plat_compound_clk_enable(struct clk *clk);
int plat_compound_clk_set_parents(struct clk *clk);

int s32gen1_scmi_request(struct clk *clk);
ulong s32gen1_scmi_get_rate(struct clk *clk);
ulong s32gen1_scmi_set_rate(struct clk *clk, ulong rate);
int s32gen1_scmi_set_parent(struct clk *clk, struct clk *parent);
int s32gen1_scmi_enable(struct clk *clk);
int s32gen1_scmi_disable(struct clk *clk);

#endif

