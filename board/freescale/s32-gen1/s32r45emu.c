// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2021 NXP
 */

#include "asm/arch-s32/siul-s32r45.h"
#include "board_common.h"
#include <asm/arch/soc.h>
#include <common.h>

void setup_iomux_uart(void)
{
#if (CONFIG_FSL_LINFLEX_MODULE == 1)
	/* Set PA_11 - MSCR[11] - for LIN1_RX */
	writel(SIUL2_MSCR_S32R45_PORT_CTRL_UART_RXD,
	       SIUL2_0_MSCRn(SIUL2_PA11_MSCR_S32R45_UART1));

	/* Set PA_11 - IMCR[674] - for LIN1_RX */
	writel(SIUL2_IMCR_S32R45_UART_RXD_to_pad,
	       SIUL2_1_IMCRn(SIUL2_PA11_IMCR_S32R45_UART1));

	/* Set PA_12 - MSCR[12] - for LIN1_TX */
	writel(SIUL2_MSCR_S32R45_PORT_CTRL_UART1_TXD,
	       SIUL2_0_MSCRn(SIUL2_PA12_MSCR_S32R45_UART1));
#else
#error "Unsupported UART pinmuxing configuration"
#endif
}
