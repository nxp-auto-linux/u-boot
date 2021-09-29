/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020-2021 NXP
 */
#ifndef S32GEN1_CLK_FUNCS_H
#define S32GEN1_CLK_FUNCS_H
#include <clk-uclass.h>
#include <inttypes.h>
#include <s32gen1_clk_modules.h>

struct s32gen1_clk *get_clock(uint32_t id);
struct s32gen1_clk *get_plat_clock(uint32_t id);
struct s32gen1_clk *get_plat_cc_clock(uint32_t id);
ulong s32gen1_set_rate(struct clk *c, ulong rate);
ulong s32gen1_plat_set_rate(struct clk *c, ulong rate);
int s32gen1_set_parent(struct clk *c, struct clk *p);
int s32gen1_enable(struct clk *c);

bool is_qspi1x_clk(uint32_t id);
bool is_qspi2x_clk(uint32_t id);
bool is_qspi_clk(uint32_t id);

ulong s32gen1_get_rate(struct clk *clk);
ulong get_module_rate(struct s32gen1_clk_obj *module,
		      struct s32gen1_clk_priv *priv);

void *get_base_addr(enum s32gen1_clk_source id, struct s32gen1_clk_priv *priv);

int pllclk2clk(u32 pll_clk_id, u32 *clk_id);
int get_pll_mfi_mfn(ulong pll_vco, ulong ref_freq, u32 *mfi, u32 *mfn);
uint32_t s32gen1_platclk2mux(uint32_t clk_id);

int cc_compound_clk_get_pid(u32 id, u32 *parent_id);

int s32gen1_get_early_clks_freqs(const struct siul2_freq_mapping **mapping);

#endif /* S32GEN1_CLK_FUNCS_H */
