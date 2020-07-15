// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020 NXP
 */
#include <clk.h>
#include <s32gen1_clk_dump.h>
#include <s32gen1_clk_funcs.h>
#include <s32gen1_clk_utils.h>

int soc_clk_dump(void)
{
	struct clk clk;
	struct udevice *dev;
	struct s32gen1_clk *s32_clk;
	struct s32gen1_clk_blk *clk_name;
	ulong i, rate, id;
	const char *name;

	dev = get_clk_device();
	if (!dev)
		return -EINVAL;

	clk.dev = dev;

	for (i = 0; clk_name = s32gen1_get_clk_blk(i), clk_name; i++) {
		id = clk_name->id;
		name = s32gen1_get_clock_name(clk_name);

		s32_clk = get_clock(id);
		if (!s32_clk)
			continue;

		clk.id = id;
		rate = s32gen1_get_rate(&clk);

		printf("%-30.30s : %lu Hz\n", name, rate);
	}

	return 0;
}

