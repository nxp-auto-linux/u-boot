/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <fdt_support.h>
#include <libfdt.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c.h>

DECLARE_GLOBAL_DATA_PTR;

static void setup_iomux_uart(void)
{
	/* Muxing for linflex0 */

	/* set PA12 - MSCR[12] - for UART0 TXD */
	writel(SIUL2_MSCR_PORT_CTRL_UART_TXD, SIUL2_MSCRn(SIUL2_MSCR_PA12));

	/* set PA11 - MSCR[11] - for UART0 RXD */
	writel(SIUL2_MSCR_PORT_CTRL_UART_RXD, SIUL2_MSCRn(SIUL2_MSCR_PA11));
	/* set UART0 RXD - IMCR[200] - to link to PA11 */
	writel(SIUL2_IMCR_UART_RXD_to_pad, SIUL2_IMCRn(SIUL2_IMCR_UART0_RXD));
}

void setup_iomux_enet(void)
{
	/* set PC13 - MSCR[45] - for MDC */
	writel(SIUL2_MSCR_ENET_MDC, SIUL2_MSCRn(SIUL2_MSCR_PC13));

	/* set PC14 - MSCR[46] - for MDIO */
	writel(SIUL2_MSCR_ENET_MDIO, SIUL2_MSCRn(SIUL2_MSCR_PC14));
	writel(SIUL2_MSCR_ENET_MDIO_IN, SIUL2_MSCRn(SIUL2_MSCR_PC14_IN));

	/* set PC15 - MSCR[47] - for RMII CLK REF IP */
	writel(SIUL2_MSCR_ENET_RMII_CLK_REF_IP, SIUL2_MSCRn(SIUL2_MSCR_PC15));
	writel(SIUL2_MSCR_ENET_TX_CLK_IN, SIUL2_MSCRn(SIUL2_MSCR_PC15_IN));

	/* set PD1 - MSCR[49] - for RX_D0 */
	writel(SIUL2_MSCR_ENET_RX_D0, SIUL2_MSCRn(SIUL2_MSCR_PD1));
	writel(SIUL2_MSCR_ENET_RX_D0_IN, SIUL2_MSCRn(SIUL2_MSCR_PD1_IN));

	/* set PD2 - MSCR[50] - for RX_D1 */
	writel(SIUL2_MSCR_ENET_RX_D1, SIUL2_MSCRn(SIUL2_MSCR_PD2));
	writel(SIUL2_MSCR_ENET_RX_D1_IN, SIUL2_MSCRn(SIUL2_MSCR_PD2_IN));

	/* set PD5 - MSCR[53] - for RX_DV */
	writel(SIUL2_MSCR_ENET_RX_DV, SIUL2_MSCRn(SIUL2_MSCR_PD5));
	writel(SIUL2_MSCR_ENET_RX_DV_IN, SIUL2_MSCRn(SIUL2_MSCR_PD5_IN));

	/* set PD6 - MSCR[54] - for RX_ER */
	writel(SIUL2_MSCR_ENET_RX_DV, SIUL2_MSCRn(SIUL2_MSCR_PD6));
	writel(SIUL2_MSCR_ENET_RX_ER_IN, SIUL2_MSCRn(SIUL2_MSCR_PD6_IN));

	/* set PD7 - MSCR[55] - for TX_D0 */
	writel(SIUL2_MSCR_ENET_TX_D0, SIUL2_MSCRn(SIUL2_MSCR_PD7));
	/* set PD8 - MSCR[56] - for TX_D1 */
	writel(SIUL2_MSCR_ENET_TX_D1, SIUL2_MSCRn(SIUL2_MSCR_PD8));
	/* set PD11 - MSCR[59] - for TX_EN */
	writel(SIUL2_MSCR_ENET_TX_EN, SIUL2_MSCRn(SIUL2_MSCR_PD11));
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

#if 0 /* Temporary disable until the clock are implemented */
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
	return fsl_esdhc_initialize(bis, &esdhc_cfg[0]);
}
#endif
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
	puts("Board: s32v234fvb\n");

	return 0;
}

#if defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	return 0;
}
#endif /* defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP) */
