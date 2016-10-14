/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/mc_cgm_regs.h>
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/mc_rgm_regs.h>
#include <asm/arch/siul.h>
#include <asm/arch/src.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <asm/arch/mmdc.h>
#include <asm/arch/ddr.h>
#if defined(CONFIG_S32V234_LPDDR2)
#include <asm/arch/lpddr2.h>
#elif defined(CONFIG_S32V234_DDR3)
#include <asm/arch/ddr3.h>
#else
#error "Please define the DDR type for S32V234 board!"
#endif
#include <netdev.h>
#include <div64.h>
#include <errno.h>

#ifdef CONFIG_FSL_ESDHC
DECLARE_GLOBAL_DATA_PTR;
#endif

u32 get_cpu_rev(void)
{
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_BASE_ADDR;
	u32 cpu = readl(&mscmir->cpxtype);

	return cpu;
}

static uintptr_t get_pllfreq(	u32 pll, u32 refclk_freq, u32 plldv,
				u32 pllfd, u32 selected_output  )
{
	u32 vco = 0, plldv_prediv = 0, plldv_mfd = 0,	pllfd_mfn = 0;
	u32 plldv_rfdphi_div = 0, fout = 0;
	u32 dfs_portn = 0, dfs_mfn = 0, dfs_mfi = 0;

	if( selected_output > DFS_MAXNUMBER )
	{
		return -1;
	}

	plldv_prediv = (plldv & PLLDIG_PLLDV_PREDIV_MASK) >> PLLDIG_PLLDV_PREDIV_OFFSET;
	plldv_mfd = (plldv & PLLDIG_PLLDV_MFD_MASK);

	pllfd_mfn = (pllfd & PLLDIG_PLLFD_MFN_MASK);

	plldv_prediv = plldv_prediv == 0 ? 1 : plldv_prediv;

	/* The formula for VCO is from TR manual, rev. 1 */
	vco = (refclk_freq / plldv_prediv) * (plldv_mfd + pllfd_mfn/(float)20480);

	if( selected_output != 0 )
	{
		/* Determine the RFDPHI for PHI1 */
		plldv_rfdphi_div = (plldv & PLLDIG_PLLDV_RFDPHI1_MASK) >> PLLDIG_PLLDV_RFDPHI1_OFFSET;
		plldv_rfdphi_div = plldv_rfdphi_div == 0 ? 1 : plldv_rfdphi_div;
		if( pll == ARM_PLL || pll == ENET_PLL || pll == DDR_PLL )
		{
			dfs_portn = readl(DFS_DVPORTn(pll, selected_output - 1));
			dfs_mfi = (dfs_portn & DFS_DVPORTn_MFI_MASK) >> DFS_DVPORTn_MFI_OFFSET;
			dfs_mfn = (dfs_portn & DFS_DVPORTn_MFN_MASK) >> DFS_DVPORTn_MFN_OFFSET;

			dfs_mfi <<= 8;
			vco /= plldv_rfdphi_div;
			fout = vco / ( dfs_mfi + dfs_mfn );
			fout <<=8;

		}
		else
		{
			fout = vco / plldv_rfdphi_div;
		}

	}
	else
	{
		/* Determine the RFDPHI for PHI0 */
		plldv_rfdphi_div = (plldv & PLLDIG_PLLDV_RFDPHI_MASK) >> PLLDIG_PLLDV_RFDPHI_OFFSET;
		fout = vco / plldv_rfdphi_div;
	}

	return fout;

}
/* Implemented for ARMPLL, PERIPH_PLL, ENET_PLL, DDR_PLL, VIDEO_LL */
static uintptr_t decode_pll( enum pll_type pll, u32 refclk_freq, u32 selected_output )
{
	u32 plldv, pllfd;
	int freq;

	plldv = readl( PLLDIG_PLLDV(pll) );
	pllfd = readl( PLLDIG_PLLFD(pll) );

	freq = get_pllfreq( pll, refclk_freq, plldv, pllfd, selected_output );
	return freq  < 0 ? 0 : freq;
}

static u32 get_mcu_main_clk(void)
{
	u32 coreclk_div;
	u32 sysclk_sel;
	u32 freq = 0;

	sysclk_sel = readl(CGM_SC_SS(MC_CGM1_BASE_ADDR)) & MC_CGM_SC_SEL_MASK;
	sysclk_sel >>= MC_CGM_SC_SEL_OFFSET;

	coreclk_div = readl(CGM_SC_DCn(MC_CGM1_BASE_ADDR, 0)) & MC_CGM_SC_DCn_PREDIV_MASK;
	coreclk_div >>= MC_CGM_SC_DCn_PREDIV_OFFSET;
	coreclk_div += 1;

	switch (sysclk_sel) {
	case MC_CGM_SC_SEL_FIRC:
		freq = FIRC_CLK_FREQ;
		break;
	case MC_CGM_SC_SEL_XOSC:
		freq = XOSC_CLK_FREQ;
		break;
	case MC_CGM_SC_SEL_ARMPLL:
		/* ARMPLL has as source XOSC and CORE_CLK has as input PHI0*/
		freq = decode_pll(ARM_PLL, XOSC_CLK_FREQ, 0);
		break;
	case MC_CGM_SC_SEL_CLKDISABLE:
		printf("Sysclk is disabled\n");
		break;
	default:
		printf("unsupported system clock select\n");
		freq = 0;
	}

	return freq / coreclk_div;
}

static u32 get_sys_clk(u32 number)
{
	u32 sysclk_div, sysclk_div_number ;
	u32 sysclk_sel;
	u32 freq = 0;

	switch (number) {
		case 3:
			sysclk_div_number = 0;
			break;
		case 6:
			sysclk_div_number = 1;
			break;
		default:
			printf("unsupported system clock \n");
			sysclk_div_number = 0;
	}
	sysclk_sel = readl(CGM_SC_SS(MC_CGM0_BASE_ADDR)) & MC_CGM_SC_SEL_MASK;
	sysclk_sel >>= MC_CGM_SC_SEL_OFFSET;


	sysclk_div = readl(CGM_SC_DCn(MC_CGM0_BASE_ADDR, sysclk_div_number)) & MC_CGM_SC_DCn_PREDIV_MASK;
	sysclk_div >>= MC_CGM_SC_DCn_PREDIV_OFFSET;
	sysclk_div += 1;

	switch (sysclk_sel) {
	case MC_CGM_SC_SEL_FIRC:
		freq = FIRC_CLK_FREQ;
		break;
	case MC_CGM_SC_SEL_XOSC:
		freq = XOSC_CLK_FREQ;
		break;
	case MC_CGM_SC_SEL_ARMPLL:
		/* ARMPLL has as source XOSC and SYSn_CLK has as input DFS1*/
		freq = decode_pll(ARM_PLL, XOSC_CLK_FREQ, 1);
		break;
	case MC_CGM_SC_SEL_CLKDISABLE:
		printf("Sysclk is disabled\n");
		freq = 0;
		break;
	default:
		printf("unsupported system clock select\n");
		freq = 0;
	}
	return freq / sysclk_div;
}

static u32 get_peripherals_clk(void)
{
	u32 auxclk5_div, auxclk5_sel, freq = 0;
	#define SOURCE_CLK_DIV (5)

	auxclk5_sel = readl(CGM_ACn_SS(MC_CGM0_BASE_ADDR, 5)) & MC_CGM_ACn_SEL_MASK;
	auxclk5_sel >>= MC_CGM_ACn_SEL_OFFSET;

	auxclk5_div = readl(CGM_ACn_DCm(MC_CGM0_BASE_ADDR, 5, 0)) & MC_CGM_ACn_DCm_PREDIV_MASK;
	auxclk5_div >>= MC_CGM_ACn_DCm_PREDIV_OFFSET;
	auxclk5_div += 1;

	switch (auxclk5_sel) {
	case MC_CGM_ACn_SEL_FIRC:
			freq = FIRC_CLK_FREQ;
			break;
	case MC_CGM_ACn_SEL_XOSC:
			freq = XOSC_CLK_FREQ;
			break;
	case MC_CGM_ACn_SEL_PERPLLDIVX:
			freq = decode_pll(PERIPH_PLL, XOSC_CLK_FREQ, 0)/SOURCE_CLK_DIV;
			break;
	default:
			printf("unsupported source clock\n");
			freq = 0;
	}

	return freq / auxclk5_div;
	#undef SOURCE_CLK_DIV
}

static u32 get_uart_clk(void)
{
	u32 auxclk3_div, auxclk3_sel, freq = 0;
	#define SOURCE_CLK_DIV (3)

	auxclk3_sel = readl(CGM_ACn_SS(MC_CGM0_BASE_ADDR, 3)) & MC_CGM_ACn_SEL_MASK;
	auxclk3_sel >>= MC_CGM_ACn_SEL_OFFSET;

	auxclk3_div =readl(CGM_ACn_DCm(MC_CGM0_BASE_ADDR, 3, 0)) & MC_CGM_ACn_DCm_PREDIV_MASK;
	auxclk3_div >>= MC_CGM_ACn_DCm_PREDIV_OFFSET;
	auxclk3_div += 1;

	switch (auxclk3_sel) {
	case MC_CGM_ACn_SEL_FIRC:
			freq = FIRC_CLK_FREQ;
			break;
	case MC_CGM_ACn_SEL_XOSC:
			freq = XOSC_CLK_FREQ;
			break;
	case MC_CGM_ACn_SEL_PERPLLDIVX:
			freq = decode_pll(PERIPH_PLL, XOSC_CLK_FREQ, 0)/SOURCE_CLK_DIV;
			break;
	case MC_CGM_ACn_SEL_SYSCLK:
			freq = get_sys_clk( 6 );
			break;
	default:
			printf("unsupported system clock select\n");
			freq = 0;
	}

	return freq/auxclk3_div;
	#undef SOURCE_CLK_DIV
}

static u32 get_fec_clk(void)
{
	u32 auxclk2_div;
	u32 freq = 0;

	auxclk2_div =  readl(CGM_ACn_DCm(MC_CGM2_BASE_ADDR, 2, 0)) & MC_CGM_ACn_DCm_PREDIV_MASK;
	auxclk2_div >>= MC_CGM_ACn_DCm_PREDIV_OFFSET;
	auxclk2_div += 1;

	freq = decode_pll(ENET_PLL, XOSC_CLK_FREQ, 0);

	return freq / auxclk2_div;

}

static u32 get_usdhc_clk(void)
{
	u32 auxclk15_div;
	u32 freq = 0;

	auxclk15_div =  readl(CGM_ACn_DCm(MC_CGM0_BASE_ADDR, 15, 0)) & MC_CGM_ACn_DCm_PREDIV_MASK;
	auxclk15_div >>= MC_CGM_ACn_DCm_PREDIV_OFFSET;
	auxclk15_div += 1;

	freq = decode_pll(ENET_PLL, XOSC_CLK_FREQ, 4);

	return freq / auxclk15_div;
}



static u32 get_i2c_clk(void)
{
	return get_peripherals_clk();
}

static u32 get_qspi_clk(void)
{
	u32 auxclk14_div, auxclk14_sel, freq = 0;
	#define AUXn 14

	auxclk14_sel = readl(CGM_ACn_SS(MC_CGM0_BASE_ADDR, AUXn)) & MC_CGM_ACn_SEL_MASK;
	auxclk14_sel >>= MC_CGM_ACn_SEL_OFFSET;

	auxclk14_div =readl(CGM_ACn_DCm(MC_CGM0_BASE_ADDR, AUXn, 0)) & MC_CGM_ACn_DCm_PREDIV_MASK;
	auxclk14_div >>= MC_CGM_ACn_DCm_PREDIV_OFFSET;
	auxclk14_div += 1;

	switch (auxclk14_sel) {
	case MC_CGM_ACn_SEL_FIRC:
			freq = FIRC_CLK_FREQ;
			break;
	case MC_CGM_ACn_SEL_XOSC:
			freq = XOSC_CLK_FREQ;
			break;
	case MC_CGM_ACn_SEL_ENETPLL:
			freq = decode_pll(ENET_PLL, XOSC_CLK_FREQ, 3);
			break;
	default:
			printf("unsupported system clock select\n");
			freq = 0;
	}

	return freq/auxclk14_div;
	#undef AUXn
}

/* return clocks in Hz */
unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return get_mcu_main_clk();
	case MXC_PERIPHERALS_CLK:
		return get_peripherals_clk();
	case MXC_UART_CLK:
		return get_uart_clk();
	case MXC_FEC_CLK:
		return get_fec_clk();
	case MXC_I2C_CLK:
		return get_i2c_clk();
	case MXC_USDHC_CLK:
		return get_usdhc_clk();
	case MXC_SYS6_CLK:
		return get_sys_clk(6);
	case MXC_QSPI_CLK:
		return get_qspi_clk();
	default:
		break;
	}
	printf("Error: Unsupported function to read the frequency! \
			Please define it correctly!");
	return 0;
}

/* Dump some core clocks */
int do_s32v234_showclocks(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	printf("Root clocks:\n");
	printf("CPU clock:%5d MHz\n",
		mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("PERIPHERALS clock: %5d MHz\n",
		mxc_get_clock(MXC_PERIPHERALS_CLK) / 1000000);
	printf("uSDHC clock:	   %5d MHz\n",
		mxc_get_clock(MXC_USDHC_CLK) / 1000000);
	printf("FEC clock:	%5d MHz\n",
		mxc_get_clock(MXC_FEC_CLK) / 1000000);
	printf("UART clock: 	%5d MHz\n",
		mxc_get_clock(MXC_UART_CLK) / 1000000);
	printf("QSPI clock:	%5d MHz\n",
		mxc_get_clock(MXC_QSPI_CLK) / 1000000);

	return 0;
}

U_BOOT_CMD(
	clocks, CONFIG_SYS_MAXARGS, 1, do_s32v234_showclocks,
	"display clocks",
	""
);

#ifdef CONFIG_FEC_MXC
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
#if 0 /* This feature will be implemented in ALB-123 */
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[4];
	struct fuse_bank4_regs *fuse =
		(struct fuse_bank4_regs *)bank->fuse_regs;

	u32 value = readl(&fuse->mac_addr0);
	mac[0] = (value >> 8);
	mac[1] = value;

	value = readl(&fuse->mac_addr1);
	mac[2] = value >> 24;
	mac[3] = value >> 16;
	mac[4] = value >> 8;
	mac[5] = value;
#else
	mac[0] = 0x00;
	mac[1] = 0x1B;
	mac[2] = 0xC3;
	mac[3] = 0x12;
	mac[4] = 0x34;
	mac[5] = 0x22;
#endif
}
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)
static char *get_reset_cause(void)
{
	u32 cause = readl(MC_RGM_FES);

	switch (cause) {
		case F_SWT4:
			return "WDOG";
		case F_JTAG:
			return "JTAG";
		case F_FCCU_SOFT:
			return "FCCU soft reaction";
		case F_FCCU_HARD:
			return "FCCU hard reaction";
		case F_SOFT_FUNC:
			return "Software Functional reset";
		case F_ST_DONE:
			return "Self Test done reset";
		case F_EXT_RST:
			return "External reset";
		default:
			return "unknown reset";
	}

}

void reset_cpu(ulong addr)
{

	entry_to_target_mode(MC_ME_MCTL_RESET);

	/* If we get there, we are not in good shape */
	mdelay(1000);
	printf("FATAL: Reset Failed!\n");
	hang();
};

int print_cpuinfo(void)
{
	printf("CPU:   Freescale Treerunner S32V234 at %d MHz\n",
		mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("Reset cause: %s\n", get_reset_cause());

	return 0;
}
#endif

int cpu_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

#if defined(CONFIG_FEC_MXC)

/* enable RGMII mode */
#if (CONFIG_FEC_XCV_TYPE == RGMII)
	volatile struct src * src = (struct src *)SRC_SOC_BASE_ADDR;
	writel( SRC_GPR3_ENET_MODE, &src->gpr3);
#endif

	rc = fecmxc_initialize(bis);
#endif

	return rc;
}

static int detect_boot_interface(void)
{
	volatile struct src * src = (struct src *)SRC_SOC_BASE_ADDR;

	u32 reg_val;
	int value;
	reg_val = readl(&src->bmr1);
	value = reg_val & SRC_BMR1_CFG1_MASK;
	value = value >> SRC_BMR1_CFG1_BOOT_SHIFT;

	if( (value != SRC_BMR1_CFG1_QuadSPI) &&
	    (value != SRC_BMR1_CFG1_SD) &&
	    (value != SRC_BMR1_CFG1_eMMC) )
	{
		printf("Unknown booting environment \n");
		value = -1;
	}

	return value;

}

int get_clocks(void)
{
#ifdef CONFIG_FSL_ESDHC
	gd->arch.sdhc_clk = mxc_get_clock(MXC_USDHC_CLK);
#endif
	return 0;
}

__weak void setup_iomux_sdhc(void)
{
	/* Set iomux PADS for USDHC */

	/* PK6 pad: uSDHC clk */
	writel(SIUL2_USDHC_PAD_CTRL_CLK, SIUL2_MSCRn(150));
	writel(0x3, SIUL2_MSCRn(902));

	/* PK7 pad: uSDHC CMD */
	writel(SIUL2_USDHC_PAD_CTRL_CMD, SIUL2_MSCRn(151));
	writel(0x3, SIUL2_MSCRn(901));

	/* PK8 pad: uSDHC DAT0 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT0_3, SIUL2_MSCRn(152));
	writel(0x3, SIUL2_MSCRn(903));

	/* PK9 pad: uSDHC DAT1 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT0_3, SIUL2_MSCRn(153));
	writel(0x3, SIUL2_MSCRn(904));

	/* PK10 pad: uSDHC DAT2 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT0_3, SIUL2_MSCRn(154));
	writel(0x3, SIUL2_MSCRn(905));

	/* PK11 pad: uSDHC DAT3 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT0_3, SIUL2_MSCRn(155));
	writel(0x3, SIUL2_MSCRn(906));

	/* PK15 pad: uSDHC DAT4 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT4_7, SIUL2_MSCRn(159));
	writel(0x3, SIUL2_MSCRn(907));

	/* PL0 pad: uSDHC DAT5 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT4_7, SIUL2_MSCRn(160));
	writel(0x3, SIUL2_MSCRn(908));

	/* PL1 pad: uSDHC DAT6 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT4_7, SIUL2_MSCRn(161));
	writel(0x3, SIUL2_MSCRn(909));

	/* PL2 pad: uSDHC DAT7 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT4_7, SIUL2_MSCRn(162));
	writel(0x3, SIUL2_MSCRn(910));
}

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{USDHC_BASE_ADDR},
};

__weak int board_mmc_getcd(struct mmc *mmc)
{
	/* eSDHC1 is always present */
	return 1;
}
__weak int sdhc_setup(bd_t *bis)
{
	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_USDHC_CLK);

	setup_iomux_sdhc();

	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}
int board_mmc_init(bd_t *bis)
{
	int ret = detect_boot_interface();
	if( ret < 0 )
		return -1;

	if( ret != SRC_BMR1_CFG1_QuadSPI )
	{
		return sdhc_setup(bis);
	}
	else
	{
		return 0;
	}
}
#endif

static int do_sdhc_setup(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	int ret;
	printf("Hyperflash is disabled. SD/eMMC is active and can be used\n");

	struct mmc * mmc = find_mmc_device(0);

	if( mmc == NULL )
	{
		return sdhc_setup(gd->bd);
	}

	/* set the sdhc pinmuxing */
	setup_iomux_sdhc();

	/* reforce the mmc's initialization */
	ret = mmc_init(mmc);
	if( ret )
	{
		printf("Impossible to configure the SDHC controller.\
			Please check the SDHC jumpers\n");
		return 1;
	}
	return 0;
}
/* sdhc setup */
U_BOOT_CMD(
	sdhcsetup, 1, 1, do_sdhc_setup,
	"setup sdhc pinmuxing and sdhc registers for access to SD",
	"\n"
	"Set up the sdhc pinmuxing and sdhc registers to access the SD\n"
	"and disconnect from the Hyperflash.\n"
);

void setup_iomux_ddr(void)
{
	ddr_config_iomux(DDR0);
	ddr_config_iomux(DDR1);
}

void ddr_ctrl_init(void)
{
	config_mmdc(0);
	config_mmdc(1);
}

#ifdef CONFIG_DDR_HANDSHAKE_AT_RESET
void ddr_check_post_func_reset(uint8_t module) {

	uint32_t ddr_self_ref_clr, mmdc_mapsr;
	unsigned long mmdc_addr;
	volatile struct src *src = (struct src *)SRC_SOC_BASE_ADDR;

	mmdc_addr = (module) ? MMDC1_BASE_ADDR : MMDC0_BASE_ADDR;
	ddr_self_ref_clr = (module) ? SRC_DDR_EN_SELF_REF_CTRL_DDR1_SLF_REF_CLR
				    : SRC_DDR_EN_SELF_REF_CTRL_DDR0_SLF_REF_CLR;

	/* Check if DDR is still in refresh mode */
	if(src->ddr_self_ref_ctrl & ddr_self_ref_clr) {

		mmdc_mapsr = readl(mmdc_addr + MMDC_MAPSR);
		writel(mmdc_mapsr | MMDC_MAPSR_EN_SLF_REF, mmdc_addr + MMDC_MAPSR);

		src->ddr_self_ref_ctrl = src->ddr_self_ref_ctrl | ddr_self_ref_clr;

		mmdc_mapsr = readl(mmdc_addr + MMDC_MAPSR);
		writel(mmdc_mapsr & ~MMDC_MAPSR_EN_SLF_REF, mmdc_addr + MMDC_MAPSR);
	}
}
#endif

__weak int dram_init(void)
{
#ifdef CONFIG_DDR_HANDSHAKE_AT_RESET
	uint32_t enabled_hs_events, func_event;

	/* Enable DDR handshake for all functional events */
	volatile struct src *src = (struct src *)SRC_SOC_BASE_ADDR;

	writel(MC_RGM_DDR_HE_VALUE, MC_RGM_DDR_HE);
	writel(MC_RGM_FRHE_ALL_VALUE, MC_RGM_FRHE);

	src->ddr_self_ref_ctrl = src->ddr_self_ref_ctrl |
				 SRC_DDR_EN_SLF_REF_VALUE;

	/* If reset event was received, check DDR state */
	func_event = readl(MC_RGM_FES);
	enabled_hs_events = readl(MC_RGM_FRHE);
	if(func_event & enabled_hs_events) {
		if(func_event & MC_RGM_FES_ANY_FUNC_EVENT) {

			/* Check if DDR handshake was done */
			while(!(readl(MC_RGM_DDR_HS) & MC_RGM_DDR_HS_HNDSHK_DONE));

			ddr_check_post_func_reset(DDR0);
			ddr_check_post_func_reset(DDR1);
		}
	}
#endif
	setup_iomux_ddr();

	ddr_ctrl_init();

	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

/* start M4 core */
static int do_start_m4(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	unsigned long addr;
	char *ep;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], &ep, 16);
	if (ep == argv[1] || *ep != '\0')
		return CMD_RET_USAGE;

	if (!IS_ADDR_IN_IRAM(addr)) {
		printf("ERROR: Address 0x%08lX not in internal SRAM ...\n", addr);
		return CMD_RET_USAGE;
	}

	printf("Starting core M4 at SRAM address 0x%08lX ...\n", addr);

	/* write the M4 core's start address
	   address is required by the hardware to be odd */
	writel(addr | 0x1, MC_ME_CADDR0);

	/* enable CM4 to be active during all modes of operation */
	writew(MC_MC_CCTL_CORE_ACTIVE, MC_MC_CCTL0);

	/* mode_enter(DRUN_M) */
	writel( MC_ME_MCTL_RUN0 | MC_ME_MCTL_KEY, MC_ME_MCTL );
	writel( MC_ME_MCTL_RUN0 | MC_ME_MCTL_INVERTEDKEY, MC_ME_MCTL );

	printf("Wait while mode entry is in progress ...\n");

	/* wait while mode entry is in process */
	while( (readl(MC_ME_GS) & MC_ME_GS_S_MTRANS) != 0x00000000 );

	printf("Wait for the run mode to be entered ...\n");

	/* check if the mode has been entered */
	while( (readl(MC_ME_GS) & MC_ME_GS_S_CRT_MODE_DRUN) != 0x00000000 );

	printf("M4 started.\n");

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
	startm4,	2,	1,	do_start_m4,
	"start M4 core from SRAM address",
	"startAddress"
);

extern void dma_mem_clr(int addr, int size);

static int do_init_sram(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
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
		printf("ERROR: Address 0x%08lX not in internal SRAM ...\n", addr);
		return CMD_RET_USAGE;
	}

	max_size = IRAM_SIZE - (addr - IRAM_BASE_ADDR);
	if (size > max_size) {
		printf("WARNING: given size exceeds SRAM boundaries.\n");
		size = max_size;
	}
	printf("Init SRAM region at address 0x%08lX, size 0x%X bytes ...\n", addr, size);
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
        if (cse_init()) {
                printf("Failed to initialize CSE3 security engine\n");
        }
#endif
        return 0;
}
#endif
