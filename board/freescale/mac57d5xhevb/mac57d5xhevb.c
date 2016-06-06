/*
 * (C) Copyright 2013-2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/mc_cgm_regs.h>
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/siul.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c.h>
#include "mmu.h"

DECLARE_GLOBAL_DATA_PTR;

/*
 * Current DDR setup is realised for cut 1 Halo chip ,
 * which has the DDR unstable. On this chip DDR works only
 * using a workaround realised in MMU that sets up the DDR
 * as a noncacheable memory area.
 * Taking into consideration the fact the cut2 Halo chip has
 * been produced, the code remains written using magic numbers
 * until the u-boot will be tested in the new cut.
 */

void setup_iomux_ddr(void)
{
	/* SIUL2 for DDR */
	writel( 0x22500000, 0x400DC680 );
	writel( 0x22500000, 0x400DC684 );
	writel( 0x22500000, 0x400DC688 );
	writel( 0x22500000, 0x400DC68C );
	writel( 0x22500000, 0x400DC690 );
	writel( 0x22500000, 0x400DC694 );
	writel( 0x22500000, 0x400DC698 );
	writel( 0x22500000, 0x400DC69C );
	writel( 0x22500000, 0x400DC6a0 );
	writel( 0x22500000, 0x400DC6a4 );
	writel( 0x22500000, 0x400DC6a8 );
	writel( 0x22500000, 0x400DC6ac );
	writel( 0x22500000, 0x400DC6b0 );
	writel( 0x22500000, 0x400DC6b4 );
	writel( 0x22500000, 0x400DC6b8 );
	writel( 0x22500000, 0x400DC6bc );
	writel( 0x22500000, 0x400DC6c0 );
	writel( 0x22500000, 0x400DC6c4 );
	writel( 0x22500000, 0x400DC6c8 );
	writel( 0x22500000, 0x400DC6cc );
	writel( 0x22500000, 0x400DC6d0 );
	writel( 0x22500000, 0x400DC6d4 );
	writel( 0x22500000, 0x400DC6d8 );
	writel( 0x22500000, 0x400DC6dc );
	writel( 0x22500000, 0x400DC6e0 );
	writel( 0x22500000, 0x400DC6e4 );
	writel( 0x22500000, 0x400DC6e8 );
	writel( 0x22500000, 0x400DC6ec );
	writel( 0x22500000, 0x400DC6f0 );
	writel( 0x22500000, 0x400DC6f4 );
	writel( 0x22500000, 0x400DC6f8 );
	writel( 0x22500000, 0x400DC6fc );
	writel( 0x22500000, 0x400DC700 );
	writel( 0x22000000, 0x400DC704 );
	writel( 0x22000000, 0x400DC708 );
	writel( 0x22000000, 0x400DC70c );
	writel( 0x22000000, 0x400DC710 );
	writel( 0x22000000, 0x400DC714 );
	writel( 0x22000000, 0x400DC718 );
	writel( 0x22000000, 0x400DC71c );
	writel( 0x22000000, 0x400DC720 );
	writel( 0x22000000, 0x400DC724 );
	writel( 0x22000000, 0x400DC728 );
	writel( 0x22000000, 0x400DC72c );
	writel( 0x22000000, 0x400DC730 );
	writel( 0x22000000, 0x400DC734 );
	writel( 0x22000000, 0x400DC738 );
	writel( 0x22000000, 0x400DC73c );
	writel( 0x22000000, 0x400DC740 );
	writel( 0x22000000, 0x400DC744 );
	writel( 0x22000000, 0x400DC748 );
	writel( 0x22000000, 0x400DC74c );
	writel( 0x22000000, 0x400DC750 );
	writel( 0x22000000, 0x400DC754 );
	writel( 0x22000000, 0x400DC758 );
	writel( 0x22500000, 0x400DC75C );
	writel( 0x22500000, 0x400DC760 );
	writel( 0x22500000, 0x400DC764 );
	writel( 0x22500000, 0x400DC768 );
	writel( 0x22000000, 0x400DC76C );
	writel( 0x22000000, 0x400DC770 );
	writel( 0x22000000, 0x400DC774 );
	writel( 0x22000000, 0x400DC778 );
	writel( 0x22000000, 0x400DC77c );
	writel( 0x22000000, 0x400DC784 );
	writel( 0x22000000, 0x400DC780 );
	writel( 0x22000000, 0x400DC788 );
	writel( 0x22080000, 0x400DC7d4 );
}

void ddr_phy_init(void)
{
	writel( 0x4d494b45, 0x400EC000 );
	writel( 0x49534d45, 0x400EC000 );
	writel( 0x00000041, 0x400EC014 );
	writel( 0x00000004, 0x400EC040 );
	writel( 0x0000000B, 0x400EC048 );
	writel( 0x00000009, 0x400EC048 );
	writel( 0x20000000, 0x400EC044 );
	writel( 0x00000005, 0x400EC040 );
	writel( 0x00000004, 0x400EC040 );
	writel( 0x80030000, 0x400D4400 );
}

void ddr_ctrl_init(void)
{
	writel( 0x01F4281E, 0x40024004 );
	writel( 0x56B9226F, 0x40024008 );
	writel( 0x58A10CA6, 0x4002400c );

	writel( 0x00000000, 0x4002403c );
	writel( 0x00000000, 0x40024038 );
	writel( 0x33000000, 0x40024060 );

	writel( 0xFA801CC0, 0x40024000 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );

	udelay(100);

	writel( 0x00100400, 0x40024010 );
	writel( 0x00020000, 0x40024010 );
	writel( 0x00030000, 0x40024010 );
	writel( 0x00010404, 0x40024010 );

	writel( 0x0952, 0x40024010 );
	writel( 0x00100400, 0x40024010 );
	writel( 0x00080000, 0x40024010 );

	udelay(100);

	writel( 0x00080000, 0x40024010 );

	writel( 0x0852, 0x40024010 );

	writel( 0x000107C4, 0x40024010 );
	writel( 0x00010444, 0x40024010 );

	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );
	writel( 0x00380000, 0x40024010 );

	writel( 0xFA801CC0, 0x40024000 );

	writel( readl(0x40024000) & 0xEFFFFFFF, 0x40024000);

}

int dram_init(void)
{
	ddr_phy_init();

	setup_iomux_ddr();

	ddr_ctrl_init();

	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

static void setup_iomux_uart(void)
{
	/* Muxing for linflex */
	SIUL2_MSCR_UART_Tx(2);
	SIUL2_MSCR_UART_Rx(2);
}


static void setup_iomux_enet(void)
{
#if 0 /* The below enet setting are for Vybrid and are
		not usable on Halo. */
	static const iomux_v3_cfg_t enet0_pads[] = {
		MAC57D5XH_PAD_PTA6__RMII0_CLKIN,
		MAC57D5XH_PAD_PTC1__RMII0_MDIO,
		MAC57D5XH_PAD_PTC0__RMII0_MDC,
		MAC57D5XH_PAD_PTC2__RMII0_CRS_DV,
		MAC57D5XH_PAD_PTC3__RMII0_RD1,
		MAC57D5XH_PAD_PTC4__RMII0_RD0,
		MAC57D5XH_PAD_PTC5__RMII0_RXER,
		MAC57D5XH_PAD_PTC6__RMII0_TD1,
		MAC57D5XH_PAD_PTC7__RMII0_TD0,
		MAC57D5XH_PAD_PTC8__RMII0_TXEN,
	};

	imx_iomux_v3_setup_multiple_pads(enet0_pads, ARRAY_SIZE(enet0_pads));
#endif
}

static void setup_iomux_i2c(void)
{
#if 0 /*  The below i2c setting are for Vybrid and are
		not usable on Halo. */
	static const iomux_v3_cfg_t i2c0_pads[] = {
		MAC57D5XH_PAD_PTB14__I2C0_SCL,
		MAC57D5XH_PAD_PTB15__I2C0_SDA,
	};

	imx_iomux_v3_setup_multiple_pads(i2c0_pads, ARRAY_SIZE(i2c0_pads));
#endif
}

#ifdef CONFIG_SYS_USE_NAND
void setup_iomux_nfc(void)
{
#if 0 /* The below enet setting are for Vybrid and are
		not usable on Halo. */
	static const iomux_v3_cfg_t nfc_pads[] = {
		MAC57D5XH_PAD_PTD31__NFC_IO15,
		MAC57D5XH_PAD_PTD30__NFC_IO14,
		MAC57D5XH_PAD_PTD29__NFC_IO13,
		MAC57D5XH_PAD_PTD28__NFC_IO12,
		MAC57D5XH_PAD_PTD27__NFC_IO11,
		MAC57D5XH_PAD_PTD26__NFC_IO10,
		MAC57D5XH_PAD_PTD25__NFC_IO9,
		MAC57D5XH_PAD_PTD24__NFC_IO8,
		MAC57D5XH_PAD_PTD23__NFC_IO7,
		MAC57D5XH_PAD_PTD22__NFC_IO6,
		MAC57D5XH_PAD_PTD21__NFC_IO5,
		MAC57D5XH_PAD_PTD20__NFC_IO4,
		MAC57D5XH_PAD_PTD19__NFC_IO3,
		MAC57D5XH_PAD_PTD18__NFC_IO2,
		MAC57D5XH_PAD_PTD17__NFC_IO1,
		MAC57D5XH_PAD_PTD16__NFC_IO0,
		MAC57D5XH_PAD_PTB24__NFC_WEB,
		MAC57D5XH_PAD_PTB25__NFC_CE0B,
		MAC57D5XH_PAD_PTB27__NFC_REB,
		MAC57D5XH_PAD_PTC26__NFC_RBB,
		MAC57D5XH_PAD_PTC27__NFC_ALE,
		MAC57D5XH_PAD_PTC28__NFC_CLE,
	};
	imx_iomux_v3_setup_multiple_pads(nfc_pads, ARRAY_SIZE(nfc_pads));
#endif /* b00450 */
}
#endif

#ifdef CONFIG_FSL_ESDHC
#if 0 /* There are mmcs on HALO. */
struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{ESDHC1_BASE_ADDR},
};
#endif

int board_mmc_getcd(struct mmc *mmc)
{
	/* eSDHC1 is always present */
	return 1;
}

int board_mmc_init(bd_t *bis)
{
#if 0 /* There are mmcs on HALO. These function should be kept
	until the configuration will be updated. */
	static const iomux_v3_cfg_t esdhc1_pads[] = {
		MAC57D5XH_PAD_PTA24__ESDHC1_CLK,
		MAC57D5XH_PAD_PTA25__ESDHC1_CMD,
		MAC57D5XH_PAD_PTA26__ESDHC1_DAT0,
		MAC57D5XH_PAD_PTA27__ESDHC1_DAT1,
		MAC57D5XH_PAD_PTA28__ESDHC1_DAT2,
		MAC57D5XH_PAD_PTA29__ESDHC1_DAT3,
	};

	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_ESDHC_CLK);

	imx_iomux_v3_setup_multiple_pads(
		esdhc1_pads, ARRAY_SIZE(esdhc1_pads));

	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
#endif
	return 0;
}
#endif

static void clock_init(void)
{
	volatile struct mc_cgm_reg * mc_cgm = (struct mc_cgm_reg *)MC_CGM0_BASE_ADDR;
	volatile struct mc_me_reg * mc_me = (struct mc_me_reg *)MC_ME0_BASE_ADDR;

	/* enable all modes, enable all peripherals */
	writel( MC_ME_ME_RUN3 | MC_ME_ME_RUN2 | MC_ME_ME_RUN1 | MC_ME_ME_RUN0, &mc_me->mc_me_me );

	writel( MC_ME_RUN_PCn_DRUN | MC_ME_RUN_PCn_RUN0 | MC_ME_RUN_PCn_RUN1 |
			MC_ME_RUN_PCn_RUN2 | MC_ME_RUN_PCn_RUN3, &mc_me->mc_me_run_pc0 );


	/* turn on FXOSC, SRIC, SXOSC */
	writel( MC_ME_RUNMODE_MC_MVRON | MC_ME_RUNMODE_MC_FLAON(0x3) |
			MC_ME_RUNMODE_MC_FXOSCON | MC_ME_RUNMODE_MC_SIRCON |
			MC_ME_RUNMODE_MC_SXOSCON, &mc_me->mc_me_run_mc0 );

	writel( MC_ME_MCTL_RUN0 | MC_ME_MCTL_KEY , &mc_me->mc_me_mctl);
	writel( MC_ME_MCTL_RUN0 | MC_ME_MCTL_INVERTEDKEY , &mc_me->mc_me_mctl);

	while( (readl(&mc_me->mc_me_gs) & MC_ME_GS_S_MTRANS) != 0x00000000 );

	/* Set PLL source to FXOSC for AUX CLK Selector 0 */
	writel( MC_CGM_ACn_SEL_SET(MC_CGM_ACn_SEL_FXOSC) , &mc_cgm->mc_cgm_ac0_sc);

	while( (readl(&mc_cgm->mc_cgm_ac0_ss) & MC_CGM_ACn_SEL_MASK) != 0x01000000 );

	/* activate the divisor 0 for DDR */
	writel( MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x1), &mc_cgm->mc_cgm_sc_dc0 );

	/* activate the divisor 1 for System Clock (CA5 core) */
	writel( MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x1), &mc_cgm->mc_cgm_sc_dc1 );

	/* activate the divisor 1 for System Clock (CM4) */
	writel( MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x3), &mc_cgm->mc_cgm_sc_dc2 );

	/* activate the divisor 3 for System Clock (Peripherals: PIT) */
	writel( MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x7), &mc_cgm->mc_cgm_sc_dc3 );

	/* activate the divisor 5 for System Clock (IOP) */
	writel( MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x7), &mc_cgm->mc_cgm_sc_dc4 );


	/* activate the divisor 5 for System Clock (2 x Platform Clock) */
	writel( MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x1), &mc_cgm->mc_cgm_sc_dc5 );

	/* activate the divisor 6 for System Clock (1 x DDR/SDR */
	writel( MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x1), &mc_cgm->mc_cgm_sc_dc6 );

	/* Configure PLL0 Dividers - 640MHz from 40Mhx XOSC
	 PLL input = FXOSC = 40MHz
	 VCO range = 600-1280MHz
	 PREDIV = 1 =? /1
	 RFDPHI1= 2 => /8
	 RFDPHI = 0 => /2
	 MFD    = 32 => /32
	 VCO frequency = PLL input / PREDIV x MFD = 40/1 * 32 = 1280 MHz
	 PLL out = VCO / (2^RFDPHI*2) = 1280 / 2 = 640 MHz
	*/

	writel( PLLDIG_PLLDV_RFDPHI_SET(0x0) | PLLDIG_PLLDV_PREDIV_SET(0x1) |
			PLLDIG_PLLDV_MFD(0x20),	 &mc_cgm->pll0_plldv );

	writel( readl(&mc_cgm->pll0_pllfd) | PLLDIG_PLLFD_MFN_SET(0x0),	 &mc_cgm->pll0_pllfd );

	writel( readl(&mc_cgm->pll0_pllcal3) | PLLDIG_PLLCAL3_MFDEN_SET(0x1), &mc_cgm->pll0_pllcal3 );

	/* turn on FXOSC, SRIC, SXOSC */
	writel( MC_ME_RUNMODE_MC_MVRON | MC_ME_RUNMODE_MC_FLAON(0x3) |
			MC_ME_RUNMODE_MC_PLL0ON | MC_ME_RUNMODE_MC_FXOSCON | MC_ME_RUNMODE_MC_SIRCON |
			MC_ME_RUNMODE_MC_SXOSCON | MC_ME_RUNMODE_MC_SYSCLK(0x4), &mc_me->mc_me_run_mc0 );

	writel( MC_ME_MCTL_RUN0 | MC_ME_MCTL_KEY , &mc_me->mc_me_mctl);
	writel( MC_ME_MCTL_RUN0 | MC_ME_MCTL_INVERTEDKEY , &mc_me->mc_me_mctl);

	while( (readl(&mc_me->mc_me_gs) & MC_ME_GS_S_MTRANS) != 0x00000000 );

	/* select PLL0 as source clock for AUXCLK Selector 2 (Linflex; SPI)*/
	writel( MC_CGM_ACn_SEL_SET(MC_CGM_ACn_SEL_PLL0), &mc_cgm->mc_cgm_ac2_sc );

	/* activate the divisor 1 for AUXCLOCK Selector 2 (Linflex) */
	writel( MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x3), &mc_cgm->mc_cgm_ac2_dc1 );

	/* select PLL0 as source clock for AUXCLK Selector 10 (ENET)*/
	writel( MC_CGM_ACn_SEL_SET(MC_CGM_ACn_SEL_PLL0), &mc_cgm->mc_cgm_ac10_sc );

	/* activate the divisor 0 for AUXCLOCK Selector 10 (ENET) */
	writel( MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0xF), &mc_cgm->mc_cgm_ac10_dc0 );

	/* select PLL0 as source clock for AUXCLK Selector 11 (ENET_TIMER)*/
	writel( MC_CGM_ACn_SEL_SET(MC_CGM_ACn_SEL_PLL0), &mc_cgm->mc_cgm_ac10_sc );

	/* activate the divisor 0 for AUXCLOCK Selector 10 (ENET_TIMER) */
	writel( MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0xF), &mc_cgm->mc_cgm_ac10_dc0 );

	/* turn on FXOSC, SRIC, SXOSC */
	writel( MC_ME_RUNMODE_MC_MVRON | MC_ME_RUNMODE_MC_FLAON(0x3) |
			MC_ME_RUNMODE_MC_PLL0ON | MC_ME_RUNMODE_MC_FXOSCON | MC_ME_RUNMODE_MC_SIRCON |
			MC_ME_RUNMODE_MC_SXOSCON | MC_ME_RUNMODE_MC_SYSCLK(0x4), &mc_me->mc_me_run_mc0 );

	writel( MC_ME_MCTL_RUN0 | MC_ME_MCTL_KEY , &mc_me->mc_me_mctl);
	writel( MC_ME_MCTL_RUN0 | MC_ME_MCTL_INVERTEDKEY , &mc_me->mc_me_mctl);


	while( (readl(&mc_me->mc_me_gs) & MC_ME_GS_S_MTRANS) != 0x00000000 );

	/* enable clock for Linflex(0,1,2) peripheral */
	writeb( 0x0, &mc_me->mc_me_pctl26 );
	writeb( 0x0, &mc_me->mc_me_pctl25 );
	writeb( 0x0, &mc_me->mc_me_pctl24 );
	/* PIT */
	writeb( 0x0, &mc_me->mc_me_pctl36 );
	/* SIUL */
	writeb( 0x0, &mc_me->mc_me_pctl60 );
	/* ENET */
	writeb( 0x0, &mc_me->mc_me_pctl196 );

}

static void mscm_init(void)
{
#if 0 /* The MSCM ip was not validated on Halo */
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_IR_BASE_ADDR;
	int i;

	for (i = 0; i < MSCM_IRSPRC_NUM; i++)
		writew(MSCM_IRSPRC_CP0_EN, &mscmir->irsprc[i]);
#endif
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_early_init_f(void)
{
#ifdef CONFIG_DDR_WORKAROUND
	enable_DDRWorkaround();
#endif
	clock_init();
	mscm_init();

	setup_iomux_uart();
	setup_iomux_enet();
	setup_iomux_i2c();
#ifdef CONFIG_SYS_USE_NAND
	setup_iomux_nfc();
#endif
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: mac57d5xhevb\n");

	return 0;
}
