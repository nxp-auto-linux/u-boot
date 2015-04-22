/*
 * Copyright 2013-2015 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/crm_regs.h>
#include <netdev.h>
#include <div64.h>
#ifdef CONFIG_FSL_ESDHC
#include <fsl_esdhc.h>
#endif

#ifdef CONFIG_FSL_ESDHC
DECLARE_GLOBAL_DATA_PTR;
#endif

#ifdef CONFIG_MXC_OCOTP
void enable_ocotp_clk(unsigned char enable)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 reg;

	reg = readl(&ccm->ccgr6);
	if (enable)
		reg |= CCM_CCGR6_OCOTP_CTRL_MASK;
	else
		reg &= ~CCM_CCGR6_OCOTP_CTRL_MASK;
	writel(reg, &ccm->ccgr6);
}
#endif


/* pll frequency decoding for pll_pfd in KHz             */
/* Only works for PLL1, PLL2, PLL3, PLL4, PLL6, PLL7     */
/* PLL5 not implemented                                  */
static u32 decode_pll(enum pll_clocks pll, u32 infreq, u8 pfd)
{
#if 0 /* b00450 */
	u32 freq_main=0;
	u32 mfi=0, pfd_frac=0;
	u32 pll_ctrl, pll_num , pll_denom, pll_pfd;

	struct anadig_reg *anadig = (struct anadig_reg *)ANADIG_BASE_ADDR;

	switch (pll) {
	case PLL1:
		pll_ctrl = readl(&anadig->pll1_ctrl);
		pll_num = readl(&anadig->pll1_num);
		pll_denom = readl(&anadig->pll1_denom);
		pll_pfd = readl(&anadig->pll1_pfd);

		mfi = (pll_ctrl & ANADIG_PLL1_CTRL_DIV_SELECT_MASK);

		if (mfi == 0)
			freq_main = infreq * 20; /* 480 MHz */
		else
			freq_main = infreq * 22; /* 528 MHz */
		
		switch(pfd){
		case 0:
			return freq_main;
		case 1:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD1_FRAC_MASK) >> ANADIG_PLL_PFD1_OFFSET;
			return freq_main*18/pfd_frac;
		case 2:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD2_FRAC_MASK) >> ANADIG_PLL_PFD2_OFFSET;
			return freq_main*18/pfd_frac;
		case 3:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD3_FRAC_MASK) >> ANADIG_PLL_PFD3_OFFSET;
			return freq_main*18/pfd_frac;
		case 4:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD4_FRAC_MASK) >> ANADIG_PLL_PFD4_OFFSET;
			return freq_main*18/pfd_frac;
		default:
			printf("pfd not supported:%d\n",pfd);
			return -1;
		}
	case PLL2:
		pll_ctrl = readl(&anadig->pll2_ctrl);
		pll_num = readl(&anadig->pll2_num);
		pll_denom = readl(&anadig->pll2_denom);
		pll_pfd = readl(&anadig->pll2_pfd);

		mfi = (pll_ctrl & ANADIG_PLL2_CTRL_DIV_SELECT_MASK);
		if (mfi == 0)
			freq_main = infreq * 20; /* 480 Mhz */
		else
			freq_main = infreq * 22; /* 528 Mhz */

		switch(pfd){
		case 0:
			return freq_main;
		case 1:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD1_FRAC_MASK) >> ANADIG_PLL_PFD1_OFFSET;
			return freq_main*18/pfd_frac;
		case 2:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD2_FRAC_MASK) >> ANADIG_PLL_PFD2_OFFSET;
			return freq_main*18/pfd_frac;
		case 3:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD3_FRAC_MASK) >> ANADIG_PLL_PFD3_OFFSET;
			return freq_main*18/pfd_frac;
		case 4:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD4_FRAC_MASK) >> ANADIG_PLL_PFD4_OFFSET;
			return freq_main*18/pfd_frac;
		default:
			printf("pfd not supported:%d\n",pfd);
			return -1;
		}
	case PLL3:
		pll_ctrl = readl(&anadig->pll3_ctrl);
		pll_pfd = readl(&anadig->pll3_pfd);

		mfi = (pll_ctrl & ANADIG_PLL3_CTRL_DIV_SELECT_MASK); /*bit 0*/
		if (mfi == 0)
			freq_main = infreq * 20; /* 480 Mhz */
		else
			freq_main = infreq * 22; /* 528 Mhz */

		switch(pfd){
		case 0:
			return freq_main;
		case 1:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD1_FRAC_MASK) >> ANADIG_PLL_PFD1_OFFSET;
			return freq_main*18/pfd_frac;
		case 2:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD2_FRAC_MASK) >> ANADIG_PLL_PFD2_OFFSET;
			return freq_main*18/pfd_frac;
		case 3:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD3_FRAC_MASK) >> ANADIG_PLL_PFD3_OFFSET;
			return freq_main*18/pfd_frac;
		case 4:
			pfd_frac = (pll_pfd & ANADIG_PLL_PFD4_FRAC_MASK) >> ANADIG_PLL_PFD4_OFFSET;
			return freq_main*18/pfd_frac;
		default:
			printf("pfd not supported:%d\n",pfd);
			return -1;
		}
	case PLL4:
		pll_ctrl = readl(&anadig->pll4_ctrl);
		pll_num = (readl(&anadig->pll4_num) & ANADIG_PLL_NUM_MASK);
		pll_denom = (readl(&anadig->pll4_denom) & ANADIG_PLL_DENOM_MASK);
		mfi = pll_ctrl & ANADIG_PLL4_CTRL_DIV_SELECT_MASK;

		return lldiv( (u64)infreq * ((u64)mfi*(u64)pll_denom + (u64)pll_num), pll_denom);
	case PLL5:
		printf("DPLL decode not supported for DPLL5\n");
		return -1;
	case PLL6:
		pll_ctrl = readl(&anadig->pll6_ctrl);
		pll_num = (readl(&anadig->pll6_num) & ANADIG_PLL_NUM_MASK);
		pll_denom = (readl(&anadig->pll6_denom) & ANADIG_PLL_DENOM_MASK);
		mfi = pll_ctrl & ANADIG_PLL6_CTRL_DIV_SELECT_MASK;

		return lldiv( (u64)infreq * ((u64)mfi*(u64)pll_denom + (u64)pll_num), pll_denom);
	case PLL7:
		pll_ctrl = readl(&anadig->pll7_ctrl);

		mfi = (pll_ctrl & ANADIG_PLL7_CTRL_DIV_SELECT) >> ANADIG_PLL7_CTRL_DIV_SELECT;
		if (mfi == 0)
			freq_main = infreq * 20; /* 480 Mhz */
		else
			freq_main = infreq * 22; /* 528 Mhz */

		switch(pfd){
		case 0:
			return freq_main;
		default:
			printf("pfd not supported:%d\n",pfd);
			return -1;
		}
	default:
		printf("not able to decode the PLL frequency - PLL ID doesn't exist\n");
		return -1;
	} /* switch(pll) */
#endif /* b00450 */
#if 1 /* b00450 */
return -1;
#endif /* b00450 */
}



static u32 get_mcu_main_clk(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_ccsr, ccm_cacrr, armclk_div;
	u32 sysclk_sel, pll_pfd_sel = 0;
	u32 freq = 0;
#if 0 /* b00450 */
	ccm_ccsr = readl(&ccm->ccsr);
	sysclk_sel = ccm_ccsr & CCM_CCSR_SYS_CLK_SEL_MASK;
	sysclk_sel >>= CCM_CCSR_SYS_CLK_SEL_OFFSET;

	ccm_cacrr = readl(&ccm->cacrr);
	armclk_div = ccm_cacrr & CCM_CACRR_ARM_CLK_DIV_MASK;
	armclk_div >>= CCM_CACRR_ARM_CLK_DIV_OFFSET;
	armclk_div += 1;

	switch (sysclk_sel) {
	case 0:
		freq = FASE_CLK_FREQ;
		break;
	case 1:
		freq = SLOW_CLK_FREQ;
		break;
	case 2:
		pll_pfd_sel = ccm_ccsr & CCM_CCSR_PLL2_PFD_CLK_SEL_MASK;
		pll_pfd_sel >>= CCM_CCSR_PLL2_PFD_CLK_SEL_OFFSET;

		freq = decode_pll(PLL2,24000,pll_pfd_sel)*1000;
		break;

	case 3:
		freq = decode_pll(PLL2,24000,0)*1000;
		break;
	case 4:
		pll_pfd_sel = ccm_ccsr & CCM_CCSR_PLL1_PFD_CLK_SEL_MASK;
		pll_pfd_sel >>= CCM_CCSR_PLL1_PFD_CLK_SEL_OFFSET;

		freq = decode_pll(PLL1,24000,pll_pfd_sel)*1000;
		break;
	case 5:
		freq = decode_pll(PLL3,24000,0)*1000;
		break;
	default:
		printf("unsupported system clock select\n");
	}

	return freq / armclk_div;
#endif /* b00450 */
	return 1; /* b00450 */
}

static u32 get_bus_clk(void)
{
#if 0 /* b00450 */	
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_cacrr, busclk_div;

	ccm_cacrr = readl(&ccm->cacrr);

	busclk_div = ccm_cacrr & CCM_CACRR_BUS_CLK_DIV_MASK;
	busclk_div >>= CCM_CACRR_BUS_CLK_DIV_OFFSET;
	busclk_div += 1;

	return get_mcu_main_clk() / busclk_div;
#endif /* b00450 */
	return 1; /* b00450 */
}

static u32 get_ipg_clk(void)
{
#if 0 /* b00450 */
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_cacrr, ipgclk_div;

	ccm_cacrr = readl(&ccm->cacrr);

	ipgclk_div = ccm_cacrr & CCM_CACRR_IPG_CLK_DIV_MASK;
	ipgclk_div >>= CCM_CACRR_IPG_CLK_DIV_OFFSET;
	ipgclk_div += 1;

	return get_bus_clk() / ipgclk_div;
#endif /* b00450 */
	return 1; /* b00450 */
}

static u32 get_uart_clk(void)
{
#if 0 /* b00450 */
	return get_ipg_clk();
#endif /* b00450 */
	return 1; /* b00450 */
}

static u32 get_sdhc_clk(void)
{
#if 0 /* b00450 */
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_cscmr1, ccm_cscdr2, sdhc_clk_sel, sdhc_clk_div;
	u32 freq = 0;

	ccm_cscmr1 = readl(&ccm->cscmr1);
	sdhc_clk_sel = ccm_cscmr1 & CCM_CSCMR1_ESDHC1_CLK_SEL_MASK;
	sdhc_clk_sel >>= CCM_CSCMR1_ESDHC1_CLK_SEL_OFFSET;

	ccm_cscdr2 = readl(&ccm->cscdr2);
	sdhc_clk_div = ccm_cscdr2 & CCM_CSCDR2_ESDHC1_CLK_DIV_MASK;
	sdhc_clk_div >>= CCM_CSCDR2_ESDHC1_CLK_DIV_OFFSET;
	sdhc_clk_div += 1;

	switch (sdhc_clk_sel) {
	case 0:
		freq = decode_pll(PLL3,24000,0)*1000;
		break;
	case 1:
		freq = decode_pll(PLL3,24000,3)*1000;
		break;
	case 2:
		freq = decode_pll(PLL1,24000,3)*1000;
		break;
	case 3:
		freq = get_bus_clk();
		break;
	}

	return freq / sdhc_clk_div;
#endif /* b00450 */
	return 1; /* b00450 */
}

u32 get_fec_clk(void)
{
#if 0 /* b00450 */
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_cscmr2, rmii_clk_sel;
	u32 freq = 0;

	ccm_cscmr2 = readl(&ccm->cscmr2);
	rmii_clk_sel = ccm_cscmr2 & CCM_CSCMR2_RMII_CLK_SEL_MASK;
	rmii_clk_sel >>= CCM_CSCMR2_RMII_CLK_SEL_OFFSET;

	switch (rmii_clk_sel) {
	case 0:
		freq = ENET_EXTERNAL_CLK;
		break;
	case 1:
		freq = AUDIO_EXTERNAL_CLK;
		break;
	case 2:
		freq = PLL5_MAIN_FREQ;
		break;
	case 3:
		freq = PLL5_MAIN_FREQ / 2;
		break;
	}

	return freq;
#endif /* b00450 */
	return 1; /* b00450 */
}

u32 get_nfc_clk(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	u32 ccm_cscmr1, ccm_cscdr3, ccm_cscdr2, nfc_clksel;
	u32 freq = 0, div, pre_div;

	ccm_cscmr1 = readl(&ccm->cscmr1);
	nfc_clksel = (ccm_cscmr1 & CCM_CSCMR1_NFC_CLK_SEL_MASK) >> CCM_CSCMR1_NFC_CLK_SEL_OFFSET;

	ccm_cscdr2 = readl(&ccm->cscdr2);
	ccm_cscdr3 = readl(&ccm->cscdr3);

	pre_div = (ccm_cscdr3 & CCM_CSCDR3_NFC_PRE_DIV_MASK) >> CCM_CSCDR3_NFC_PRE_DIV_OFFSET;
	div = (ccm_cscdr2 & CCM_CSCDR2_NFC_CLK_FRAC_DIV_MASK) >> CCM_CSCDR2_NFC_CLK_FRAC_DIV_OFFSET;
	
	switch(nfc_clksel)
	{
	case 0:
		freq = get_bus_clk();
		break;
	case 1:
		freq = decode_pll(PLL1,24000,1)*1000;
		break;
	case 2:
		freq = decode_pll(PLL3,24000,1)*1000;
		break;
	case 3:
		freq = decode_pll(PLL3,24000,3)*1000;
		break;
	}

	return freq / pre_div / div;
}
static u32 get_i2c_clk(void)
{
	return get_ipg_clk();
}

/* return clocks in Hz */
unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_ARM_CLK:
		return get_mcu_main_clk();
	case MXC_BUS_CLK:
		return get_bus_clk();
	case MXC_IPG_CLK:
		return get_ipg_clk();
	case MXC_UART_CLK:
#if 0 /* b00450 */
		return get_uart_clk();
#endif /* b00450 */
		return 40000000;
	case MXC_ESDHC_CLK:
		return get_sdhc_clk();
	case MXC_FEC_CLK:
		return get_fec_clk();
	case MXC_I2C_CLK:
		return get_i2c_clk();
	case MXC_NFC_CLK:
		return get_nfc_clk();
	default:
		break;
	}
	return -1;
}

/* Dump some core clocks */
int do_mac57d5xh_showclocks(cmd_tbl_t *cmdtp, int flag, int argc,
			 char * const argv[])
{
	printf("\n");
	printf("-------------------------------------------------------------------------------------------------------\n");
	printf("DPLLs settings:\n");
	printf("PLL1 main:%5d MHz - PFD1:%5d MHz - PFD2:%5d MHz - PFD3:%5d MHz - PFD4:%5d MHz\n",decode_pll(PLL1,24000,0)/1000,
		decode_pll(PLL1,24000,1)/1000, decode_pll(PLL1,24000,2)/1000, decode_pll(PLL1,24000,3)/1000, decode_pll(PLL1,24000,4)/1000);
	printf("PLL2 main:%5d MHz - PFD1:%5d MHz - PFD2:%5d MHz - PFD3:%5d MHz - PFD4:%5d MHz\n",decode_pll(PLL2,24000,0)/1000,
		decode_pll(PLL2,24000,1)/1000, decode_pll(PLL2,24000,2)/1000, decode_pll(PLL2,24000,3)/1000, decode_pll(PLL2,24000,4)/1000);
	printf("PLL3 main:%5d MHz - PFD1:%5d MHz - PFD2:%5d MHz - PFD3:%5d MHz - PFD4:%5d MHz\n",decode_pll(PLL3,24000,0)/1000,
		decode_pll(PLL3,24000,1)/1000, decode_pll(PLL3,24000,2)/1000, decode_pll(PLL3,24000,3)/1000, decode_pll(PLL3,24000,4)/1000);
	printf("PLL4 main:%5d MHz\n",decode_pll(PLL4,24000,0)/1000);
	printf("PLL6 main:%5d MHz\n",decode_pll(PLL6,24000,0)/1000);
	printf("--------------------------------------------------------------------------------------------------------\n");
	printf("Root clocks:\n");
	printf("CPU CA5 clock: %5d MHz\n", mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("BUS clock:     %5d MHz\n", mxc_get_clock(MXC_BUS_CLK) / 1000000);
	printf("IPG clock:     %5d MHz\n", mxc_get_clock(MXC_IPG_CLK) / 1000000);
	printf("eSDHC1 clock:  %5d MHz\n", mxc_get_clock(MXC_ESDHC_CLK) / 1000000);
	printf("FEC clock:     %5d MHz\n", mxc_get_clock(MXC_FEC_CLK) / 1000000);
	printf("UART clock:    %5d MHz\n", mxc_get_clock(MXC_UART_CLK) / 1000000);
	printf("NFC clock:     %5d MHz\n", mxc_get_clock(MXC_NFC_CLK) / 1000000);

	return 0;
}

U_BOOT_CMD(
	clocks, CONFIG_SYS_MAXARGS, 1, do_mac57d5xh_showclocks,
	"display clocks",
	""
);

#ifdef CONFIG_FEC_MXC
void imx_get_mac_from_fuse(int dev_id, unsigned char *mac)
{
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
}
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)
static char *get_reset_cause(void)
{
	u32 cause;
	struct src *src_regs = (struct src *)SRC_BASE_ADDR;

#if 0 /* b00450 */
	cause = readl(&src_regs->srsr);
	writel(cause, &src_regs->srsr);
	cause &= 0xff;
#endif /* b00450 */

	switch (cause) {
	case 0x08:
		return "WDOG";
	case 0x20:
		return "JTAG HIGH-Z";
	case 0x80:
		return "EXTERNAL RESET";
	case 0xfd:
		return "POR";
	default:
		return "unknown reset";
	}
}

#define SRC_SCR_SW_RST                  (1<<12)

void reset_cpu(ulong addr)
{
        struct src *src_regs = (struct src *)SRC_BASE_ADDR;

        /* Generate a SW reset from SRC SCR register */
        writel(SRC_SCR_SW_RST, &src_regs->scr);

        /* If we get there, we are not in good shape */
        mdelay(1000);
        printf("FATAL: Reset Failed!\n");
        hang();
};

int print_cpuinfo(void)
{
	printf("CPU:   Freescale Halo MAC57D5XH at %d MHz\n",
		mxc_get_clock(MXC_ARM_CLK) / 1000000);
	printf("Reset cause: %s\n", get_reset_cause());

	return 0;
}
#endif

int cpu_eth_init(bd_t *bis)
{
	int rc = -ENODEV;

#if defined(CONFIG_FEC_MXC)
	rc = fecmxc_initialize(bis);
#endif

	return rc;
}

#ifdef CONFIG_FSL_ESDHC
int cpu_mmc_init(bd_t *bis)
{
	return fsl_esdhc_mmc_init(bis);
}
#endif

int get_clocks(void)
{
#ifdef CONFIG_FSL_ESDHC
	gd->arch.sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);
#endif
	return 0;
}
