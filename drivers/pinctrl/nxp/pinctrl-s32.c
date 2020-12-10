// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/pinctrl.h>

#define MSCR_OFF	0x0240

struct s32_range {
	u32 begin;
	u32 end;
};

struct s32_pinctrl {
	void __iomem *base_addr;
	struct s32_range mscr;
	struct s32_range imcr;
};

static inline bool s32_is_mscr_pin(struct s32_pinctrl *ctlr, u32 pin)
{
	return pin >= ctlr->mscr.begin && pin <= ctlr->mscr.end;
}

static inline bool s32_is_imcr_pin(struct s32_pinctrl *ctlr, u32 pin)
{
	return pin >= ctlr->imcr.begin && pin <= ctlr->imcr.end;
}

static inline bool s32_pin_is_valid(struct s32_pinctrl *ctlr, u32 pin)
{
	return s32_is_imcr_pin(ctlr, pin) || s32_is_mscr_pin(ctlr, pin);
}

static inline void *s32_get_mscr(struct s32_pinctrl *ctlr, u32 pin)
{
	return (void *)(ctlr->base_addr + MSCR_OFF + pin * 4);
}

static int s32_set_state(struct udevice *dev, struct udevice *config)
{
	struct s32_pinctrl *priv = dev_get_priv(dev);
	u32 pin, function;
	int index = 0;
	int sz;
	int ret;

	if (!dev_read_prop(config, "fsl,pins", &sz)) {
		pr_err("fsl,pins property not found\n");
		return -EINVAL;
	}

	sz >>= 2;
	if (sz % 2) {
		pr_err("fsl,pins invalid array size: %d\n", sz);
		return -EINVAL;
	}

	while (index < sz) {
		ret = dev_read_u32_index(config, "fsl,pins", index++, &pin);
		if (ret) {
			pr_err("failed to read pin ID\n");
			return ret;
		}
		ret = dev_read_u32_index(config, "fsl,pins", index++,
					 &function);
		if (ret) {
			pr_err("failed to read pin function\n");
			return ret;
		}

		if (s32_pin_is_valid(priv, pin)) {
			writel(function, s32_get_mscr(priv, pin));
			pr_debug("%s function:reg 0x%x:0x%p\n", config->name,
				 function, s32_get_mscr(priv, pin));
			continue;
		}

		pr_err("%s invalid pin found function:reg 0x%x:0x%p\n",
		       config->name, function, s32_get_mscr(priv, pin));

		return -EINVAL;
	};

	return 0;
}

static const struct pinctrl_ops s32_pinctrl_ops = {
	.set_state = s32_set_state,
};

static int s32_pinctrl_probe(struct udevice *dev)
{
	struct s32_pinctrl *priv = dev_get_priv(dev);
	struct ofnode_phandle_args out_args;
	int cnt, ret;

	fdt_addr_t addr;

	addr = devfdt_get_addr(dev->parent);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base_addr = (void *)addr;

	cnt = dev_count_phandle_with_args(dev, "pins", "#pinctrl-cells");
	if (cnt < 0)
		return cnt;
	else if (cnt != 2)
		return -EINVAL;

	ret = dev_read_phandle_with_args(dev, "pins", "#pinctrl-cells", 0, 0,
					 &out_args);
	if (ret)
		return ret;
	if (out_args.args_count != 2)
		return -EINVAL;

	priv->mscr.begin = out_args.args[0];
	priv->mscr.end = out_args.args[1];

	ret = dev_read_phandle_with_args(dev, "pins", "#pinctrl-cells", 0, 1,
					 &out_args);
	if (ret)
		return ret;
	if (out_args.args_count != 2)
		return -EINVAL;

	priv->imcr.begin = out_args.args[0];
	priv->imcr.end = out_args.args[1];

	return 0;
}

static const struct udevice_id s32_pinctrl_ids[] = {
	{ .compatible = "fsl,s32-gen1-siul2-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(s32_pinctrl) = {
	.name = "s32_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(s32_pinctrl_ids),
	.probe = s32_pinctrl_probe,
	.priv_auto_alloc_size = sizeof(struct s32_pinctrl),
	.ops = &s32_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
