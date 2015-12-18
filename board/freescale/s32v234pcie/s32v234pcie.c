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
#if defined(CONFIG_S32V234_LPDDR2)
#include <asm/arch/lpddr2.h>
#elif defined(CONFIG_S32V234_DDR3)
#include <asm/arch/ddr3.h>
#else
#error "Please define the DDR type for S32V234 board!"
#endif
#include <asm/arch/clock.h>
#include <asm/arch/xrdc.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

void setup_iomux_ddr(void)
{
	ddr_config_iomux(DDR0);
	ddr_config_iomux(DDR1);
}

void ddr_phy_init(void)
{
	/* TODO: Add initialisation code for ddr phy. */
}

void ddr_ctrl_init(void)
{
	config_mmdc(0);
	config_mmdc(1);
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
	/* Muxing for linflex1 */

	/* set TXD - MSCR[14] PA14 */
	writel(SIUL2_UART_TXD, SIUL2_MSCRn(SIUL2_UART1_TXD_PAD));

	/* set RXD - MSCR[13] - PA13*/
	writel(SIUL2_UART_MSCR_RXD, SIUL2_MSCRn(SIUL2_UART1_MSCR_RXD_PAD));

	/* set RXD - IMCR[202] - 202 */
	writel(SIUL2_UART_IMCR_RXD, SIUL2_IMCRn(SIUL2_UART1_IMCR_RXD_PAD));
}
static void setup_iomux_enet(void)
{
#ifndef CONFIG_PHY_RGMII_DIRECT_CONNECTED
	writel(0x0020c701, SIUL2_MSCRn(45));	//MDC   //PC13
	writel(0x0028c701, SIUL2_MSCRn(46));	//MDIO  //PC14
	writel(       0x2, SIUL2_MSCRn(981));
#endif


	writel(0x0020c701, SIUL2_MSCRn(47));	//TX_CLK //PC15
	writel(       0x2, SIUL2_MSCRn(978));

	writel(0x0008c700, SIUL2_MSCRn(48));	//RX_CLK //PD0
	writel(       0x2, SIUL2_MSCRn(979));

	writel(0x0008c700, SIUL2_MSCRn(49));	//RX_D0  //PD1
	writel(       0x2, SIUL2_MSCRn(974));

	writel(0x0008c700, SIUL2_MSCRn(50));	//RX_D1  //PD2
	writel(       0x2, SIUL2_MSCRn(975));


	writel(0x0008c700, SIUL2_MSCRn(51));	//RX_D2  //PD3
	writel(       0x2, SIUL2_MSCRn(976));

	writel(0x0008c700, SIUL2_MSCRn(52));	//RX_D3  //PD4
	writel(       0x2, SIUL2_MSCRn(977));

	writel(0x0008c700, SIUL2_MSCRn(53));	//RX_DV  //PD5
	writel(       0x2, SIUL2_MSCRn(973));

	writel(0x0020c701, SIUL2_MSCRn(55));	//TX_D0  //PD7
	writel(0x0020c701, SIUL2_MSCRn(56));	//TX_D1  //PD8
	writel(0x0020c701, SIUL2_MSCRn(57));	//TX_D2  //PD9
	writel(0x0020c701, SIUL2_MSCRn(58));	//TX_D3  //PD10
	writel(0x0020c701, SIUL2_MSCRn(59));	//TX_EN  //PD11
}

static void setup_iomux_i2c(void)
{
	/* I2C0 - Serial Data Input */
	writel(SIUL2_PAD_CTRL_I2C0_MSCR_SDA, SIUL2_MSCRn(99));
	writel(SIUL2_PAD_CTRL_I2C0_IMCR_SDA, SIUL2_IMCRn(269));

	/* I2C0 - Serial Clock Input */
	writel(SIUL2_PAD_CTRL_I2C0_MSCR_SCLK, SIUL2_MSCRn(100));
	writel(SIUL2_PAD_CTRL_I2C0_IMCR_SCLK, SIUL2_IMCRn(268));

	/* I2C1 - Serial Data Input */
	writel(SIUL2_PAD_CTRL_I2C1_MSCR_SDA, SIUL2_MSCRn(101));
	writel(SIUL2_PAD_CTRL_I2C1_IMCR_SDA, SIUL2_IMCRn(271));

	/* I2C1 - Serial Clock Input */
	writel(SIUL2_PAD_CTRL_I2C1_MSCR_SCLK, SIUL2_MSCRn(102));
	writel(SIUL2_PAD_CTRL_I2C1_IMCR_SCLK, SIUL2_IMCRn(270));

	/* I2C2 - Serial Data Input */
	writel(SIUL2_PAD_CTRL_I2C2_MSCR_SDA, SIUL2_MSCRn(19));
	writel(SIUL2_PAD_CTRL_I2C2_IMCR_SDA, SIUL2_IMCRn(273));

	/* I2C2 - Serial Clock Input */
	writel(SIUL2_PAD_CTRL_I2C2_MSCR_SCLK, SIUL2_MSCRn(20));
	writel(SIUL2_PAD_CTRL_I2C2_IMCR_SCLK, SIUL2_IMCRn(272));
}

#ifdef CONFIG_SYS_USE_NAND
void setup_iomux_nfc(void)
{
	/*TODO: Implement nfc iomux when it is activated.*/
}
#endif

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[1] = {
	{USDHC_BASE_ADDR},
};

int board_mmc_getcd(struct mmc *mmc)
{
	/* eSDHC1 is always present */
	return 1;
}

int board_mmc_init(bd_t *bis)
{
	esdhc_cfg[0].sdhc_clk = mxc_get_clock(MXC_USDHC_CLK);

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

	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}
#endif

static void mscm_init(void)
{
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_BASE_ADDR;
	int i;

	for (i = 0; i < MSCM_IRSPRC_NUM; i++)
		writew(MSCM_IRSPRC_CPn_EN, &mscmir->irsprc[i]);
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

void setup_xrdc(void)
{
	writel(XRDC_ADDR_MIN, XRDC_MRGD_W0_16);
	writel(XRDC_ADDR_MAX, XRDC_MRGD_W1_16);
	writel(XRDC_VALID, XRDC_MRGD_W3_16);

	writel(XRDC_ADDR_MIN, XRDC_MRGD_W0_17);
	writel(XRDC_ADDR_MAX, XRDC_MRGD_W1_17);
	writel(XRDC_VALID, XRDC_MRGD_W3_17);

	writel(XRDC_ADDR_MIN, XRDC_MRGD_W0_18);
	writel(XRDC_ADDR_MAX, XRDC_MRGD_W1_18);
	writel(XRDC_VALID, XRDC_MRGD_W3_18);

	writel(XRDC_ADDR_MIN, XRDC_MRGD_W0_19);
	writel(XRDC_ADDR_MAX, XRDC_MRGD_W1_19);
	writel(XRDC_VALID, XRDC_MRGD_W3_19);
}

#ifdef CONFIG_DCU_QOS_FIX
int board_dcu_qos()
{
	writel(0x0, 0x40012380);
	writel(0x0, 0x40012384);
	writel(0x0, 0x40012480);
	writel(0x0, 0x40012484);
	writel(0x0, 0x40012580);
	writel(0x0, 0x40012584);
	writel(0x0, 0x40012680);
	writel(0x0, 0x40012684);
	writel(0x0, 0x40012780);
	writel(0x0, 0x40012784);
	writel(0x0, 0x40012880);
	writel(0x0, 0x40012884);
	writel(0x0, 0x40012980);
	writel(0x0, 0x40012984);
	writel(0x0, 0x40012A80);
	writel(0x0, 0x40012A84);
	writel(0x0, 0x40012B80);
	writel(0x0, 0x40012B84);
	writel(0x0, 0x40012C80);
	writel(0x0, 0x40012C84);
	writel(0x0, 0x40012D80);
	writel(0x0, 0x40012D84);
	writel(0x0, 0x40012E80);
	writel(0x0, 0x40012E84);
	writel(0x0, 0x40012F80);
	writel(0x0, 0x40012F84);
	
	return 0;
}
#endif

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

#ifdef CONFIG_DCU_QOS_FIX
	board_dcu_qos();
#endif

	setup_xrdc();
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
	puts("Board: s32v234pcie\n");

	return 0;
}
