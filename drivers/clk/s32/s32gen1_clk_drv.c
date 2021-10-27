// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020-2021 NXP
 */
#include <asm/arch/mc_cgm_regs.h>
#include <asm/io.h>
#include <command.h>
#include <dm/device.h>
#include <dm/lists.h>
#include <dm/of_access.h>
#include <dm/read.h>
#include <dt-bindings/clock/s32gen1-clock.h>
#include <linux/ioport.h>
#include <log.h>
#include <s32gen1_clk_funcs.h>
#include <s32gen1_clk_modules.h>
#include <s32gen1_scmi_clk.h>

static int bind_clk_provider(struct udevice *pdev, const char *compatible,
			     void **base_addr)
{
	struct resource res;
	struct udevice *dev;
	ofnode node;
	int ret;

	*base_addr = NULL;
	node = ofnode_by_compatible(ofnode_null(), compatible);
	if (!ofnode_valid(node)) {
		/* Don't print here an error if the node doesn't exist.
		 * It may be never used.
		 */
		pr_debug("Could not find '%s' node\n", compatible);
		return -EIO;
	}

	ret = ofnode_read_resource(node, 0, &res);
	if (ret) {
		pr_err("Failed to get '%s' registers\n", compatible);
		return -EIO;
	}

	*base_addr = map_physmem(res.start, res.end - res.start + 1,
				 MAP_NOCACHE);
	if (!*base_addr) {
		pr_err("Failed to map '%s'\n", compatible);
		return -EIO;
	}

	ret = device_bind_driver_to_node(pdev, "s32gen1_clk",
					 ofnode_get_name(node), node, &dev);
	if (ret) {
		pr_err("Failed to bind '%s'\n", compatible);
		return -EIO;
	}

	/* Pre probe */
	ret = clk_set_defaults(dev, 0);
	if (ret) {
		pr_err("Failed to set default clocks(0) for '%s'\n",
		       compatible);
		return -EIO;
	}

	/* Post probe */
	ret = clk_set_defaults(dev, 1);
	if (ret) {
		pr_err("Failed to set default clocks(1) for '%s'\n",
		       compatible);
		return -EIO;
	}

	return ret;
}

static int s32gen1_clk_probe(struct udevice *dev)
{
	size_t i;
	struct s32gen1_clk_priv *priv = dev_get_priv(dev);

	struct clk_dep {
		void **base_addr;
		const char *compat;
	} deps[] = {
		{
			.base_addr = &priv->fxosc,
			.compat = "fsl,s32gen1-fxosc",
		},
		{
			.base_addr = &priv->cgm0,
			.compat = "fsl,s32gen1-mc_cgm0",
		},
		{
			.base_addr = &priv->mc_me,
			.compat = "fsl,s32gen1-mc_me",
		},
		{
			.base_addr = &priv->rdc,
			.compat = "fsl,s32gen1-rdc",
		},
		{
			.base_addr = &priv->rgm,
			.compat = "fsl,s32gen1-rgm",
		},
		{
			.base_addr = &priv->cgm1,
			.compat = "fsl,s32gen1-mc_cgm1",
		},
		{
			.base_addr = &priv->cgm2,
			.compat = "fsl,s32gen1-mc_cgm2",
		},
		{
			.base_addr = &priv->cgm5,
			.compat = "fsl,s32gen1-mc_cgm5",
		},
		{
			.base_addr = &priv->cgm6,
			.compat = "fsl,s32gen1-mc_cgm6",
		},
		{
			.base_addr = &priv->armpll,
			.compat = "fsl,s32gen1-armpll",
		},
		{
			.base_addr = &priv->periphpll,
			.compat = "fsl,s32gen1-periphpll",
		},
		{
			.base_addr = &priv->accelpll,
			.compat = "fsl,s32gen1-accelpll",
		},
		{
			.base_addr = &priv->ddrpll,
			.compat = "fsl,s32gen1-ddrpll",
		},
		{
			.base_addr = &priv->armdfs,
			.compat = "fsl,s32gen1-armdfs",
		},
		{
			.base_addr = &priv->periphdfs,
			.compat = "fsl,s32gen1-periphdfs",
		},
	};

	for (i = 0; i < ARRAY_SIZE(deps); i++)
		bind_clk_provider(dev, deps[i].compat, deps[i].base_addr);

	return 0;
}

void *get_base_addr(enum s32gen1_clk_source id, struct s32gen1_clk_priv *priv)
{
	switch (id) {
	case S32GEN1_ACCEL_PLL:
		return priv->accelpll;
	case S32GEN1_ARM_DFS:
		return priv->armdfs;
	case S32GEN1_ARM_PLL:
		return priv->armpll;
	case S32GEN1_CGM0:
		return priv->cgm0;
	case S32GEN1_CGM1:
		return priv->cgm1;
	case S32GEN1_CGM2:
		return priv->cgm2;
	case S32GEN1_CGM5:
		return priv->cgm5;
	case S32GEN1_CGM6:
		return priv->cgm6;
	case S32GEN1_DDR_PLL:
		return priv->ddrpll;
	case S32GEN1_FXOSC:
		return priv->fxosc;
	case S32GEN1_PERIPH_DFS:
		return priv->periphdfs;
	case S32GEN1_PERIPH_PLL:
		return priv->periphpll;
	default:
		pr_err("Unknown clock source id: %d\n", id);
	}

	return NULL;
}

static struct clk_ops s32gen1_clk_ops = {
	.request = s32gen1_scmi_request,
	.get_rate = s32gen1_scmi_get_rate,
	.set_rate = s32gen1_plat_set_rate,
	.set_parent = s32gen1_scmi_set_parent,
	.enable = s32gen1_scmi_enable,
	.disable = s32gen1_scmi_disable,
};

static const struct udevice_id s32gen1_clk_ids[] = {
	{ .compatible = "fsl,s32-gen1-clocking", },
	{}
};

U_BOOT_DRIVER(s32gen1_clk) = {
	.name		= "s32gen1_clk",
	.id		= UCLASS_CLK,
	.of_match	= s32gen1_clk_ids,
	.ops		= &s32gen1_clk_ops,
	.priv_auto_alloc_size = sizeof(struct s32gen1_clk_priv),
	.probe		= s32gen1_clk_probe,
	.flags		= DM_FLAG_PRE_RELOC,
};
