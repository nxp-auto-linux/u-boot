// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017, 2019, 2021-2022 NXP
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <serial.h>
#include <asm/io.h>
#include <linux/compiler.h>

#define LINCR1_INIT			BIT_32(0)
#define LINCR1_MME			BIT(4)
#define LINCR1_RESERVED7	BIT(7)
#define LINSR_LINS_INITMODE		(0x00001000)
#define LINSR_LINS_MASK			(0x0000F000)
#define UARTCR_UART			BIT(0)
#define UARTCR_WL0			BIT(1)
#define UARTCR_PC0			BIT(3)
#define UARTCR_TXEN			BIT(4)
#define UARTCR_RXEN			BIT(5)
#define UARTCR_PC1			BIT(6)
#define UARTCR_TFBM			BIT(8)
#define UARTCR_RFBM			BIT(9)
#define UARTCR_TFC			GENMASK(15, 13)
#define UARTSR_DTF			BIT(1)
#define UARTSR_RFE			BIT(2)
#define UARTSR_RMB			BIT(9)
#define LINFLEXD_UARTCR_OSR_MASK	(0xF << 24)
#define LINFLEXD_UARTCR_OSR(uartcr)	(((uartcr) \
					& LINFLEXD_UARTCR_OSR_MASK) >> 24)

#define LINFLEXD_UARTCR_ROSE		BIT(23)
#define LINFLEX_LDIV_MULTIPLIER		(16)

struct linflex_fsl {
	u32 lincr1;
	u32 linier;
	u32 linsr;
	u32 linesr;
	u32 uartcr;
	u32 uartsr;
	u32 lintcsr;
	u32 linocr;
	u32 lintocr;
	u32 linfbrr;
	u32 linibrr;
	u32 lincfr;
	u32 lincr2;
	u32 bidr;
	u32 bdrl;
	u32 bdrm;
	u32 reserved[3];
	u32 gcr;
	u32 uartpto;
} __packed;

static void _linflex_enter_init(struct linflex_fsl *base)
{
	u32 ctrl = LINCR1_INIT | LINCR1_RESERVED7;

	/* wait the TX fifo to be empty */
	while (__raw_readl(&base->uartcr) & UARTCR_TFC)
		;

	/* set the Linflex in init mode */
	__raw_writel(ctrl, &base->lincr1);

	/* waiting for init mode entry - TODO: add a timeout */
	while ((__raw_readl(&base->linsr) & LINSR_LINS_MASK) !=
	       LINSR_LINS_INITMODE)
		;

	ctrl |= LINCR1_MME;
	__raw_writel(ctrl, &base->lincr1);
}

static void _linflex_exit_init(struct linflex_fsl *base)
{
	u32 ctrl;

	ctrl = __raw_readl(&base->lincr1);
	ctrl &= ~LINCR1_INIT;
	__raw_writel(ctrl, &base->lincr1);
}

static u32 linflex_ldiv_multiplier(struct linflex_fsl *base)
{
	u32 mul = LINFLEX_LDIV_MULTIPLIER;
	u32 cr;

	cr = __raw_readl(&base->uartcr);

	if (cr & LINFLEXD_UARTCR_ROSE)
		mul = LINFLEXD_UARTCR_OSR(cr);

	return mul;
}

static void _linflex_serial_setbrg(struct linflex_fsl *base, ulong rate,
				   int baudrate)
{
	u32 clk = rate;
	u32 ibr, fbr, divisr, dividr;

	if (!baudrate)
		baudrate = CONFIG_BAUDRATE;

	divisr = clk;
	dividr = (u32)(baudrate * linflex_ldiv_multiplier(base));

	ibr = (u32)(divisr / dividr);
	fbr = (u32)((divisr % dividr) * 16 / dividr) & 0xF;

	__raw_writel(ibr, &base->linibrr);
	__raw_writel(fbr, &base->linfbrr);
}

static int _linflex_serial_getc(struct linflex_fsl *base)
{
	while ((__raw_readl(&base->uartsr) & UARTSR_RFE) == UARTSR_RFE)
		;

	return __raw_readb(&base->bdrm);
}

static int _linflex_serial_putc(struct linflex_fsl *base, const char c)
{
	/* wait for Tx FIFO not full */
	while (__raw_readb(&base->uartsr) & UARTSR_DTF)
		;

	__raw_writeb(c, &base->bdrl);

	return 0;
}

/*
 * Initialise the serial port with the given baudrate. The settings
 * are always 8 data bits, no parity, 1 stop bit, no start bits.
 */
static int _linflex_serial_init(struct linflex_fsl *base, ulong rate)
{
	volatile u32 ctrl;

	_linflex_enter_init(base);

	ctrl = __raw_readl(&base->lincr1);
	ctrl |= LINCR1_MME;
	__raw_writel(ctrl, &base->lincr1);

	/* set UART bit to allow writing other bits */
	__raw_writel(UARTCR_UART, &base->uartcr);

	/* 8 bit data, no parity, Tx and Rx enabled, UART mode */
	__raw_writel(UARTCR_PC1 | UARTCR_RXEN | UARTCR_TXEN | UARTCR_PC0
		     | UARTCR_WL0 | UARTCR_UART | UARTCR_RFBM | UARTCR_TFBM,
		     &base->uartcr);

	/* provide data bits, parity, stop bit, etc */
	_linflex_serial_setbrg(base, rate, CONFIG_BAUDRATE);

	_linflex_exit_init(base);

	return 0;
}

struct linflex_serial_priv {
	struct linflex_fsl *lfuart;
	struct clk clk;
	unsigned long rate;
};

int linflex_serial_setbrg(struct udevice *dev, int baudrate)
{
	struct linflex_serial_priv *priv = dev_get_priv(dev);

	_linflex_enter_init(priv->lfuart);
	_linflex_serial_setbrg(priv->lfuart, priv->rate,
			       baudrate);
	_linflex_exit_init(priv->lfuart);

	return 0;
}

static int linflex_serial_getc(struct udevice *dev)
{
	struct linflex_serial_priv *priv = dev_get_priv(dev);

	return _linflex_serial_getc(priv->lfuart);
}

static int linflex_serial_putc(struct udevice *dev, const char ch)
{

	struct linflex_serial_priv *priv = dev_get_priv(dev);

	return _linflex_serial_putc(priv->lfuart, ch);
}

static int linflex_serial_pending(struct udevice *dev, bool input)
{
	struct linflex_serial_priv *priv = dev_get_priv(dev);
	uint32_t uartsr = __raw_readl(&priv->lfuart->uartsr);

	if (input)
		/* RX FIFO not empty. */
		return !(uartsr & UARTSR_RFE);
	else
		/* TX FIFO not full. */
		return !(uartsr & UARTSR_DTF);
}

static int linflex_serial_probe(struct udevice *dev)
{
	struct linflex_serial_priv *priv = dev_get_priv(dev);
	fdt_addr_t base_addr;
	const char *clk_name = "lin";
	int ret;

	base_addr = dev_read_addr(dev);
	if (base_addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	ret = clk_get_by_name(dev, clk_name, &priv->clk);
	if (ret) {
		printf("Failed to get %s clock %d\n", clk_name, ret);
		return ret;
	}

	ret = clk_enable(&priv->clk);
	if (ret) {
		printf("Failed to enable %s clock %d\n", clk_name, ret);
		return ret;
	}

	priv->rate = clk_get_rate(&priv->clk);

	priv->lfuart = (struct linflex_fsl *)base_addr;
	_linflex_serial_init(priv->lfuart, priv->rate);

	return 0;
}

static const struct dm_serial_ops linflex_serial_ops = {
	.putc = linflex_serial_putc,
	.pending = linflex_serial_pending,
	.getc = linflex_serial_getc,
	.setbrg = linflex_serial_setbrg,
};

static const struct udevice_id linflex_serial_ids[] = {
	{ .compatible = "nxp,s32cc-linflexuart", },
	{}
};

U_BOOT_DRIVER(serial_linflex) = {
	.name	= "serial_linflex",
	.id	= UCLASS_SERIAL,
	.of_match = linflex_serial_ids,
	.probe = linflex_serial_probe,
	.ops	= &linflex_serial_ops,
	.flags = DM_FLAG_PRE_RELOC,
	.priv_auto	= sizeof(struct linflex_serial_priv),
};

#ifdef CONFIG_DEBUG_UART_LINFLEXUART

#include <debug_uart.h>


static inline void _debug_uart_init(void)
{
	struct linflex_fsl *base = (struct linflex_fsl *)CONFIG_DEBUG_UART_BASE;

	if (IS_ENABLED(CONFIG_DEBUG_UART_SKIP_INIT))
		return;

	_linflex_serial_init(base, CONFIG_DEBUG_UART_CLOCK);
}

static inline void _debug_uart_putc(int ch)
{
	struct linflex_fsl *base = (struct linflex_fsl *)CONFIG_DEBUG_UART_BASE;

	/* XXX: Is this OK? Should this use the non-DM version? */
	_linflex_serial_putc(base, ch);
}

DEBUG_UART_FUNCS

#endif /* CONFIG_DEBUG_UART_LINFLEXUART */
