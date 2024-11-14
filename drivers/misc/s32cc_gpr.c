// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022-2024 NXP
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <regmap.h>
#include <syscon.h>
#include <asm/io.h>
#include <dm/device.h>
#include <dm/device_compat.h>
#include <linux/err.h>
#include <dt-bindings/nvmem/s32cc-gpr-nvmem.h>
#include <dt-bindings/nvmem/s32g-gpr-nvmem.h>
#include <dt-bindings/nvmem/s32r45-gpr-nvmem.h>

#define SRC_0_OFF				(0x0)
#define A53_GPR_OFF				(0x400)
#define SRC_1_OFF				(0xA00)

#define A53_CLUSTER_GPR_GPR(x)			((x) * 0x4)
#define GPR06_CA53_LOCKSTEP_ENABLED_MASK	BIT(0)
#define GPR06_CA53_LOCKSTEP_ENABLED_SHIFT	0

#define GMAC_0_CTRL_STS_OFF		0x4
#define GMAC_0_CTRL_STS_PHY_INTF_SHIFT	0
#define GMAC_0_CTRL_STS_PHY_INTF_MASK	\
	GENMASK(3, GMAC_0_CTRL_STS_PHY_INTF_SHIFT)

#define GMAC_1_CTRL_STS_OFF		0x0
#define GMAC_1_CTRL_STS_PHY_INTF_SHIFT	0
#define GMAC_1_CTRL_STS_PHY_INTF_MASK	\
	GENMASK(3, GMAC_1_CTRL_STS_PHY_INTF_SHIFT)

#define PFE_COH_EN_OFF			0x0
#define PFE_COH_EN_SHIFT		0
#define PFE_COH_EN_MASK			GENMASK(5, PFE_COH_EN_SHIFT)

#define PFE_EMACS_INTF_SEL_OFF		0x4
#define PFE_EMACS_INTF_SEL_SHIFT	0
#define PFE_EMACS_INTF_SEL_MASK		\
	GENMASK(11, PFE_EMACS_INTF_SEL_SHIFT)

#define PFE_PWR_CTRL_OFF		0x20
#define PFE_PWR_CTRL_SHIFT		0
#define PFE_PWR_CTRL_MASK		GENMASK(8, PFE_PWR_CTRL_SHIFT)

#define PFE_EMACS_GENCTRL1_OFF		0xE4
#define PFE_EMACS_GENCTRL1_SHIFT	0
#define PFE_EMACS_GENCTRL1_MASK		GENMASK(2, PFE_EMACS_GENCTRL1_SHIFT)

#define PFE_GENCTRL3_OFF		0xEC
#define PFE_GENCTRL3_SHIFT		16
#define PFE_GENCTRL3_MASK		BIT(PFE_GENCTRL3_SHIFT)

struct s32cc_gpr {
	const struct s32cc_gpr_plat *plat;
	struct regmap *regmap;
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
	{
		.gpr_misc_off = S32CC_GPR_GMAC0_PHY_INTF_SEL_OFFSET,
		.gpr_off = SRC_0_OFF,
		.reg_off = GMAC_0_CTRL_STS_OFF,
		.mask = GMAC_0_CTRL_STS_PHY_INTF_MASK,
		.shift = GMAC_0_CTRL_STS_PHY_INTF_SHIFT,
		.read_only = false,
	},
};

static const struct s32cc_gpr_mapping s32g_gpr_mappings[] = {
	{
		.gpr_misc_off = S32G_GPR_PFE_EMACS_INTF_SEL_OFFSET,
		.gpr_off = SRC_1_OFF,
		.reg_off = PFE_EMACS_INTF_SEL_OFF,
		.mask = PFE_EMACS_INTF_SEL_MASK,
		.shift = PFE_EMACS_INTF_SEL_SHIFT,
		.read_only = false,
	},
	{
		.gpr_misc_off = S32G_GPR_PFE_COH_EN_OFFSET,
		.gpr_off = SRC_1_OFF,
		.reg_off = PFE_COH_EN_OFF,
		.mask = PFE_COH_EN_MASK,
		.shift = PFE_COH_EN_SHIFT,
		.read_only = false,
	},
	{
		.gpr_misc_off = S32G_GPR_PFE_PWR_CTRL_OFFSET,
		.gpr_off = SRC_1_OFF,
		.reg_off = PFE_PWR_CTRL_OFF,
		.mask = PFE_PWR_CTRL_MASK,
		.shift = PFE_PWR_CTRL_SHIFT,
		.read_only = false,
	},
	{
		.gpr_misc_off = S32G_GPR_PFE_EMACS_GENCTRL1_OFFSET,
		.gpr_off = SRC_1_OFF,
		.reg_off = PFE_EMACS_GENCTRL1_OFF,
		.mask = PFE_EMACS_GENCTRL1_MASK,
		.shift = PFE_EMACS_GENCTRL1_SHIFT,
		.read_only = false,
	},
	{
		.gpr_misc_off = S32G_GPR_PFE_GENCTRL3_OFFSET,
		.gpr_off = SRC_1_OFF,
		.reg_off = PFE_GENCTRL3_OFF,
		.mask = PFE_GENCTRL3_MASK,
		.shift = PFE_GENCTRL3_SHIFT,
		.read_only = false,
	},
};

static const struct s32cc_gpr_mapping s32r_gpr_mappings[] = {
	{
		.gpr_misc_off = S32R45_GPR_GMAC1_PHY_INTF_SEL_OFFSET,
		.gpr_off = SRC_1_OFF,
		.reg_off = GMAC_1_CTRL_STS_OFF,
		.mask = GMAC_1_CTRL_STS_PHY_INTF_MASK,
		.shift = GMAC_1_CTRL_STS_PHY_INTF_SHIFT,
		.read_only = false,
	},
};

static const struct s32cc_gpr_plat s32cc_gpr_plat = {
	.mappings = &s32cc_gpr_mappings[0],
	.n_mappings = ARRAY_SIZE(s32cc_gpr_mappings),
};

static const struct s32cc_gpr_plat s32g_gpr_plat = {
	.mappings = &s32g_gpr_mappings[0],
	.n_mappings = ARRAY_SIZE(s32g_gpr_mappings),
	.next = &s32cc_gpr_plat,
};

static const struct s32cc_gpr_plat s32r_gpr_plat = {
	.mappings = &s32r_gpr_mappings[0],
	.n_mappings = ARRAY_SIZE(s32r_gpr_mappings),
	.next = &s32cc_gpr_plat,
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

	ret = regmap_read(s32cc_gpr_data->regmap,
			  mapping->gpr_off + mapping->reg_off, &val);
	if (ret)
		return ret;

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
	const u32 *buf32 = buf;
	u32 val, max_val;
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

	ret = regmap_update_bits(s32cc_gpr_data->regmap,
				 mapping->gpr_off + mapping->reg_off,
				 mapping->mask,
				 val << mapping->shift);
	if (ret)
		return ret;

	return size;
}

static int s32cc_gpr_set_plat(struct udevice *dev)
{
	struct s32cc_gpr *s32cc_gpr_data = dev_get_plat(dev);

	s32cc_gpr_data->regmap = syscon_node_to_regmap(dev_ofnode(dev->parent));
	if (IS_ERR(s32cc_gpr_data->regmap)) {
		dev_err(dev, "Failed to retrieve regmap: %ld!\n",
			PTR_ERR(s32cc_gpr_data->regmap));
		return PTR_ERR(s32cc_gpr_data->regmap);
	}

	s32cc_gpr_data->plat = (struct s32cc_gpr_plat *)
		dev_get_driver_data(dev);

	return 0;
}

static const struct misc_ops s32cc_gpr_ops = {
	.read = s32cc_gpr_read,
	.write = s32cc_gpr_write,
};

static const struct udevice_id s32cc_gpr_ids[] = {
	{ .compatible = "nxp,s32cc-gpr",  .data = (ulong)&s32cc_gpr_plat, },
	{ .compatible = "nxp,s32g-gpr",   .data = (ulong)&s32g_gpr_plat,  },
	{ .compatible = "nxp,s32r45-gpr", .data = (ulong)&s32r_gpr_plat,  },
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
