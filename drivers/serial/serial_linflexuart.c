/*
 * Copyright 2013-2015 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include <serial.h>
#include <linux/compiler.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>

#define US1_TDRE        (1 << 7)
#define US1_RDRF        (1 << 5)
#define UC2_TE          (1 << 3)
#define LINCR1_INIT     (1 << 0)
#define DTF		(1<<1)
#define DRF		(1<<2)
#define RMB		(1<<9)

DECLARE_GLOBAL_DATA_PTR;

struct linflex_fsl *base = (struct linflex_fsl *)LINFLEXUART_BASE;

static void linflex_serial_setbrg(void)
{
#if 1 /* b00450 */
	u32 clk = mxc_get_clock(MXC_UART_CLK);
	u32 sbr;	
	
	if (!gd->baudrate)
		gd->baudrate = CONFIG_BAUDRATE;

	sbr = (u32)(clk / (16 * gd->baudrate));
	//frac= modf(sbr, &mant);
	/* place adjustment later - n/16 linfbrr */

	__raw_writel(sbr, &base->linibrr);
	__raw_writel(0, &base->linfbrr);
#endif /* b00450 */
}

static int linflex_serial_getc(void)
{
	char c;
	while((__raw_readb(&base->uartsr) & DRF)!=DRF){} /* waiting for data reception complete - TODO: add a timeout */
	while((__raw_readw(&base->uartsr) & RMB)!=RMB){} /* waiting for data buffer to be ready - TODO: add a timeout */
	c = __raw_readl(&base->bdrm);
	__raw_writeb((__raw_readb(&base->uartsr)|(DRF|RMB)), &base->uartsr);
	return c;
}

static void linflex_serial_putc(const char c)
{
	if (c == '\n')
		serial_putc('\r');

	__raw_writeb(c, &base->bdrl);
	while((__raw_readb(&base->uartsr) & DTF)!=DTF){} /* waiting for data transmission completed - TODO: add a timeout */
	__raw_writeb((__raw_readb(&base->uartsr)|DTF), &base->uartsr);
}

/*
 * Test whether a character is in the RX buffer
 */
static int linflex_serial_tstc(void)
{
#if 0 /* b00450 */
	if (__raw_readb(&base->urcfifo) == 0)
		return 0;

	return 1;
#endif /* b00450 */
	return 0; /* b00450 */
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
static int linflex_serial_init(void)
{
#if 1 /* b00450 */
	volatile u32 ctrl;

	ctrl = 0x90;
	__raw_writel(ctrl, &base->lincr1); /* init mode */
	ctrl |= LINCR1_INIT;
	__raw_writel(ctrl, &base->lincr1); /* init mode */

	while((__raw_readl(&base->linsr) & 0xF000)!=0x1000){} /* waiting for init mode entry - TODO: add a timeout */

	__raw_writel(1, &base->uartcr); /* set UART bit to allow writing other bits */
	
	/* provide data bits, parity, stop bit, etc */

	serial_setbrg();

	__raw_writel(0x7B, &base->uartcr); /* 8 bit data, no parity, Tx and Rx enabled, UART mode */
	
	ctrl = __raw_readl(&base->lincr1);
	ctrl &= ~LINCR1_INIT;
	__raw_writel(ctrl, &base->lincr1); /* end init mode */
#endif /* b00450 */
	return 0;
}

static struct serial_device linflex_serial_drv = {
	.name = "linflex_serial",
	.start = linflex_serial_init,
	.stop = NULL,
	.setbrg = linflex_serial_setbrg,
	.putc = linflex_serial_putc,
	.puts = default_serial_puts,
	.getc = linflex_serial_getc,
	.tstc = linflex_serial_tstc,
};

void linflex_serial_initialize(void)
{
	serial_register(&linflex_serial_drv);
}

__weak struct serial_device *default_serial_console(void)
{
	return &linflex_serial_drv;
}
