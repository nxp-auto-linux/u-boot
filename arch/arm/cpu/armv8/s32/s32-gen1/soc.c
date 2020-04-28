// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2017-2020 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <div64.h>
#include <errno.h>
#include <asm/arch/cse.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/s32-gen1/mc_me_regs.h>
#include <asm/arch/s32-gen1/mc_rgm_regs.h>
#if defined(CONFIG_SYS_FSL_DDRSS) && defined(CONFIG_TARGET_TYPE_S32GEN1_EMULATOR)
#include <asm/arch/s32-gen1/ddrss.h>
#elif defined(CONFIG_SYS_FSL_DDRSS) && defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
#include <ddr_init.h>
#endif
#include <board_common.h>
#ifdef CONFIG_FSL_DSPI
#include <fsl_dspi.h>
#endif

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
	u32 div = 1, dc;

	/* MC_CGM_0/5 MUXes below don't have a divider register */
	if (cgm == MC_CGM0_BASE_ADDR) {
		if ((mux == 7) || (mux == 8) || (mux == 11) || (mux == 16))
			return div;
	} else if (cgm == MC_CGM5_BASE_ADDR) {
		if (mux == 0)
			return div;
	}

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

u32 get_xbar_clk(void)
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

	return freq / div;
}

#if CONFIG_IS_ENABLED(FSL_PFENG)
static u32 get_pfe_clk(void)
{
	u32 div, css_sel, freq = 0;

	css_sel = get_sel(MC_CGM2_BASE_ADDR, 0);
	div = get_div(MC_CGM2_BASE_ADDR, 0);

	switch (css_sel) {
	case MC_CGM_MUXn_CSC_SEL_FIRC:
		freq = FIRC_CLK_FREQ;
		break;
	case MC_CGM_MUXn_CSC_SEL_ACCEL_PLL_PHI1:
		freq = decode_pll(ACCEL_PLL, XOSC_CLK_FREQ, 0);
		break;
	default:
		printf("unsupported system clock select: 0x%x\n", css_sel);
		freq = 0;
	}

	return freq / div;
}
#endif

/* return clocks in Hz */
unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_UART_CLK:
		return get_uart_clk();
	case MXC_ESDHC_CLK:
	case MXC_USDHC_CLK:
		return get_usdhc_clk();
	case MXC_QSPI_CLK:
		return get_qspi_clk();
	case MXC_DSPI_CLK:
		return get_dspi_clk();
	case MXC_XBAR_CLK:
		return get_xbar_clk();
#if CONFIG_IS_ENABLED(FSL_PFENG)
	case MXC_PFE_CLK:
		return get_pfe_clk();
#endif
	/* TBD: get DDR clock */
	case MXC_DDR_CLK:
		return get_ddr_clk();
#ifdef CONFIG_SYS_I2C_MXC
	case MXC_I2C_CLK:
		return get_xbar_clk() / 3;
#endif
	default:
		break;
	}
	printf("Error: Unsupported function to read the frequency!\n");
	printf("Please define it correctly!\n");
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
#if CONFIG_IS_ENABLED(FSL_PFENG)
	printf("PFE  clock:     %5d MHz\n",
	       mxc_get_clock(MXC_PFE_CLK) / 1000000);
#endif
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
#ifdef CONFIG_S32G274A
	printf("CPU:\tNXP S32G274A");
	#ifdef CONFIG_TARGET_TYPE_S32GEN1_SIMULATOR
	printf("\n");
	#else
	printf(" rev. %d.%d.%d\n",
		   get_siul2_midr1_major() + 1,
		   get_siul2_midr1_minor(),
		   get_siul2_midr2_subminor());
	#endif  /* CONFIG_TARGET_TYPE_S32GEN1_SIMULATOR */
#elif defined(CONFIG_S32R45X)
	printf("CPU:\tNXP S32R45X\n");
#elif defined(CONFIG_S32V344)
	printf("CPU:\tNXP S32V344\n");
#endif
	printf("Reset cause: %s\n", get_reset_cause());

	return 0;
}
#endif

#if defined(CONFIG_SYS_FSL_DDRSS) && defined(CONFIG_TARGET_TYPE_S32GEN1_EMULATOR)
extern struct ddrss_conf ddrss_conf;
extern struct ddrss_firmware ddrss_firmware;
#endif

__weak int dram_init(void)
{
#if defined(CONFIG_SYS_FSL_DDRSS) && defined(CONFIG_TARGET_TYPE_S32GEN1_EMULATOR)
	ddrss_init(&ddrss_conf, &ddrss_firmware);
#elif defined(CONFIG_SYS_FSL_DDRSS) && defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
	uint32_t ret = 0;
	ret = ddr_init();
	if (ret) {
		printf("Error %d on ddr_init\n", ret);
		return ret;
	}
#endif
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

static int do_startm7(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 coreid = 0;
	unsigned long addr;
	char *ep;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], &ep, 16);
	if (ep == argv[1] || *ep != '\0')
		return CMD_RET_USAGE;

	if (!IS_ADDR_IN_IRAM(addr)) {
		printf("ERROR: Address 0x%08lX is not in internal SRAM ...\n",
		       addr);
		return CMD_RET_USAGE;
	}

	printf("Starting CM7_%d core at SRAM address 0x%08lX ... ",
	       coreid, addr);

	writel(readl(RGM_PRST(MC_RGM_PRST_CM7)) | PRST_PERIPH_CM7n_RST(coreid),
	       RGM_PRST(MC_RGM_PRST_CM7));
	while (!(readl(RGM_PSTAT(MC_RGM_PSTAT_CM7)) &
		 PSTAT_PERIPH_CM7n_STAT(coreid)))
		;

	/* Run in Thumb mode by setting BIT(0) of the address*/
	writel(addr | BIT(0), MC_ME_PRTN_N_CORE_M_ADDR(MC_ME_CM7_PRTN, coreid));

	writel(MC_ME_PRTN_N_CORE_M_PCONF_CCE,
	       MC_ME_PRTN_N_CORE_M_PCONF(MC_ME_CM7_PRTN, coreid));
	writel(MC_ME_PRTN_N_CORE_M_PUPD_CCUPD,
	       MC_ME_PRTN_N_CORE_M_PUPD(MC_ME_CM7_PRTN, coreid));
	writel(MC_ME_CTL_KEY_KEY, (MC_ME_BASE_ADDR));
	writel(MC_ME_CTL_KEY_INVERTEDKEY, (MC_ME_BASE_ADDR));
	while (!(readl(MC_ME_PRTN_N_CORE_M_STAT(MC_ME_CM7_PRTN, coreid)) &
		 MC_ME_PRTN_N_CORE_M_STAT_CCS))
		;

	writel(readl(RGM_PRST(MC_RGM_PRST_CM7)) &
	       (~PRST_PERIPH_CM7n_RST(coreid)),
	       RGM_PRST(MC_RGM_PRST_CM7));
	while (readl(RGM_PSTAT(MC_RGM_PSTAT_CM7)) &
	       PSTAT_PERIPH_CM7n_STAT(coreid))
		;

	printf("done.\n");

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
		startm7,	2,	1,	do_startm7,
		"start CM7_0 core from SRAM address",
		"<start_address>"
	  );

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
	return 0;
}
#endif

#ifdef CONFIG_FSL_DSPI
int mmap_dspi(unsigned short bus, struct dspi **base_addr)
{
	unsigned long addr;

	switch (bus) {
	case 0:
		addr = SPI0_BASE_ADDR;
		break;
	case 1:
		addr = SPI1_BASE_ADDR;
		break;
	case 2:
		addr = SPI2_BASE_ADDR;
		break;
	case 3:
		addr = SPI3_BASE_ADDR;
		break;
	case 4:
		addr = SPI4_BASE_ADDR;
		break;
	case 5:
		addr = SPI5_BASE_ADDR;
		break;
	default:
		return -ENODEV;
	}

	*base_addr = (struct dspi *)addr;
	return 0;
}
#endif
