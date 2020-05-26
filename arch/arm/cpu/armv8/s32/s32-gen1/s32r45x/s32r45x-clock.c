// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2019-2020 NXP
 */
#include <asm/arch/clock.h>
#include <asm/arch/mc_cgm_regs.h>
#include <asm/arch/mc_me_regs.h>
#include <s32gen1-clock.h>

#define ARM_PLL_PHI0_FREQ (800000000)
#define ARM_PLL_PHI1_FREQ (500000000)
#define ARM_PLL_PHI_Nr (2)

/* ARM_PLL_DFS1_FREQ - 800 Mhz */
#define ARM_PLL_DFS1_EN (1)
#define ARM_PLL_DFS1_MFI (1)
#define ARM_PLL_DFS1_MFN (0)
/* ARM_PLL_DFS2_REQ - 800 Mhz */
#define ARM_PLL_DFS2_EN (1)
#define ARM_PLL_DFS2_MFI (1)
#define ARM_PLL_DFS2_MFN (0)
/* ARM_PLL_DFS3_freq - 200 mhz */
#define ARM_PLL_DFS3_EN (1)
#define ARM_PLL_DFS3_MFI (5)
#define ARM_PLL_DFS3_MFN (0)
/* ARM_PLL_DFS4_freq - 400 mhz */
#define ARM_PLL_DFS4_EN (1)
#define ARM_PLL_DFS4_MFI (2)
#define ARM_PLL_DFS4_MFN (0)
/* ARM_PLL_DFS5_freq - 400 mhz */
#define ARM_PLL_DFS5_EN (1)
#define ARM_PLL_DFS5_MFI (2)
#define ARM_PLL_DFS5_MFN (0)
/* ARM_PLL_DFS6_freq - 400 mhz */
#define ARM_PLL_DFS6_EN (1)
#define ARM_PLL_DFS6_MFI (2)
#define ARM_PLL_DFS6_MFN (0)

#define ARM_PLL_DFS_Nr (6)
#define ARM_PLL_PLLDV_RDIV (1)
#define ARM_PLL_PLLDV_MFI (40)
#define ARM_PLL_PLLFD_MFN (0)

#define PERIPH_PLL_PHI0_FREQ (125000000)
#define PERIPH_PLL_PHI1_FREQ (80000000)
#define PERIPH_PLL_PHI2_FREQ (80000000)
#define PERIPH_PLL_PHI3_FREQ (125000000)
#define PERIPH_PLL_PHI4_FREQ (200000000)
#define PERIPH_PLL_PHI5_FREQ (125000000)
#define PERIPH_PLL_PHI6_FREQ (100000000)
#define PERIPH_PLL_PHI7_FREQ (100000000)
#define PERIPH_PLL_PHI_Nr (8)

/* PERIPH_PLL_DFS1_freq - 800 mhz */
#define PERIPH_PLL_DFS1_EN (1)
#define PERIPH_PLL_DFS1_MFI (1)
#define PERIPH_PLL_DFS1_MFN (9)
/* PERIPH_PLL_DFS2_freq - 400 mhz */
#define PERIPH_PLL_DFS2_EN (1)
#define PERIPH_PLL_DFS2_MFI (2)
#define PERIPH_PLL_DFS2_MFN (18)
/* PERIPH_PLL_DFS3_freq - 800 mhz */
#define PERIPH_PLL_DFS3_EN (1)
#define PERIPH_PLL_DFS3_MFI (1)
#define PERIPH_PLL_DFS3_MFN (9)
/* PERIPH_PLL_DFS4_freq - 400 mhz */
#define PERIPH_PLL_DFS4_EN (1)
#define PERIPH_PLL_DFS4_MFI (2)
#define PERIPH_PLL_DFS4_MFN (18)
/* PERIPH_PLL_DFS5_freq - 400 mhz */
#define PERIPH_PLL_DFS5_EN (1)
#define PERIPH_PLL_DFS5_MFI (2)
#define PERIPH_PLL_DFS5_MFN (18)
/* PERIPH_PLL_DFS6_freq - 400 mhz */
#define PERIPH_PLL_DFS6_EN (1)
#define PERIPH_PLL_DFS6_MFI (2)
#define PERIPH_PLL_DFS6_MFN (18)

#define PERIPH_PLL_DFS_Nr (6)
#define PERIPH_PLL_PLLDV_RDIV (1)
#define PERIPH_PLL_PLLDV_MFI (50)
#define PERIPH_PLL_PLLFD_MFN (0)

#define DDR_PLL_PHI0_FREQ (800000000)
#define DDR_PLL_PHI_Nr (1)
#define DDR_PLL_DFS_Nr (0)
#define DDR_PLL_PLLDV_RDIV (1)
#define DDR_PLL_PLLDV_MFI (40)
#ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
#define DDR_PLL_PLLFD_MFN (1)
#else
#define DDR_PLL_PLLFD_MFN (0)
#endif

#define ACCEL_PLL_PHI0_FREQ (600000000)
#define ACCEL_PLL_PHI1_FREQ (600000000)
#define ACCEL_PLL_PHI_Nr (2)
#define ACCEL_PLL_DFS_Nr (0)
#define ACCEL_PLL_PLLDV_RDIV (1)
#define ACCEL_PLL_PLLDV_MFI (60)
#define ACCEL_PLL_PLLFD_MFN (0)

static void setup_mux_clocks(void)
{
	/* setup the mux clock divider for PER_CLK (80 MHz) */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 3,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI1);
	mux_div_clk_config(MC_CGM0_BASE_ADDR, 3, 0, 0);

	/* FTM 0 (40MHz) */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 4,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI1);
	mux_div_clk_config(MC_CGM0_BASE_ADDR, 4, 0, 1);

	/* FTM 1 (40MHz) */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 5,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI1);
	mux_div_clk_config(MC_CGM0_BASE_ADDR, 5, 0, 1);

	/* setup the mux clock divider for CAN_CLK (80 MHz) */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 7,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI2);

	/* setup the mux clock divider for LIN_CLK (62,5 MHz),
	 * LIN_BAUD_CLK (125 MHz)
	 */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 8,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI3);

	/* setup the mux clock divider for XBAR_CLK (400 MHz),
	 * XBAR_DIV2_CLK (200 MHz), IPS_CLK (133 MHz), SbSW_CLK (66.5 MHz)
	 */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 0,
			      MC_CGM_MUXn_CSC_SEL_ARM_PLL_DFS1);
	mux_div_clk_config(MC_CGM0_BASE_ADDR, 0, 0, 1);

	/* ACCEL_3 Clock (600 MHz) */
	mux_source_clk_config(MC_CGM2_BASE_ADDR, 0,
			      MC_CGM_MUXn_CSC_SEL_ACCEL_PLL_PHI0_2);
	mux_div_clk_config(MC_CGM2_BASE_ADDR, 0, 0, 0);

	/* ACCEL_4 Clock (600 MHz) */
	mux_source_clk_config(MC_CGM2_BASE_ADDR, 1,
			      MC_CGM_MUXn_CSC_SEL_ARM_PLL_DFS4_2);
	mux_div_clk_config(MC_CGM2_BASE_ADDR, 1, 0, 0);

	/* setup the mux clock divider for DSPI_CLK (100 MHz) */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 16,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI7);

	/* setup the mux clock divider for QSPI_2X_CLK (266 MHz),
	 * QSPI_1X_CLK (133 MHz)
	 */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 12,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_DFS1);
	mux_div_clk_config(MC_CGM0_BASE_ADDR, 12, 0, 2);

	/* setup the mux clock divider for SDHC_CLK (200 MHz) */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 14,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_DFS3);
	mux_div_clk_config(MC_CGM0_BASE_ADDR, 14, 0, 3);

	/* setup the mux clock divider for DDR_CLK (800 MHz) */
	mux_source_clk_config(MC_CGM5_BASE_ADDR, 0,
			      MC_CGM_MUXn_CSC_SEL_DDR_PLL_PHI0);

	mux_source_clk_config(MC_CGM1_BASE_ADDR, 0,
			      MC_CGM_MUXn_CSC_SEL_ARM_PLL_PHI0);
}

void clock_init(void)
{
	u32 arm_dfs[ARM_PLL_DFS_Nr][DFS_PARAMS_Nr] = {
		{ ARM_PLL_DFS1_EN, ARM_PLL_DFS1_MFN, ARM_PLL_DFS1_MFI },
		{ ARM_PLL_DFS2_EN, ARM_PLL_DFS2_MFN, ARM_PLL_DFS2_MFI },
		{ ARM_PLL_DFS3_EN, ARM_PLL_DFS3_MFN, ARM_PLL_DFS3_MFI },
		{ ARM_PLL_DFS4_EN, ARM_PLL_DFS4_MFN, ARM_PLL_DFS4_MFI },
		{ ARM_PLL_DFS5_EN, ARM_PLL_DFS5_MFN, ARM_PLL_DFS5_MFI },
		{ ARM_PLL_DFS6_EN, ARM_PLL_DFS6_MFN, ARM_PLL_DFS6_MFI }
	};
	u64 arm_phi[ARM_PLL_PHI_Nr] = { ARM_PLL_PHI0_FREQ, ARM_PLL_PHI1_FREQ };

	u32 periph_dfs[PERIPH_PLL_DFS_Nr][DFS_PARAMS_Nr] = {
		{ PERIPH_PLL_DFS1_EN, PERIPH_PLL_DFS1_MFN,
		  PERIPH_PLL_DFS1_MFI },
		{ PERIPH_PLL_DFS2_EN, PERIPH_PLL_DFS2_MFN,
		  PERIPH_PLL_DFS2_MFI },
		{ PERIPH_PLL_DFS3_EN, PERIPH_PLL_DFS3_MFN,
		  PERIPH_PLL_DFS3_MFI },
		{ PERIPH_PLL_DFS4_EN, PERIPH_PLL_DFS4_MFN,
		  PERIPH_PLL_DFS4_MFI },
		{ PERIPH_PLL_DFS5_EN, PERIPH_PLL_DFS5_MFN,
		  PERIPH_PLL_DFS5_MFI },
		{ PERIPH_PLL_DFS6_EN, PERIPH_PLL_DFS6_MFN, PERIPH_PLL_DFS6_MFI }
	};
	u64 periph_phi[PERIPH_PLL_PHI_Nr] = {
		PERIPH_PLL_PHI0_FREQ, PERIPH_PLL_PHI1_FREQ,
		PERIPH_PLL_PHI2_FREQ, PERIPH_PLL_PHI3_FREQ,
		PERIPH_PLL_PHI4_FREQ, PERIPH_PLL_PHI5_FREQ,
		PERIPH_PLL_PHI6_FREQ, PERIPH_PLL_PHI7_FREQ,
	};

	u64 ddr_phi[DDR_PLL_PHI_Nr] = { DDR_PLL_PHI0_FREQ };
	u64 accel_phi[ACCEL_PLL_PHI_Nr] = { ACCEL_PLL_PHI0_FREQ,
					    ACCEL_PLL_PHI1_FREQ };
	u32 part0_blocks[] = { MC_ME_USDHC_REQ, MC_ME_DDR_0_REQ,
			      MC_ME_MIPICSI2_0_REQ, MC_ME_MIPICSI2_1_REQ,
			      MC_ME_MIPICSI2_2_REQ, MC_ME_MIPICSI2_3_REQ,
			      MC_ME_FDMA_REQ, MC_ME_FLEXCAN4_REQ,
			      MC_ME_FLEXCAN5_REQ, MC_ME_FLEXCAN6_REQ,
			      MC_ME_FLEXCAN7_REQ };

	u32 part2_blocks[] = { MC_ME_LAX1_REQ, MC_ME_LAX2_REQ };

	u32 part3_blocks[] = { MC_ME_CTE_XBAR3_REQ, MC_ME_EIM_DSP_REQ,
			       MC_ME_BBE32EP_REQ, MC_ME_SPT_REQ };
	s32gen1_setup_fxosc();

	/* Enable all blocks from partion 0, 2, 3 */
	s32gen1_enable_partition_blocks(MC_ME_USDHC_PRTN, &part0_blocks[0],
					ARRAY_SIZE(part0_blocks));
	s32gen1_enable_partition_blocks(MC_ME_LAX_PRTN, &part2_blocks[0],
					ARRAY_SIZE(part2_blocks));
	s32gen1_enable_partition_blocks(MC_ME_SPT_PRTN, &part3_blocks[0],
					ARRAY_SIZE(part3_blocks));

	s32gen1_program_pll(ARM_PLL, XOSC_CLK_FREQ, ARM_PLL_PHI_Nr, arm_phi,
			    ARM_PLL_DFS_Nr, arm_dfs, ARM_PLL_PLLDV_RDIV,
			    ARM_PLL_PLLDV_MFI, ARM_PLL_PLLFD_MFN);

	s32gen1_program_pll(PERIPH_PLL, XOSC_CLK_FREQ, PERIPH_PLL_PHI_Nr,
			    periph_phi, PERIPH_PLL_DFS_Nr, periph_dfs,
			    PERIPH_PLL_PLLDV_RDIV, PERIPH_PLL_PLLDV_MFI,
			    PERIPH_PLL_PLLFD_MFN);

	s32gen1_program_pll(ACCEL_PLL, XOSC_CLK_FREQ, ACCEL_PLL_PHI_Nr,
			    accel_phi, ACCEL_PLL_DFS_Nr, NULL,
			    ACCEL_PLL_PLLDV_RDIV, ACCEL_PLL_PLLDV_MFI,
			    ACCEL_PLL_PLLFD_MFN);

	s32gen1_program_pll(DDR_PLL, XOSC_CLK_FREQ, DDR_PLL_PHI_Nr, ddr_phi,
			    DDR_PLL_DFS_Nr, NULL, DDR_PLL_PLLDV_RDIV,
			    DDR_PLL_PLLDV_MFI, DDR_PLL_PLLFD_MFN);

	setup_mux_clocks();
}
