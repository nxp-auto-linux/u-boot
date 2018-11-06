/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017-2018 NXP
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
	MXC_USDHC_CLK,
	MXC_FEC_CLK,
	MXC_I2C_CLK,
	MXC_SYS6_CLK,
	MXC_QSPI_CLK,
	MXC_DCU_PIX_CLK,
	MXC_DSPI_CLK,
	MXC_XBAR_CLK,
	MXC_DDR_CLK,
};

#if defined(CONFIG_S32V234)
enum pll_type {
	ARM_PLL = 0,
	PERIPH_PLL,
	ENET_PLL,
	DDR_PLL,
	VIDEO_PLL,
};
#elif defined(CONFIG_S32_GEN1)
enum pll_type {
	ARM_PLL = 0,
	PERIPH_PLL,
	ACCEL_PLL,
	DDR_PLL,
};
#endif

unsigned int mxc_get_clock(enum mxc_clock clk);
void clock_init(void);
void entry_to_target_mode( u32 mode );

#define imx_get_fecclk() mxc_get_clock(MXC_FEC_CLK)

#endif /* __ASM_ARCH_CLOCK_H */
