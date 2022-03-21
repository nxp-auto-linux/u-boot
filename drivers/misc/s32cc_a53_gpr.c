// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <asm/io.h>
#include <dm/device.h>
#include <s32-cc/a53_gpr.h>

#define A53_CLUSTER_GPR_GPR(x) ((x) * 0x4)
#define GPR06_CA53_LOCKSTEP_EN_MASK	BIT(0)
#define GPR06_CA53_LOCKSTEP_EN_SHIFT	0

struct a53_gpr {
	const struct a53_gpr_plat *plat;
	void __iomem *base;
};

struct a53_gpr_mapping {
	u32 gpr_misc_off;
	u32 gpr_off;
	u32 mask;
	u32 shift;
};

struct a53_gpr_plat {
	const struct a53_gpr_mapping *mappings;
	size_t n_mappings;
	const struct a53_gpr_plat *next;
};

static const struct a53_gpr_mapping a53_gpr_mappings[] = {
	{
		.gpr_misc_off = S32CC_A53_GPR_LOCKSTEP_EN,
		.gpr_off = A53_CLUSTER_GPR_GPR(6),
		.mask = GPR06_CA53_LOCKSTEP_EN_MASK,
		.shift = GPR06_CA53_LOCKSTEP_EN_SHIFT,
	},
};

static const struct a53_gpr_plat s32cc_a53_gpr_plat = {
	.mappings = &a53_gpr_mappings[0],
	.n_mappings = ARRAY_SIZE(a53_gpr_mappings),
};

static int a53_gpr_read(struct udevice *dev,
			int offset, void *buf, int size)
{
	struct a53_gpr *a53_gpr_data = dev_get_platdata(dev);
	const struct a53_gpr_mapping *mapping = NULL;
	const struct a53_gpr_plat *plat = a53_gpr_data->plat;
	size_t i;
	u32 val;

	if (size != sizeof(u32))
		return 0;

	while (plat) {
		for (i = 0u; i < plat->n_mappings; i++) {
			if (plat->mappings[i].gpr_misc_off == offset) {
				mapping = &plat->mappings[i];
				break;
			}
		}

		if (mapping)
			break;

		plat = plat->next;
	}

	if (!mapping)
		return 0;

	val = readl(a53_gpr_data->base + mapping->gpr_off);
	val = (val & mapping->mask) >> mapping->shift;

	*((u32 *)buf) = val;
	return size;
}

static int a53_gpr_set_plat(struct udevice *dev)
{
	struct a53_gpr *a53_gpr_data = dev_get_platdata(dev);

	a53_gpr_data->base = dev_read_addr_ptr(dev);
	if (!a53_gpr_data->base)
		return -EINVAL;

	a53_gpr_data->plat = (struct a53_gpr_plat *)
		dev_get_driver_data(dev);

	return 0;
}

static const struct misc_ops s32cc_a53_gpr_ops = {
	.read = a53_gpr_read,
};

static const struct udevice_id s32cc_a53_gpr_ids[] = {
	{ .compatible = "nxp,s32cc-a53-gpr",
	  .data = (ulong)&s32cc_a53_gpr_plat,
	},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(s32cc_a53_gpr) = {
	.name = "s32cc-a53-gpr",
	.id = UCLASS_MISC,
	.ops = &s32cc_a53_gpr_ops,
	.of_match = s32cc_a53_gpr_ids,
	.platdata_auto_alloc_size = sizeof(struct a53_gpr),
	.ofdata_to_platdata = a53_gpr_set_plat,
};

