// SPDX-License-Identifier:     GPL-2.0+
/*
 * (C) Copyright 2018-2019 NXP
 */

#include <asm/io.h>
#include <asm/arch/src.h>
#include <asm/arch/mc_cgm_regs.h>
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/mc_rgm_regs.h>
#include <asm/arch/clock.h>

/*
 * Select the clock reference for required pll.
 * pll - ARM_PLL, PERIPH_PLL, ACCEL_PLL, DDR_PLL.
 * refclk_freq - input referece clock frequency (FXOSC - 40 MHZ, FIRC - 48 MHZ)
 */
static int select_pll_source_clk(enum pll_type pll, u32 refclk_freq)
{
	u32 clk_src;

	/* select the pll clock source */
	switch (refclk_freq) {
	case FIRC_CLK_FREQ:
		clk_src = PLLDIG_PLLCLKMUX_REFCLKSEL_SET_FIRC;
		break;
	case XOSC_CLK_FREQ:
		clk_src = PLLDIG_PLLCLKMUX_REFCLKSEL_SET_XOSC;
		break;
	default:
		/* The clock frequency unknown */
		return -1;
	}
	writel(clk_src, PLLDIG_PLLCLKMUX(pll));

	return 0;
}

/*
 * Program the pll according to the input parameters.
 * pll - ARM_PLL, PERIPH_PLL, ACCEL_PLL, DDR_PLL.
 * refclk_freq - input reference clock frequency (FXOSC - 40 MHZ, FIRC - 48 MHZ)
 * phi_nr - number of PHIn
 * freq - array of PHY frequencies
 * dfs_nr - number of DFS modules for current PLL
 * dfs - array with the activation dfs field, mfn and mfi
 * plldv_rdiv - divider of clkfreq_ref
 * plldv_mfi - loop multiplication factor divider
 * pllfd_mfn - numerator loop multiplication factor divider
 * Please consult the PLLDIG chapter of platform manual
 * before to use this function.
 *)
 */
static int program_pll(enum pll_type pll, u32 refclk_freq, u32 phi_nr,
		u64 freq[], u32 dfs_nr, u32 dfs[][DFS_PARAMS_Nr],
		u32 plldv_rdiv, u32 plldv_mfi, u32 pllfd_mfn)
{
	u32 i, dfs_on = 0, fvco;

	/*
	 * This formula is from S32R45 platform reference manual
	 * (Rev. 1, draft C), PLL programming chapter.
	 */
	fvco = refclk_freq * (plldv_mfi + (pllfd_mfn/(float)18432)) /
		(float)plldv_rdiv;

	/*
	 * VCO should have value in [ PLL_MIN_FREQ, PLL_MAX_FREQ ].
	 * Please consult the platform DataSheet in order to determine
	 * the allowed values.
	 */

	if (fvco < PLL_MIN_FREQ || fvco > PLL_MAX_FREQ)
		return -1;

	/* Disable deviders. */
	for (i = 0; i < phi_nr; i++)
		writel(0x0, PLLDIG_PLLODIV(pll, i));

	/* Disable PLL. */
	writel(PLLDIG_PLLCR_PLLPD, PLLDIG_PLLCR(pll));

	if (select_pll_source_clk(pll, refclk_freq) < 0)
		return -1;

	writel(PLLDIG_PLLDV_RDIV_SET(plldv_rdiv) | PLLDIG_PLLDV_MFI(plldv_mfi),
	       PLLDIG_PLLDV(pll));
	writel(readl(PLLDIG_PLLFD(pll)) | PLLDIG_PLLFD_MFN_SET(pllfd_mfn) |
	       PLLDIG_PLLFD_SMDEN, PLLDIG_PLLFD(pll));

	/* Calculate Output Frequency Divider for required PHIn outputs. */
	for (i = 0; i < phi_nr; i++) {
		if (freq[i])
			writel(readl(PLLDIG_PLLODIV(pll, i)) |
				     PLLDIG_PLLODIV_DIV_SET(fvco / freq[i] - 1),
				     PLLDIG_PLLODIV(pll, i));
	}

	/* Enable the PLL. */
	writel(0x0, PLLDIG_PLLCR(pll));

	/* Poll until PLL acquires lock. */
	while (!(readl(PLLDIG_PLLSR(pll)) & PLLDIG_PLLSR_LOCK))
		;

	/* Enable deviders. */
	for (i = 0; i < phi_nr; i++)
		writel(PLLDIG_PLLODIV_DE | readl(PLLDIG_PLLODIV(pll, i)),
		       PLLDIG_PLLODIV(pll, i));

	/* Only ARM_PLL and PERIPH_PLL. */
	if ((pll == ARM_PLL) || (pll == PERIPH_PLL)) {
		writel(DFS_PORTRESET_PORTRESET_MAXVAL, DFS_PORTRESET(pll));
		writel(DFS_CTL_RESET, DFS_CTL(pll));
		for (i = 0; i < dfs_nr; i++) {
			if (dfs[i][0]) {
				writel(DFS_DVPORTn_MFI_SET(dfs[i][2]) |
				       DFS_DVPORTn_MFN_SET(dfs[i][1]),
				       DFS_DVPORTn(pll, i));
				dfs_on |= (dfs[i][0] << i);
			}
		}
		/* DFS clk enable programming */
		writel(~DFS_CTL_RESET, DFS_CTL(pll));

		writel(DFS_PORTRESET_PORTRESET_SET(~dfs_on),
		       DFS_PORTRESET(pll));
		while ((readl(DFS_PORTSR(pll)) & dfs_on) != dfs_on)
			;
	}
	return 0;
}

static int mux_source_clk_config(uintptr_t cgm_addr, u8 mux, u8 source)
{
	u32 css, csc;
	/* Ongoing clock switch? */
	while (readl(CGM_MUXn_CSS(cgm_addr, mux)) & MC_CGM_MUXn_CSS_SWIP)
			;

	csc = readl(CGM_MUXn_CSC(cgm_addr, mux));
	/* Clear previous source. */
	csc &= ~(MC_CGM_MUXn_CSC_SELCTL_MASK);
	/* select the clock source and trigger the clock switch. */
	writel(csc | MC_CGM_MUXn_CSC_SELCTL(source) | MC_CGM_MUXn_CSC_CLK_SW,
	       CGM_MUXn_CSC(cgm_addr, mux));
	/* Wait for configuration bit to auto-clear. */
	while (readl(CGM_MUXn_CSC(cgm_addr, mux)) & MC_CGM_MUXn_CSC_CLK_SW)
		;

	/* Is the clock switch completed? */
	while (readl(CGM_MUXn_CSS(cgm_addr, mux)) & MC_CGM_MUXn_CSS_SWIP)
			;

	/* Chech if the switch succeeded.
	 * Check switch trigger cause and the source.
	 */
	css = readl(CGM_MUXn_CSS(cgm_addr, mux));
	if ((MC_CGM_MUXn_CSS_SWTRG(css) == MC_CGM_MUXn_CSS_SWTRG_SUCCESS) &&
	    (MC_CGM_MUXn_CSS_SELSTAT(css) == source))
		return 0;

	return -1;
}

static void mux_div_clk_config(uintptr_t cgm_addr, u8 mux, u8 dc, u8 divider)
{
	/* set the divider */
	writel(MC_CGM_MUXn_DCm_DE | MC_CGM_MUXn_DCm_DIV(divider),
	       CGM_MUXn_DCm(cgm_addr, mux, dc));
}

static void setup_sys_clocks(void)
{
	return;
}

static void setup_mux_clocks(void)
{
	/* setup the mux clock divider for PER_CLK (80 MHz) */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 3,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI1);
	mux_div_clk_config(MC_CGM0_BASE_ADDR, 3, 0, 0);

	/* setup the mux clock divider for CAN_CLK (80 MHz) */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 7,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI2);

	/* setup the mux clock divider for LIN_CLK (66,5 MHz),
	 * LIN_BAUD_CLK (133 MHz)
	 */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 8,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI3);

	/* setup the mux clock divider for XBAR_CLK (400 MHz),
	 * XBAR_DIV2_CLK (200 MHz), IPS_CLK (133 MHz), SbSW_CLK (66.5 MHz) */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 0,
			      MC_CGM_MUXn_CSC_SEL_ARM_PLL_DFS1);
	mux_div_clk_config(MC_CGM0_BASE_ADDR, 0, 0, 1);

	/* setup the mux clock divider for DSPI_CLK (100 MHz) */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 16,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI7);

	/* setup the mux clock divider for QSPI_2X_CLK (400 MHz),
	 * QSPI_1X_CLK (200 MHz)
	 */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 12,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_DFS1);
	mux_div_clk_config(MC_CGM0_BASE_ADDR, 12, 0, 1);

	/* setup the mux clock divider for SDHC_CLK (208 MHz) */
	mux_source_clk_config(MC_CGM0_BASE_ADDR, 14,
			      MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_DFS3);
	mux_div_clk_config(MC_CGM0_BASE_ADDR, 14, 0, 3);

	/* setup the mux clock divider for DDR_CLK (800 MHz),
	 */
	mux_source_clk_config(MC_CGM5_BASE_ADDR, 0,
			      MC_CGM_MUXn_CSC_SEL_DDR_PLL_PHI0);
}

static void setup_fxosc(void)
{
	/* According to "20.4 Initialization information" from
	 * S32S247RM_Rev1D.pdf, "Once FXOSC is turned ON, DO NOT change the
	 * any signal (on the fly) which is going to analog module input.
	 * The inputs can be changed when the analog module is OFF...When
	 * disabling the IP through Software do not change any other values in
	 * the registers for at least 4 crystal clock cycles."
	 *
	 * Just make sure that FXOSC wasn't aleady started by BootROM.
	 */
	u32 ctrl;
	if (readl(FXOSC_CTRL) & FXOSC_CTRL_OSCON)
		return;

	/* TODO Write GM_SEL value according to crystal specification. */
	ctrl = readl(FXOSC_CTRL) | FXOSC_CTRL_COMP_EN;
	ctrl &= ~FXOSC_CTRL_OSC_BYP;
	writel(ctrl, FXOSC_CTRL);

	/* Switch ON the crystal oscillator. */
	writel(FXOSC_CTRL_OSCON | readl(FXOSC_CTRL), FXOSC_CTRL);

	/* Wait until the clock is stable. */
	while (!(readl(FXOSC_STAT) & FXOSC_STAT_OSC_STAT))
		;
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
	u64 arm_phi[ARM_PLL_PHI_Nr] = {
			ARM_PLL_PHI0_FREQ, ARM_PLL_PHI1_FREQ};

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
			{ PERIPH_PLL_DFS6_EN, PERIPH_PLL_DFS6_MFN,
				PERIPH_PLL_DFS6_MFI }
		};
	u64 periph_phi[PERIPH_PLL_PHI_Nr] = {
			PERIPH_PLL_PHI0_FREQ, PERIPH_PLL_PHI1_FREQ,
			PERIPH_PLL_PHI2_FREQ, PERIPH_PLL_PHI3_FREQ,
			PERIPH_PLL_PHI4_FREQ, PERIPH_PLL_PHI5_FREQ,
			PERIPH_PLL_PHI6_FREQ, PERIPH_PLL_PHI7_FREQ,
		};

	u64 ddr_phi[DDR_PLL_PHI_Nr] = { DDR_PLL_PHI0_FREQ };
	u64 accel_phi[ACCEL_PLL_PHI_Nr] = {
				ACCEL_PLL_PHI0_FREQ, ACCEL_PLL_PHI1_FREQ
				};
	setup_fxosc();

	program_pll(
				ARM_PLL, XOSC_CLK_FREQ, ARM_PLL_PHI_Nr, arm_phi,
				ARM_PLL_DFS_Nr, arm_dfs, ARM_PLL_PLLDV_RDIV,
				ARM_PLL_PLLDV_MFI, ARM_PLL_PLLDV_MFN
				);

	setup_sys_clocks();

	program_pll(
				PERIPH_PLL, XOSC_CLK_FREQ, PERIPH_PLL_PHI_Nr,
				periph_phi, PERIPH_PLL_DFS_Nr, periph_dfs,
				PERIPH_PLL_PLLDV_RDIV, PERIPH_PLL_PLLDV_MFI,
				PERIPH_PLL_PLLDV_MFN
				);

	program_pll(
				ACCEL_PLL, XOSC_CLK_FREQ, ACCEL_PLL_PHI_Nr,
				accel_phi, ACCEL_PLL_DFS_Nr, NULL,
				ACCEL_PLL_PLLDV_RDIV, ACCEL_PLL_PLLDV_MFI,
				ACCEL_PLL_PLLDV_MFN
				);

	program_pll(
				DDR_PLL, XOSC_CLK_FREQ, DDR_PLL_PHI_Nr, ddr_phi,
				DDR_PLL_DFS_Nr, NULL, DDR_PLL_PLLDV_RDIV,
				DDR_PLL_PLLDV_MFI, DDR_PLL_PLLDV_MFN
				);

	setup_mux_clocks();
}
