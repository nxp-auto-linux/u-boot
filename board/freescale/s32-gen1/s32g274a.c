// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 NXP
 */

#include <config.h>
#include <asm/io.h>
#include <asm/arch/soc.h>

#if defined(CONFIG_TARGET_S32G274ASIM) || defined(CONFIG_TARGET_S32G274AEMU)
void setup_iomux_uart1_pb09_pb10(void)
{
	/* Muxing for linflex1 */

	/* set PB09 - MSCR[25] - for UART1 TXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART1_TXD,
	       SIUL2_0_MSCRn(SIUL2_PB09_MSCR_S32_G1_UART1));

	/* set PB10 - MSCR[26] - for UART1 RXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART_RXD,
	       SIUL2_0_MSCRn(SIUL2_PB10_MSCR_S32_G1_UART1));

	/* set PB10 - MSCR[736]/IMCR[224] - for UART1 RXD */
	writel(SIUL2_IMCR_S32G_G1_UART1_RXD_to_pad,
	       SIUL2_1_IMCRn(SIUL2_PB10_IMCR_S32_G1_UART1));
}
#endif
