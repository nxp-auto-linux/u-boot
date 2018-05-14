/*
 * Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/soc.h>

void setup_iomux_uart(void)
{
	/* Muxing for linflex0 */

	/* set PK15 - MSCR[175] - for UART0 TXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART_TXD,
	       SIUL2_MSCRn(SIUL2_PK15_MSCR_S32_G1_UART0));

	/* set PL00 - MSCR[176] - for UART0 RXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART_RXD,
	       SIUL2_MSCRn(SIUL2_PL0_MSCR_S32_G1_UART0));
	/* set UART0 RXD - IMCR[512] - to link to PL0 */
	writel(SIUL2_IMCR_S32G_G1_UART_RXD_to_pad,
	       SIUL2_IMCRn(SIUL2_PL0_ISCR_S32_G1_UART0));
}
