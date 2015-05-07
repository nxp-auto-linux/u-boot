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
#include <asm/arch/siul.h>
#include <asm/arch/mc_cgm_regs.h>
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/clock.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

void setup_iomux_ddr(void)
{
	/* TODO: Add iomux code for ddr */
}

void ddr_phy_init(void)
{
	/* TODO: Add initialisation code for ddr phy. */
}

void ddr_ctrl_init(void)
{
	/* TODO: Add setup code for ddr controller. */
}

int dram_init(void)
{
	setup_iomux_ddr();

	ddr_ctrl_init();

	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

static void setup_iomux_uart(void)
{
    /* Muxing for linflex */
    writel( 0x3, 0x400DCD68 );

    writel( 0x00080000, 0x400DC4F4 );

    writel( 0x02030003, 0x400DC4F8 );

}

static void setup_iomux_enet(void)
{
	/* TODO: Implement enet iomux when it is activated. */
}

static void setup_iomux_i2c(void)
{
	/* TODO: Implement i2c iomux when it is activated. */
}

#ifdef CONFIG_SYS_USE_NAND
void setup_iomux_nfc(void)
{
	/*TODO: Implement nfc iomux when it is activated.*/
}
#endif

#ifdef CONFIG_FSL_ESDHC
#if 0 /* Disable until the sdhc support will be activated */
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
	/*TODO: Implement this when shdc support is enabled.*/
	return 0;
}
#endif

static void clock_init(void)
{
#if 0 /* Disable until full suport for clock tree is added */
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

    /* activate the divisor 4 for System Clock (IOP) */
    writel( MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x7), &mc_cgm->mc_cgm_sc_dc4 );

    /* activate the divisor 4 for System Clock (2 x Platform Clock) */
    writel( MC_CGM_SC_DCn_DE | MC_CGM_SC_DCn_PREDIV(0x1), &mc_cgm->mc_cgm_sc_dc5 );

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
            PLLDIG_PLLDV_MFD(0x20),  &mc_cgm->pll0_plldv );

    writel( readl(&mc_cgm->pll0_pllfd) | PLLDIG_PLLFD_MFN_SET(0x0),  &mc_cgm->pll0_pllfd );

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
#endif

}

static void mscm_init(void)
{
#if 0 /* Desactivate until the memory map will be updated. */
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_IR_BASE_ADDR;
	int i;

	for (i = 0; i < MSCM_IRSPRC_NUM; i++)
		writew(MSCM_IRSPRC_CP0_EN, &mscmir->irsprc[i]);
#endif /* b00450 */
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_early_init_f(void)
{
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
	puts("Board: s32v234evb\n");

	return 0;
}
