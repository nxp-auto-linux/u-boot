/*
 * (C) Copyright 2016 NXP Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/siul.h>
#include <asm/arch/xrdc.h>
#include <i2c.h>


#ifdef CONFIG_FSL_LINFLEXUART
void setup_iomux_uart(void)
{
	/* Muxing for linflex0 and linflex1 */

	/* set PA12 - MSCR[12] - for UART0 TXD */
	writel(SIUL2_MSCR_PORT_CTRL_UART_TXD, SIUL2_MSCRn(SIUL2_MSCR_PA12));

	/* set PA11 - MSCR[11] - for UART0 RXD */
	writel(SIUL2_MSCR_PORT_CTRL_UART_RXD, SIUL2_MSCRn(SIUL2_MSCR_PA11));
	/* set UART0 RXD - IMCR[200] - to link to PA11 */
	writel(SIUL2_IMCR_UART_RXD_to_pad, SIUL2_IMCRn(SIUL2_IMCR_UART0_RXD));

	/* set PA14 - MSCR[14] - for UART1 TXD*/
	writel(SIUL2_MSCR_PORT_CTRL_UART_TXD, SIUL2_MSCRn(SIUL2_MSCR_PA14));

	/* set PA13 - MSCR[13] - for UART1 RXD */
	writel(SIUL2_MSCR_PORT_CTRL_UART_RXD, SIUL2_MSCRn(SIUL2_MSCR_PA13));
	/* set UART1 RXD - IMCR[202] - to link to PA13 */
	writel(SIUL2_IMCR_UART_RXD_to_pad, SIUL2_IMCRn(SIUL2_IMCR_UART1_RXD));
}
#endif

#ifdef CONFIG_NET
void setup_iomux_enet(void)
{
#ifndef CONFIG_PHY_RGMII_DIRECT_CONNECTED
	writel(0x0020c701, SIUL2_MSCRn(45));	//MDC   //PC13
	writel(0x0028c701, SIUL2_MSCRn(46));	//MDIO  //PC14
	writel(       0x2, SIUL2_MSCRn(981));
#endif

#ifdef CONFIG_PHY_RGMII_DIRECT_CONNECTED
	writel(0x0020c701, SIUL2_MSCRn(47));	//TX_CLK //PC15
#else
	writel(0x00203701, SIUL2_MSCRn(47));	//TX_CLK //PC15
#endif
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

#if 0
	/* reset the Ethernet controller */
	writel(0x1, 0x40032024);
	while (readl(0x40032024) & 0x1);
#endif
}
#endif /* CONFIG_NET */


#if defined(CONFIG_SYS_I2C) || defined(CONFIG_I2C_MULTI_BUS)
void setup_iomux_i2c(void)
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
#endif /* defined(CONFIG_SYS_I2C) || defined(CONFIG_I2C_MULTI_BUS)  */

#ifdef CONFIG_SYS_USE_NAND
void setup_iomux_nfc(void)
{
	/*TODO: Implement nfc iomux when it is activated.*/
}
#endif

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



