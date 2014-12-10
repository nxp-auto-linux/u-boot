/*
 * Copyright 2013 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __ASM_ARCH_CLOCK_H
#define __ASM_ARCH_CLOCK_H

#include <common.h>

enum mxc_clock {
	MXC_ARM_CLK = 0,
	MXC_BUS_CLK,
	MXC_DDR_CLK,
	MXC_IPG_CLK,
	MXC_UART_CLK,
	MXC_USDHC0_CLK,
	MXC_USDHC1_CLK,
	MXC_USDHC2_CLK,
	MXC_FEC_CLK,
	MXC_I2C_CLK,
	MXC_NFC_CLK,
};

enum pll_clocks {
	PLL_ARM = 0,	/* ARM PLL         => PLL1 */
	PLL_SYS,		/* System Bus PLL  => PLL2 */
	PLL_USBOTG0,	/* OTG USB0 PLL    => PLL3 */
	PLL_USBOTG1,	/* OTG USB1 PLL    => PLL4 */
	PLL_AUDIO0,		/* AUDIO0 PLL      => PLL5 */
	PLL_AUDIO1,     /* AUDIO1 PLL      => PLL6 */
	PLL_VIDEO,		/* VIDEO PLL       => PLL7 */
	PLL_ENET,		/* ENET PLL        => PLL8 */
};

int enable_i2c_clk(unsigned char enable, unsigned i2c_num);
void enable_periph_clk(u32 aips_num, u32 periph_number);
void disable_periph_clk(u32 aips_num, u32 periph_number);
void enable_ocotp_clk(unsigned char enable);
int enable_fec_clock(void);
int enable_pll(enum pll_clocks pll);
int config_pll(enum pll_clocks pll, int mult, int mfn, int mfd);
int is_pll_locked(enum pll_clocks pll);


unsigned int mxc_get_clock(enum mxc_clock clk);

#define imx_get_fecclk() mxc_get_clock(MXC_FEC_CLK)

#endif /* __ASM_ARCH_CLOCK_H */

