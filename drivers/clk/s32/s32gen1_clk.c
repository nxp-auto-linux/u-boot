// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020-2021 NXP
 */
#include <asm/arch/siul.h>
#include <dt-bindings/clock/s32gen1-clock.h>
#include <dt-bindings/clock/s32gen1-scmi-clock.h>
#include <s32gen1_clk_funcs.h>
#include <s32gen1_clk_modules.h>
#include <s32gen1_shared_clks.h>

/* Clock generation modules */
static struct s32gen1_osc fxosc =
		S32GEN1_OSC_INIT(S32GEN1_FXOSC);
static struct s32gen1_osc firc =
		S32GEN1_OSC_INIT(S32GEN1_FIRC);
static struct s32gen1_osc sirc =
		S32GEN1_OSC_INIT(S32GEN1_SIRC);

static struct s32gen1_fixed_clock serdes1_lane1_tx =
		S32GEN1_FIXED_CLK_INIT();
struct s32gen1_clk serdes1_lane1_tx_clk =
		S32GEN1_MODULE_CLK(serdes1_lane1_tx);
static struct s32gen1_fixed_clock serdes1_lane1_cdr =
		S32GEN1_FIXED_CLK_INIT();
struct s32gen1_clk serdes1_lane1_cdr_clk =
		S32GEN1_MODULE_CLK(serdes1_lane1_cdr);

/* Muxes */
static struct s32gen1_mux arm_pll_mux =
	S32GEN1_MUX_INIT(S32GEN1_ARM_PLL, 0, 2,
			 S32GEN1_CLK_FIRC,
			 S32GEN1_CLK_FXOSC);

static struct s32gen1_mux cgm1_mux0 =
	S32GEN1_SHARED_MUX_INIT(S32GEN1_CGM1, 0, 3,
				S32GEN1_CLK_FIRC,
				S32GEN1_CLK_ARM_PLL_PHI0,
				S32GEN1_CLK_ARM_PLL_DFS2);

static struct s32gen1_mux cgm0_mux0 =
	S32GEN1_SHARED_MUX_INIT(S32GEN1_CGM0, 0, 2,
				S32GEN1_CLK_FIRC,
				S32GEN1_CLK_ARM_PLL_DFS1);

/* Static part of the clock tree */

static struct s32gen1_clk firc_clk =
		S32GEN1_MODULE_CLK(firc);
static struct s32gen1_clk fxosc_clk =
		S32GEN1_MODULE_CLK(fxosc);
static struct s32gen1_clk sirc_clk =
		S32GEN1_MODULE_CLK(sirc);

/* ARM PLL */
static struct s32gen1_clk arm_pll_mux_clk =
		S32GEN1_MODULE_CLK(arm_pll_mux);

static struct s32gen1_pll armpll = {
	.desc = {
		.type = s32gen1_pll_t,
	},
	.ndividers = 2,
	.source = &arm_pll_mux_clk.desc,
	.instance = S32GEN1_ARM_PLL,
};

static struct s32gen1_clk arm_pll_vco_clk =
		S32GEN1_FREQ_MODULE_CLK(armpll, 1300 * MHZ,
					S32GEN1_ARM_PLL_VCO_MAX_FREQ);
static struct s32gen1_pll_out_div arm_pll_phi0_div =
		S32GEN1_PLL_OUT_DIV_INIT(armpll, 0);
static struct s32gen1_clk arm_pll_phi0_clk =
		S32GEN1_FREQ_MODULE_CLK(arm_pll_phi0_div, 0,
					S32GEN1_ARM_PLL_PHI0_MAX_FREQ);

/* ARM DFS */
struct s32gen1_dfs armdfs = {
	.desc = {
		.type = s32gen1_dfs_t,
	},
	.source = &armpll.desc,
	.instance = S32GEN1_ARM_DFS,
};

static struct s32gen1_dfs_div arm_dfs1_div =
		S32GEN1_DFS_DIV_INIT(armdfs, 0);
static struct s32gen1_clk arm_dfs1_clk =
		S32GEN1_FREQ_MODULE_CLK(arm_dfs1_div, 0, 800 * MHZ);

static struct s32gen1_dfs_div arm_dfs2_div =
		S32GEN1_DFS_DIV_INIT(armdfs, 1);
static struct s32gen1_clk arm_dfs2_clk =
		S32GEN1_FREQ_MODULE_CLK(arm_dfs2_div, 0, 800 * MHZ);

/* Peripherals' clocks */

/* ARM - MC_CGM_1 */
static struct s32gen1_clk cgm1_mux0_clk =
		S32GEN1_MODULE_CLK(cgm1_mux0);

/* A53_CORE */
static struct s32gen1_clk a53_core_clk =
		S32GEN1_FREQ_MODULE_CLK(cgm1_mux0_clk, 0,
					S32GEN1_A53_MAX_FREQ);

/* A53_CORE_DIV2 */
static struct s32gen1_fixed_div a53_core_div2 =
		S32GEN1_FIXED_DIV_INIT(cgm1_mux0_clk, 2);
static struct s32gen1_clk a53_core_div2_clk =
		S32GEN1_FREQ_MODULE_CLK(a53_core_div2, 0,
					S32GEN1_A53_MAX_FREQ / 2);
/* A53_CORE_DIV10 */
static struct s32gen1_fixed_div a53_core_div10 =
		S32GEN1_FIXED_DIV_INIT(cgm1_mux0_clk, 10);
static struct s32gen1_clk a53_core_div10_clk =
		S32GEN1_FREQ_MODULE_CLK(a53_core_div10, 0,
					S32GEN1_A53_MAX_FREQ / 10);

/* ARM - MC_CGM_0 */
struct s32gen1_clk cgm0_mux0_clk =
		S32GEN1_MODULE_CLK(cgm0_mux0);
/* XBAR */
static struct s32gen1_fixed_div xbar_div2 =
		S32GEN1_FIXED_DIV_INIT(cgm0_mux0_clk, 2);
static struct s32gen1_clk xbar_clk =
		S32GEN1_FREQ_MODULE_CLK(xbar_div2, 24 * MHZ, 400 * MHZ);
/* XBAR_DIV2 */
static struct s32gen1_fixed_div xbar_div4 =
		S32GEN1_FIXED_DIV_INIT(cgm0_mux0_clk, 4);
static struct s32gen1_clk xbar_div2_clk =
		S32GEN1_FREQ_MODULE_CLK(xbar_div4, 12 * MHZ, 200 * MHZ);
/* XBAR_DIV3 */
static struct s32gen1_fixed_div xbar_div6 =
		S32GEN1_FIXED_DIV_INIT(cgm0_mux0_clk, 6);
static struct s32gen1_clk xbar_div3_clk =
		S32GEN1_FREQ_MODULE_CLK(xbar_div6, 8 * MHZ, 133333333);
/* XBAR_DIV4 */
static struct s32gen1_fixed_div xbar_div8 =
		S32GEN1_FIXED_DIV_INIT(cgm0_mux0_clk, 8);
static struct s32gen1_clk xbar_div4_clk =
		S32GEN1_FREQ_MODULE_CLK(xbar_div8, 6 * MHZ, 100 * MHZ);
/* XBAR_DIV6 */
static struct s32gen1_fixed_div xbar_div12 =
		S32GEN1_FIXED_DIV_INIT(cgm0_mux0_clk, 12);
static struct s32gen1_clk xbar_div6_clk =
		S32GEN1_FREQ_MODULE_CLK(xbar_div12, 4 * MHZ, 66666666);

/* PERIPH PLL */
static struct s32gen1_mux periph_pll_mux =
	S32GEN1_MUX_INIT(S32GEN1_PERIPH_PLL, 0, 2,
			 S32GEN1_CLK_FIRC,
			 S32GEN1_CLK_FXOSC);
static struct s32gen1_clk periph_pll_mux_clk =
		S32GEN1_MODULE_CLK(periph_pll_mux);

static struct s32gen1_pll periphpll = {
	.desc = {
		.type = s32gen1_pll_t,
	},
	.ndividers = 8,
	.source = &periph_pll_mux_clk.desc,
	.instance = S32GEN1_PERIPH_PLL,
};

static struct s32gen1_clk periph_pll_vco_clk =
		S32GEN1_FREQ_MODULE_CLK(periphpll, 1300 * MHZ, 2000 * MHZ);

static struct s32gen1_pll_out_div periph_pll_phi0_div =
		S32GEN1_PLL_OUT_DIV_INIT(periphpll, 0);
static struct s32gen1_clk periph_pll_phi0_clk =
		S32GEN1_FREQ_MODULE_CLK(periph_pll_phi0_div,
					S32GEN1_PERIPH_PLL_PHI0_MIN_FREQ,
					125 * MHZ);

static struct s32gen1_pll_out_div periph_pll_phi1_div =
		S32GEN1_PLL_OUT_DIV_INIT(periphpll, 1);
static struct s32gen1_clk periph_pll_phi1_clk =
		S32GEN1_FREQ_MODULE_CLK(periph_pll_phi1_div, 0, 80 * MHZ);

static struct s32gen1_pll_out_div periph_pll_phi2_div =
		S32GEN1_PLL_OUT_DIV_INIT(periphpll, 2);
static struct s32gen1_clk periph_pll_phi2_clk =
		S32GEN1_FREQ_MODULE_CLK(periph_pll_phi2_div,
					S32GEN1_PERIPH_PLL_PHI2_MIN_FREQ,
					80 * MHZ);

static struct s32gen1_pll_out_div periph_pll_phi3_div =
		S32GEN1_PLL_OUT_DIV_INIT(periphpll, 3);
static struct s32gen1_clk periph_pll_phi3_clk =
		S32GEN1_FREQ_MODULE_CLK(periph_pll_phi3_div, 0, 133333333);

static struct s32gen1_pll_out_div periph_pll_phi4_div =
		S32GEN1_PLL_OUT_DIV_INIT(periphpll, 4);
static struct s32gen1_clk periph_pll_phi4_clk =
		S32GEN1_FREQ_MODULE_CLK(periph_pll_phi4_div, 0, 200 * MHZ);

static struct s32gen1_pll_out_div periph_pll_phi5_div =
		S32GEN1_PLL_OUT_DIV_INIT(periphpll, 5);
static struct s32gen1_clk periph_pll_phi5_clk =
		S32GEN1_FREQ_MODULE_CLK(periph_pll_phi5_div, 0, 500 * MHZ);

static struct s32gen1_pll_out_div periph_pll_phi7_div =
		S32GEN1_PLL_OUT_DIV_INIT(periphpll, 7);
static struct s32gen1_clk periph_pll_phi7_clk =
		S32GEN1_FREQ_MODULE_CLK(periph_pll_phi7_div, 0, 100 * MHZ);

/* PERIPH DFS */
static struct s32gen1_dfs periphdfs = {
	.desc = {
		.type = s32gen1_dfs_t,
	},
	.source = &periphpll.desc,
	.instance = S32GEN1_PERIPH_DFS,
};

static struct s32gen1_dfs_div periph_dfs1_div =
		S32GEN1_DFS_DIV_INIT(periphdfs, 0);
static struct s32gen1_clk periph_dfs1_clk =
		S32GEN1_FREQ_MODULE_CLK(periph_dfs1_div, 532 * MHZ, 800 * MHZ);

static struct s32gen1_dfs_div periph_dfs2_div =
		S32GEN1_DFS_DIV_INIT(periphdfs, 1);
static struct s32gen1_clk periph_dfs2_clk =
		S32GEN1_FREQ_MODULE_CLK(periph_dfs2_div, 0, 800 * MHZ);

static struct s32gen1_dfs_div periph_dfs3_div =
		S32GEN1_DFS_DIV_INIT(periphdfs, 2);
static struct s32gen1_clk periph_dfs3_clk =
		S32GEN1_FREQ_MODULE_CLK(periph_dfs3_div, 416 * MHZ, 800 * MHZ);

static struct s32gen1_dfs_div periph_dfs5_div =
		S32GEN1_DFS_DIV_INIT(periphdfs, 4);
static struct s32gen1_clk periph_dfs5_clk =
		S32GEN1_FREQ_MODULE_CLK(periph_dfs5_div, 0, 800 * MHZ);

/* PERIPH - CGM0 */
/* SERDES_REF_CLK */
static struct s32gen1_clk serdes_ref_clk =
		S32GEN1_CHILD_CLK(periph_pll_phi0_clk, 100 * MHZ, 125 * MHZ);

/* PER_CLK */
static struct s32gen1_mux cgm0_mux3 =
		S32GEN1_MUX_INIT(S32GEN1_CGM0, 3, 2,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI1);
static struct s32gen1_clk cgm0_mux3_clk =
		S32GEN1_MODULE_CLK(cgm0_mux3);
struct s32gen1_cgm_div per_div =
		S32GEN1_CGM_DIV_INIT(cgm0_mux3_clk, 0);

/* FTM_0_REF_CLK */
static struct s32gen1_fixed_clock ftm0_ext =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk ftm0_ext_clk =
		S32GEN1_MODULE_CLK(ftm0_ext);
static struct s32gen1_mux cgm0_mux4 =
		S32GEN1_MUX_INIT(S32GEN1_CGM0, 4, 3,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI1,
				 S32GEN1_CLK_FTM0_EXT_REF);
static struct s32gen1_clk cgm0_mux4_clk =
		S32GEN1_MODULE_CLK(cgm0_mux4);
static struct s32gen1_cgm_div ftm0_div =
		S32GEN1_CGM_DIV_INIT(cgm0_mux4_clk, 0);
static struct s32gen1_clk ftm0_ref_clk =
		S32GEN1_FREQ_MODULE_CLK(ftm0_div, 0, 40 * MHZ);

/* FTM_1_REF_CLK */
static struct s32gen1_fixed_clock ftm1_ext =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk ftm1_ext_clk =
		S32GEN1_MODULE_CLK(ftm1_ext);
static struct s32gen1_mux cgm0_mux5 =
		S32GEN1_MUX_INIT(S32GEN1_CGM0, 5, 3,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI1,
				 S32GEN1_CLK_FTM1_EXT_REF);
static struct s32gen1_clk cgm0_mux5_clk =
		S32GEN1_MODULE_CLK(cgm0_mux5);
static struct s32gen1_cgm_div ftm1_div =
		S32GEN1_CGM_DIV_INIT(cgm0_mux5_clk, 0);
static struct s32gen1_clk ftm1_ref_clk =
		S32GEN1_FREQ_MODULE_CLK(ftm1_div, 0, 40 * MHZ);

/* FLEXRAY_PE_CLK */
static struct s32gen1_mux cgm0_mux6 =
		S32GEN1_MUX_INIT(S32GEN1_CGM0, 6, 3,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI1,
				 S32GEN1_CLK_FXOSC);
static struct s32gen1_clk cgm0_mux6_clk =
		S32GEN1_MODULE_CLK(cgm0_mux6);
static struct s32gen1_cgm_div flexray_pe_div =
		S32GEN1_CGM_DIV_INIT(cgm0_mux6_clk, 0);
static struct s32gen1_clk flexray_pe_clk =
		S32GEN1_FREQ_MODULE_CLK(flexray_pe_div, 0, 40 * MHZ);

/* CAN_PE_CLK */
static struct s32gen1_mux cgm0_mux7 =
		S32GEN1_MUX_INIT(S32GEN1_CGM0, 7, 3,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI2,
				 S32GEN1_CLK_FXOSC);
struct s32gen1_clk cgm0_mux7_clk =
		S32GEN1_MODULE_CLK(cgm0_mux7);

/* LIN_BAUD_CLK */
static struct s32gen1_mux cgm0_mux8 =
		S32GEN1_MUX_INIT(S32GEN1_CGM0, 8, 3,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI3,
				 S32GEN1_CLK_FXOSC);
static struct s32gen1_clk cgm0_mux8_clk =
		S32GEN1_MODULE_CLK(cgm0_mux8);
static struct s32gen1_clk lin_baud_clk =
		S32GEN1_CHILD_CLK(cgm0_mux8_clk, 0, 133333333);

static struct s32gen1_fixed_div linflexd_div =
		S32GEN1_FIXED_DIV_INIT(lin_baud_clk, 2);
static struct s32gen1_clk linflexd_clk =
		S32GEN1_FREQ_MODULE_CLK(linflexd_div, 0, 66666666);

/* S32GEN1_CLK_SERDES0_LANE0_TX */
static struct s32gen1_fixed_clock serdes0_lane0_tx =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk serdes0_lane0_tx_clk =
		S32GEN1_MODULE_CLK(serdes0_lane0_tx);

static struct s32gen1_fixed_clock serdes0_lane0_cdr =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk serdes0_lane0_cdr_clk =
		S32GEN1_MODULE_CLK(serdes0_lane0_cdr);

/* SPI_CLK */
static struct s32gen1_mux cgm0_mux16 =
		S32GEN1_MUX_INIT(S32GEN1_CGM0, 16, 2,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI7);
static struct s32gen1_clk cgm0_mux16_clk =
		S32GEN1_MODULE_CLK(cgm0_mux16);
static struct s32gen1_clk spi_clk =
		S32GEN1_CHILD_CLK(cgm0_mux16_clk, 10 * MHZ, 100 * MHZ);

/* QSPI_CLK */
static struct s32gen1_mux cgm0_mux12 =
		S32GEN1_SHARED_MUX_INIT(S32GEN1_CGM0, 12, 2,
					S32GEN1_CLK_FIRC,
					S32GEN1_CLK_PERIPH_PLL_DFS1);
static struct s32gen1_clk cgm0_mux12_clk =
		S32GEN1_MODULE_CLK(cgm0_mux12);
static struct s32gen1_cgm_div qspi_div =
		S32GEN1_CGM_DIV_INIT(cgm0_mux12_clk, 0);
static struct s32gen1_clk qspi_2x_clk =
		S32GEN1_FREQ_MODULE_CLK(qspi_div, 0, S32GEN1_QSPI_MAX_FREQ * 2);
static struct s32gen1_fixed_div qspi_div2 =
		S32GEN1_FIXED_DIV_INIT(qspi_2x_clk, 2);
static struct s32gen1_clk qspi_clk =
		S32GEN1_FREQ_MODULE_CLK(qspi_div2, 0, S32GEN1_QSPI_MAX_FREQ);

/* SDHC_CLK */
static struct s32gen1_mux cgm0_mux14 =
		S32GEN1_MUX_INIT(S32GEN1_CGM0, 14, 2,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_DFS3);
static struct s32gen1_clk cgm0_mux14_clk =
		S32GEN1_MODULE_CLK(cgm0_mux14);
static struct s32gen1_cgm_div sdhc_div =
		S32GEN1_CGM_DIV_INIT(cgm0_mux14_clk, 0);
static struct s32gen1_part_block sdhc_block =
		S32GEN1_PART_BLOCK(sdhc_div, 0, s32gen1_part_block0);
static struct s32gen1_clk sdhc_clk =
		S32GEN1_FREQ_MODULE_CLK(sdhc_block, 0, 400 * MHZ);

/* DDR PLL */
static struct s32gen1_mux ddr_pll_mux =
	S32GEN1_MUX_INIT(S32GEN1_DDR_PLL, 0, 2,
			 S32GEN1_CLK_FIRC,
			 S32GEN1_CLK_FXOSC);
static struct s32gen1_clk ddr_pll_mux_clk =
		S32GEN1_MODULE_CLK(ddr_pll_mux);

static struct s32gen1_pll ddrpll = {
	.desc = {
		.type = s32gen1_pll_t,
	},
	.ndividers = 1,
	.source = &ddr_pll_mux_clk.desc,
	.instance = S32GEN1_DDR_PLL,
};

static struct s32gen1_clk ddr_pll_vco_clk =
		S32GEN1_FREQ_MODULE_CLK(ddrpll, 1300 * MHZ, 1600 * MHZ);

static struct s32gen1_pll_out_div ddr_pll_phi0_div =
		S32GEN1_PLL_OUT_DIV_INIT(ddrpll, 0);
static struct s32gen1_clk ddr_pll_phi0_clk =
		S32GEN1_FREQ_MODULE_CLK(ddr_pll_phi0_div, 0, 800 * MHZ);

/* DDR_CLK */
static struct s32gen1_mux cgm5_mux0 =
		S32GEN1_MUX_INIT(S32GEN1_CGM5, 0, 2,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_DDR_PLL_PHI0);
static struct s32gen1_clk cgm5_mux0_clk =
		S32GEN1_MODULE_CLK(cgm5_mux0);
static struct s32gen1_part_block ddr_block =
		S32GEN1_PART_BLOCK(cgm5_mux0_clk, 0, s32gen1_part_block1);
static struct s32gen1_clk ddr_clk =
		S32GEN1_FREQ_MODULE_CLK(ddr_block, 0, 800 * MHZ);

/* ACCEL PLL */
static struct s32gen1_mux accel_pll_mux =
	S32GEN1_MUX_INIT(S32GEN1_ACCEL_PLL, 0, 2,
			 S32GEN1_CLK_FIRC,
			 S32GEN1_CLK_FXOSC);
static struct s32gen1_clk accel_pll_mux_clk =
		S32GEN1_MODULE_CLK(accel_pll_mux);

static struct s32gen1_pll accelpll = {
	.desc = {
		.type = s32gen1_pll_t,
	},
	.ndividers = 2,
	.source = &accel_pll_mux_clk.desc,
	.instance = S32GEN1_ACCEL_PLL,
};

static struct s32gen1_clk accel_pll_vco_clk =
		S32GEN1_FREQ_MODULE_CLK(accelpll, 1300 * MHZ, 2400 * MHZ);

static struct s32gen1_pll_out_div accel_pll_phi0_div =
		S32GEN1_PLL_OUT_DIV_INIT(accelpll, 0);
struct s32gen1_clk accel_pll_phi0_clk =
		S32GEN1_FREQ_MODULE_CLK(accel_pll_phi0_div, 0, 600 * MHZ);
static struct s32gen1_pll_out_div accel_pll_phi1_div =
		S32GEN1_PLL_OUT_DIV_INIT(accelpll, 1);
struct s32gen1_clk accel_pll_phi1_clk =
		S32GEN1_FREQ_MODULE_CLK(accel_pll_phi1_div, 0, 600 * MHZ);

/* CLKOUT */
static struct s32gen1_mux cgm0_mux1 =
		S32GEN1_MUX_INIT(S32GEN1_CGM0, 1, 4,
				 S32GEN1_CLK_FXOSC,
				 S32GEN1_CLK_PERIPH_PLL_PHI0,
				 S32GEN1_CLK_PERIPH_PLL_DFS2,
				 S32GEN1_CLK_PERIPH_PLL_DFS5);
static struct s32gen1_clk cgm0_mux1_clk =
		S32GEN1_MODULE_CLK(cgm0_mux1);
static struct s32gen1_cgm_div clkout0_div =
		S32GEN1_CGM_DIV_INIT(cgm0_mux1_clk, 0);
static struct s32gen1_clk clkout0_clk =
		S32GEN1_MODULE_CLK(clkout0_div);

static struct s32gen1_mux cgm0_mux2 =
		S32GEN1_MUX_INIT(S32GEN1_CGM0, 2, 4,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI0,
				 S32GEN1_CLK_PERIPH_PLL_DFS2,
				 S32GEN1_CLK_PERIPH_PLL_DFS5);
static struct s32gen1_clk cgm0_mux2_clk =
		S32GEN1_MODULE_CLK(cgm0_mux2);
static struct s32gen1_cgm_div clkout1_div =
		S32GEN1_CGM_DIV_INIT(cgm0_mux2_clk, 0);
static struct s32gen1_clk clkout1_clk =
		S32GEN1_MODULE_CLK(clkout1_div);

static struct s32gen1_clk *plat_clocks[] = {
	/* Oscillators */
	[CC_ARR_CLK(S32GEN1_CLK_FIRC)] = &firc_clk,
	[CC_ARR_CLK(S32GEN1_CLK_SIRC)] = &sirc_clk,
	[CC_ARR_CLK(S32GEN1_CLK_FXOSC)] = &fxosc_clk,
	/* ARM PLL */
	[CC_ARR_CLK(S32GEN1_CLK_ARM_PLL_MUX)] = &arm_pll_mux_clk,
	[CC_ARR_CLK(S32GEN1_CLK_ARM_PLL_VCO)] = &arm_pll_vco_clk,
	[CC_ARR_CLK(S32GEN1_CLK_ARM_PLL_PHI0)] = &arm_pll_phi0_clk,
	/* ARM DFS */
	[CC_ARR_CLK(S32GEN1_CLK_ARM_PLL_DFS1)] = &arm_dfs1_clk,
	[CC_ARR_CLK(S32GEN1_CLK_ARM_PLL_DFS2)] = &arm_dfs2_clk,
	/* ARM - MC_CGM1 */
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM1_MUX0)] = &cgm1_mux0_clk,
	[CC_ARR_CLK(S32GEN1_CLK_A53_CORE)] = &a53_core_clk,
	[CC_ARR_CLK(S32GEN1_CLK_A53_CORE_DIV2)] = &a53_core_div2_clk,
	[CC_ARR_CLK(S32GEN1_CLK_A53_CORE_DIV10)] = &a53_core_div10_clk,
	/* ARM - MC_CGM0 */
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM0_MUX0)] = &cgm0_mux0_clk,
	[CC_ARR_CLK(S32GEN1_CLK_XBAR_2X)] = &xbar_2x_clk,
	[CC_ARR_CLK(S32GEN1_CLK_XBAR)] = &xbar_clk,
	[CC_ARR_CLK(S32GEN1_CLK_XBAR_DIV2)] = &xbar_div2_clk,
	[CC_ARR_CLK(S32GEN1_CLK_XBAR_DIV3)] = &xbar_div3_clk,
	[CC_ARR_CLK(S32GEN1_CLK_XBAR_DIV4)] = &xbar_div4_clk,
	[CC_ARR_CLK(S32GEN1_CLK_XBAR_DIV6)] = &xbar_div6_clk,
	/* PERIPH PLL */
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_MUX)] = &periph_pll_mux_clk,
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_VCO)] = &periph_pll_vco_clk,
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_PHI0)] = &periph_pll_phi0_clk,
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_PHI1)] = &periph_pll_phi1_clk,
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_PHI2)] = &periph_pll_phi2_clk,
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_PHI3)] = &periph_pll_phi3_clk,
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_PHI4)] = &periph_pll_phi4_clk,
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_PHI5)] = &periph_pll_phi5_clk,
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_PHI7)] = &periph_pll_phi7_clk,
	/* PERIPH DFS */
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_DFS1)] = &periph_dfs1_clk,
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_DFS2)] = &periph_dfs2_clk,
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_DFS3)] = &periph_dfs3_clk,
	[CC_ARR_CLK(S32GEN1_CLK_PERIPH_PLL_DFS5)] = &periph_dfs5_clk,
	/* PERIPH - MC_CGM0 */
	[CC_ARR_CLK(S32GEN1_CLK_SERDES_REF)] = &serdes_ref_clk,
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM0_MUX3)] = &cgm0_mux3_clk,
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM0_MUX4)] = &cgm0_mux4_clk,
	[CC_ARR_CLK(S32GEN1_CLK_FTM0_EXT_REF)] = &ftm0_ext_clk,
	[CC_ARR_CLK(S32GEN1_CLK_FTM0_REF)] = &ftm0_ref_clk,
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM0_MUX5)] = &cgm0_mux5_clk,
	[CC_ARR_CLK(S32GEN1_CLK_FTM1_EXT_REF)] = &ftm1_ext_clk,
	[CC_ARR_CLK(S32GEN1_CLK_FTM1_REF)] = &ftm1_ref_clk,
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM0_MUX6)] = &cgm0_mux6_clk,
	[CC_ARR_CLK(S32GEN1_CLK_FLEXRAY_PE)] = &flexray_pe_clk,
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM0_MUX7)] = &cgm0_mux7_clk,
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM0_MUX8)] = &cgm0_mux8_clk,
	[CC_ARR_CLK(S32GEN1_CLK_LIN_BAUD)] = &lin_baud_clk,
	[CC_ARR_CLK(S32GEN1_CLK_LINFLEXD)] = &linflexd_clk,
	[CC_ARR_CLK(S32GEN1_CLK_SERDES0_LANE0_CDR)] = &serdes0_lane0_cdr_clk,
	[CC_ARR_CLK(S32GEN1_CLK_SERDES0_LANE0_TX)] = &serdes0_lane0_tx_clk,
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM0_MUX16)] = &cgm0_mux16_clk,
	[CC_ARR_CLK(S32GEN1_CLK_SPI)] = &spi_clk,
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM0_MUX12)] = &cgm0_mux12_clk,
	[CC_ARR_CLK(S32GEN1_CLK_QSPI_2X)] = &qspi_2x_clk,
	[CC_ARR_CLK(S32GEN1_CLK_QSPI)] = &qspi_clk,
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM0_MUX14)] = &cgm0_mux14_clk,
	[CC_ARR_CLK(S32GEN1_CLK_SDHC)] = &sdhc_clk,
	/* DDR PLL */
	[CC_ARR_CLK(S32GEN1_CLK_DDR_PLL_MUX)] = &ddr_pll_mux_clk,
	[CC_ARR_CLK(S32GEN1_CLK_DDR_PLL_VCO)] = &ddr_pll_vco_clk,
	[CC_ARR_CLK(S32GEN1_CLK_DDR_PLL_PHI0)] = &ddr_pll_phi0_clk,
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM5_MUX0)] = &cgm5_mux0_clk,
	[CC_ARR_CLK(S32GEN1_CLK_DDR)] = &ddr_clk,
	/* ACCEL PLL */
	[CC_ARR_CLK(S32GEN1_CLK_ACCEL_PLL_MUX)] = &accel_pll_mux_clk,
	[CC_ARR_CLK(S32GEN1_CLK_ACCEL_PLL_VCO)] = &accel_pll_vco_clk,
	/* CLKOUT */
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM0_MUX1)] = &cgm0_mux1_clk,
	[CC_ARR_CLK(S32GEN1_CLK_CLKOUT0)] = &clkout0_clk,
	[CC_ARR_CLK(S32GEN1_CLK_MC_CGM0_MUX2)] = &cgm0_mux2_clk,
	[CC_ARR_CLK(S32GEN1_CLK_CLKOUT1)] = &clkout1_clk,
};

struct s32gen1_clk *get_clock(uint32_t id)
{
	u32 index;

	if (id < S32GEN1_CLK_ID_BASE)
		return NULL;

	if (id >= S32GEN1_PLAT_CLK_ID_BASE)
		return get_plat_clock(id);

	index = s32gen1_platclk2mux(id);

	if (index >= ARRAY_SIZE(plat_clocks) || !plat_clocks[index])
		return get_plat_cc_clock(id);

	return plat_clocks[index];
}

bool is_qspi1x_clk(uint32_t id)
{
	return (id == S32GEN1_SCMI_CLK_QSPI_FLASH1X) ||
		(id == S32GEN1_CLK_QSPI);
}

bool is_qspi2x_clk(uint32_t id)
{
	return (id == S32GEN1_SCMI_CLK_QSPI_FLASH2X) ||
		(id == S32GEN1_CLK_QSPI_2X);
}

bool is_qspi_clk(uint32_t id)
{
	return is_qspi1x_clk(id) || is_qspi2x_clk(id);
}

int s32gen1_get_early_clks_freqs(const struct siul2_freq_mapping **mapping)
{
	u32 freq;
	size_t i;

	freq = get_siul2_midr2_freq();

	/* Last entry is empty */
	for (i = 0; siul2_clk_freq_map[i].siul2_midr2_freq != 0; i++)

		if (siul2_clk_freq_map[i].siul2_midr2_freq == freq) {
			*mapping = &siul2_clk_freq_map[i];
			return 0;
		}

	return -EINVAL;
}
