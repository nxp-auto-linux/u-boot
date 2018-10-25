/*
 * (C) Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <netdev.h>
#include <div64.h>
#include <errno.h>
#include <asm/arch/cse.h>
#include <asm/arch/imx-regs.h>

DECLARE_GLOBAL_DATA_PTR;


u32 cpu_mask(void)
{
	/* 0 means out of reset. */
	/* Bit 0 corresponds to cluster reset and is 0 if any
	 * of the other bits 1-4 are 0.
	 */
	return ((~(readl(RGM_PSTAT(RGM_CORES_RESET_GROUP)))) >> 1) & 0xf;
}

/*
 * Return the number of cores on this SOC.
 */
int cpu_numcores(void)
{
	return hweight32(cpu_mask());
}

/* There are 3 possible ranges for selected_output:
 * - < PHI_MAXNUMBER - the selected output is a PHI
 * - >= PHI_MAXNUMBER and <= PHI_MAXNUMBER + DFS_MAXNUMBER -
 *   the selected output is a DFS if supported or error
 * - > PHI_MAXNUMBER + DFS_MAXNUMBER - error
 */
static u32 get_pllfreq(u32 pll, u32 refclk_freq, u32 plldv,
		u32 pllfd, u32 selected_output)
{
	u32 plldv_rdiv = 0, plldv_mfi = 0, pllfd_mfn = 0;
	u32 pllodiv_div = 0, fout = 0;
	u32 dfs_portn = 0, dfs_mfn = 0, dfs_mfi = 0;
	u32 phi_nr, dfs_nr;
	double vco = 0;

	if (selected_output > PHI_MAXNUMBER + DFS_MAXNUMBER) {
		printf("Unsupported selected output\n");
		return -1;
	}

	plldv_rdiv = (plldv & PLLDIG_PLLDV_RDIV_MASK) >>
		PLLDIG_PLLDV_RDIV_OFFSET;
	plldv_mfi = (plldv & PLLDIG_PLLDV_MFI_MASK);

	pllfd_mfn = (pllfd & PLLDIG_PLLFD_MFN_MASK);

	plldv_rdiv = plldv_rdiv == 0 ? 1 : plldv_rdiv;

	/* The formula for VCO is from S32RS RefMan Rev. 1, draft D) */
	vco = (refclk_freq / (double)plldv_rdiv) *
		(plldv_mfi + pllfd_mfn / (double)18432);

	if (selected_output < PHI_MAXNUMBER) {
		/* Determine the div for PHI. */
		phi_nr = selected_output;
		pllodiv_div = readl(PLLDIG_PLLODIV(pll, phi_nr));
		pllodiv_div = (pllodiv_div & PLLDIG_PLLODIV_DIV_MASK) >>
			PLLDIG_PLLODIV_DIV_OFFSET;
		fout = vco / (pllodiv_div + 1);
	} else if (pll == ARM_PLL || pll == PERIPH_PLL) {
		/* Determine the div for DFS. */
		dfs_nr = selected_output - PHI_MAXNUMBER + 1;

		dfs_portn = readl(DFS_DVPORTn(pll,
					dfs_nr - 1));
		dfs_mfi = (dfs_portn & DFS_DVPORTn_MFI_MASK) >>
			DFS_DVPORTn_MFI_OFFSET;
		dfs_mfn = (dfs_portn & DFS_DVPORTn_MFN_MASK) >>
			DFS_DVPORTn_MFN_OFFSET;

		/* According to the formula:
		 * fdfs_clckout = fdfs_clkin /
		 *     (2 * (DFS_DVPORTn[MFI] + (DFS_DVPORTn[MFN]/36)))
		 */
		fout = (18 * vco) / (36 * dfs_mfi + dfs_mfn);
	} else {
		printf("Selected PLL doesn't have DFS as output\n");
		return -1;
	}

	return fout;
}

/* There are 3 possible ranges for selected_output:
 * - < PHI_MAXNUMBER - the selected output is a PHI
 * - >= PHI_MAXNUMBER and <= PHI_MAXNUMBER + DFS_MAXNUMBER -
 *   the selected output is a DFS if supported or error
 * - > PHI_MAXNUMBER + DFS_MAXNUMBER - error
 */
/* Implemented for ARM_PLL, PERIPH_PLL, ACCEL_PLL, DDR_PLL. */
static u32 decode_pll(enum pll_type pll, u32 refclk_freq,
		u32 selected_output)
{
	u32 plldv, pllfd, freq;

	plldv = readl(PLLDIG_PLLDV(pll));
	pllfd = readl(PLLDIG_PLLFD(pll));

	freq = get_pllfreq(pll, refclk_freq, plldv, pllfd, selected_output);
	return freq  < 0 ? 0 : freq;
}

static u32 get_sel(u64 cgm, u8 mux)
{
	u32 css_sel;

	css_sel = readl(CGM_MUXn_CSS(cgm, mux));
	return MC_CGM_MUXn_CSS_SELSTAT(css_sel);
}

static u32 get_div(u64 cgm, u8 mux)
{
	u32 div, dc;

	dc = readl(CGM_MUXn_DCm(cgm, mux, 0));
	/* If div is enabled. */
	if (dc & MC_CGM_MUXn_DCm_DE) {
		div = (dc & MC_CGM_MUXn_DCm_DIV_MASK) >>
			MC_CGM_MUXn_DCm_DIV_OFFSET;
		div += 1;
	} else {
		div = 1;
	}

	return div;
}

static u32 get_uart_clk(void)
{
	u32 div, css_sel, freq = 0;

	css_sel = get_sel(MC_CGM0_BASE_ADDR, 8);
	div = get_div(MC_CGM0_BASE_ADDR, 8);

	switch (css_sel) {
	case MC_CGM_MUXn_CSC_SEL_FIRC:
		freq = FIRC_CLK_FREQ;
		break;
	case MC_CGM_MUXn_CSC_SEL_FXOSC:
		freq = XOSC_CLK_FREQ;
		break;
	case MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI3:
		freq = decode_pll(PERIPH_PLL, XOSC_CLK_FREQ, 3);
		break;
	default:
		printf("unsupported system clock select\n");
		freq = 0;
	}

	return freq/div;
}

static u32 get_usdhc_clk(void)
{
	u32 div, css_sel;
	u32 freq = 0;

	css_sel = get_sel(MC_CGM0_BASE_ADDR, 14);
	div = get_div(MC_CGM0_BASE_ADDR, 14);

	switch (css_sel) {
	case MC_CGM_MUXn_CSC_SEL_FIRC:
		freq = FIRC_CLK_FREQ;
		break;
	case MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_DFS3:
		freq = decode_pll(PERIPH_PLL, XOSC_CLK_FREQ, 10);
		break;
	default:
		printf("unsupported system clock select\n");
		freq = 0;
	}

	return freq / div;
}

static u32 get_xbar_clk(void)
{
	u32 div, css_sel;
	u32 freq = 0;

	css_sel = get_sel(MC_CGM0_BASE_ADDR, 0);
	div = get_div(MC_CGM0_BASE_ADDR, 0);

	switch (css_sel) {
	case MC_CGM_MUXn_CSC_SEL_FIRC:
		freq = FIRC_CLK_FREQ;
		break;
	case MC_CGM_MUXn_CSC_SEL_ARM_PLL_DFS1:
		freq = decode_pll(ARM_PLL, XOSC_CLK_FREQ, 8);
		break;
	default:
		printf("unsupported system clock select\n");
		freq = 0;
	}

	return freq / div;
}

static u32 get_dspi_clk(void)
{
	u32 div, css_sel, freq = 0;

	css_sel = get_sel(MC_CGM0_BASE_ADDR, 16);
	div = get_div(MC_CGM0_BASE_ADDR, 16);

	switch (css_sel) {
	case MC_CGM_MUXn_CSC_SEL_FIRC:
		freq = FIRC_CLK_FREQ;
		break;
	case MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_PHI7:
		freq = decode_pll(PERIPH_PLL, XOSC_CLK_FREQ, 7);
		break;
	default:
		printf("unsupported system clock select\n");
		freq = 0;
	}

	return freq/div;
}

static u32 get_qspi_clk(void)
{
	u32 div, css_sel, freq = 0;

	css_sel = get_sel(MC_CGM0_BASE_ADDR, 12);
	div = get_div(MC_CGM0_BASE_ADDR, 12);

	switch (css_sel) {
	case MC_CGM_MUXn_CSC_SEL_FIRC:
		freq = FIRC_CLK_FREQ;
		break;
	case MC_CGM_MUXn_CSC_SEL_PERIPH_PLL_DFS1:
		freq = decode_pll(PERIPH_PLL, XOSC_CLK_FREQ, 8);
		break;
	default:
		printf("unsupported system clock select\n");
		freq = 0;
	}

	return freq/div;
}

static u32 get_ddr_clk(void)
{
	u32 div, css_sel, freq = 0;

	css_sel = get_sel(MC_CGM5_BASE_ADDR, 0);
	div = get_div(MC_CGM5_BASE_ADDR, 0);

	switch (css_sel) {
	case MC_CGM_MUXn_CSC_SEL_FIRC:
		freq = FIRC_CLK_FREQ;
		break;
	case MC_CGM_MUXn_CSC_SEL_DDR_PLL_PHI0:
		freq = decode_pll(DDR_PLL, XOSC_CLK_FREQ, 0);
		break;
	default:
		printf("unsupported system clock select\n");
		freq = 0;
	}

	return freq/div;
}

/* return clocks in Hz */
unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_UART_CLK:
		return get_uart_clk();
	case MXC_USDHC_CLK:
		return get_usdhc_clk();
	case MXC_QSPI_CLK:
		return get_qspi_clk();
	case MXC_DSPI_CLK:
		return get_dspi_clk();
	case MXC_XBAR_CLK:
		return get_xbar_clk();
	case MXC_DDR_CLK:
		return get_ddr_clk();
	default:
		break;
	}
	printf("Error: Unsupported function to read the frequency! Please define it correctly!");
	return 0;
}

/* Dump some core clocks */
int do_s32_showclocks(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	printf("Root clocks:\n");
	printf("UART clock:	%5d MHz\n",
	       mxc_get_clock(MXC_UART_CLK) / 1000000);
	printf("SDHC clock:	%5d MHz\n",
	       mxc_get_clock(MXC_USDHC_CLK) / 1000000);
	printf("DSPI clock:     %5d MHz\n",
	       mxc_get_clock(MXC_DSPI_CLK) / 1000000);
	printf("QSPI clock:     %5d MHz\n",
	       mxc_get_clock(MXC_QSPI_CLK) / 1000000);
	printf("XBAR clock:     %5d MHz\n",
	       mxc_get_clock(MXC_XBAR_CLK) / 1000000);
	printf("DDR  clock:     %5d MHz\n",
	       mxc_get_clock(MXC_DDR_CLK) / 1000000);

	return 0;
}

U_BOOT_CMD(
		clocks, CONFIG_SYS_MAXARGS, 1, do_s32_showclocks,
		"display clocks",
		""
	 );

#if defined(CONFIG_DISPLAY_CPUINFO)
static const char *get_reset_cause(void)
{
	u32 val;

	val = readl(RGM_DES);
	if (val & RGM_DES_POR) {
		/* Clear bit */
		writel(RGM_DES_POR, RGM_DES);
		return "Power-On Reset";
	}

	if (val) {
		writel(~RGM_DES_POR, RGM_DES);
		return "Destructive Reset";
	}

	val = readl(RGM_FES);
	if (val & RGM_FES_EXT) {
		writel(RGM_FES_EXT, RGM_FES);
		return "External Reset";
	}

	if (val) {
		writel(~RGM_FES_EXT, RGM_FES);
		return "Functional Reset";
	}

	val = readl(MC_ME_MODE_STAT);
	if ((val & MC_ME_MODE_STAT_PREVMODE) == 0)
		return "Reset";

	return "unknown reset";
}

void reset_cpu(ulong addr)
{
	writel(MC_ME_MODE_CONF_FUNC_RST, MC_ME_MODE_CONF);

	writel(MC_ME_MODE_UPD_UPD, MC_ME_MODE_UPD);

	writel(MC_ME_CTL_KEY_KEY, MC_ME_CTL_KEY);
	writel(MC_ME_CTL_KEY_INVERTEDKEY, MC_ME_CTL_KEY);

	/* If we get there, we are not in good shape */
	mdelay(1000);
	printf("FATAL: Reset Failed!\n");
	hang();
}

int print_cpuinfo(void)
{
	printf("CPU:   NXP S32X\n");
	printf("Reset cause: %s\n", get_reset_cause());

	return 0;
}
#endif

__weak void setup_iomux_enet(void)
{
	/* set PE2 - MSCR[66] - for TX CLK */
	writel(SIUL2_MSCR_S32_G1_ENET_TX_CLK,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE2));
	writel(SIUL2_MSCR_S32_G1_ENET_TX_CLK_IN,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE2_IN));

	/* set PE3 - MSCR[67] - for TX_EN */
	writel(SIUL2_MSCR_S32_G1_ENET_TX_EN,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE3));

	/* set PE4 - MSCR[68] - for TX_D0 */
	writel(SIUL2_MSCR_S32_G1_ENET_TX_D0,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE4));

	/* set PE5 - MSCR[69] - for TX_D1 */
	writel(SIUL2_MSCR_S32_G1_ENET_TX_D1,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE5));

	/* set PE6 - MSCR[70] - for TX_D2 */
	writel(SIUL2_MSCR_S32_G1_ENET_TX_D2,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE6));

	/* set PE7 - MSCR[71] - for TX_D3 */
	writel(SIUL2_MSCR_S32_G1_ENET_TX_D3,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE7));

	/* set PE8 - MSCR[72] - for RX_CLK */
	writel(SIUL2_MSCR_S32_G1_ENET_RX_CLK,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE8));
	writel(SIUL2_MSCR_S32_G1_ENET_RX_CLK_IN,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE8_IN));

	/* set PD9 - MSCR[73] - for RX_DV */
	writel(SIUL2_MSCR_S32_G1_ENET_RX_DV,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE9));
	writel(SIUL2_MSCR_S32_G1_ENET_RX_DV_IN,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE9_IN));

	/* set PE10 - MSCR[74] - for RX_D0 */
	writel(SIUL2_MSCR_S32_G1_ENET_RX_D0,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE10));
	writel(SIUL2_MSCR_S32_G1_ENET_RX_D0_IN,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE10_IN));

	/* set PE11 - MSCR[75] - for RX_D1 */
	writel(SIUL2_MSCR_S32_G1_ENET_RX_D1,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE11));
	writel(SIUL2_MSCR_S32_G1_ENET_RX_D1_IN,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE11_IN));

	/* set PE12 - MSCR[76] - for RX_D2 */
	writel(SIUL2_MSCR_S32_G1_ENET_RX_D2,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE12));
	writel(SIUL2_MSCR_S32_G1_ENET_RX_D2_IN,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE12_IN));

	/* set PE13 - MSCR[77] - for RX_D3 */
	writel(SIUL2_MSCR_S32_G1_ENET_RX_D3,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE13));
	writel(SIUL2_MSCR_S32_G1_ENET_RX_D3_IN,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PE13_IN));
}

#ifdef CONFIG_FSL_DCU_FB

__weak void setup_iomux_dcu(void)
{
	/* set PH0 - MSCR[105] - for B0 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH0));
	/* set PH1 - MSCR[106] - for B1 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH1));
	/* set PH2 - MSCR[107] - for B2 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH2));
	/* set PH3 - MSCR[108] - for B3 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH3));
	/* set PH4 - MSCR[109] - for B4 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH4));
	/* set PH5 - MSCR[110] - for B5 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH5));
	/* set PH6 - MSCR[111] - for B6 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH6));
	/* set PH7 - MSCR[112] - for B7 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH7));
	/* set PH8 - MSCR[113] - for G0 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH8));
	/* set PH9 - MSCR[114] - for G1 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH9));
	/* set PH10 - MSCR[115] - for G2 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH10));
	/* set PH11 - MSCR[116] - for G3 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH11));
	/* set PH12 - MSCR[117] - for G4 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH12));
	/* set PH13 - MSCR[118] - for G5 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH13));
	/* set PH14 - MSCR[119] - for G6 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH14));
	/* set PH15 - MSCR[120] - for G7 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PH15));
	/* set PI0 - MSCR[121] - for R0 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI0));
	/* set PI1 - MSCR[122] - for R1 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI1));
	/* set PI2 - MSCR[123] - for R2 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI2));
	/* set PI3 - MSCR[124] - for R3 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI3));
	/* set PI4 - MSCR[125] - for R4 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI4));
	/* set PI5 - MSCR[126] - for R5 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI5));
	/* set PI6 - MSCR[127] - for R6 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI6));
	/* set PI7 - MSCR[128] - for R7 */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI7));
	/* set PI8 - MSCR[129] - for VSYNC */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI8));
	/* set PI9 - MSCR[130] - for HSYNC */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI9));
	/* set PI10 - MSCR[131] - for DE */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI10));
	/* set PI11 - MSCR[132] - for PCLK */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI11));
	/* set PI12 - MSCR[133] - for TAG */
	writel(SIUL2_MSCR_S32V_DCU_CFG, SIUL2_MSCRn(SIUL2_MSCR_S32V_PI12));
}

#endif

__weak void setup_iomux_sdhc(void)
{
	/* Set iomux PADS for USDHC */

	/* PC14 pad: uSDHC clk */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_CLK, SIUL2_MSCRn(46));

	/* PC15 pad: uSDHC CMD */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_CMD, SIUL2_MSCRn(47));
	writel(0x2, SIUL2_MSCRn(515));

	/* PD00 pad: uSDHC DAT0 */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_MSCRn(48));
	writel(0x2, SIUL2_MSCRn(516));

	/* PD01 pad: uSDHC DAT1 */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_MSCRn(49));
	writel(0x2, SIUL2_MSCRn(517));

	/* PD02 pad: uSDHC DAT2 */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_MSCRn(50));
	writel(0x2, SIUL2_MSCRn(520));

	/* PD03 pad: uSDHC DAT3 */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_MSCRn(51));
	writel(0x2, SIUL2_MSCRn(521));

	/* PD04 pad: uSDHC DAT4 */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_MSCRn(52));
	writel(0x2, SIUL2_MSCRn(522));

	/* PD05 pad: uSDHC DAT5 */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_MSCRn(53));
	writel(0x2, SIUL2_MSCRn(523));

	/* PD06 pad: uSDHC DAT6 */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_MSCRn(54));
	writel(0x2, SIUL2_MSCRn(519));

	/* PD07 pad: uSDHC DAT7 */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_MSCRn(55));
	writel(0x2, SIUL2_MSCRn(518));
}

__weak int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

/* start M7 core */
static int do_start_m7(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	unsigned long addr;
	char *ep;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], &ep, 16);
	if (ep == argv[1] || *ep != '\0')
		return CMD_RET_USAGE;

	if (!IS_ADDR_IN_IRAM(addr)) {
		printf("ERROR: Address 0x%08lX not in internal SRAM ...\n",
		       addr);
		return CMD_RET_USAGE;
	}

	printf("Starting core M7 at SRAM address 0x%08lX ...\n", addr);
	printf("Current functionality isn't supported.\n");
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
		startm7,	2,	1,	do_start_m7,
		"start M7 core from SRAM address",
		"startAddress"
	  );

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
#ifdef CONFIG_FSL_CSE3
	int ret;
	ret = cse_init();
	if (ret && ret != -ENODEV)
		printf("Failed to initialize CSE3 security engine\n");
#endif
	return 0;
}
#endif
