// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <common.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm.h>
#include <dm/pinctrl.h>
#include <dt-bindings/gpio/gpio.h>
#include <errno.h>

#include <dt-bindings/pinctrl/s32-gen1-pinctrl.h>

#define MSCR_OFF	0x0240
#define GPDO_BASE	0x1300
#define GPDI_BASE	0x1500

#define SIUL2_MAX_VALID_RANGES		4

struct gpio_range {
	u16 gpio_offset;
	u16 pinctrl_offset;
	u16 cnt;
};

struct s32_gpio {
	void __iomem *base_addr;
	struct gpio_range valid_ranges[SIUL2_MAX_VALID_RANGES];
	u8 valid_ranges_cnt;
};

static inline void *s32_get_in_reg(struct s32_gpio *priv,
				   unsigned int pinctrl_offset)
{
	uintptr_t addr;

	addr = (uintptr_t)(priv->base_addr + GPDI_BASE + pinctrl_offset) ^ 3;

	return (void *)addr;
}

static inline void *s32_get_out_reg(struct s32_gpio *priv,
				    unsigned int pinctrl_offset)
{
	uintptr_t addr;

	addr = (uintptr_t)(priv->base_addr + GPDO_BASE + pinctrl_offset) ^ 3;

	return (void *)addr;
}

static inline void *s32_get_mscr_reg(struct s32_gpio *priv,
				     unsigned int pinctrl_offset)
{
	uintptr_t addr;

	addr = (uintptr_t)priv->base_addr + MSCR_OFF + pinctrl_offset * 4;

	return (void *)addr;
}

static int s32_get_pinctrl_offset(struct s32_gpio *priv, unsigned int offset,
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

static int s32_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct s32_gpio *priv = dev_get_priv(dev);
	unsigned int pinctrl_offset;
	u32 mscr;
	int ret;

	ret = s32_get_pinctrl_offset(priv, offset, &pinctrl_offset);
	if (ret)
		return ret;

	mscr = readl(s32_get_mscr_reg(priv, pinctrl_offset));
	mscr &= ~SIUL2_MSCR_S32_G1_OBE;
	mscr |= SIUL2_MSCR_S32_G1_IBE;
	writel(mscr, s32_get_mscr_reg(priv, pinctrl_offset));

	return 0;
}

static int s32_gpio_set_value(struct udevice *dev, unsigned int offset,
			      int value)
{
	struct s32_gpio *priv = dev_get_priv(dev);
	unsigned int pinctrl_offset;
	int ret;

	ret = s32_get_pinctrl_offset(priv, offset, &pinctrl_offset);
	if (ret)
		return ret;

	writeb(value, s32_get_out_reg(priv, pinctrl_offset));

	return 0;
}

static int s32_gpio_direction_output(struct udevice *dev, unsigned int offset,
				     int value)
{
	struct s32_gpio *priv = dev_get_priv(dev);
	unsigned int pinctrl_offset;
	u32 mscr;
	int ret;

	ret = s32_get_pinctrl_offset(priv, offset, &pinctrl_offset);
	if (ret)
		return ret;

	mscr = readl(s32_get_mscr_reg(priv, pinctrl_offset));
	mscr &= ~SIUL2_MSCR_S32_G1_IBE;
	mscr |= SIUL2_MSCR_S32_G1_OBE;
	writel(mscr, s32_get_mscr_reg(priv, pinctrl_offset));

	return s32_gpio_set_value(dev, offset, value);
}

static int s32_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct s32_gpio *priv = dev_get_priv(dev);
	unsigned int pinctrl_offset;
	u32 mscr;
	int ret;

	ret = s32_get_pinctrl_offset(priv, offset, &pinctrl_offset);
	if (ret)
		return ret;

	mscr = readl(s32_get_mscr_reg(priv, pinctrl_offset));

	/* First check if the pin is muxed as gpio. The input buffer and the
	 * output buffer might be enabled at the same time.
	 */
	if (mscr & SIUL2_MSCR_S32_G1_SSS_MASK)
		return GPIOF_FUNC;

	if (mscr & SIUL2_MSCR_S32_G1_OBE)
		return GPIOF_OUTPUT;

	if (mscr & SIUL2_MSCR_S32_G1_IBE)
		return GPIOF_INPUT;

	return GPIOF_UNUSED;
}

static int s32_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct s32_gpio *priv = dev_get_priv(dev);
	unsigned int pinctrl_offset;
	int ret;

	ret = s32_get_pinctrl_offset(priv, offset, &pinctrl_offset);
	if (ret)
		return ret;

	switch (s32_gpio_get_function(dev, offset)) {
	case GPIOF_OUTPUT:
		return readb(s32_get_out_reg(priv, pinctrl_offset));
	default:
		return readb(s32_get_in_reg(priv, pinctrl_offset));
	}
}

static int s32_gpio_get_xlate(struct udevice *dev, struct gpio_desc *desc,
			      struct ofnode_phandle_args *args)
{
	struct s32_gpio *priv = dev_get_priv(dev);
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

static int s32_gpio_set_open_drain(struct udevice *dev, unsigned int offset,
				   int value)
{
	struct s32_gpio *priv = dev_get_priv(dev);
	unsigned int pinctrl_offset;
	u32 mscr;
	int ret;

	ret = s32_get_pinctrl_offset(priv, offset, &pinctrl_offset);
	if (ret)
		return ret;

	mscr = readl(s32_get_mscr_reg(priv, pinctrl_offset));

	if (value)
		mscr |= SIUL2_MSCR_S32_G1_ODE;
	else
		mscr &= ~SIUL2_MSCR_S32_G1_ODE;

	writel(mscr, s32_get_mscr_reg(priv, pinctrl_offset));

	return 0;
}

static int s32_gpio_get_open_drain(struct udevice *dev, unsigned int offset)
{
	struct s32_gpio *priv = dev_get_priv(dev);
	unsigned int pinctrl_offset;
	u32 mscr;
	int ret;

	ret = s32_get_pinctrl_offset(priv, offset, &pinctrl_offset);
	if (ret)
		return ret;

	mscr = readl(s32_get_mscr_reg(priv, pinctrl_offset));
	mscr &= SIUL2_MSCR_S32_G1_ODE;

	return !!mscr;
}

static const struct dm_gpio_ops s32_gpio_ops = {
	.direction_input = s32_gpio_direction_input,
	.direction_output = s32_gpio_direction_output,
	.get_value = s32_gpio_get_value,
	.set_value = s32_gpio_set_value,
	.get_open_drain = s32_gpio_get_open_drain,
	.set_open_drain = s32_gpio_set_open_drain,
	.get_function = s32_gpio_get_function,
	.xlate = s32_gpio_get_xlate,
};

static int s32_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct s32_gpio *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args;
	fdt_addr_t addr;
	int i = 0;
	int ret;

	addr = devfdt_get_addr(dev->parent);
	if (addr == FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->base_addr = (void *)addr;

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

static const struct udevice_id s32_gpio_ids[] = {
	{ .compatible = "fsl,s32-gen1-siul2-gpio"},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(s32_gpio) = {
	.name = "s32_gpio",
	.id = UCLASS_GPIO,
	.of_match = of_match_ptr(s32_gpio_ids),
	.ops = &s32_gpio_ops,
	.priv_auto_alloc_size = sizeof(struct s32_gpio),
	.probe = s32_gpio_probe,
};
