// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020-2021 NXP
 */
#include <dm/device.h>
#include <dt-bindings/clock/s32gen1-clock.h>
#include <dt-bindings/clock/s32gen1-clock-freq.h>
#include <s32-gen1-regs.h>
#include <s32gen1_clk_funcs.h>
#include <asm/arch/siul.h>

#define CLK_INIT(ID)          \
{                             \
	.id = (ID),           \
	.dev = &fake_clk_dev, \
}

static struct s32gen1_clk_priv s32_priv = {
	.accelpll = (void *)ACCEL_PLL_BASE_ADDR,
	.armdfs = (void *)ARM_DFS_BASE_ADDR,
	.armpll = (void *)ARM_PLL_BASE_ADDR,
	.cgm0 = (void *)MC_CGM0_BASE_ADDR,
	.cgm1 = (void *)MC_CGM1_BASE_ADDR,
	.cgm2 = (void *)MC_CGM2_BASE_ADDR,
	.cgm5 = (void *)MC_CGM5_BASE_ADDR,
	.ddrpll = (void *)DRAM_PLL_BASE_ADDR,
	.fxosc = (void *)XOSC_BASE_ADDR,
	.mc_me = (void *)MC_ME_BASE_ADDR,
	.periphdfs = (void *)PERIPH_DFS_BASE_ADDR,
	.periphpll = (void *)PERIPH_PLL_BASE_ADDR,
	.rdc = (void *)RDC_BASE_ADDR,
	.rgm = (void *)MC_RGM_BASE_ADDR,
};

static struct udevice fake_clk_dev = {
	.priv = &s32_priv,
};

/* A53 clocks */
static struct clk fxosc = CLK_INIT(S32GEN1_CLK_FXOSC);
static struct clk arm_pll_mux = CLK_INIT(S32GEN1_CLK_ARM_PLL_MUX);
static struct clk arm_pll_vco = CLK_INIT(S32GEN1_CLK_ARM_PLL_VCO);
static struct clk arm_pll_phi0 = CLK_INIT(S32GEN1_CLK_ARM_PLL_PHI0);
static struct clk mc_cgm1_mux0 = CLK_INIT(S32GEN1_CLK_MC_CGM1_MUX0);
static struct clk a53_clk = CLK_INIT(S32GEN1_CLK_A53_CORE);

/* XBAR clock */
#ifdef CONFIG_HSE_SECBOOT
static struct clk firc = CLK_INIT(S32GEN1_CLK_FIRC);
#endif
static struct clk arm_dfs1 = CLK_INIT(S32GEN1_CLK_ARM_PLL_DFS1);
static struct clk mc_cgm0_mux0 = CLK_INIT(S32GEN1_CLK_MC_CGM0_MUX0);
static struct clk xbar_2x = CLK_INIT(S32GEN1_CLK_XBAR_2X);

/* LINFLEX clock */
#ifndef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
static struct clk periph_pll_mux = CLK_INIT(S32GEN1_CLK_PERIPH_PLL_MUX);
static struct clk periph_pll_vco = CLK_INIT(S32GEN1_CLK_PERIPH_PLL_VCO);
static struct clk periph_pll_phi3 = CLK_INIT(S32GEN1_CLK_PERIPH_PLL_PHI3);
static struct clk mc_cgm0_mux8 = CLK_INIT(S32GEN1_CLK_MC_CGM0_MUX8);
static struct clk lin_baud = CLK_INIT(S32GEN1_CLK_LIN_BAUD);
#endif

/* DDR clock */
static struct clk ddr_pll_mux = CLK_INIT(S32GEN1_CLK_DDR_PLL_MUX);
static struct clk ddr_pll_vco = CLK_INIT(S32GEN1_CLK_DDR_PLL_VCO);
static struct clk ddr_pll_phi0 = CLK_INIT(S32GEN1_CLK_DDR_PLL_PHI0);
static struct clk mc_cgm5_mux0 = CLK_INIT(S32GEN1_CLK_MC_CGM5_MUX0);
static struct clk ddr = CLK_INIT(S32GEN1_CLK_DDR);

#ifdef CONFIG_HSE_SECBOOT
static int secboot_xbar_to_firc(void)
{
	int ret;

	ret = s32gen1_set_parent(&mc_cgm0_mux0, &firc);
	if (ret)
		return ret;

	return s32gen1_enable(&xbar_2x);
}
#endif

static int enable_a53_clock(void)
{
	int ret;
	ulong rate;

	ret = s32gen1_set_parent(&arm_pll_mux, &fxosc);
	if (ret)
		return ret;

	ret = s32gen1_set_parent(&mc_cgm1_mux0, &arm_pll_phi0);
	if (ret)
		return ret;

	rate = s32gen1_set_rate(&fxosc, S32GEN1_FXOSC_FREQ);
	if (rate != S32GEN1_FXOSC_FREQ)
		return -EINVAL;

	rate = s32gen1_set_rate(&arm_pll_vco, S32GEN1_ARM_PLL_VCO_MAX_FREQ);
	if (rate != S32GEN1_ARM_PLL_VCO_MAX_FREQ)
		return -EINVAL;

	rate = s32gen1_set_rate(&a53_clk, S32GEN1_A53_MAX_FREQ);
	if (rate != S32GEN1_A53_MAX_FREQ)
		return -EINVAL;

	return s32gen1_enable(&a53_clk);
}

static int enable_xbar_clock(void)
{
	int ret;
	ulong rate;

	ret = s32gen1_set_parent(&mc_cgm0_mux0, &arm_dfs1);
	if (ret)
		return ret;

	rate = s32gen1_set_rate(&xbar_2x, S32GEN1_XBAR_2X_FREQ);
	if (rate != S32GEN1_XBAR_2X_FREQ)
		return -EINVAL;

	return s32gen1_enable(&xbar_2x);
}

#ifndef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
static int enable_lin_clock(void)
{
	int ret;
	ulong rate;

	ret = s32gen1_set_parent(&periph_pll_mux, &fxosc);
	if (ret)
		return ret;

	ret = s32gen1_set_parent(&mc_cgm0_mux8, &periph_pll_phi3);
	if (ret)
		return ret;

	rate = s32gen1_set_rate(&periph_pll_vco,
				S32GEN1_PERIPH_PLL_VCO_FREQ);
	if (rate != S32GEN1_PERIPH_PLL_VCO_FREQ)
		return -EINVAL;

	rate = s32gen1_set_rate(&lin_baud, S32GEN1_LIN_BAUD_CLK_FREQ);
	if (rate != S32GEN1_LIN_BAUD_CLK_FREQ)
		return -EINVAL;

	return s32gen1_enable(&lin_baud);
}
#endif

static int enable_ddr_clock(void)
{
	int ret;
	ulong rate;
	ulong ddr_pll_freq, ddr_freq;

	ddr_pll_freq = S32GEN1_DDR_PLL_VCO_FREQ;
	ddr_freq = S32GEN1_DDR_FREQ;

	ret = s32gen1_set_parent(&ddr_pll_mux, &fxosc);
	if (ret)
		return ret;

	ret = s32gen1_set_parent(&mc_cgm5_mux0, &ddr_pll_phi0);
	if (ret)
		return ret;

	rate = s32gen1_set_rate(&ddr_pll_vco, ddr_pll_freq);
	if (rate != ddr_pll_freq)
		return -EINVAL;

	rate = s32gen1_set_rate(&ddr, ddr_freq);
	if (rate != ddr_freq)
		return -EINVAL;

	return s32gen1_enable(&ddr);
}

int enable_early_clocks(void)
{
	int ret;

#ifdef CONFIG_HSE_SECBOOT
	ret = secboot_xbar_to_firc();
	if (ret)
		return ret;
#endif

	ret = enable_a53_clock();
	if (ret)
		return ret;

	ret = enable_xbar_clock();
	if (ret)
		return ret;

#ifndef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
	/* On emulator, changing the LIN clock frequency will
	 * make the print very slow.
	 */
	ret = enable_lin_clock();
	if (ret)
		return ret;
#endif

	return enable_ddr_clock();
}
