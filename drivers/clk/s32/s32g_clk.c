// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020-2021 NXP
 */
#include <asm/arch/siul.h>
#include <dt-bindings/clock/s32g-clock.h>
#include <s32g_clk_funcs.h>
#include <s32gen1_clk_modules.h>
#include <s32gen1_shared_clks.h>

/* XBAR_2X */
static struct s32gen1_part_block llce_block =
		S32GEN1_PART_BLOCK_NO_STATUS(cgm0_mux0_clk, 3,
					     s32gen1_part_block0);
struct s32gen1_clk xbar_2x_clk =
		S32GEN1_FREQ_MODULE_CLK(llce_block, 48 * MHZ, 800 * MHZ);

/* PER_CLK */
static struct s32gen1_clk per_clk =
		S32GEN1_FREQ_MODULE_CLK(per_div, 0, 80 * MHZ);

/* CAN_PE_CLK */
static struct s32gen1_clk can_pe_clk =
		S32GEN1_CHILD_CLK(cgm0_mux7_clk, 40 * MHZ, 80 * MHZ);

/* PFE_MAC0_TX_DIV */
static struct s32gen1_fixed_clock pfe_mac0_ext_tx =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk pfe_mac0_ext_tx_clk =
		S32GEN1_MODULE_CLK(pfe_mac0_ext_tx);

static struct s32gen1_fixed_clock pfe_mac0_ext_ref =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk pfe_mac0_ext_ref_clk =
		S32GEN1_MODULE_CLK(pfe_mac0_ext_ref);

static struct s32gen1_mux cgm2_mux1 =
		S32GEN1_MUX_INIT(S32GEN1_CGM2, 1, 5,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI5,
				 S32G_CLK_PFE_MAC0_EXT_TX,
				 S32G_CLK_PFE_MAC0_EXT_REF,
				 S32G_CLK_SERDES1_LANE0_TX);
static struct s32gen1_clk cgm2_mux1_clk =
		S32GEN1_MODULE_CLK(cgm2_mux1);
static struct s32gen1_cgm_div cgm2_mux1_div =
		S32GEN1_CGM_DIV_INIT(cgm2_mux1_clk, 0);
static struct s32gen1_part_block pfe0_tx_block =
		S32GEN1_PART_BLOCK_NO_STATUS(cgm2_mux1_div, 2,
					     s32gen1_part_block0);
static struct s32gen1_clk pfe_mac0_tx_div_clk =
		S32GEN1_FREQ_MODULE_CLK(pfe0_tx_block, 2500000, 312500000);

/* PFE_MAC0_REF_DIV */
static struct s32gen1_mux cgm2_mux7 =
		S32GEN1_MUX_INIT(S32GEN1_CGM2, 7, 2,
				 S32GEN1_CLK_FIRC,
				 S32G_CLK_PFE_MAC0_EXT_REF);
static struct s32gen1_clk cgm2_mux7_clk =
		S32GEN1_MODULE_CLK(cgm2_mux7);
static struct s32gen1_cgm_div cgm2_mux7_div =
		S32GEN1_CGM_DIV_INIT(cgm2_mux7_clk, 0);
static struct s32gen1_part_block pfe0_ref_block =
		S32GEN1_PART_BLOCK_NO_STATUS(cgm2_mux7_div, 2,
					     s32gen1_part_block0);
static struct s32gen1_clk pfe_mac0_ref_div_clk =
		S32GEN1_FREQ_MODULE_CLK(pfe0_ref_block, 0, 50 * MHZ);

/* PFE_MAC0_RX */
static struct s32gen1_fixed_clock pfe_mac0_ext_rx =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk pfe_mac0_ext_rx_clk =
		S32GEN1_MODULE_CLK(pfe_mac0_ext_rx);

static struct s32gen1_mux cgm2_mux4 =
		S32GEN1_MUX_INIT(S32GEN1_CGM2, 4, 4,
				 S32GEN1_CLK_FIRC,
				 S32G_CLK_PFE_MAC0_EXT_RX,
				 S32G_CLK_SERDES1_LANE0_CDR,
				 S32G_CLK_PFE_MAC0_REF_DIV);
static struct s32gen1_clk cgm2_mux4_clk =
		S32GEN1_MODULE_CLK(cgm2_mux4);
static struct s32gen1_part_block pfe0_rx_block =
		S32GEN1_PART_BLOCK_NO_STATUS(cgm2_mux4_clk, 2,
					     s32gen1_part_block0);
static struct s32gen1_clk pfe_mac0_rx_clk =
		S32GEN1_FREQ_MODULE_CLK(pfe0_rx_block, 2500000, 312500000);

/* PFE_MAC1_TX */
static struct s32gen1_fixed_clock serdes1_lane1_tx =
		S32GEN1_FIXED_CLK_INIT();
struct s32gen1_clk serdes1_lane1_tx_clk =
		S32GEN1_MODULE_CLK(serdes1_lane1_tx);
static struct s32gen1_fixed_clock pfe_mac1_ext_tx =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk pfe_mac1_ext_tx_clk =
		S32GEN1_MODULE_CLK(pfe_mac1_ext_tx);

static struct s32gen1_fixed_clock pfe_mac1_ext_ref =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk pfe_mac1_ext_ref_clk =
		S32GEN1_MODULE_CLK(pfe_mac1_ext_ref);

static struct s32gen1_mux cgm2_mux2 =
		S32GEN1_MUX_INIT(S32GEN1_CGM2, 2, 5,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI5,
				 S32G_CLK_PFE_MAC1_EXT_TX,
				 S32G_CLK_PFE_MAC1_EXT_REF,
				 S32G_CLK_SERDES1_LANE1_TX);
static struct s32gen1_clk cgm2_mux2_clk =
		S32GEN1_MODULE_CLK(cgm2_mux2);
static struct s32gen1_cgm_div cgm2_mux2_div =
		S32GEN1_CGM_DIV_INIT(cgm2_mux2_clk, 0);
static struct s32gen1_part_block pfe1_tx_block =
		S32GEN1_PART_BLOCK_NO_STATUS(cgm2_mux2_div, 2,
					     s32gen1_part_block1);
static struct s32gen1_clk pfe_mac1_tx_clk =
		S32GEN1_FREQ_MODULE_CLK(pfe1_tx_block, 2500000, 125 * MHZ);

/* PFE_MAC1_REF_DIV */
static struct s32gen1_mux cgm2_mux8 =
		S32GEN1_MUX_INIT(S32GEN1_CGM2, 8, 2,
				 S32GEN1_CLK_FIRC,
				 S32G_CLK_PFE_MAC1_EXT_REF);
static struct s32gen1_clk cgm2_mux8_clk =
		S32GEN1_MODULE_CLK(cgm2_mux8);
static struct s32gen1_cgm_div cgm2_mux8_div =
		S32GEN1_CGM_DIV_INIT(cgm2_mux8_clk, 0);
static struct s32gen1_part_block pfe1_ref_block =
		S32GEN1_PART_BLOCK_NO_STATUS(cgm2_mux8_div, 2,
					     s32gen1_part_block1);
static struct s32gen1_clk pfe_mac1_ref_div_clk =
		S32GEN1_FREQ_MODULE_CLK(pfe1_ref_block, 0, 50 * MHZ);

/* PFE_MAC1_RX */
static struct s32gen1_fixed_clock serdes1_lane1_cdr =
		S32GEN1_FIXED_CLK_INIT();
struct s32gen1_clk serdes1_lane1_cdr_clk =
		S32GEN1_MODULE_CLK(serdes1_lane1_cdr);

static struct s32gen1_fixed_clock pfe_mac1_ext_rx =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk pfe_mac1_ext_rx_clk =
		S32GEN1_MODULE_CLK(pfe_mac1_ext_rx);

static struct s32gen1_mux cgm2_mux5 =
		S32GEN1_MUX_INIT(S32GEN1_CGM2, 5, 4,
				 S32GEN1_CLK_FIRC,
				 S32G_CLK_PFE_MAC1_EXT_RX,
				 S32G_CLK_PFE_MAC1_REF_DIV,
				 S32G_CLK_SERDES1_LANE1_CDR);
static struct s32gen1_clk cgm2_mux5_clk =
		S32GEN1_MODULE_CLK(cgm2_mux5);
static struct s32gen1_part_block pfe1_rx_block =
		S32GEN1_PART_BLOCK_NO_STATUS(cgm2_mux5_clk, 2,
					     s32gen1_part_block1);
static struct s32gen1_clk pfe_mac1_rx_clk =
		S32GEN1_FREQ_MODULE_CLK(pfe1_rx_block, 2500000, 125 * MHZ);

/* PFE_MAC2_TX */
static struct s32gen1_fixed_clock serdes0_lane1_tx =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk serdes0_lane1_tx_clk =
		S32GEN1_MODULE_CLK(serdes0_lane1_tx);

static struct s32gen1_fixed_clock pfe_mac2_ext_tx =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk pfe_mac2_ext_tx_clk =
		S32GEN1_MODULE_CLK(pfe_mac2_ext_tx);

static struct s32gen1_fixed_clock pfe_mac2_ext_ref =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk pfe_mac2_ext_ref_clk =
		S32GEN1_MODULE_CLK(pfe_mac2_ext_ref);

static struct s32gen1_mux cgm2_mux3 =
		S32GEN1_MUX_INIT(S32GEN1_CGM2, 3, 5,
				 S32GEN1_CLK_FIRC,
				 S32GEN1_CLK_PERIPH_PLL_PHI5,
				 S32G_CLK_PFE_MAC2_EXT_TX,
				 S32G_CLK_PFE_MAC2_EXT_REF,
				 S32G_CLK_SERDES0_LANE1_TX);
static struct s32gen1_clk cgm2_mux3_clk =
		S32GEN1_MODULE_CLK(cgm2_mux3);
static struct s32gen1_cgm_div cgm2_mux3_div =
		S32GEN1_CGM_DIV_INIT(cgm2_mux3_clk, 0);
static struct s32gen1_part_block pfe2_tx_block =
		S32GEN1_PART_BLOCK_NO_STATUS(cgm2_mux3_div, 2,
					     s32gen1_part_block2);
static struct s32gen1_clk pfe_mac2_tx_clk =
		S32GEN1_FREQ_MODULE_CLK(pfe2_tx_block, 2500000, 125 * MHZ);

/* PFE_MAC2_REF_DIV */
static struct s32gen1_mux cgm2_mux9 =
		S32GEN1_MUX_INIT(S32GEN1_CGM2, 9, 2,
				 S32GEN1_CLK_FIRC,
				 S32G_CLK_PFE_MAC2_EXT_REF);
static struct s32gen1_clk cgm2_mux9_clk =
		S32GEN1_MODULE_CLK(cgm2_mux9);
static struct s32gen1_cgm_div cgm2_mux9_div =
		S32GEN1_CGM_DIV_INIT(cgm2_mux9_clk, 0);
static struct s32gen1_part_block pfe2_ref_block =
		S32GEN1_PART_BLOCK_NO_STATUS(cgm2_mux9_div, 2,
					     s32gen1_part_block2);
static struct s32gen1_clk pfe_mac2_ref_div_clk =
		S32GEN1_FREQ_MODULE_CLK(pfe2_ref_block, 0, 50 * MHZ);

/* PFE_MAC2_RX */
static struct s32gen1_fixed_clock pfe_mac2_ext_rx =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk pfe_mac2_ext_rx_clk =
		S32GEN1_MODULE_CLK(pfe_mac2_ext_rx);

static struct s32gen1_fixed_clock serdes0_lane1_cdr =
		S32GEN1_FIXED_CLK_INIT();
static struct s32gen1_clk serdes0_lane1_cdr_clk =
		S32GEN1_MODULE_CLK(serdes0_lane1_cdr);

static struct s32gen1_mux cgm2_mux6 =
		S32GEN1_MUX_INIT(S32GEN1_CGM2, 6, 4,
				 S32GEN1_CLK_FIRC,
				 S32G_CLK_PFE_MAC2_EXT_RX,
				 S32G_CLK_PFE_MAC2_REF_DIV,
				 S32G_CLK_SERDES0_LANE1_CDR);
static struct s32gen1_clk cgm2_mux6_clk =
		S32GEN1_MODULE_CLK(cgm2_mux6);
static struct s32gen1_part_block pfe2_rx_block =
		S32GEN1_PART_BLOCK_NO_STATUS(cgm2_mux6_clk, 2,
					     s32gen1_part_block2);
static struct s32gen1_clk pfe_mac2_rx_clk =
		S32GEN1_FREQ_MODULE_CLK(pfe2_rx_block, 2500000, 125 * MHZ);

/* PFE_SYS_CLK */
static struct s32gen1_mux cgm2_mux0 =
		S32GEN1_MUX_INIT(S32GEN1_CGM2, 0, 2,
				 S32GEN1_CLK_FIRC,
				 S32G_CLK_ACCEL_PLL_PHI1);
static struct s32gen1_clk cgm2_mux0_clk =
		S32GEN1_MODULE_CLK(cgm2_mux0);
static struct s32gen1_cgm_div cgm2_mux0_div =
		S32GEN1_CGM_DIV_INIT(cgm2_mux0_clk, 0);
static struct s32gen1_part_block pfe_sys_block =
		S32GEN1_PART_BLOCK_NO_STATUS(cgm2_mux0_div, 2,
					     s32gen1_part_block3);
static struct s32gen1_clk pfe_pe_clk =
		S32GEN1_FREQ_MODULE_CLK(pfe_sys_block, 0, 600 * MHZ);
static struct s32gen1_fixed_div pfe_sys_div =
		S32GEN1_FIXED_DIV_INIT(pfe_sys_block, 2);
static struct s32gen1_clk pfe_sys_clk =
		S32GEN1_FREQ_MODULE_CLK(pfe_sys_div, 0, 300 * MHZ);

static struct s32gen1_clk *s32g_clocks[] = {
	/* PFE_MAC0 */
	[ARR_CLK(S32G_CLK_PFE_MAC0_RX)] = &pfe_mac0_rx_clk,
	[ARR_CLK(S32G_CLK_PFE_MAC0_TX_DIV)] = &pfe_mac0_tx_div_clk,
	[ARR_CLK(S32G_CLK_MC_CGM2_MUX1)] = &cgm2_mux1_clk,
	[ARR_CLK(S32G_CLK_MC_CGM2_MUX4)] = &cgm2_mux4_clk,
	[ARR_CLK(S32G_CLK_MC_CGM2_MUX7)] = &cgm2_mux7_clk,
	/* PFE_MAC1 */
	[ARR_CLK(S32G_CLK_PFE_MAC1_RX)] = &pfe_mac1_rx_clk,
	[ARR_CLK(S32G_CLK_PFE_MAC1_TX)] = &pfe_mac1_tx_clk,
	[ARR_CLK(S32G_CLK_MC_CGM2_MUX2)] = &cgm2_mux2_clk,
	[ARR_CLK(S32G_CLK_MC_CGM2_MUX5)] = &cgm2_mux5_clk,
	[ARR_CLK(S32G_CLK_MC_CGM2_MUX8)] = &cgm2_mux8_clk,
	/* PFE_MAC2 */
	[ARR_CLK(S32G_CLK_PFE_MAC2_RX)] = &pfe_mac2_rx_clk,
	[ARR_CLK(S32G_CLK_PFE_MAC2_TX)] = &pfe_mac2_tx_clk,
	[ARR_CLK(S32G_CLK_MC_CGM2_MUX3)] = &cgm2_mux3_clk,
	[ARR_CLK(S32G_CLK_MC_CGM2_MUX6)] = &cgm2_mux6_clk,
	[ARR_CLK(S32G_CLK_MC_CGM2_MUX9)] = &cgm2_mux9_clk,
	[ARR_CLK(S32G_CLK_MC_CGM2_MUX0)] = &cgm2_mux0_clk,
	[ARR_CLK(S32G_CLK_PFE_SYS)] = &pfe_sys_clk,
	[ARR_CLK(S32G_CLK_PFE_PE)] = &pfe_pe_clk,
};

static struct s32gen1_clk *s32g_cc_clocks[] = {
	[CC_ARR_CLK(S32GEN1_CLK_PER)] = &per_clk,
	[CC_ARR_CLK(S32GEN1_CLK_CAN_PE)] = &can_pe_clk,
	[CC_ARR_CLK(S32G_CLK_ACCEL_PLL_PHI0)] = &accel_pll_phi0_clk,
	[CC_ARR_CLK(S32G_CLK_ACCEL_PLL_PHI1)] = &accel_pll_phi1_clk,
	[CC_ARR_CLK(S32G_CLK_SERDES0_LANE1_CDR)] = &serdes0_lane1_cdr_clk,
	[CC_ARR_CLK(S32G_CLK_SERDES0_LANE1_TX)] = &serdes0_lane1_tx_clk,
	[CC_ARR_CLK(S32G_CLK_PFE_MAC0_EXT_TX)] = &pfe_mac0_ext_tx_clk,
	[CC_ARR_CLK(S32G_CLK_PFE_MAC0_EXT_RX)] = &pfe_mac0_ext_rx_clk,
	[CC_ARR_CLK(S32G_CLK_PFE_MAC0_EXT_REF)] = &pfe_mac0_ext_ref_clk,
	[CC_ARR_CLK(S32G_CLK_PFE_MAC1_EXT_TX)] = &pfe_mac1_ext_tx_clk,
	[CC_ARR_CLK(S32G_CLK_PFE_MAC1_EXT_RX)] = &pfe_mac1_ext_rx_clk,
	[CC_ARR_CLK(S32G_CLK_PFE_MAC1_EXT_REF)] = &pfe_mac1_ext_ref_clk,
	[CC_ARR_CLK(S32G_CLK_PFE_MAC2_EXT_TX)] = &pfe_mac2_ext_tx_clk,
	[CC_ARR_CLK(S32G_CLK_PFE_MAC2_EXT_RX)] = &pfe_mac2_ext_rx_clk,
	[CC_ARR_CLK(S32G_CLK_PFE_MAC2_EXT_REF)] = &pfe_mac2_ext_ref_clk,
	[CC_ARR_CLK(S32G_CLK_SERDES1_LANE0_TX)] = &serdes1_lane0_tx_clk,
	[CC_ARR_CLK(S32G_CLK_SERDES1_LANE0_CDR)] = &serdes1_lane0_cdr_clk,
	[CC_ARR_CLK(S32G_CLK_PFE_MAC0_REF_DIV)] = &pfe_mac0_ref_div_clk,
	[CC_ARR_CLK(S32G_CLK_PFE_MAC1_REF_DIV)] = &pfe_mac1_ref_div_clk,
	[CC_ARR_CLK(S32G_CLK_PFE_MAC2_REF_DIV)] = &pfe_mac2_ref_div_clk,
	[CC_ARR_CLK(S32G_CLK_SERDES1_LANE1_TX)] = &serdes1_lane1_tx_clk,
	[CC_ARR_CLK(S32G_CLK_SERDES1_LANE1_CDR)] = &serdes1_lane1_cdr_clk,
};

struct s32gen1_clk *get_plat_cc_clock(uint32_t id)
{
	u32 index = s32gen1_platclk2mux(id);

	if (index >= ARRAY_SIZE(s32g_cc_clocks) || !s32g_cc_clocks[index])
		return s32g_get_plat_cc_clock(id);

	return s32g_cc_clocks[index];
}

struct s32gen1_clk *get_plat_clock(uint32_t id)
{
	u32 index;

	if (id < S32GEN1_PLAT_CLK_ID_BASE)
		return NULL;

	index = id - S32GEN1_PLAT_CLK_ID_BASE;

	if (index >= ARRAY_SIZE(s32g_clocks) || !s32g_clocks[index])
		return s32g_get_plat_clock(id);

	return s32g_clocks[index];
}
