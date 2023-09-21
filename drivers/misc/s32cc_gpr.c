// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022-2023 NXP
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dt-bindings/nvmem/s32cc-gpr-nvmem.h>

#define A53_GPR_OFF				(0x400)

#define A53_CLUSTER_GPR_GPR(x)			((x) * 0x4)
#define GPR06_CA53_LOCKSTEP_ENABLED_MASK	BIT(0)
#define GPR06_CA53_LOCKSTEP_ENABLED_SHIFT	0

struct s32cc_gpr {
	const struct s32cc_gpr_plat *plat;
	void __iomem *base;
};

struct s32cc_gpr_mapping {
	u32 gpr_misc_off;
	u32 gpr_off;
	u32 reg_off;
	u32 mask;
	u32 shift;
	bool read_only;
};

struct s32cc_gpr_plat {
	const struct s32cc_gpr_mapping *mappings;
	size_t n_mappings;
	const struct s32cc_gpr_plat *next;
};

static const struct s32cc_gpr_mapping s32cc_gpr_mappings[] = {
	{
		.gpr_misc_off = S32CC_GPR_LOCKSTEP_ENABLED_OFFSET,
		.gpr_off = A53_GPR_OFF,
		.reg_off = A53_CLUSTER_GPR_GPR(6),
		.mask = GPR06_CA53_LOCKSTEP_ENABLED_MASK,
		.shift = GPR06_CA53_LOCKSTEP_ENABLED_SHIFT,
		.read_only = true,
	},
};

static const struct s32cc_gpr_plat s32cc_gpr_plat = {
	.mappings = &s32cc_gpr_mappings[0],
	.n_mappings = ARRAY_SIZE(s32cc_gpr_mappings),
};

static int find_gpr_mapping(const struct s32cc_gpr_plat *plat, int offset,
			    const struct s32cc_gpr_mapping **mapping)
{
	size_t i;

	*mapping = NULL;
	while (plat) {
		for (i = 0u; i < plat->n_mappings; i++) {
			if (plat->mappings[i].gpr_misc_off == offset) {
				*mapping = &plat->mappings[i];
				break;
			}
		}

		if (*mapping)
			break;

		plat = plat->next;
	}

	if (!(*mapping))
		return -EINVAL;

	return 0;
}

static int s32cc_gpr_read(struct udevice *dev, int offset, void *buf, int size)
{
	struct s32cc_gpr *s32cc_gpr_data = dev_get_plat(dev);
	const struct s32cc_gpr_mapping *mapping = NULL;
	const struct s32cc_gpr_plat *plat = s32cc_gpr_data->plat;
	u32 val, *buf32 = buf;
	int ret;

	if (size != sizeof(*buf32))
		return -EINVAL;

	ret = find_gpr_mapping(plat, offset, &mapping);
	if (ret)
		return -EINVAL;

	val = readl(s32cc_gpr_data->base + mapping->gpr_off + mapping->reg_off);
	val = (val & mapping->mask) >> mapping->shift;

	*buf32 = val;
	return size;
}

static int s32cc_gpr_write(struct udevice *dev, int offset, const void *buf,
			   int size)
{
	struct s32cc_gpr *s32cc_gpr_data = dev_get_plat(dev);
	const struct s32cc_gpr_mapping *mapping = NULL;
	const struct s32cc_gpr_plat *plat = s32cc_gpr_data->plat;
	u32 val, max_val, reg;
	const u32 *buf32 = buf;
	int ret;

	if (size != sizeof(*buf32))
		return -EINVAL;

	ret = find_gpr_mapping(plat, offset, &mapping);
	if (ret || mapping->read_only)
		return -EINVAL;

	val = *buf32;

	max_val = mapping->mask >> mapping->shift;
	if (val > max_val)
		return -EINVAL;

	reg = readl(s32cc_gpr_data->base + mapping->gpr_off + mapping->reg_off);
	reg = (reg & ~mapping->mask) |
	      ((val << mapping->shift) & mapping->mask);
	writel(reg, s32cc_gpr_data->base + mapping->gpr_off + mapping->reg_off);

	return size;
}

static int s32cc_gpr_set_plat(struct udevice *dev)
{
	struct s32cc_gpr *s32cc_gpr_data = dev_get_plat(dev);

	s32cc_gpr_data->base = dev_read_addr_ptr(dev);
	if (!s32cc_gpr_data->base)
		return -EINVAL;

	s32cc_gpr_data->plat = (struct s32cc_gpr_plat *)
		dev_get_driver_data(dev);

	return 0;
}

static const struct misc_ops s32cc_gpr_ops = {
	.read = s32cc_gpr_read,
	.write = s32cc_gpr_write,
};

static const struct udevice_id s32cc_gpr_ids[] = {
	{ .compatible = "nxp,s32cc-gpr", .data = (ulong)&s32cc_gpr_plat, },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(s32cc_gpr) = {
	.name = "s32cc-gpr",
	.id = UCLASS_MISC,
	.ops = &s32cc_gpr_ops,
	.of_match = s32cc_gpr_ids,
	.plat_auto = sizeof(struct s32cc_gpr),
	.of_to_plat = s32cc_gpr_set_plat,
};
