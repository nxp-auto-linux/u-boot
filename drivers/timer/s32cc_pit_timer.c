// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
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

struct s32cc_pit_priv {
	void __iomem *base;
};

static u64 s32cc_pit_get_lifetime_counter(struct s32cc_pit_priv *priv)
{
	u64 ltmr64h, ltmr64l;
	u64 cntr;

	ltmr64h = readl(priv->base + PIT_LTMR64H);
	ltmr64l = readl(priv->base + PIT_LTMR64L);

	cntr = (ltmr64h << 32) + ltmr64l;

	return cntr;
}

static u64 s32cc_pit_get_count(struct udevice *dev)
{
	struct s32cc_pit_priv *priv = dev_get_priv(dev);
	u64 cntr;

	if (!priv)
		return -EINVAL;

	cntr = s32cc_pit_get_lifetime_counter(priv);

	return ~0ull - cntr;
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

static int s32cc_pit_probe(struct udevice *dev)
{
	struct s32cc_pit_priv *priv = dev_get_priv(dev);
	u32 tmp;

	if (!priv)
		return -EINVAL;

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

static int s32cc_pit_of_to_plat(struct udevice *dev)
{
	struct s32cc_pit_priv *priv = dev_get_priv(dev);

	if (!priv)
		return -EINVAL;

	priv->base = dev_read_addr_ptr(dev);
	if (!priv->base)
		return -EINVAL;

	return 0;
}

static const struct timer_ops s32cc_pit_ops = {
	.get_count = s32cc_pit_get_count,
};

static const struct udevice_id s32cc_pit_ids[] = {
	{ .compatible = "nxp,s32cc-pit" },
	{}
};

U_BOOT_DRIVER(s32cc_pit_timer) = {
	.name	= "s32cc_pit_timer",
	.id	= UCLASS_TIMER,
	.of_match = s32cc_pit_ids,
	.probe	= s32cc_pit_probe,
	.of_to_plat = s32cc_pit_of_to_plat,
	.priv_auto = sizeof(struct s32cc_pit_priv),
	.ops	= &s32cc_pit_ops,
};
