/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020,2022 NXP
 */
#ifndef S32GEN1_CLOCK_UTILS_H
#define S32GEN1_CLOCK_UTILS_H

#include <clk-uclass.h>
#include <inttypes.h>

int enable_early_clocks(void);

int s32gen1_enable_dev_clk(const char *name, struct udevice *dev);
ulong s32gen1_set_dev_clk_rate(const char *name,
			       struct udevice *dev, ulong rate);
ulong s32gen1_get_dev_clk_rate(const char *name, struct udevice *dev);

int s32gen1_set_parent_clk_id(ulong clk_id, ulong parent_clk_id);

#endif
