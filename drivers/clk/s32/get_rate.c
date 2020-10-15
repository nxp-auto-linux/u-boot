// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020 NXP
 */
#include <asm/arch/mc_cgm_regs.h>
#include <asm/io.h>
#include <dm/device.h>
#include <linux/printk.h>
#include <s32gen1_clk_funcs.h>

static ulong get_osc_freq(struct s32gen1_clk_obj *module,
			  struct s32gen1_clk_priv *priv)
{
	struct s32gen1_osc *osc = obj2osc(module);

	if (!osc->freq) {
		pr_err("Uninitialized oscillator\n");
		return 0;
	}
	return osc->freq;
}

static ulong get_clk_freq(struct s32gen1_clk_obj *module,
			  struct s32gen1_clk_priv *priv)
{
	struct s32gen1_clk *clk = obj2clk(module);

	if (!clk) {
		pr_err("Invalid clock\n");
		return 0;
	}

	if (clk->module)
		return get_module_rate(clk->module, priv);

	if (!clk->pclock) {
		pr_err("Invalid clock parent\n");
		return 0;
	}

	return get_clk_freq(&clk->pclock->desc, priv);
}

static ulong get_mux_freq(struct s32gen1_clk_obj *module,
			  struct s32gen1_clk_priv *priv)
{
	struct s32gen1_mux *mux = obj2mux(module);
	struct s32gen1_clk *clk = get_clock(mux->source_id);

	if (!clk) {
		pr_err("Mux without a valid source\n");
		return 0;
	}
	return get_clk_freq(&clk->desc, priv);
}

static ulong get_dfs_freq(struct s32gen1_clk_obj *module,
			  struct s32gen1_clk_priv *priv)
{
	struct s32gen1_dfs *dfs = obj2dfs(module);
	void *dfs_addr;
	u32 ctl;

	dfs_addr = get_base_addr(dfs->instance, priv);
	if (!dfs_addr) {
		pr_err("Failed to detect DFS instance\n");
		return 0;
	}

	ctl = readl(DFS_CTL(dfs_addr));
	/* Disabled DFS */
	if (ctl & DFS_CTL_RESET)
		return 0;

	return get_module_rate(dfs->source, priv);
}

static ulong get_dfs_div_freq(struct s32gen1_clk_obj *module,
			      struct s32gen1_clk_priv *priv)
{
	struct s32gen1_dfs_div *div = obj2dfsdiv(module);
	struct s32gen1_dfs *dfs;
	void *dfs_addr;
	u32 dvport, mfi, mfn;
	ulong pfreq;
	double freq;

	dfs = get_div_dfs(div);
	if (!dfs)
		return 0;

	pfreq = get_module_rate(div->parent, priv);
	if (!pfreq)
		return 0;

	dfs_addr = get_base_addr(dfs->instance, priv);
	if (!dfs_addr) {
		pr_err("Failed to detect DFS instance\n");
		return 0;
	}

	dvport = readl(DFS_DVPORTn(dfs_addr, div->index));

	mfi = DFS_DVPORTn_MFI(dvport);
	mfn = DFS_DVPORTn_MFN(dvport);

	/* Disabled port */
	if (!mfi && !mfn)
		return 0;

	/**
	 * Equation for input and output clocks of each port divider.
	 * See 'Digital Frequency Synthesizer' chapter from Reference Manual.
	 */
	freq = (double)pfreq / (2 * (mfi + (double)mfn / 36.0));

	return (ulong)freq;
}

static ulong get_pll_freq(struct s32gen1_clk_obj *module,
			  struct s32gen1_clk_priv *priv)
{
	struct s32gen1_pll *pll = obj2pll(module);
	struct s32gen1_clk *source;
	ulong prate;
	void *pll_addr;
	u32 pllpd;
	u32 mfi, mfn, rdiv, plldv;
	u32 clk_src;

	pll_addr = get_base_addr(pll->instance, priv);
	if (!pll_addr) {
		pr_err("Failed to detect PLL instance\n");
		return 0;
	}

	/* Disabled PLL */
	pllpd = readl(PLLDIG_PLLCR(pll_addr)) & PLLDIG_PLLCR_PLLPD;
	if (pllpd)
		return 0;

	clk_src = readl(PLLDIG_PLLCLKMUX(pll_addr));
	if (pllclk2clk(clk_src, &clk_src)) {
		pr_err("Failed to get PLL clock id\n");
		return -EINVAL;
	}

	source = get_clock(clk_src);
	if (!source) {
		pr_err("Failed to get PLL source clock\n");
		return 0;
	}

	prate = get_module_rate(&source->desc, priv);
	plldv = readl(PLLDIG_PLLDV(pll_addr));
	mfi = PLLDIG_PLLDV_MFI(plldv);
	rdiv = PLLDIG_PLLDV_RDIV(plldv);
	if (rdiv == 0)
		rdiv = 1;

	/* Frac-N mode */
	mfn = PLLDIG_PLLFD_MFN_SET(readl(PLLDIG_PLLFD(pll_addr)));
	/* PLL VCO frequency in Fractional mode when PLLDV[RDIV] is not 0 */
	return (ulong)((double)prate / rdiv * (mfi + ((double)mfn / 18432)));
}

static ulong get_fixed_clk_freq(struct s32gen1_clk_obj *module,
				struct s32gen1_clk_priv *priv)
{
	struct s32gen1_fixed_clock *clk = obj2fixedclk(module);

	return clk->freq;
}

static ulong get_fixed_div_freq(struct s32gen1_clk_obj *module,
				struct s32gen1_clk_priv *priv)
{
	struct s32gen1_fixed_div *div = obj2fixeddiv(module);
	ulong pfreq = get_module_rate(div->parent, priv);

	return (ulong)((double)pfreq / div->div);
}

static ulong get_pll_div_freq(struct s32gen1_clk_obj *module,
			      struct s32gen1_clk_priv *priv)
{
	struct s32gen1_pll_out_div *div = obj2plldiv(module);
	struct s32gen1_pll *pll;
	void *pll_addr;
	u32 pllodiv;
	ulong pfreq;
	u32 dc;

	pll = get_div_pll(div);
	if (!pll) {
		pr_err("The parent of the PLL DIV is invalid\n");
		return 0;
	}

	pll_addr = get_base_addr(pll->instance, priv);
	if (!pll_addr) {
		pr_err("Failed to detect PLL instance\n");
		return 0;
	}

	pfreq = get_module_rate(div->parent, priv);
	if (!pfreq) {
		pr_err("Failed to get the frequency of CGM MUX\n");
		return 0;
	}

	pllodiv = readl(PLLDIG_PLLODIV(pll_addr, div->index));
	/* Disabled module */
	if (!(pllodiv & PLLDIG_PLLODIV_DE))
		return 0;

	dc = PLLDIG_PLLODIV_DIV(pllodiv);
	return (ulong)((double)pfreq / (dc + 1));
}

static ulong get_part_block_freq(struct s32gen1_clk_obj *module,
				 struct s32gen1_clk_priv *priv)
{
	struct s32gen1_part_block *block = obj2partblock(module);

	return get_module_rate(block->parent, priv);
}

static ulong calc_cgm_div_freq(ulong pfreq, void *cgm_addr,
			       u32 mux, u32 div_index)
{
	u32 dc_val = readl(CGM_MUXn_DCm(cgm_addr, mux, div_index));
	u32 div;

	dc_val &= (MC_CGM_MUXn_DCm_DIV_MASK | MC_CGM_MUXn_DCm_DE);

	if (!(dc_val & MC_CGM_MUXn_DCm_DE))
		return 0;

	div = MC_CGM_MUXn_DCm_DIV_VAL(dc_val) + 1;
	return (ulong)((double)pfreq / div);
}

static ulong get_cgm_div_freq(struct s32gen1_clk_obj *module,
			      struct s32gen1_clk_priv *priv)
{
	struct s32gen1_cgm_div *div = obj2cgmdiv(module);
	struct s32gen1_mux *mux;
	void *cgm_addr;
	ulong pfreq;

	if (!div->parent) {
		pr_err("Failed to identify CGM divider's parent\n");
		return 0;
	}

	pfreq = get_module_rate(div->parent, priv);
	if (!pfreq) {
		pr_err("Failed to get the frequency of CGM MUX\n");
		return 0;
	}

	mux = get_cgm_div_mux(div);
	if (!mux)
		return -EINVAL;

	cgm_addr = get_base_addr(mux->module, priv);
	if (!cgm_addr) {
		pr_err("Failed to get CGM base address of the module %d\n",
		       mux->module);
	}

	return calc_cgm_div_freq(pfreq, cgm_addr, mux->index, div->index);
}

ulong get_module_rate(struct s32gen1_clk_obj *module,
		      struct s32gen1_clk_priv *priv)
{
	if (!module) {
		pr_err("Invalid module\n");
		return 0;
	}

	switch (module->type) {
	case s32gen1_shared_mux_t:
	case s32gen1_mux_t:
		return get_mux_freq(module, priv);
	case s32gen1_clk_t:
		return get_clk_freq(module, priv);
	case s32gen1_osc_t:
		return get_osc_freq(module, priv);
	case s32gen1_pll_t:
		return get_pll_freq(module, priv);
	case s32gen1_dfs_t:
		return get_dfs_freq(module, priv);
	case s32gen1_dfs_div_t:
		return get_dfs_div_freq(module, priv);
	case s32gen1_fixed_clk_t:
		return get_fixed_clk_freq(module, priv);
	case s32gen1_fixed_div_t:
		return get_fixed_div_freq(module, priv);
	case s32gen1_pll_out_div_t:
		return get_pll_div_freq(module, priv);
	case s32gen1_cgm_div_t:
		return get_cgm_div_freq(module, priv);
	case s32gen1_part_block_t:
		return get_part_block_freq(module, priv);
	};

	return 0;
}

ulong s32gen1_get_rate(struct clk *c)
{
	struct s32gen1_clk *clk;
	struct s32gen1_clk_priv *priv = dev_get_priv(c->dev);

	if (!c)
		return 0;

	clk = get_clock(c->id);
	if (!clk) {
		pr_err("Invalid clock\n");
		return 0;
	}

	if (clk->desc.type != s32gen1_clk_t) {
		pr_err("Invalid clock type: %d\n", clk->desc.type);
		return 0;
	}

	return get_module_rate(&clk->desc, priv);
}
