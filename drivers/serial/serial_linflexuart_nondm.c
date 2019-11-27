// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017, 2019 NXP
 */

#include <common.h>
#include <watchdog.h>
#include <asm/io.h>
#include <serial.h>
#include <linux/compiler.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>

#define US1_TDRE			BIT(7)
#define US1_RDRF			BIT(5)
#define UC2_TE				BIT(3)
#define LINCR1_INIT			BIT(0)
#define LINCR1_MME			BIT(4)
#define LINCR1_BF			BIT(7)
#define LINSR_LINS_INITMODE		(0x00001000)
#define LINSR_LINS_MASK			(0x0000F000)
#define UARTCR_UART			BIT(0)
#define UARTCR_WL0			BIT(1)
#define UARTCR_PCE			BIT(2)
#define UARTCR_PC0			BIT(3)
#define UARTCR_TXEN			BIT(4)
#define UARTCR_RXEN			BIT(5)
#define UARTCR_PC1			BIT(6)
#define UARTCR_TFBM			BIT(8)
#define UARTCR_RFBM			BIT(9)
#define UARTSR_DTF			BIT(1)
#define UARTSR_DRF			BIT(2)
#define UARTSR_RFE			BIT(2)
#define UARTSR_RFNE			BIT(4)
#define UARTSR_RMB			BIT(9)

#define UARTCR_OSR_MASK		(0xF << 24)
#define UARTCR_OSR_GET(uartcr)	(((uartcr) \
					 & UARTCR_OSR_MASK) >> 24)
#define UARTCR_OSR(val)	((val << 24) & UARTCR_OSR_MASK)

#define UARTCR_CSP_MASK		(0x7 << 28)
#define UARTCR_CSP_GET(uartcr)	(((uartcr) \
					 & UARTCR_CSP_MASK) >> 28)
#define UARTCR_CSP(val)		((val << 28) & UARTCR_CSP_MASK)

#define UARTCR_ROSE			BIT(23)
#define LINFLEX_LDIV_MULTIPLIER		(16)

DECLARE_GLOBAL_DATA_PTR;

struct linflex_fsl *base = (struct linflex_fsl *)LINFLEXUART_BASE;

#ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
#define LIN_CLK			133000	// LIN_CLK in Hz
#endif

static u32 linflex_ldiv_multiplier(void)
{
#ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
	return 1;
#else
	u32 mul = LINFLEX_LDIV_MULTIPLIER;
	u32 cr;

	cr = __raw_readl(&base->uartcr);

	if (cr & UARTCR_ROSE)
		mul = UARTCR_OSR_GET(cr);

	return mul;
#endif
}

static void linflex_serial_setbrg(void)
{
	/* using crt code/settings, we get ibr = 26 and fbr = 0 */
	u32 clk;
	u32 ibr, fbr, divisr, dividr;

#ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
	clk = LIN_CLK;
#else
	clk = mxc_get_clock(MXC_UART_CLK);
#endif

	if (!gd->baudrate)
		gd->baudrate = CONFIG_BAUDRATE;

	divisr = clk;
	dividr = (u32)(gd->baudrate * linflex_ldiv_multiplier());

	ibr = (u32)(divisr / dividr);
	fbr = (u32)((divisr % dividr) * 16 / dividr) & 0xF;

	__raw_writel(ibr, &base->linibrr);
	__raw_writel(fbr, &base->linfbrr);
}

static int linflex_serial_getc(void)
{
	u32 fifo_mode = __raw_readl(&base->uartcr) & UARTCR_RFBM;
	u32 status;

	/* Protect ourselves from configuration errors and
	 * handle both Buffer and Queue modes
	 */
	if (fifo_mode) {
		/* FIFO mode: Busy waiting while FIFO is empty */
		while ((__raw_readl(&base->uartsr) & UARTSR_RFE) == UARTSR_RFE)
			;
	} else {
		/* Buffer mode: wait until the bytes programmed
		 * in RDFL are received */
		while ((__raw_readl(&base->uartsr) & UARTSR_RFE) != UARTSR_RFE)
			;
	}

	status = __raw_readl(&base->uartsr) | UARTSR_RFE;
	__raw_writel(status, &base->uartsr);
	return __raw_readb(&base->bdrm);
}

static void linflex_serial_putc(const char c)
{
	volatile u32 status;
	u32 fifo_mode = 0;

	status = __raw_readl(&base->uartcr);
	fifo_mode = status & UARTCR_TFBM;

	if (c == '\n')
		serial_putc('\r');

	/* Protect ourselves from configuration errors and
	 * handle both Buffer and Queue modes
	 */
	if (fifo_mode) {
		/* wait for Tx FIFO not full */
		while ((__raw_readl(&base->uartsr) & UARTSR_DTF))
			;
	}

	__raw_writeb(c, &base->bdrl);

	if (!fifo_mode) {
		/* wait for Tx done */
		while ((__raw_readl(&base->uartsr) & UARTSR_DTF) == 0)
			;
		status = __raw_readl(&base->uartsr) | UARTSR_DTF;
		__raw_writel(status, &base->uartsr);  // clear the DTF bit
	}
}

/*
 * Test whether a character is in the RX buffer
 */
static int linflex_serial_tstc(void)
{
	u32 fifo_mode = __raw_readl(&base->uartcr) & UARTCR_RFBM;

	if (fifo_mode) {
		if ((__raw_readb(&base->uartsr) & UARTSR_RFE) == UARTSR_RFE)
			return 0;
	} else {
		if ((__raw_readl(&base->uartsr) & UARTSR_RFE) != UARTSR_RFE)
			return 0;
	}

	return 1;
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
static int linflex_serial_init(void)
{
	volatile u32 ctrl;

	/* set the Linflex in master mode */
	ctrl = LINCR1_MME;
	__raw_writel(ctrl, &base->lincr1);

	/* init mode */
	ctrl |= LINCR1_INIT;
	__raw_writel(ctrl, &base->lincr1);

	/* waiting for init mode entry - TODO: add a timeout */
	while ((__raw_readl(&base->linsr) & LINSR_LINS_MASK) !=
	       LINSR_LINS_INITMODE)
		;

	/* set UART bit to allow writing other bits */
	__raw_writel(UARTCR_UART, &base->uartcr);

	/* provide data bits, parity, stop bit, etc */
	serial_setbrg();

	/* Set preset timeout register value. */
	__raw_writel(0xf, &base->uartpto);

	/* 8 bit data, no parity, Tx and Rx enabled, UART mode */
	__raw_writel(UARTCR_PC1 | UARTCR_RXEN | UARTCR_TXEN | UARTCR_PC0
		     | UARTCR_WL0 | UARTCR_UART | UARTCR_RFBM | UARTCR_TFBM,
		     &base->uartcr);

	ctrl = __raw_readl(&base->lincr1);
	ctrl &= ~LINCR1_INIT;
	__raw_writel(ctrl, &base->lincr1);	/* end init mode */

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
