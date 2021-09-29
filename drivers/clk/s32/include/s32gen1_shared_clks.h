/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020-2021 NXP
 */
#ifndef S32GEN1_SHARED_CLKS_H
#define S32GEN1_SHARED_CLKS_H

extern struct s32gen1_clk cgm0_mux7_clk;
extern struct s32gen1_clk xbar_2x_clk;
extern struct s32gen1_cgm_div per_div;
extern struct s32gen1_clk cgm0_mux0_clk;
extern struct s32gen1_dfs armdfs;
extern struct s32gen1_clk serdes1_lane1_tx_clk;
extern struct s32gen1_clk serdes1_lane1_cdr_clk;
extern struct s32gen1_clk accel_pll_phi0_clk;
extern struct s32gen1_clk accel_pll_phi1_clk;
extern const struct siul2_freq_mapping siul2_clk_freq_map[];

#endif

