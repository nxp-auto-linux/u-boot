// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021-2022 NXP
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dt-bindings/gpio/gpio.h>

#define MSCR_OFF	0x0240
#define GPDO_BASE	0x1300
#define GPDI_BASE	0x1500

/* S32CC SIUL2_MSCR masks */
#define SIUL2_MSCR_S32CC_OBE		BIT(21)
#define SIUL2_MSCR_S32CC_ODE		BIT(20)
#define SIUL2_MSCR_S32CC_IBE		BIT(19)
#define SIUL2_MSCR_S32CC_SSS_MASK	GENMASK(2, 0)

#define SIUL2_MAX_VALID_RANGES		4

struct gpio_range {
	u16 gpio_offset;
	u16 pinctrl_offset;
	u16 cnt;
};

struct s32cc_gpio {
	void __iomem *base_addr;
	struct gpio_range valid_ranges[SIUL2_MAX_VALID_RANGES];
	u8 valid_ranges_cnt;
};

static inline void *s32cc_get_in_reg(struct s32cc_gpio *priv,
				     unsigned int pinctrl_offset)
{
	uintptr_t addr;

	addr = (uintptr_t)(priv->base_addr + GPDI_BASE + pinctrl_offset) ^ 3;

	return (void *)addr;
}

static inline void *s32cc_get_out_reg(struct s32cc_gpio *priv,
				      unsigned int pinctrl_offset)
{
	uintptr_t addr;

	addr = (uintptr_t)(priv->base_addr + GPDO_BASE + pinctrl_offset) ^ 3;

	return (void *)addr;
}

static inline void *s32cc_get_mscr_reg(struct s32cc_gpio *priv,
				       unsigned int pinctrl_offset)
{
	uintptr_t addr;

	addr = (uintptr_t)priv->base_addr + MSCR_OFF + pinctrl_offset * 4;

	return (void *)addr;
}

static int s32cc_get_pinctrl_offset(struct s32cc_gpio *priv,
				    unsigned int offset,
				    unsigned int *gpio_id)
{
	struct gpio_range *range;
	u16 range_begin;
	u16 range_end;
	int i;

	for (i = 0; i < priv->valid_ranges_cnt; i++) {
		range = &priv->valid_ranges[i];
		range_begin = range->pinctrl_offset;
		range_end = range_begin + range->cnt;
		*gpio_id = offset - range->gpio_offset + range_begin;
		if (*gpio_id >= range_begin && *gpio_id < range_end)
			return 0;
	}

	return -EINVAL;
}

static int s32cc_gpio_set_value(struct udevice *dev, unsigned int offset,
			        int value)
{
	struct s32cc_gpio *priv = dev_get_priv(dev);
	unsigned int pinctrl_offset;
	int ret;

	ret = s32cc_get_pinctrl_offset(priv, offset, &pinctrl_offset);
	if (ret)
		return ret;

	writeb(value, s32cc_get_out_reg(priv, pinctrl_offset));

	return 0;
}

static int s32cc_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct s32cc_gpio *priv = dev_get_priv(dev);
	unsigned int pinctrl_offset;
	u32 mscr;
	int ret;

	ret = s32cc_get_pinctrl_offset(priv, offset, &pinctrl_offset);
	if (ret)
		return ret;

	mscr = readl(s32cc_get_mscr_reg(priv, pinctrl_offset));

	/* First check if the pin is muxed as gpio. The input buffer and the
	 * output buffer might be enabled at the same time.
	 */
	if (mscr & SIUL2_MSCR_S32CC_SSS_MASK)
		return GPIOF_FUNC;

	if (mscr & SIUL2_MSCR_S32CC_OBE)
		return GPIOF_OUTPUT;

	if (mscr & SIUL2_MSCR_S32CC_IBE)
		return GPIOF_INPUT;

	return GPIOF_UNUSED;
}

static int s32cc_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct s32cc_gpio *priv = dev_get_priv(dev);
	unsigned int pinctrl_offset;
	int ret;

	ret = s32cc_get_pinctrl_offset(priv, offset, &pinctrl_offset);
	if (ret)
		return ret;

	switch (s32cc_gpio_get_function(dev, offset)) {
	case GPIOF_OUTPUT:
		return readb(s32cc_get_out_reg(priv, pinctrl_offset));
	default:
		return readb(s32cc_get_in_reg(priv, pinctrl_offset));
	}
}

static int s32cc_gpio_get_xlate(struct udevice *dev, struct gpio_desc *desc,
			        struct ofnode_phandle_args *args)
{
	struct s32cc_gpio *priv = dev_get_priv(dev);
	struct gpio_range *range;
	u16 range_begin;
	u16 range_end;
	int i;

	if (args->args_count < 1)
		return -EINVAL;

	for (i = 0; i < priv->valid_ranges_cnt; i++) {
		range = &priv->valid_ranges[i];
		range_begin = range->pinctrl_offset;
		range_end = range_begin + range->cnt;
		if (args->args[0] >= range_begin && args->args[0] < range_end) {
			desc->offset += args->args[0] - range_begin;
			break;
		}
	}

	if (!desc->offset)
		return -EINVAL;

	debug("%s offset %u arg %d\n", __func__, desc->offset, args->args[0]);

	if (args->args_count < 2)
		return 0;

	if (args->args[1] & GPIO_ACTIVE_LOW)
		desc->flags = GPIOD_ACTIVE_LOW;

	return 0;
}

static int s32cc_gpio_set_flags(struct udevice *dev, unsigned int offset,
			        ulong flags)
{
	struct s32cc_gpio *priv = dev_get_priv(dev);
	unsigned int pinctrl_offset;
	u32 mscr;
	u8 value;
	int ret;

	ret = s32cc_get_pinctrl_offset(priv, offset, &pinctrl_offset);
	if (ret)
		return ret;

	mscr = readl(s32cc_get_mscr_reg(priv, pinctrl_offset));

	if (flags & GPIOD_IS_OUT) {
		mscr &= ~SIUL2_MSCR_S32CC_IBE;
		mscr |= SIUL2_MSCR_S32CC_OBE;

		/* To enable open drain both OBE and ODE bits need to be set */
		if (flags & GPIOD_OPEN_DRAIN)
			mscr |= SIUL2_MSCR_S32CC_ODE;
		else
			mscr &= ~SIUL2_MSCR_S32CC_ODE;

		value = !!(flags & GPIOD_IS_OUT_ACTIVE);
		writeb(value, s32cc_get_out_reg(priv, pinctrl_offset));

	} else if (flags & GPIOD_IS_IN) {
		mscr &= ~SIUL2_MSCR_S32CC_OBE;
		mscr |= SIUL2_MSCR_S32CC_IBE;
	}

	writel(mscr, s32cc_get_mscr_reg(priv, pinctrl_offset));

	return 0;
}

int s32cc_gpio_get_flags(struct udevice *dev, unsigned int offset,
		         ulong *flagsp)
{
	struct s32cc_gpio *priv = dev_get_priv(dev);
	unsigned int pinctrl_offset;
	u32 mscr;
	ulong flags = 0;
	int ret;

	ret = s32cc_get_pinctrl_offset(priv, offset, &pinctrl_offset);
	if (ret)
		return ret;

	mscr = readl(s32cc_get_mscr_reg(priv, pinctrl_offset));

	if (mscr & SIUL2_MSCR_S32CC_OBE) {
		flags |= GPIOD_IS_OUT;

		if (mscr & SIUL2_MSCR_S32CC_ODE)
			flags |= GPIOD_OPEN_DRAIN;

		if (readb(s32cc_get_out_reg(priv, pinctrl_offset)))
			flags |= GPIOD_IS_OUT_ACTIVE;

	} else if (mscr & SIUL2_MSCR_S32CC_IBE) {
		flags |= GPIOD_IS_IN;
	}

	*flagsp = flags;

	return 0;
}

static const struct dm_gpio_ops s32cc_gpio_ops = {
	.get_value = s32cc_gpio_get_value,
	.set_value = s32cc_gpio_set_value,
	.get_function = s32cc_gpio_get_function,
	.xlate = s32cc_gpio_get_xlate,
	.set_flags = s32cc_gpio_set_flags,
	.get_flags = s32cc_gpio_get_flags,
};

static int s32cc_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct s32cc_gpio *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args = {.args = {0}};
	fdt_addr_t addr;
	int i = 0;
	int ret;

	addr = devfdt_get_addr(dev->parent);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base_addr = (void __iomem *)addr;

	uc_priv->bank_name = dev->name;
	uc_priv->gpio_count = 0;

	ret = dev_read_phandle_with_args(dev, "gpio-ranges", NULL, 3, i, &args);
	if (ret < 0) {
		pr_err("gpio-ranges: property missing or invalid\n");
		return ret;
	}

	do  {
		priv->valid_ranges[i].gpio_offset = args.args[0];
		priv->valid_ranges[i].pinctrl_offset = args.args[1];
		priv->valid_ranges[i].cnt = args.args[2];
		uc_priv->gpio_count += args.args[2];
		ret = dev_read_phandle_with_args(dev, "gpio-ranges", NULL, 3,
						 ++i, &args);
		if (ret == -EINVAL) {
			pr_err("gpio-ranges: property invalid\n");
			return ret;
		}

		if (i >= SIUL2_MAX_VALID_RANGES) {
			pr_err("Too many gpio ranges\n");
			return -ENOMEM;
		}
	} while (ret != -ENOENT);

	priv->valid_ranges_cnt = i;

	return 0;
}

static const struct udevice_id s32cc_gpio_ids[] = {
	{ .compatible = "nxp,s32cc-siul2-gpio"},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(s32cc_gpio) = {
	.name = "s32cc_gpio",
	.id = UCLASS_GPIO,
	.of_match = of_match_ptr(s32cc_gpio_ids),
	.ops = &s32cc_gpio_ops,
	.priv_auto = sizeof(struct s32cc_gpio),
	.probe = s32cc_gpio_probe,
};
