// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020-2021 NXP
 */
#include <linux/bitops.h>
#include <asm/arch/mc_cgm_regs.h>
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/mc_rgm_regs.h>
#include <asm/io.h>
#include <command.h>
#include <dm/device.h>
#include <dt-bindings/clock/s32gen1-clock.h>
#include <linux/printk.h>
#include <log.h>
#include <s32gen1_clk_funcs.h>
#include <s32gen1_clk_modules.h>

static int enable_module(struct s32gen1_clk_obj *module,
			 struct s32gen1_clk_priv *priv);

static void setup_fxosc(struct s32gen1_clk_priv *priv)
{
	void *fxosc = priv->fxosc;

	/* According to "Initialization information" chapter from
	 * S32G274A Reference Manual, "Once FXOSC is turned ON, DO NOT change
	 * any signal (on the fly) which is going to analog module input.
	 * The inputs can be changed when the analog module is OFF...When
	 * disabling the IP through Software do not change any other values in
	 * the registers for at least 4 crystal clock cycles."
	 *
	 * Just make sure that FXOSC wasn't already started by BootROM.
	 */
	u32 ctrl;

	if (readl(FXOSC_CTRL(fxosc)) & FXOSC_CTRL_OSCON)
		return;

	ctrl = FXOSC_CTRL_COMP_EN;
	ctrl &= ~FXOSC_CTRL_OSC_BYP;
	ctrl |= FXOSC_CTRL_EOCV(0x1);
	ctrl |= FXOSC_CTRL_GM_SEL(0x7);
	writel(ctrl, FXOSC_CTRL(fxosc));

	/* Switch ON the crystal oscillator. */
	writel(FXOSC_CTRL_OSCON | readl(FXOSC_CTRL(fxosc)), FXOSC_CTRL(fxosc));

	/* Wait until the clock is stable. */
	while (!(readl(FXOSC_STAT(fxosc)) & FXOSC_STAT_OSC_STAT))
		;
}

static void mc_me_wait_update(u32 partition_n, u32 mask,
			      struct s32gen1_clk_priv *priv)
{
	void *mc_me = priv->mc_me;
	u32 pupd = readl(MC_ME_PRTN_N_PUPD(mc_me, partition_n));

	writel(pupd | mask, MC_ME_PRTN_N_PUPD(mc_me, partition_n));
	writel(MC_ME_CTL_KEY_KEY, MC_ME_CTL_KEY(mc_me));
	writel(MC_ME_CTL_KEY_INVERTEDKEY, MC_ME_CTL_KEY(mc_me));

	while (readl(MC_ME_PRTN_N_PUPD(mc_me, partition_n)) & mask)
		;
}

static void enable_partition(u32 partition_n, struct s32gen1_clk_priv *priv)
{
	void *mc_me = priv->mc_me;
	void *rdc = priv->rdc;
	void *rgm = priv->rgm;
	u32 rdc_ctrl;

	writel(readl(MC_ME_PRTN_N_PCONF(mc_me, partition_n)) | MC_ME_PRTN_N_PCE,
	       MC_ME_PRTN_N_PCONF(mc_me, partition_n));

	mc_me_wait_update(partition_n, MC_ME_PRTN_N_PCUD, priv);

	while (!(readl(MC_ME_PRTN_N_STAT(mc_me, partition_n)) &
	       MC_ME_PRTN_N_PCS))
		;

	/* Unlock RDC register write */
	rdc_ctrl = readl(RDC_RD_N_CTRL(rdc, partition_n));
	writel(rdc_ctrl | RD_CTRL_UNLOCK_MASK, RDC_RD_N_CTRL(rdc, partition_n));

	/* Enable the XBAR interface */
	rdc_ctrl = readl(RDC_RD_N_CTRL(rdc, partition_n));
	rdc_ctrl &= ~RDC_RD_INTERCONNECT_DISABLE;
	writel(rdc_ctrl, RDC_RD_N_CTRL(rdc, partition_n));

	/* Wait until XBAR interface enabled */
	while ((readl(RDC_RD_N_STATUS(rdc, partition_n)) &
		RDC_RD_INTERCONNECT_DISABLE_STAT))
		;

	/* Lift reset for partition */
	writel(readl(RGM_PRST(rgm, partition_n)) & (~PRST_PERIPH_n_RST(0)),
	       RGM_PRST(rgm, partition_n));

	/* Follow steps to clear OSSE bit */
	writel(readl(MC_ME_PRTN_N_PCONF(mc_me, partition_n)) &
			~MC_ME_PRTN_N_OSSE,
			MC_ME_PRTN_N_PCONF(mc_me, partition_n));

	mc_me_wait_update(partition_n, MC_ME_PRTN_N_OSSUD, priv);

	while (readl(MC_ME_PRTN_N_STAT(mc_me, partition_n)) &
			MC_ME_PRTN_N_OSSS)
		;

	while (readl(RGM_PSTAT(rgm, partition_n)) &
			PSTAT_PERIPH_n_STAT(0))
		;

	/* Lock RDC register write */
	writel(readl(RDC_RD_N_CTRL(rdc, partition_n)) & ~RD_CTRL_UNLOCK_MASK,
	       RDC_RD_N_CTRL(rdc, partition_n));
}

static void enable_part_cofb(u32 partition_n, u32 block,
			     struct s32gen1_clk_priv *priv,
			     bool check_status)
{
	void *mc_me = priv->mc_me;
	u32 block_mask = MC_ME_PRTN_N_REQ(block);
	u32 part_status;

	part_status = readl(MC_ME_PRTN_N_STAT(mc_me, partition_n));

	/* Enable a partition only if it's disabled */
	if (!(MC_ME_PRTN_N_PCS & part_status))
		enable_partition(partition_n, priv);

#ifndef CONFIG_TARGET_TYPE_S32GEN1_SIMULATOR
	writel(readl(MC_ME_PRTN_N_COFB0_CLKEN(mc_me, partition_n)) | block_mask,
	       MC_ME_PRTN_N_COFB0_CLKEN(mc_me, partition_n));

	writel(readl(MC_ME_PRTN_N_PCONF(mc_me, partition_n)) | MC_ME_PRTN_N_PCE,
	       MC_ME_PRTN_N_PCONF(mc_me, partition_n));

	mc_me_wait_update(partition_n, MC_ME_PRTN_N_PCUD, priv);

	if (check_status)
		while (!(readl(MC_ME_PRTN_N_COFB0_STAT(mc_me, partition_n)) &
			 block_mask))
			;
#endif
}

static int enable_clock(struct s32gen1_clk_obj *module,
			struct s32gen1_clk_priv *priv)
{
	struct s32gen1_clk *clk = obj2clk(module);

	if (clk->module)
		return enable_module(clk->module, priv);

	if (clk->pclock)
		return enable_clock(&clk->pclock->desc, priv);

	return -EINVAL;
}

static int enable_part_block(struct s32gen1_clk_obj *module,
			     struct s32gen1_clk_priv *priv)
{
	struct s32gen1_part_block *block = obj2partblock(module);
	u32 cofb;

	switch (block->block) {
	case s32gen1_part_block0 ... s32gen1_part_block15:
		cofb = block->block - s32gen1_part_block0;
		enable_part_cofb(block->partition, cofb,
				 priv, block->status);
		break;
	default:
		pr_err("Unknown partition block type: %d\n",
		       block->block);
		return -EINVAL;
	};

	return enable_module(block->parent, priv);
}

uint32_t s32gen1_platclk2mux(uint32_t clk_id)
{
	return clk_id - S32GEN1_CLK_ID_BASE;
}

static int cgm_mux_clk_config(void *cgm_addr, u32 mux, u32 source)
{
	u32 css, csc;

	css = readl(CGM_MUXn_CSS(cgm_addr, mux));

	/* Platform ID translation */
	source = s32gen1_platclk2mux(source);

	/* Already configured */
	if (MC_CGM_MUXn_CSS_SELSTAT(css) == source &&
	    MC_CGM_MUXn_CSS_SWTRG(css) == MC_CGM_MUXn_CSS_SWTRG_SUCCESS &&
	    !(css & MC_CGM_MUXn_CSS_SWIP))
		return 0;

	/* Ongoing clock switch? */
	while (readl(CGM_MUXn_CSS(cgm_addr, mux)) & MC_CGM_MUXn_CSS_SWIP)
		;

	csc = readl(CGM_MUXn_CSC(cgm_addr, mux));
	/* Clear previous source. */

	csc &= ~(MC_CGM_MUXn_CSC_SELCTL_MASK);

	/* Select the clock source and trigger the clock switch. */
	writel(csc | MC_CGM_MUXn_CSC_SELCTL(source) | MC_CGM_MUXn_CSC_CLK_SW,
	       CGM_MUXn_CSC(cgm_addr, mux));

	/* Wait for configuration bit to auto-clear. */
	while (readl(CGM_MUXn_CSC(cgm_addr, mux)) & MC_CGM_MUXn_CSC_CLK_SW)
		;

	/* Is the clock switch completed? */
	while (readl(CGM_MUXn_CSS(cgm_addr, mux)) & MC_CGM_MUXn_CSS_SWIP)
		;

	/*
	 * Check if the switch succeeded.
	 * Check switch trigger cause and the source.
	 */
	css = readl(CGM_MUXn_CSS(cgm_addr, mux));
	if ((MC_CGM_MUXn_CSS_SWTRG(css) == MC_CGM_MUXn_CSS_SWTRG_SUCCESS) &&
	    (MC_CGM_MUXn_CSS_SELSTAT(css) == source))
		return 0;

	pr_err("Failed to change the clock source of mux %d to %d (CGM = %p)\n",
	       mux, source, cgm_addr);

	return -EINVAL;
}

static int enable_cgm_mux(struct s32gen1_mux *mux,
			  struct s32gen1_clk_priv *priv)
{
	void *module_addr;

	module_addr = get_base_addr(mux->module, priv);

	if (!module_addr) {
		pr_err("Failed to get the base address of the module %d\n",
		       mux->module);
		return -EINVAL;
	}

	return cgm_mux_clk_config(module_addr, mux->index, mux->source_id);
}

static int enable_mux(struct s32gen1_clk_obj *module,
		      struct s32gen1_clk_priv *priv)
{
	int ret;
	struct s32gen1_mux *mux = obj2mux(module);
	struct s32gen1_clk *clk = get_clock(mux->source_id);

	if (!clk) {
		pr_err("Invalid parent (%d) for mux %d\n",
		       mux->source_id, mux->index);
		return -EINVAL;
	}

	ret = enable_module(&clk->desc, priv);

	if (ret)
		return ret;

	switch (mux->module) {
	/* PLL mux will be enabled by PLL setup */
	case S32GEN1_ACCEL_PLL:
	case S32GEN1_ARM_PLL:
	case S32GEN1_DDR_PLL:
	case S32GEN1_PERIPH_PLL:
		return 0;
	case S32GEN1_CGM0:
	case S32GEN1_CGM1:
	case S32GEN1_CGM2:
	case S32GEN1_CGM5:
	case S32GEN1_CGM6:
		return enable_cgm_mux(mux, priv);
	default:
		pr_err("Unknown mux parent type: %d\n", mux->module);
		return -EINVAL;
	};

	return -EINVAL;
}

static void cgm_mux_div_config(void *cgm_addr, u32 mux, u32 dc, u32 div_index)
{
	u32 updstat;
	u32 dc_val = readl(CGM_MUXn_DCm(cgm_addr, mux, div_index));

	dc_val &= (MC_CGM_MUXn_DCm_DIV_MASK | MC_CGM_MUXn_DCm_DE);

	if (dc_val == (MC_CGM_MUXn_DCm_DE | MC_CGM_MUXn_DCm_DIV(dc)))
		return;

	/* Set the divider */
	writel(MC_CGM_MUXn_DCm_DE | MC_CGM_MUXn_DCm_DIV(dc),
	       CGM_MUXn_DCm(cgm_addr, mux, div_index));

	/* Wait for divider gets updated */
	do {
		updstat = readl(CGM_MUXn_DIV_UPD_STAT(cgm_addr, mux));
	} while (MC_CGM_MUXn_DIV_UPD_STAT_DIVSTAT(updstat));
}

static int enable_cgm_div(struct s32gen1_clk_obj *module,
			  struct s32gen1_clk_priv *priv)
{
	struct s32gen1_cgm_div *div = obj2cgmdiv(module);
	struct s32gen1_mux *mux;
	void *cgm_addr;
	int ret;
	double pfreq;
	u32 dc;

	if (!div->parent) {
		pr_err("Failed to identify CGM divider's parent\n");
		return -EINVAL;
	}

	if (!div->freq) {
		pr_err("The frequency of the divider %d is not set\n",
		       div->index);
		return -EINVAL;
	}

	ret = enable_module(div->parent, priv);
	if (ret)
		return ret;

	pfreq = (double)get_module_rate(div->parent, priv);
	if (!pfreq) {
		pr_err("Failed to get the frequency of CGM MUX\n");
		return -EINVAL;
	}

	dc = (u32)(pfreq / div->freq);
	if ((ulong)(pfreq / dc) != div->freq) {
		pr_err("Cannot set CGM divider for input = %lu & output = %lu. Nearest freq = %lu\n",
		       (ulong)pfreq, div->freq, (ulong)(pfreq / dc));
#ifndef CONFIG_S32GEN1_SET_NEAREST_FREQ
		return -EINVAL;
#endif
	}

	mux = get_cgm_div_mux(div);
	if (!mux)
		return -EINVAL;

	cgm_addr = get_base_addr(mux->module, priv);
	if (!cgm_addr) {
		pr_err("Failed to get CGM base address of the module %d\n",
		       mux->module);
	}

	cgm_mux_div_config(cgm_addr, mux->index,
			   dc - 1, div->index);
	return ret;
}

static int get_dfs_mfi_mfn(ulong dfs_freq, struct s32gen1_dfs_div *div,
			   u32 *mfi, u32 *mfn)
{
	double dmfn;
	double div_freq;

	ulong in = dfs_freq;
	ulong out = div->freq;

	*mfi = in / out / 2;
	dmfn = ((double)in / (2 * out) - *mfi) * 36;
	*mfn = (u32)dmfn;
	div_freq = (double)in / (2 * (*mfi + (double)*mfn / 36.0));

	if ((ulong)div_freq != div->freq) {
		pr_err("Failed to find MFI and MFN settings for DFS DIV freq %lu. Nearest freq = %lu\n",
		       div->freq, (ulong)div_freq);
#ifndef CONFIG_S32GEN1_SET_NEAREST_FREQ
		return -EINVAL;
#endif
	}

	return 0;
}

static int init_dfs_port(void *dfs_addr, u32 port, u32 mfi, u32 mfn)
{
	u32 portsr, portreset, portolsr;
	bool init_dfs;
	u32 mask, old_mfi, old_mfn;
	u32 dvport = readl(DFS_DVPORTn(dfs_addr, port));

	old_mfi = DFS_DVPORTn_MFI(dvport);
	old_mfn = DFS_DVPORTn_MFN(dvport);

	portsr = readl(DFS_PORTSR(dfs_addr));
	portolsr = readl(DFS_PORTOLSR(dfs_addr));

	/* Skip configuration if it's not needed */
	if (portsr & BIT(port) && !(portolsr & BIT(port)) &&
	    mfi == old_mfi && mfn == old_mfn)
		return 0;

	init_dfs = (!portsr);

	if (init_dfs)
		mask = DFS_PORTRESET_PORTRESET_MAXVAL;
	else
		mask = DFS_PORTRESET_PORTRESET_SET(BIT(port));

	writel(mask, DFS_PORTOLSR(dfs_addr));
	writel(mask, DFS_PORTRESET(dfs_addr));

#ifndef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
	while (readl(DFS_PORTSR(dfs_addr)) & mask)
		;
#endif

	if (init_dfs)
		writel(DFS_CTL_RESET, DFS_CTL(dfs_addr));

	writel(DFS_DVPORTn_MFI_SET(mfi) | DFS_DVPORTn_MFN_SET(mfn),
	       DFS_DVPORTn(dfs_addr, port));

	if (init_dfs)
		/* DFS clk enable programming */
		writel(~DFS_CTL_RESET, DFS_CTL(dfs_addr));

	portreset = readl(DFS_PORTRESET(dfs_addr));
	portreset &= ~BIT(port);
	writel(portreset, DFS_PORTRESET(dfs_addr));

#ifndef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
	while ((readl(DFS_PORTSR(dfs_addr)) & BIT(port)) != BIT(port))
		;
#endif

	portolsr = readl(DFS_PORTOLSR(dfs_addr));
	if (portolsr & DFS_PORTOLSR_LOL(port)) {
		pr_err("Failed to lock DFS divider\n");
		return -EINVAL;
	}

	return 0;
}

static int enable_dfs_div(struct s32gen1_clk_obj *module,
			  struct s32gen1_clk_priv *priv)
{
	int ret;
	struct s32gen1_dfs_div *div = obj2dfsdiv(module);
	void *dfs_addr;
	struct s32gen1_dfs *dfs;
	u32 mfi, mfn;
	u32 ctl;
	ulong dfs_freq;

	ret = enable_module(div->parent, priv);
	if (ret)
		return ret;

	dfs = get_div_dfs(div);
	if (!dfs)
		return -EINVAL;

	if (!dfs->source) {
		pr_err("Failed to identify DFS divider's parent\n");
		return -EINVAL;
	}

	dfs_addr = get_base_addr(dfs->instance, priv);
	if (!dfs_addr)
		return -EINVAL;

	ctl = readl(DFS_CTL(dfs_addr));

	/* Enabled DFS */
	if (!(ctl & DFS_CTL_RESET))
		dfs_freq = get_module_rate(&dfs->desc, priv);
	else
		dfs_freq = get_module_rate(dfs->source, priv);

	if (!dfs_freq) {
		pr_err("Failed to get DFS's freqeuncy\n");
		return -EINVAL;
	}

	ret = get_dfs_mfi_mfn(dfs_freq, div, &mfi, &mfn);
	if (ret)
		return -EINVAL;

	return init_dfs_port(dfs_addr, div->index, mfi, mfn);
}

static int enable_dfs(struct s32gen1_clk_obj *module,
		      struct s32gen1_clk_priv *priv)
{
	struct s32gen1_dfs *dfs = obj2dfs(module);

	if (!dfs->source) {
		pr_err("Failed to identify DFS's parent\n");
		return -EINVAL;
	}

	return enable_module(dfs->source, priv);
}

static bool is_pll_enabled(void *pll_base)
{
	u32 pllcr, pllsr;

	pllcr = readl(PLLDIG_PLLCR(pll_base));
	pllsr = readl(PLLDIG_PLLSR(pll_base));

	/* Enabled and locked PLL */
	return !(pllcr & PLLDIG_PLLCR_PLLPD) && (pllsr & PLLDIG_PLLSR_LOCK);
}

int get_pll_mfi_mfn(ulong pll_vco, ulong ref_freq, u32 *mfi, u32 *mfn)

{
	double dmfn, vco;

	/* FRAC-N mode */
	*mfi = (pll_vco / ref_freq);

	dmfn = (double)(pll_vco % ref_freq) / ref_freq * 18432.0;
	*mfn = (u32)dmfn;

	/* Round MFN value */
	if (dmfn - (double)*mfn >= 0.5)
		*mfn += 1;

	vco = ref_freq * (*mfi + (double)*mfn / 18432.0);

	if ((ulong)vco != pll_vco) {
		pr_err("Failed to find MFI and MFN settings for PLL freq %lu. Nearest freq = %lu\n",
		       pll_vco, (ulong)vco);
#ifndef CONFIG_S32GEN1_SET_NEAREST_FREQ
		return -EINVAL;
#endif
	}

	return 0;
}

static void disable_pll_hw(void *pll_addr)
{
	writel(PLLDIG_PLLCR_PLLPD, PLLDIG_PLLCR(pll_addr));
}

static void enable_pll_hw(void *pll_addr)
{
	/* Enable the PLL. */
	writel(0x0, PLLDIG_PLLCR(pll_addr));

	/* Poll until PLL acquires lock. */
	while (!(readl(PLLDIG_PLLSR(pll_addr)) & PLLDIG_PLLSR_LOCK))
		;
}

static u32 get_enabled_odivs(void *pll_addr, u32 ndivs)
{
	u32 i;
	u32 mask = 0;
	u32 pllodiv;

	for (i = 0; i < ndivs; i++) {
		pllodiv = readl(PLLDIG_PLLODIV(pll_addr, i));
		if (pllodiv & PLLDIG_PLLODIV_DE)
			mask |= BIT(i);
	}

	return mask;
}

static void disable_odiv(void *pll_addr, u32 div_index)
{
	u32 pllodiv = readl(PLLDIG_PLLODIV(pll_addr, div_index));

	writel(pllodiv & ~PLLDIG_PLLODIV_DE,
	       PLLDIG_PLLODIV(pll_addr, div_index));
}

static void enable_odiv(void *pll_addr, u32 div_index)
{
	u32 pllodiv = readl(PLLDIG_PLLODIV(pll_addr, div_index));

	writel(pllodiv | PLLDIG_PLLODIV_DE,
	       PLLDIG_PLLODIV(pll_addr, div_index));
}

static void disable_odivs(void *pll_addr, u32 ndivs)
{
	u32 i;

	for (i = 0; i < ndivs; i++)
		disable_odiv(pll_addr, i);
}

static void enable_odivs(void *pll_addr, u32 ndivs, u32 mask)
{
	u32 i;

	for (i = 0; i < ndivs; i++) {
		if (mask & BIT(i))
			enable_odiv(pll_addr, i);
	}
}

static int adjust_odiv_settings(struct s32gen1_pll *pll, void *pll_addr,
				struct s32gen1_clk_priv *priv,
				u32 odivs_mask, ulong old_vco)
{
	u32 i, pllodiv, div;
	double old_odiv_freq, odiv_freq;
	int ret = 0;

	if (!old_vco)
		return 0;

	for (i = 0; i < pll->ndividers; i++) {
		if (!(odivs_mask & BIT(i)))
			continue;

		pllodiv = readl(PLLDIG_PLLODIV(pll_addr, i));

		div = PLLDIG_PLLODIV_DIV(pllodiv);
		old_odiv_freq = (double)old_vco / (div + 1);
		div = (u32)((double)pll->vco_freq / old_odiv_freq);
		odiv_freq = (double)pll->vco_freq / div;

		if (old_odiv_freq != odiv_freq) {
			pr_err("Failed to adjust ODIV %d to match previous frequency\n",
			       i);
			pr_err("Previous freq: %lu Nearest freq: %lu\n",
			       (ulong)old_odiv_freq, (ulong)odiv_freq);
		}

		pllodiv = PLLDIG_PLLODIV_DIV_SET(div - 1);
		writel(pllodiv, PLLDIG_PLLODIV(pll_addr, i));
	}

	return ret;
}

static int clk2pllclk(u32 clk_id, u32 *pll_clk_id)
{
	switch (clk_id) {
	case S32GEN1_CLK_FIRC:
		*pll_clk_id = 0;
		return 0;
	case S32GEN1_CLK_FXOSC:
		*pll_clk_id = 1;
		return 0;
	};

	return -EINVAL;
}

int pllclk2clk(u32 pll_clk_id, u32 *clk_id)
{
	switch (pll_clk_id) {
	case 0:
		*clk_id = S32GEN1_CLK_FIRC;
		return 0;
	case 1:
		*clk_id = S32GEN1_CLK_FXOSC;
		return 0;
	};

	return -EINVAL;
}

static int program_pll(struct s32gen1_pll *pll, void *pll_addr,
		       struct s32gen1_clk_priv *priv, u32 clk_src)
{
	ulong sfreq;
	struct s32gen1_clk *sclk = get_clock(clk_src);
	u32 rdiv = 1, mfi, mfn;
	int ret;
	u32 odivs_mask;
	ulong old_vco;

	if (!sclk)
		return -EINVAL;

	sfreq = get_module_rate(&sclk->desc, priv);
	if (!sfreq)
		return -EINVAL;

	ret = get_pll_mfi_mfn(pll->vco_freq, sfreq, &mfi, &mfn);
	if (ret)
		return -EINVAL;

	if (clk2pllclk(clk_src, &clk_src)) {
		pr_err("Failed to translate PLL clock\n");
		return -EINVAL;
	}

	odivs_mask = get_enabled_odivs(pll_addr, pll->ndividers);

	old_vco = get_module_rate(&pll->desc, priv);

	/* Disable ODIVs*/
	disable_odivs(pll_addr, pll->ndividers);

	/* Disable PLL */
	disable_pll_hw(pll_addr);

	/* Program PLLCLKMUX */
	writel(clk_src, PLLDIG_PLLCLKMUX(pll_addr));

	/* Program VCO */
	writel(PLLDIG_PLLDV_RDIV_SET(rdiv) | PLLDIG_PLLDV_MFI(mfi),
	       PLLDIG_PLLDV(pll_addr));
	writel(PLLDIG_PLLFD_MFN_SET(mfn) |
	       PLLDIG_PLLFD_SMDEN, PLLDIG_PLLFD(pll_addr));

	ret = adjust_odiv_settings(pll, pll_addr, priv, odivs_mask, old_vco);

	enable_pll_hw(pll_addr);

	/* Enable out dividers */
	enable_odivs(pll_addr, pll->ndividers, odivs_mask);

	return ret;
}

static struct s32gen1_mux *get_pll_mux(struct s32gen1_pll *pll)
{
	struct s32gen1_clk_obj *source = pll->source;
	struct s32gen1_clk *clk;

	if (!source) {
		pr_err("Failed to identify PLL's parent\n");
		return NULL;
	}

	if (source->type != s32gen1_clk_t) {
		pr_err("The parent of the PLL isn't a clock\n");
		return NULL;
	}

	clk = obj2clk(source);

	if (!clk->module) {
		pr_err("The clock isn't connected to a module\n");
		return NULL;
	}

	source = clk->module;

	if (source->type != s32gen1_mux_t &&
	    source->type != s32gen1_shared_mux_t) {
		pr_err("The parent of the PLL isn't a MUX\n");
		return NULL;
	}

	return obj2mux(source);
}

static int enable_pll(struct s32gen1_clk_obj *module,
		      struct s32gen1_clk_priv *priv)
{
	int ret;
	struct s32gen1_pll *pll = obj2pll(module);
	struct s32gen1_mux *mux;
	void *pll_addr;
	u32 clk_src;

	mux = get_pll_mux(pll);
	if (!mux)
		return -EINVAL;

	if (pll->instance != mux->module) {
		pr_err("MUX type is not in sync with PLL ID\n");
		return -EINVAL;
	}

	/* Enable MUX & OSC */
	ret = enable_module(pll->source, priv);
	if (ret)
		return ret;

	pll_addr = get_base_addr(pll->instance, priv);
	if (!pll_addr) {
		pr_err("Failed to detect PLL instance\n");
		return -EINVAL;
	}

	clk_src = readl(PLLDIG_PLLCLKMUX(pll_addr));
	if (pllclk2clk(clk_src, &clk_src)) {
		pr_err("Failed to get PLL clock id\n");
		return -EINVAL;
	}

	if (clk_src == mux->source_id &&
	    is_pll_enabled(pll_addr) &&
	    get_module_rate(module, priv) == pll->vco_freq) {
		return 0;
	}

	return program_pll(pll, pll_addr, priv, mux->source_id);
}

static void config_pll_out_div(void *pll_addr, u32 div_index, u32 dc)
{
	u32 pllodiv;
	u32 div;

	pllodiv = readl(PLLDIG_PLLODIV(pll_addr, div_index));
	div = PLLDIG_PLLODIV_DIV(pllodiv);

	if (div + 1 == dc && pllodiv & PLLDIG_PLLODIV_DE)
		return;

	if (pllodiv & PLLDIG_PLLODIV_DE)
		disable_odiv(pll_addr, div_index);

	pllodiv = PLLDIG_PLLODIV_DIV_SET(dc - 1);
	writel(pllodiv, PLLDIG_PLLODIV(pll_addr, div_index));

	enable_odiv(pll_addr, div_index);
}

static int enable_pll_div(struct s32gen1_clk_obj *module,
			  struct s32gen1_clk_priv *priv)
{
	int ret;
	struct s32gen1_pll_out_div *div = obj2plldiv(module);
	struct s32gen1_clk_obj *parent = div->parent;
	struct s32gen1_pll *pll;
	double pfreq;
	u32 dc;
	void *pll_addr;

	ret = enable_module(parent, priv);
	if (ret)
		return ret;

	pll = get_div_pll(div);
	if (!pll) {
		pr_err("The parent of the PLL DIV is invalid\n");
		return 0;
	}

	pll_addr = get_base_addr(pll->instance, priv);
	if (!pll_addr) {
		pr_err("Failed to detect PLL instance\n");
		return -EINVAL;
	}

	pfreq = (double)get_module_rate(parent, priv);
	if (!pfreq) {
		pr_err("Failed to get the frequency of CGM MUX\n");
		return -EINVAL;
	}

	dc = (u32)(pfreq / div->freq);
	if ((ulong)(pfreq / dc) != div->freq) {
		pr_err("Cannot set PLL divider for input = %lu & output = %lu. Nearest freq = %lu\n",
		       (ulong)pfreq, div->freq, (ulong)(pfreq / dc));
#ifndef CONFIG_S32GEN1_SET_NEAREST_FREQ
		return -EINVAL;
#endif
	}

	config_pll_out_div(pll_addr, div->index, dc);

	return 0;
}

static int enable_osc(struct s32gen1_clk_obj *module,
		      struct s32gen1_clk_priv *priv)
{
	struct s32gen1_osc *osc = obj2osc(module);

	switch (osc->source) {
	/* FIRC and SIRC oscillators are enabled by default */
	case S32GEN1_FIRC:
	case S32GEN1_SIRC:
		return 0;
	case S32GEN1_FXOSC:
		setup_fxosc(priv);
		return 0;
	default:
		pr_err("Invalid oscillator %d\n", osc->source);
		return -EINVAL;
	};

	return -EINVAL;
}

static int enable_fixed_div(struct s32gen1_clk_obj *module,
			    struct s32gen1_clk_priv *priv)
{
	struct s32gen1_fixed_div *pll = obj2fixeddiv(module);

	return enable_module(pll->parent, priv);
}

static int enable_module(struct s32gen1_clk_obj *module,
			 struct s32gen1_clk_priv *priv)
{
	switch (module->type) {
	case s32gen1_clk_t:
		return enable_clock(module, priv);
	case s32gen1_part_block_t:
		return enable_part_block(module, priv);
	case s32gen1_shared_mux_t:
	case s32gen1_mux_t:
		return enable_mux(module, priv);
	case s32gen1_cgm_div_t:
		return enable_cgm_div(module, priv);
	case s32gen1_dfs_div_t:
		return enable_dfs_div(module, priv);
	case s32gen1_dfs_t:
		return enable_dfs(module, priv);
	case s32gen1_pll_t:
		return enable_pll(module, priv);
	case s32gen1_osc_t:
		return enable_osc(module, priv);
	case s32gen1_fixed_clk_t:
		return 0;
	case s32gen1_fixed_div_t:
		return enable_fixed_div(module, priv);
	case s32gen1_pll_out_div_t:
		return enable_pll_div(module, priv);
	default:
		pr_err("Undefined module type: %d\n", module->type);
		return -EINVAL;
	};

	return -EINVAL;
}

int s32gen1_enable(struct clk *c)
{
	int ret;
	struct s32gen1_clk *clk;
	struct s32gen1_clk_priv *priv = dev_get_priv(c->dev);

	clk = get_clock(c->id);
	if (!clk) {
		pr_err("Clock %ld is not part of the clock tree\n", c->id);
		return 0;
	}

	ret = enable_module(&clk->desc, priv);
	if (ret)
		pr_err("Failed to enable clock: %ld\n", c->id);

	return ret;
}
