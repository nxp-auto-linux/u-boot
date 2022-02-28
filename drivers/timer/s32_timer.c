// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <timer.h>
#include <asm/io.h>

#define PIT_LTMR64H	0xE0
#define PIT_LTMR64L	0xE4

#define PIT_LDVAL(x)	(0x100 + (x) * 0x10)
#define PIT_TCTRL(x)	(0x108 + (x) * 0x10)

/* Enable bit for timer */
#define TCTRL_TEN	BIT(0)

/* Chain Mode bit for timer */
#define TCTRL_CHN	BIT(2)

/* Timer start value for count down */
#define TSV	0xFFFFFFFF

struct s32_timer_priv {
	fdt_addr_t base;
};

static u64 s32_time_get_lifetime_counter(struct s32_timer_priv *priv)
{
	u64 ltmr64h, ltmr64l;
	u64 cntr;

	ltmr64h = readl(priv->base + PIT_LTMR64H);
	ltmr64l = readl(priv->base + PIT_LTMR64L);

	cntr = (ltmr64h << 32) + ltmr64l;

	return cntr;
}

static int s32_timer_get_count(struct udevice *dev, u64 *count)
{
	struct s32_timer_priv *priv = dev_get_priv(dev);
	u64 cntr = s32_time_get_lifetime_counter(priv);

	*count = ~0ull - cntr;

	return 0;
}

ulong timer_get_boot_us(void)
{
	u64 ticks = 0, us;
	u32 rate;
	int ret;

	ret = dm_timer_init();

	if (ret)
		return 0;

	/* The timer is available */
	rate = timer_get_rate(gd->timer);
	timer_get_count(gd->timer, &ticks);

	us = (ticks * 1000000) / rate;
	return us;
}

static int s32_timer_probe(struct udevice *dev)
{
	struct s32_timer_priv *priv = dev_get_priv(dev);
	u32 tmp;

	/* Load timer0 and timer1 start value */
	writel(TSV, priv->base + PIT_LDVAL(0));
	writel(TSV, priv->base + PIT_LDVAL(1));

	/* Activate chain mode on timer1 */
	tmp = readl(priv->base + PIT_TCTRL(1)) | TCTRL_CHN;
	writel(tmp, priv->base + PIT_TCTRL(1));

	/* Enable timer0 and timer1 */
	tmp = readl(priv->base + PIT_TCTRL(0)) | TCTRL_TEN;
	writel(tmp, priv->base + PIT_TCTRL(0));

	tmp = readl(priv->base + PIT_TCTRL(1)) | TCTRL_TEN;
	writel(tmp, priv->base + PIT_TCTRL(1));

	return 0;
}

static int s32_timer_ofdata_to_platdata(struct udevice *dev)
{
	struct s32_timer_priv *priv = dev_get_priv(dev);

	priv->base = devfdt_get_addr(dev);
	if (priv->base == (fdt_addr_t)FDT_ADDR_T_NONE)
		return -EINVAL;

	return 0;
}

static const struct timer_ops s32_timer_ops = {
	.get_count = s32_timer_get_count,
};

static const struct udevice_id s32_timer_ids[] = {
	{ .compatible = "fsl,s32gen1-timer" },
	{}
};

U_BOOT_DRIVER(s32_timer) = {
	.name	= "s32_timer",
	.id	= UCLASS_TIMER,
	.of_match = s32_timer_ids,
	.probe	= s32_timer_probe,
	.ofdata_to_platdata = s32_timer_ofdata_to_platdata,
	.priv_auto_alloc_size = sizeof(struct s32_timer_priv),
	.ops	= &s32_timer_ops,
};
