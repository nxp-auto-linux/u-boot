/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/siul.h>
#include <asm/arch/clock.h>
#include <asm/arch/xrdc.h>
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

static void setup_iomux_enet(void)
{
	writel(0x0020c701, SIUL2_MSCRn(45));	//MDC   //PC13
	writel(0x0028c701, SIUL2_MSCRn(46));	//MDIO  //PC14
	writel(       0x2, SIUL2_MSCRn(981));

	writel(0x0008c700, SIUL2_MSCRn(47));	//RMII_CLK_REF_IP //PC15
	writel(       0x2, SIUL2_MSCRn(978));

	writel(0x0008c700, SIUL2_MSCRn(49));	//RX_D0  //PD1
	writel(       0x2, SIUL2_MSCRn(974));
	writel(0x0008c700, SIUL2_MSCRn(50));	//RX_D1  //PD2
	writel(       0x2, SIUL2_MSCRn(975));
	writel(0x0008c700, SIUL2_MSCRn(53));	//RX_DV  //PD5
	writel(       0x2, SIUL2_MSCRn(973));
	writel(0x0008c700, SIUL2_MSCRn(54));	//RX_ER  //PD6
	writel(       0x2, SIUL2_MSCRn(970));

	writel(0x0020c701, SIUL2_MSCRn(55));	//TX_D0  //PD7
	writel(0x0020c701, SIUL2_MSCRn(56));	//TX_D1  //PD8
	writel(0x0020c701, SIUL2_MSCRn(59));	//TX_EN  //PD11

#if 0
	/* reset the Ethernet controller */
	writel(0x1, 0x40032024);
	while (readl(0x40032024) & 0x1);
#endif
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
