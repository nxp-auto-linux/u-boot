/*
 * (C) Copyright 2013-2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_CLOCK_H
#define __ASM_ARCH_CLOCK_H

#include <common.h>

enum mxc_clock {
	MXC_ARM_CLK = 0,
	MXC_BUS_CLK,
	MXC_PERIPHERALS_CLK,
	MXC_UART_CLK,
	MXC_FEC_CLK,
	MXC_I2C_CLK,
};

/* PLL0 => PLL_SYS; PLL_LINFLEX; PLL_DDR; PLL_ENET */
/* PLL1 => NOT USED YET */
/* PLL2 => NOT USED YET */
/* PLL3 => NOT USED YET */

enum clocks {
	FIRC = 0,
	FXOSC,
	RESERVED1,
	RESERVED2,
	PLL0,
	PLL1,
	PLL2,
	PLL3,
};

unsigned int mxc_get_clock(enum mxc_clock clk);

#define imx_get_fecclk() mxc_get_clock(MXC_FEC_CLK)

#endif /* __ASM_ARCH_CLOCK_H */
