// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2018-2022 NXP
 */

#include <asm/arch/clock.h>
#include <dm/uclass.h>
#include "s32gen1_clk_utils.h"

static struct udevice *get_clk_device(void)
{
	static struct udevice *dev;
	int ret;

	if (dev)
		return dev;

	ret = uclass_get_device_by_name(UCLASS_CLK, "clks", &dev);
	if (ret)
		return NULL;

	return dev;
}

ulong s32gen1_set_dev_clk_rate(const char *name, struct udevice *dev,
			       ulong rate)
{
	struct clk clk;
	int ret = 0;

	ret = clk_get_by_name(dev, name, &clk);
	if (ret)
		return 0;

	rate = clk_set_rate(&clk, rate);
	(void)clk_free(&clk);

	return rate;
}

ulong s32gen1_get_dev_clk_rate(const char *name, struct udevice *dev)
{
	struct clk clk;
	int ret = 0;
	ulong rate;

	ret = clk_get_by_name(dev, name, &clk);
	if (ret)
		return 0;

	rate = clk_get_rate(&clk);
	(void)clk_free(&clk);

	return rate;
}

int s32gen1_enable_dev_clk(const char *name, struct udevice *dev)
{
	struct clk clk;
	int ret = 0;

	ret = clk_get_by_name(dev, name, &clk);
	if (ret) {
		printf("Failed to get %s\n", name);
		return ret;
	}

	ret = clk_enable(&clk);
	(void)clk_free(&clk);

	return ret;
}

int s32gen1_set_parent_clk_id(ulong clk_id, ulong parent_clk_id)
{
	struct clk clk, pclk;
	int ret;
	struct udevice *clkdev = get_clk_device();

	if (!clkdev)
		return -EINVAL;

	pclk.id = parent_clk_id;
	ret = clk_request(clkdev, &pclk);
	if (ret)
		return ret;

	clk.id = clk_id;
	ret = clk_request(clkdev, &clk);
	if (ret)
		goto free_pclk;

	ret = clk_set_parent(&clk, &pclk);

	(void)clk_free(&clk);
free_pclk:
	(void)clk_free(&pclk);
	return ret;
}
