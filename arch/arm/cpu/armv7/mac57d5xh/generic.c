/*
 * (C) Copyright 2013-2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/mc_cgm_regs.h>
#include <asm/arch/mc_me_regs.h>
#include <netdev.h>
#include <div64.h>
#include <errno.h>
static u32 get_plldv_rfdphi_div( u32 plldv )
{
    u32 plldv_rfdphi = (plldv & PLLDIG_PLLDV_RFDPHI_MASK) >> PLLDIG_PLLDV_RFDPHI_OFFSET;

    if( plldv_rfdphi & ~PLLDIG_PLLDV_RFDPHI_MAXVALUE )
    {
        printf("Error: plldv_rfdphi with value %d is incorrect set \n", plldv_rfdphi);
        return -1;
    }

    if( (plldv_rfdphi & PLLDIG_PLLDV_RFDPHIBY_32)  )
    {
        plldv_rfdphi = PLLDIG_PLLDV_RFDPHIBY_32 + 1;
    }
    return 1 << (plldv_rfdphi+1);
}

static u32 get_pllfreq( u32 pll, u32 infreq, u32 pllcal3, u32 plldv, u32 pllfd )
{
    u32 vco = 0, plldv_prediv = 0, plldv_mfd = 0, pllcal3_mfen = 0, pllfd_mfn = 0, plldv_rfdphi_div = 0;
    plldv_prediv = (plldv & PLLDIG_PLLDV_PREDIV_MASK) >> PLLDIG_PLLDV_PREDIV_OFFSET;
    plldv_mfd = (plldv & PLLDIG_PLLDV_MFD_MASK);

    pllcal3_mfen = (pllcal3 & PLLDIG_PLLCAL3_MFDEN_MASK) >> PLLDIG_PLLCAL3_MFDEN_OFFSET;

    pllfd_mfn = (pllfd & PLLDIG_PLLFD_MFN_MASK);

    switch(plldv_prediv){
        case 0:
            plldv_prediv = 1;
            break;
        case 7:
            printf("plldv_prediv value is not supported:%d for PLL%d\n", plldv_prediv, pll);
            return -1;
    }

    vco  = infreq / plldv_prediv * (plldv_mfd + pllfd_mfn/(pllcal3_mfen + 1));

    plldv_rfdphi_div =  get_plldv_rfdphi_div(plldv);
    if( plldv_rfdphi_div == -1)
    {
        return -1;
    }

    return vco / plldv_rfdphi_div;

}
/* Only works for PLL0, PLL1, PLL2, PLL3 */
/* PLL5 not implemented                                  */
static u32 decode_pll(enum clocks pll, u32 infreq)
{
    u32 pllcal3, plldv , pllfd;

    volatile struct mc_cgm_reg * mc_cgm = (struct mc_cgm_reg *)MC_CGM0_BASE_ADDR;

    switch (pll) {
    case PLL0:
        pllcal3 = readl(&mc_cgm->pll0_pllcal3);
        plldv = readl(&mc_cgm->pll0_plldv);
        pllfd = readl(&mc_cgm->pll0_pllfd);
        break;
    case PLL1:
    case PLL2:
        case PLL3:

    default:
        printf("not able to decode the PLL frequency - PLL ID doesn't exist\n");
        return -1;
    } /* switch(pll) */

    return get_pllfreq( pll, infreq, pllcal3, plldv, pllfd );
}



static u32 get_mcu_main_clk(void)
{

    volatile struct mc_cgm_reg * mc_cgm = (struct mc_cgm_reg *)MC_CGM0_BASE_ADDR;
    u32 armclk_div;
    u32 sysclk_sel;
    u32 freq = 0;

    sysclk_sel = readl(&mc_cgm->mc_cgm_sc_ss) & MC_CGM_SC_SEL_MASK;
    sysclk_sel >>= MC_CGM_SC_SEL_OFFSET;

    armclk_div = readl(&mc_cgm->mc_cgm_sc_dc1) & MC_CGM_SC_DCn_PREDIV_MASK;
    armclk_div >>= MC_CGM_SC_DCn_PREDIV_OFFSET;
    armclk_div += 1;

    switch (sysclk_sel) {
    case FIRC:
        freq = FIRC_CLK_FREQ*1000;
        break;
    case FXOSC:
        freq = FXOSC_CLK_FREQ*1000;
        break;
    case PLL0:
    case PLL1:
        case PLL2:
        case PLL3:
        /* PLLx has as source FXOSC */
        freq = decode_pll(sysclk_sel, FXOSC_CLK_FREQ)*1000;
        break;
    default:
        printf("unsupported system clock select\n");
    }

    return freq / armclk_div;

}

static u32 get_peripherals_clk(void)
{

    volatile struct mc_cgm_reg * mc_cgm = (struct mc_cgm_reg *)MC_CGM0_BASE_ADDR;
    u32 perclk_div;

    perclk_div =  readl(&mc_cgm->mc_cgm_sc_dc3) & MC_CGM_SC_DCn_PREDIV_MASK;
    perclk_div >>= MC_CGM_SC_DCn_PREDIV_OFFSET;
    perclk_div += 1;

    return get_mcu_main_clk() / perclk_div;
}

static u32 get_uart_clk(void)
{
    volatile struct mc_cgm_reg * mc_cgm = (struct mc_cgm_reg *)MC_CGM0_BASE_ADDR;
    u32 linflexclk_div;
    u32 auxclk_sel2;
    u32 freq = 0;

    auxclk_sel2 = readl(&mc_cgm->mc_cgm_ac2_ss) & MC_CGM_ACn_SEL_MASK;
    auxclk_sel2 >>= MC_CGM_ACn_SEL_OFFSET;

    linflexclk_div = readl(&mc_cgm->mc_cgm_ac2_dc1) & MC_CGM_SC_DCn_PREDIV_MASK;
    linflexclk_div >>= MC_CGM_ACn_DCn_PREDIV_OFFSET;
    linflexclk_div += 1;

    switch (auxclk_sel2) {
        case FIRC:
                freq = FIRC_CLK_FREQ*1000;
                break;
        case FXOSC:
                freq = FXOSC_CLK_FREQ*1000;
                break;
        case PLL0:
        case PLL1:
        case PLL2:
        case PLL3:
                freq = decode_pll(PLL0, FXOSC_CLK_FREQ)*1000;
                break;
        default:
                printf("unsupported system clock select\n");
    }

    return freq/linflexclk_div;
}

u32 get_fec_clk(void)
{
    volatile struct mc_cgm_reg * mc_cgm = (struct mc_cgm_reg *)MC_CGM0_BASE_ADDR;
    u32 enetclk_div;
    u32 auxclk_sel10;
    u32 freq = 0;

    auxclk_sel10 = readl(&mc_cgm->mc_cgm_ac10_ss) & MC_CGM_ACn_SEL_MASK;
    auxclk_sel10 >>= MC_CGM_ACn_SEL_OFFSET;

    enetclk_div = readl(&mc_cgm->mc_cgm_ac10_dc0) & MC_CGM_SC_DCn_PREDIV_MASK;
    enetclk_div >>= MC_CGM_ACn_DCn_PREDIV_OFFSET;
    enetclk_div += 1;

    switch (auxclk_sel10) {
    case FIRC:
            freq = FIRC_CLK_FREQ*1000;
            break;
    case FXOSC:
            freq = FXOSC_CLK_FREQ*1000;
            break;
    case PLL0:
    case PLL1:
    case PLL2:
    case PLL3:
            freq = decode_pll(auxclk_sel10, FXOSC_CLK_FREQ)*1000;
            break;
    default:
            printf("unsupported system clock select\n");
    }

    return freq/enetclk_div;
}

static u32 get_i2c_clk(void)
{
    return get_peripherals_clk();
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
    printf("PLL1 main:%5d MHz - PFD1:%5d MHz - PFD2:%5d MHz - PFD3:%5d MHz - PFD4:%5d MHz\n",decode_pll(PLL1,24000)/1000,
        decode_pll(PLL0,24000)/1000, decode_pll(PLL1,24000)/1000, decode_pll(PLL1,24000)/1000, decode_pll(PLL1,24000)/1000);
#if 0
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
#endif

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
#if 0 /* This feature will be enabled when the enet muxing and clock will 
	be implemented */
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
#endif
}
#endif

#if defined(CONFIG_DISPLAY_CPUINFO)
static char *get_reset_cause(void)
{
    u32 cause;
#if 0 /* SRC IP is not available on Halo. This platform has
	RGM ip.
	 */
    struct src *src_regs = (struct src *)SRC_BASE_ADDR;

    cause = readl(&src_regs->srsr);
    writel(cause, &src_regs->srsr);
    cause &= 0xff;
#endif

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



void reset_cpu(ulong addr)
{
#if 0 /* This feature will be enabled when the enet muxing and clock will
        be implemented */

	#define SRC_SCR_SW_RST                  (1<<12)
    struct src *src_regs = (struct src *)SRC_BASE_ADDR;

    /* Generate a SW reset from SRC SCR register */
    writel(SRC_SCR_SW_RST, &src_regs->scr);

    /* If we get there, we are not in good shape */
    mdelay(1000);
    printf("FATAL: Reset Failed!\n");
    hang();
#endif
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

int get_clocks(void)
{
    return 0;
}
