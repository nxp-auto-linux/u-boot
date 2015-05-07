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
#include <asm/arch/iomux-mac57d5xh.h>
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
#if 0 /* The below iomux settings are from Vybrid. They should
	be implemented for Halo using SIUL*/
    static const iomux_v3_cfg_t ddr_pads[] = {
        MAC57D5XH_PAD_DDR_A15__DDR_A_15,
        MAC57D5XH_PAD_DDR_A15__DDR_A_15,
        MAC57D5XH_PAD_DDR_A14__DDR_A_14,
        MAC57D5XH_PAD_DDR_A13__DDR_A_13,
        MAC57D5XH_PAD_DDR_A12__DDR_A_12,
        MAC57D5XH_PAD_DDR_A11__DDR_A_11,
        MAC57D5XH_PAD_DDR_A10__DDR_A_10,
        MAC57D5XH_PAD_DDR_A9__DDR_A_9,
        MAC57D5XH_PAD_DDR_A8__DDR_A_8,
        MAC57D5XH_PAD_DDR_A7__DDR_A_7,
        MAC57D5XH_PAD_DDR_A6__DDR_A_6,
        MAC57D5XH_PAD_DDR_A5__DDR_A_5,
        MAC57D5XH_PAD_DDR_A4__DDR_A_4,
        MAC57D5XH_PAD_DDR_A3__DDR_A_3,
        MAC57D5XH_PAD_DDR_A2__DDR_A_2,
        MAC57D5XH_PAD_DDR_A1__DDR_A_1,
        MAC57D5XH_PAD_DDR_BA2__DDR_BA_2,
        MAC57D5XH_PAD_DDR_BA1__DDR_BA_1,
        MAC57D5XH_PAD_DDR_BA0__DDR_BA_0,
        MAC57D5XH_PAD_DDR_CAS__DDR_CAS_B,
        MAC57D5XH_PAD_DDR_CKE__DDR_CKE_0,
        MAC57D5XH_PAD_DDR_CLK__DDR_CLK_0,
        MAC57D5XH_PAD_DDR_CS__DDR_CS_B_0,
        MAC57D5XH_PAD_DDR_D15__DDR_D_15,
        MAC57D5XH_PAD_DDR_D14__DDR_D_14,
        MAC57D5XH_PAD_DDR_D13__DDR_D_13,
        MAC57D5XH_PAD_DDR_D12__DDR_D_12,
        MAC57D5XH_PAD_DDR_D11__DDR_D_11,
        MAC57D5XH_PAD_DDR_D10__DDR_D_10,
        MAC57D5XH_PAD_DDR_D9__DDR_D_9,
        MAC57D5XH_PAD_DDR_D8__DDR_D_8,
        MAC57D5XH_PAD_DDR_D7__DDR_D_7,
        MAC57D5XH_PAD_DDR_D6__DDR_D_6,
        MAC57D5XH_PAD_DDR_D5__DDR_D_5,
        MAC57D5XH_PAD_DDR_D4__DDR_D_4,
        MAC57D5XH_PAD_DDR_D3__DDR_D_3,
        MAC57D5XH_PAD_DDR_D2__DDR_D_2,
        MAC57D5XH_PAD_DDR_D1__DDR_D_1,
        MAC57D5XH_PAD_DDR_D0__DDR_D_0,
        MAC57D5XH_PAD_DDR_DQM1__DDR_DQM_1,
        MAC57D5XH_PAD_DDR_DQM0__DDR_DQM_0,
        MAC57D5XH_PAD_DDR_DQS1__DDR_DQS_1,
        MAC57D5XH_PAD_DDR_DQS0__DDR_DQS_0,
        MAC57D5XH_PAD_DDR_RAS__DDR_RAS_B,
        MAC57D5XH_PAD_DDR_WE__DDR_WE_B,
        MAC57D5XH_PAD_DDR_ODT1__DDR_ODT_0,
        MAC57D5XH_PAD_DDR_ODT0__DDR_ODT_1,
    };

    imx_iomux_v3_setup_multiple_pads(ddr_pads, ARRAY_SIZE(ddr_pads));
#endif



}

void ddr_phy_init(void)
{
#if 0 /* The below DDR settings are for Vybrid */
    struct ddrmr_regs *ddrmr = (struct ddrmr_regs *)DDR_BASE_ADDR;

    writel(DDRMC_PHY_DQ_TIMING, &ddrmr->phy[0]);
    writel(DDRMC_PHY_DQ_TIMING, &ddrmr->phy[16]);
    writel(DDRMC_PHY_DQ_TIMING, &ddrmr->phy[32]);
    writel(DDRMC_PHY_DQ_TIMING, &ddrmr->phy[48]);

    writel(DDRMC_PHY_DQS_TIMING, &ddrmr->phy[1]);
    writel(DDRMC_PHY_DQS_TIMING, &ddrmr->phy[17]);
    writel(DDRMC_PHY_DQS_TIMING, &ddrmr->phy[33]);
    writel(DDRMC_PHY_DQS_TIMING, &ddrmr->phy[49]);

    writel(DDRMC_PHY_CTRL, &ddrmr->phy[2]);
    writel(DDRMC_PHY_CTRL, &ddrmr->phy[18]);
    writel(DDRMC_PHY_CTRL, &ddrmr->phy[34]);
    writel(DDRMC_PHY_CTRL, &ddrmr->phy[50]);

    writel(DDRMC_PHY_MASTER_CTRL, &ddrmr->phy[3]);
    writel(DDRMC_PHY_MASTER_CTRL, &ddrmr->phy[19]);
    writel(DDRMC_PHY_MASTER_CTRL, &ddrmr->phy[35]);
    writel(DDRMC_PHY_MASTER_CTRL, &ddrmr->phy[51]);

    writel(DDRMC_PHY_SLAVE_CTRL, &ddrmr->phy[4]);
    writel(DDRMC_PHY_SLAVE_CTRL, &ddrmr->phy[20]);
    writel(DDRMC_PHY_SLAVE_CTRL, &ddrmr->phy[36]);
    writel(DDRMC_PHY_SLAVE_CTRL, &ddrmr->phy[52]);

    writel(DDRMC_PHY50_DDR3_MODE | DDRMC_PHY50_EN_SW_HALF_CYCLE,
        &ddrmr->phy[50]);
#endif
}

void ddr_ctrl_init(void)
{
#if 0 /* The below DDR settings are for Vybrid */
    struct ddrmr_regs *ddrmr = (struct ddrmr_regs *)DDR_BASE_ADDR;

    writel(DDRMC_CR00_DRAM_CLASS_DDR3, &ddrmr->cr[0]);
    writel(DDRMC_CR02_DRAM_TINIT(32), &ddrmr->cr[2]);
    writel(DDRMC_CR10_TRST_PWRON(124), &ddrmr->cr[10]);

    writel(DDRMC_CR11_CKE_INACTIVE(80000), &ddrmr->cr[11]);
    writel(DDRMC_CR12_WRLAT(5) | DDRMC_CR12_CASLAT_LIN(12), &ddrmr->cr[12]);
    writel(DDRMC_CR13_TRC(21) | DDRMC_CR13_TRRD(4) | DDRMC_CR13_TCCD(4) |
        DDRMC_CR13_TBST_INT_INTERVAL(4), &ddrmr->cr[13]);
    writel(DDRMC_CR14_TFAW(20) | DDRMC_CR14_TRP(6) | DDRMC_CR14_TWTR(4) |
        DDRMC_CR14_TRAS_MIN(15), &ddrmr->cr[14]);
    writel(DDRMC_CR16_TMRD(4) | DDRMC_CR16_TRTP(4), &ddrmr->cr[16]);
    writel(DDRMC_CR17_TRAS_MAX(28080) | DDRMC_CR17_TMOD(12),
        &ddrmr->cr[17]);
    writel(DDRMC_CR18_TCKESR(4) | DDRMC_CR18_TCKE(3), &ddrmr->cr[18]);

    writel(DDRMC_CR20_AP_EN, &ddrmr->cr[20]);
    writel(DDRMC_CR21_TRCD_INT(6) | DDRMC_CR21_TRAS_LOCKOUT |
        DDRMC_CR21_CCMAP_EN, &ddrmr->cr[21]);

    writel(DDRMC_CR22_TDAL(11), &ddrmr->cr[22]);
    writel(DDRMC_CR23_BSTLEN(3) | DDRMC_CR23_TDLL(512), &ddrmr->cr[23]);
    writel(DDRMC_CR24_TRP_AB(6), &ddrmr->cr[24]);

    writel(DDRMC_CR25_TREF_EN, &ddrmr->cr[25]);
    writel(DDRMC_CR26_TREF(3112) | DDRMC_CR26_TRFC(44), &ddrmr->cr[26]);
    writel(DDRMC_CR28_TREF_INT(5), &ddrmr->cr[28]);
    writel(DDRMC_CR29_TPDEX(3), &ddrmr->cr[29]);

    writel(DDRMC_CR30_TXPDLL(10), &ddrmr->cr[30]);
    writel(DDRMC_CR31_TXSNR(68) | DDRMC_CR31_TXSR(512), &ddrmr->cr[31]);
    writel(DDRMC_CR33_EN_QK_SREF, &ddrmr->cr[33]);
    writel(DDRMC_CR34_CKSRX(5) | DDRMC_CR34_CKSRE(5), &ddrmr->cr[34]);

    writel(DDRMC_CR38_FREQ_CHG_EN, &ddrmr->cr[38]);
    writel(DDRMC_CR39_PHY_INI_COM(1024) | DDRMC_CR39_PHY_INI_STA(16) |
        DDRMC_CR39_FRQ_CH_DLLOFF(2), &ddrmr->cr[39]);

    writel(DDRMC_CR41_PHY_INI_STRT_INI_DIS, &ddrmr->cr[41]);
    writel(DDRMC_CR48_MR1_DA_0(70) | DDRMC_CR48_MR0_DA_0(1056),
        &ddrmr->cr[48]);

    writel(DDRMC_CR66_ZQCL(256) | DDRMC_CR66_ZQINIT(512), &ddrmr->cr[66]);
    writel(DDRMC_CR67_ZQCS(64), &ddrmr->cr[67]);
    writel(DDRMC_CR69_ZQ_ON_SREF_EX(2), &ddrmr->cr[69]);

    writel(DDRMC_CR70_REF_PER_ZQ(64), &ddrmr->cr[70]);
    writel(DDRMC_CR72_ZQCS_ROTATE, &ddrmr->cr[72]);

    writel(DDRMC_CR73_APREBIT(10) | DDRMC_CR73_COL_DIFF(1) |
        DDRMC_CR73_ROW_DIFF(3), &ddrmr->cr[73]);
    writel(DDRMC_CR74_BANKSPLT_EN | DDRMC_CR74_ADDR_CMP_EN |
        DDRMC_CR74_CMD_AGE_CNT(255) | DDRMC_CR74_AGE_CNT(255),
        &ddrmr->cr[74]);
    writel(DDRMC_CR75_RW_PG_EN | DDRMC_CR75_RW_EN | DDRMC_CR75_PRI_EN |
        DDRMC_CR75_PLEN, &ddrmr->cr[75]);
    writel(DDRMC_CR76_NQENT_ACTDIS(3) | DDRMC_CR76_D_RW_G_BKCN(3) |
        DDRMC_CR76_W2R_SPLT_EN | DDRMC_CR76_CS_EN, &ddrmr->cr[76]);
    writel(DDRMC_CR77_CS_MAP | DDRMC_CR77_DI_RD_INTLEAVE |
        DDRMC_CR77_SWAP_EN, &ddrmr->cr[77]);
    writel(DDRMC_CR78_BUR_ON_FLY_BIT(12), &ddrmr->cr[78]);
    writel(DDRMC_CR79_CTLUPD_AREF, &ddrmr->cr[79]);

    writel(DDRMC_CR82_INT_MASK, &ddrmr->cr[82]);

    writel(DDRMC_CR87_ODT_WR_MAPCS0 | DDRMC_CR87_ODT_RD_MAPCS0,
        &ddrmr->cr[87]);
    writel(DDRMC_CR88_TODTL_CMD(4), &ddrmr->cr[88]);
    writel(DDRMC_CR89_AODT_RWSMCS(2), &ddrmr->cr[89]);

    writel(DDRMC_CR91_R2W_SMCSDL(2), &ddrmr->cr[91]);
    writel(DDRMC_CR96_WLMRD(40) | DDRMC_CR96_WLDQSEN(25), &ddrmr->cr[96]);

    writel(DDRMC_CR105_RDLVL_DL_0(32), &ddrmr->cr[105]);
    writel(DDRMC_CR110_RDLVL_DL_1(32), &ddrmr->cr[110]);
    writel(DDRMC_CR114_RDLVL_GTDL_2(8224), &ddrmr->cr[114]);

    writel(DDRMC_CR117_AXI0_W_PRI(1) | DDRMC_CR117_AXI0_R_PRI(1),
        &ddrmr->cr[117]);
    writel(DDRMC_CR118_AXI1_W_PRI(1) | DDRMC_CR118_AXI1_R_PRI(1),
        &ddrmr->cr[118]);

    writel(DDRMC_CR120_AXI0_PRI1_RPRI(2) | DDRMC_CR120_AXI0_PRI0_RPRI(2),
        &ddrmr->cr[120]);
    writel(DDRMC_CR121_AXI0_PRI3_RPRI(2) | DDRMC_CR121_AXI0_PRI2_RPRI(2),
        &ddrmr->cr[121]);
    writel(DDRMC_CR122_AXI1_PRI1_RPRI(1) | DDRMC_CR122_AXI1_PRI0_RPRI(1) |
        DDRMC_CR122_AXI0_PRIRLX(100), &ddrmr->cr[122]);
    writel(DDRMC_CR123_AXI1_PRI3_RPRI(1) | DDRMC_CR123_AXI1_PRI2_RPRI(1),
        &ddrmr->cr[123]);
    writel(DDRMC_CR124_AXI1_PRIRLX(100), &ddrmr->cr[124]);

    writel(DDRMC_CR126_PHY_RDLAT(11), &ddrmr->cr[126]);
    writel(DDRMC_CR132_WRLAT_ADJ(5) | DDRMC_CR132_RDLAT_ADJ(6),
        &ddrmr->cr[132]);
    writel(DDRMC_CR139_PHY_WRLV_RESPLAT(4) | DDRMC_CR139_PHY_WRLV_LOAD(7) |
        DDRMC_CR139_PHY_WRLV_DLL(3) | DDRMC_CR139_PHY_WRLV_EN(3),
        &ddrmr->cr[139]);

    writel(DDRMC_CR154_PAD_ZQ_EARLY_CMP_EN_TIMER(13) |
        DDRMC_CR154_PAD_ZQ_MODE(1), &ddrmr->cr[154]);
    writel(DDRMC_CR155_AXI0_AWCACHE | DDRMC_CR155_PAD_ODT_BYTE1(2),
        &ddrmr->cr[155]);
    writel(DDRMC_CR158_TWR(6), &ddrmr->cr[158]);

    ddr_phy_init();

    writel(DDRMC_CR00_DRAM_CLASS_DDR3 | DDRMC_CR00_START, &ddrmr->cr[0]);

    udelay(200);
#endif
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
