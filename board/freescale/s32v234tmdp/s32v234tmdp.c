/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <fdt_support.h>
#include <libfdt.h>
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
	setup_iomux_dcu();
	board_dcu_qos();
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
	puts("Board: s32v234tmdp\n");

	return 0;
}

#if defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	return 0;
}
#endif /* defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP) */
