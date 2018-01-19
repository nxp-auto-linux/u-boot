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
	return (~(readl(RGM_PSTAT(RGM_CORES_RESET_GROUP)))) >> 1;
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
	float vco = 0;

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
	vco = (refclk_freq / (float)plldv_rdiv) *
		(plldv_mfi + pllfd_mfn / (float)18432);

	if (selected_output < PHI_MAXNUMBER) {
		/* Determine the div for PHI. */
		phi_nr = selected_output;
		pllodiv_div = readl(PLLDIG_PLLODIV(pll, phi_nr));
		pllodiv_div = (pllodiv_div & PLLDIG_PLLODIV_DIV_MASK) >>
			PLLDIG_PLLODIV_DIV_OFFSET;
		fout = vco / (pllodiv_div + 1);
	} else if (pll == ARM_PLL || pll == PERIPH_PLL) {
		/* Determine the div for DFS. */
		dfs_nr = selected_output - DFS_MAXNUMBER + 1;

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

/* Implemented for ARM_PLL, PERIPH_PLL, ACCEL_PLL, DDR_PLL, AURORA_PLL. */
static u32 decode_pll(enum pll_type pll, u32 refclk_freq,
		u32 selected_output)
{
	u32 plldv, pllfd, freq;

	plldv = readl(PLLDIG_PLLDV(pll));
	pllfd = readl(PLLDIG_PLLFD(pll));

	freq = get_pllfreq(pll, refclk_freq, plldv, pllfd, selected_output);
	return freq  < 0 ? 0 : freq;
}

static u32 get_uart_clk(void)
{
	u32 div, css_sel, dc, freq = 0;

	css_sel = readl(CGM_MUXn_CSS(MC_CGM0_BASE_ADDR, 8));
	css_sel = MC_CGM_MUXn_CSS_SELSTAT(css_sel);

	dc = readl(CGM_MUXn_DCm(MC_CGM0_BASE_ADDR, 8, 0));
	/* If div is enabled. */
	if (dc & MC_CGM_MUXn_DCm_DE) {
		div = (dc & MC_CGM_MUXn_DCm_DIV_MASK) >>
			MC_CGM_MUXn_DCm_DIV_OFFSET;
		div += 1;
	} else {
		div = 1;
	}

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

/* return clocks in Hz */
unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_UART_CLK:
		return get_uart_clk();
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

	return 0;
}

U_BOOT_CMD(
		clocks, CONFIG_SYS_MAXARGS, 1, do_s32_showclocks,
		"display clocks",
		""
	 );

#if defined(CONFIG_DISPLAY_CPUINFO)
static char *get_reset_cause(void)
{
	return "unknown reset";
}

void reset_cpu(ulong addr)
{
	printf("FATAL: Reset Failed!\n");
};

int print_cpuinfo(void)
{
	printf("CPU:   NXP S32X\n");
	printf("Reset cause: %s\n", get_reset_cause());

	return 0;
}
#endif


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

extern void dma_mem_clr(int addr, int size);

static int do_init_sram(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	unsigned long addr;
	unsigned size, max_size;
	char *ep;

	if (argc < 3)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], &ep, 16);
	if (ep == argv[1] || *ep != '\0')
		return CMD_RET_USAGE;

	size = simple_strtoul(argv[2], &ep, 16);
	if (ep == argv[2] || *ep != '\0')
		return CMD_RET_USAGE;

	if (!IS_ADDR_IN_IRAM(addr)) {
		printf("ERROR: Address 0x%08lX not in internal SRAM ...\n",
		       addr);
		return CMD_RET_USAGE;
	}

	max_size = IRAM_SIZE - (addr - IRAM_BASE_ADDR);
	if (size > max_size) {
		printf("WARNING: given size exceeds SRAM boundaries.\n");
		size = max_size;
	}
	printf("Init SRAM region at address 0x%08lX, size 0x%X bytes ...\n",
	       addr, size);
	dma_mem_clr(addr, size);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
		initsram,	3,	1,	do_init_sram,
		"init SRAM from address",
		"startAddress[hex] size[hex]"
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
