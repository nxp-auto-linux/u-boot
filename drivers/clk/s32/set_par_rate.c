// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020 NXP
 */
#include <command.h>
#include <log.h>
#include <linux/printk.h>
#include <s32gen1_clk_funcs.h>
#include <s32gen1_clk_modules.h>

static ulong set_module_rate(struct s32gen1_clk_obj *module, ulong rate);

static ulong set_pll_freq(struct s32gen1_clk_obj *module, ulong rate)
{
	struct s32gen1_pll *pll = obj2pll(module);

	if (pll->vco_freq && pll->vco_freq != rate) {
		pr_err("PLL frequency was already set\n");
		return 0;
	}

	pll->vco_freq = rate;
	return rate;
}

static ulong set_pll_div_freq(struct s32gen1_clk_obj *module, ulong rate)
{
	struct s32gen1_pll_out_div *div = obj2plldiv(module);
	struct s32gen1_pll *pll;

	if (!div->parent) {
		pr_err("Failed to identify PLL divider's parent\n");
		return 0;
	}

	pll = obj2pll(div->parent);
	if (!pll) {
		pr_err("The parent of the PLL DIV is invalid\n");
		return 0;
	}

	if (div->freq && div->freq != rate) {
		pr_err("PLL DIV frequency was already set to %lu\n", div->freq);
		return 0;
	}

	div->freq = rate;
	return rate;
}

static ulong set_dfs_div_freq(struct s32gen1_clk_obj *module, ulong rate)
{
	struct s32gen1_dfs_div *div = obj2dfsdiv(module);
	struct s32gen1_dfs *dfs;

	if (!div->parent) {
		pr_err("Failed to identify DFS divider's parent\n");
		return 0;
	}

	/* Sanity check */
	dfs = obj2dfs(div->parent);
	if (!dfs->source) {
		pr_err("Failed to identify DFS's parent\n");
		return 0;
	}

	if (div->freq && div->freq != rate) {
		pr_err("DFS DIV frequency was already set to %lu\n", div->freq);
		return 0;
	}

	div->freq = rate;
	return rate;
}

static ulong set_cgm_div_freq(struct s32gen1_clk_obj *module, ulong rate)
{
	struct s32gen1_cgm_div *div = obj2cgmdiv(module);

	if (!div->parent) {
		pr_err("Failed to identify DCGM divider's parent\n");
		return 0;
	}

	div->freq = rate;
	return rate;
}

static ulong set_clk_freq(struct s32gen1_clk_obj *module, ulong rate)
{
	struct s32gen1_clk *clk = obj2clk(module);

	if ((clk->min_freq && clk->max_freq) &&
	    (rate < clk->min_freq || rate > clk->max_freq)) {
		pr_err("%lu frequency is out of the allowed range: [%lu:%lu]\n",
		       rate, clk->min_freq, clk->max_freq);
		return 0;
	}

	if (clk->module)
		return set_module_rate(clk->module, rate);

	if (clk->pclock)
		return set_clk_freq(&clk->pclock->desc, rate);

	return 0;
}

static ulong set_fixed_div_freq(struct s32gen1_clk_obj *module, ulong rate)
{
	struct s32gen1_fixed_div *div = obj2fixeddiv(module);

	if (!div->parent) {
		pr_err("The divider doesn't have a valid parent\b");
		return 0;
	}

	return set_module_rate(div->parent, rate * div->div);
}

static ulong set_mux_freq(struct s32gen1_clk_obj *module, ulong rate)
{
	struct s32gen1_mux *mux = obj2mux(module);
	struct s32gen1_clk *clk = get_clock(mux->source_id);

	if (!clk) {
		pr_err("Mux without a valid source\n");
		return 0;
	}

	return set_module_rate(&clk->desc, rate);
}

static ulong set_osc_freq(struct s32gen1_clk_obj *module, ulong rate)
{
	struct s32gen1_osc *osc = obj2osc(module);

	if (osc->freq && rate != osc->freq) {
		pr_err("Already initialized oscillator. freq = %lu\n",
		       osc->freq);
		return 0;
	}

	osc->freq = rate;

	return osc->freq;
}

static ulong set_fixed_clk_freq(struct s32gen1_clk_obj *module, ulong rate)
{
	struct s32gen1_fixed_clock *fixed_clk = obj2fixedclk(module);

	if (fixed_clk->freq && rate != fixed_clk->freq) {
		pr_err("Already initialized clock. Current freq = %lu Req freq = %lu\n",
		       fixed_clk->freq, rate);
		return 0;
	}

	fixed_clk->freq = rate;

	return fixed_clk->freq;
}

static ulong set_part_block_freq(struct s32gen1_clk_obj *module, ulong rate)
{
	struct s32gen1_part_block *block = obj2partblock(module);

	if (!block->parent)
		pr_err("Partition block with no parent\n");

	return set_module_rate(block->parent, rate);
}

static ulong set_module_rate(struct s32gen1_clk_obj *module, ulong rate)
{
	switch (module->type) {
	case s32gen1_fixed_clk_t:
		return set_fixed_clk_freq(module, rate);
	case s32gen1_osc_t:
		return set_osc_freq(module, rate);
	case s32gen1_pll_t:
		return set_pll_freq(module, rate);
	case s32gen1_pll_out_div_t:
		return set_pll_div_freq(module, rate);
	case s32gen1_dfs_div_t:
		return set_dfs_div_freq(module, rate);
	case s32gen1_clk_t:
		return set_clk_freq(module, rate);
	case s32gen1_mux_t:
	case s32gen1_shared_mux_t:
		return set_mux_freq(module, rate);
	case s32gen1_fixed_div_t:
		return set_fixed_div_freq(module, rate);
	case s32gen1_part_block_t:
		return set_part_block_freq(module, rate);
	case s32gen1_cgm_div_t:
		return set_cgm_div_freq(module, rate);
	case s32gen1_dfs_t:
		pr_err("It's not allowed to set the frequency of a DFS !");
		return 0;
	};

	return 0;
}

ulong s32gen1_set_rate(struct clk *c, ulong rate)
{
	struct s32gen1_clk *clk;

	clk = get_clock(c->id);
	if (!clk)
		return 0;

	rate = set_module_rate(&clk->desc, rate);
	if (rate == 0)
		pr_err("Failed to set frequency for clock %ld\n", c->id);

	return rate;
}

static bool check_mux_source(struct s32gen1_mux *mux, uint32_t source_id)
{
	u8 i;

	for (i = 0; i < mux->nclks; i++) {
		if (mux->clkids[i] == source_id)
			return true;
	}

	return false;
}

static int update_frequency(struct clk *c, struct clk *p,
			    struct s32gen1_clk *clk, struct s32gen1_clk *parent)
{
	ulong rate;

	if (!(is_osc(parent) || is_fixed_clk(parent))) {
		pr_err("Unknown module type: %d\n", parent->desc.type);
		return -EINVAL;
	}

	rate = clk_get_rate(p);
	if (rate == 0) {
		pr_err("Failed to get the frequency of clock %lu\n", p->id);
		return -EINVAL;
	}

	if (set_module_rate(parent->module, rate) != rate)
		return -EINVAL;

	return 0;
}

int s32gen1_set_parent(struct clk *c, struct clk *p)
{
	struct s32gen1_clk *clk, *parent;
	struct s32gen1_mux *mux;
	int ret;

	if (!c || !p)
		return -EINVAL;

	clk = get_clock(c->id);
	parent = get_clock(p->id);

	if (!clk) {
		pr_err("Invalid clock\n");
		return -EINVAL;
	}

	if (!parent) {
		pr_err("Invalid parent\n");
		return -EINVAL;
	}

	/* The parent is a fixed /external clock */
	if (p->dev != c->dev && (is_fixed_clk(clk) || is_osc(clk))) {
		ret = update_frequency(c, p, clk, clk);
		if (ret)
			return ret;
		return 0;
	}

	if (!is_mux(clk)) {
		pr_err("Clock %ld is not a mux\n", c->id);
		return -EINVAL;
	}

	mux = clk2mux(clk);
	if (!mux) {
		pr_err("Failed to cast clock %ld to clock mux\n", c->id);
		return -EINVAL;
	}

	if (!check_mux_source(mux, p->id)) {
		pr_err("Clock %ld is not a valid clock for mux %ld\n",
		       p->id, c->id);
		return -EINVAL;
	}

	mux->source_id = p->id;
	return 0;
}
