/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017-2018, 2020-2021 NXP
 *
 */

#ifndef __ASM_ARCH_CLOCK_H
#define __ASM_ARCH_CLOCK_H

#ifdef __KERNEL__
#include <common.h>
#endif

enum mxc_clock {
	MXC_UART_CLK,
	MXC_ESDHC_CLK,
	MXC_I2C_CLK,
	MXC_DSPI_CLK,
};

#ifdef __KERNEL__

unsigned int mxc_get_clock(enum mxc_clock clk);
void entry_to_target_mode( u32 mode );

#define imx_get_fecclk() mxc_get_clock(MXC_FEC_CLK)
#endif /* __KERNEL__ */

#endif /* __ASM_ARCH_CLOCK_H */
