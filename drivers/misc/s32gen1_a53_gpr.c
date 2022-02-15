// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <dm/device.h>
#include <dm/fdtaddr.h>
#include <misc.h>
#include <asm/io.h>
#include "s32gen1_a53_gpr.h"

#define A53_CLUSTER_GPR_GPR(x) ((x) * 0x4)
#define GPR06_CA53_LOCKSTEP_EN_MASK	BIT(0)
#define GPR06_CA53_LOCKSTEP_EN_SHIFT	0

struct a53_gpr {
	const struct a53_gpr_platdata *platdata;
	fdt_addr_t base;
};

struct a53_gpr_mapping {
	u32 gpr_misc_off;
	u32 gpr_off;
	u32 mask;
	u32 shift;
};

struct a53_gpr_platdata {
	const struct a53_gpr_mapping *mappings;
	size_t n_mappings;
	const struct a53_gpr_platdata *next;
};

static const struct a53_gpr_mapping a53_gpr_mappings[] = {
	{
		.gpr_misc_off = S32GEN1_A53_GPR_LOCKSTEP_EN,
		.gpr_off = A53_CLUSTER_GPR_GPR(6),
		.mask = GPR06_CA53_LOCKSTEP_EN_MASK,
		.shift = GPR06_CA53_LOCKSTEP_EN_SHIFT,
	},
};

static const struct a53_gpr_platdata s32gen1_a53_gpr_platdata = {
	.mappings = &a53_gpr_mappings[0],
	.n_mappings = ARRAY_SIZE(a53_gpr_mappings),
};

static int a53_gpr_read(struct udevice *dev,
			int offset, void *buf, int size)
{
	struct a53_gpr *a53_gpr_data = dev_get_platdata(dev);
	const struct a53_gpr_mapping *mapping = NULL;
	const struct a53_gpr_platdata *platdata = a53_gpr_data->platdata;
	size_t i;
	u32 val;

	if (size != sizeof(u32))
		return 0;

	while (platdata) {
		for (i = 0u; i < platdata->n_mappings; i++) {
			if (platdata->mappings[i].gpr_misc_off == offset) {
				mapping = &platdata->mappings[i];
				break;
			}
		}

		if (mapping)
			break;

		platdata = platdata->next;
	}

	if (!mapping)
		return 0;

	val = readl(a53_gpr_data->base + mapping->gpr_off);
	val = (val & mapping->mask) >> mapping->shift;

	*((u32 *)buf) = val;
	return size;
}

static int a53_gpr_set_platdata(struct udevice *dev)
{
	struct a53_gpr *a53_gpr_data = dev_get_platdata(dev);

	a53_gpr_data->base = devfdt_get_addr(dev);
	if (a53_gpr_data->base == (fdt_addr_t)FDT_ADDR_T_NONE)
		return -EINVAL;

	a53_gpr_data->platdata = (struct a53_gpr_platdata *)
		dev_get_driver_data(dev);

	return 0;
}

static const struct misc_ops s32gen1_a53_gpr_ops = {
	.read = a53_gpr_read,
};

static const struct udevice_id s32gen1_a53_gpr_ids[] = {
	{ .compatible = "fsl,s32gen1-a53-gpr",
	  .data = (ulong)&s32gen1_a53_gpr_platdata,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(s32gen1_a53_gpr) = {
	.name = "s32gen1-a53-gpr",
	.id = UCLASS_MISC,
	.ops = &s32gen1_a53_gpr_ops,
	.of_match = s32gen1_a53_gpr_ids,
	.platdata_auto_alloc_size = sizeof(struct a53_gpr),
	.ofdata_to_platdata = a53_gpr_set_platdata,
};

