// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021-2022,2024 NXP
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <dt-bindings/gpio/gpio.h>

#define MSCR_OFF	0x0240
#define GPDO_BASE	0x1300
#define GPDI_BASE	0x1500

#define SIUL2_NUM		2
#define SIUL2_PAD_NAME_LEN	7
#define SIUL2_GPIO_16_PAD_SIZE	16

#define SIUL2_PGPDO(N)		(((N) ^ 1) * 2)

struct gpio_range {
	u16 start_gpio;
	u16 cnt;
};

struct siul2_info {
	void __iomem *opads;
	void __iomem *ipads;
	struct gpio_range range;
};

struct s32cc_gpio {
	struct siul2_info siul2[SIUL2_NUM];
	struct udevice *pinctrl;
};

static u16 siul2_pin2mask(int pin)
{
	/**
	 * From Reference manual :
	 * PGPDOx[PPDOy] = GPDO(x × 16) + (15 - y)[PDO_(x × 16) + (15 - y)]
	 */
	return BIT(15 - pin % SIUL2_GPIO_16_PAD_SIZE);
}

static unsigned int siul2_pin2pad(int pin)
{
	return pin / SIUL2_GPIO_16_PAD_SIZE;
}

static inline u32 siul2_get_pad_offset(unsigned int pad)
{
	return SIUL2_PGPDO(pad);
}

static void __iomem *siul2_get_pad_base_addr(struct udevice *dev,
					     unsigned int gpio,
					     bool input)
{
	struct s32cc_gpio *priv;
	int i;
	u32 start_gpio, cnt;

	priv = dev_get_priv(dev);

	for (i = 0; i < ARRAY_SIZE(priv->siul2); ++i) {
		start_gpio = priv->siul2[i].range.start_gpio;
		cnt = priv->siul2[i].range.cnt;

		if (gpio < start_gpio || gpio - start_gpio >= cnt)
			continue;

		return input ? priv->siul2[i].ipads : priv->siul2[i].opads;
	}

	return NULL;
}

static int s32cc_gpio_get_function(struct udevice *dev, unsigned int gpio)
{
	struct s32cc_gpio *priv = dev_get_priv(dev);

	return pinctrl_get_gpio_mux(priv->pinctrl, 0, gpio);
}

static int s32cc_gpio_set_value_raw(struct udevice *dev, unsigned int gpio,
				    int value)
{
	int reg_offset;
	void __iomem *addr;
	u32 pad;
	u16 mask, val;

	if (value != 0 && value != 1)
		return -EINVAL;

	mask = siul2_pin2mask(gpio);
	pad = siul2_pin2pad(gpio);
	reg_offset = siul2_get_pad_offset(pad);

	addr = siul2_get_pad_base_addr(dev, gpio, false);
	if (!addr)
		return -EINVAL;

	val = readw(((uintptr_t)addr) + reg_offset);
	if (value)
		val |= mask;
	else
		val &= ~mask;
	writew(val, ((uintptr_t)addr) + reg_offset);

	return 0;
}

static int s32cc_gpio_set_value(struct udevice *dev, unsigned int gpio,
				int value)
{
	if (s32cc_gpio_get_function(dev, gpio) != GPIOF_OUTPUT)
		return -EINVAL;

	return s32cc_gpio_set_value_raw(dev, gpio, value);
}

static int s32cc_gpio_get_value(struct udevice *dev, unsigned int gpio)
{
	int reg_offset;
	void __iomem *addr;
	u32 pad;
	u16 mask;
	bool input = true;

	mask = siul2_pin2mask(gpio);
	pad = siul2_pin2pad(gpio);
	reg_offset = siul2_get_pad_offset(pad);

	if (s32cc_gpio_get_function(dev, gpio) == GPIOF_OUTPUT)
		input = false;

	addr = siul2_get_pad_base_addr(dev, gpio, input);
	if (!addr)
		return -EINVAL;

	return !!(readw(((uintptr_t)addr) + reg_offset) & mask);
}

static int s32cc_gpio_direction_input(struct udevice *dev, unsigned int gpio)
{
	int ret;

	ret = pinctrl_gpio_request(dev, gpio);
	if (ret)
		return ret;

	ret = pinctrl_gpio_set_config(dev, gpio, PIN_CONFIG_INPUT_ENABLE, 1);
	if (ret)
		return ret;

	return pinctrl_gpio_set_config(dev, gpio, PIN_CONFIG_OUTPUT_ENABLE, 0);
}

static int s32cc_gpio_direction_output(struct udevice *dev, unsigned int gpio,
				       int value)
{
	int ret = pinctrl_gpio_request(dev, gpio);

	if (ret)
		return ret;

	/* We preconfigure the value to avoid a glitch on the output */
	ret = s32cc_gpio_set_value_raw(dev, gpio, value);
	if (ret)
		return ret;

	ret = pinctrl_gpio_set_config(dev, gpio, PIN_CONFIG_OUTPUT_ENABLE, 1);
	if (ret)
		return ret;

	return pinctrl_gpio_set_config(dev, gpio, PIN_CONFIG_INPUT_ENABLE, 1);
}

static int s32cc_gpio_get_xlate(struct udevice *dev, struct gpio_desc *desc,
				struct ofnode_phandle_args *args)
{
	if (args->args_count < 1)
		return -EINVAL;

	if (!siul2_get_pad_base_addr(dev, args->args[0], false))
		return -EINVAL;

	desc->offset = args->args[0];

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
	int value, ret;

	if (flags & GPIOD_IS_OUT) {
		if (flags & GPIOD_OPEN_DRAIN) {
			ret =
			  pinctrl_gpio_set_config(dev, offset,
						  PIN_CONFIG_DRIVE_OPEN_DRAIN,
						  1);
			if (ret)
				return ret;
		}

		value = !!(flags & GPIOD_IS_OUT_ACTIVE);
		return s32cc_gpio_direction_output(dev, offset, value);
	} else if (flags & GPIOD_IS_IN) {
		return s32cc_gpio_direction_input(dev, offset);
	}

	return 0;
}


static const struct dm_gpio_ops s32cc_gpio_ops = {
	.get_value = s32cc_gpio_get_value,
	.set_value = s32cc_gpio_set_value,
	.get_function = s32cc_gpio_get_function,
	.xlate = s32cc_gpio_get_xlate,
	.set_flags = s32cc_gpio_set_flags,
};

static int s32cc_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct s32cc_gpio *priv = dev_get_priv(dev);
	struct ofnode_phandle_args args = {.args = {0}};
	fdt_addr_t addr;
	int i = 0;
	int ret;

	uc_priv->bank_name = dev->name;
	uc_priv->gpio_count = 0;

	for (i = 0; i < ARRAY_SIZE(priv->siul2); ++i) {
		char temp[SIUL2_PAD_NAME_LEN];

		snprintf(temp, ARRAY_SIZE(temp), "opads%d", i);
		addr = dev_read_addr_name(dev, temp);
		if (addr == FDT_ADDR_T_NONE) {
			dev_err(dev, "Error retrieving reg: %s\n", temp);
			return -EINVAL;
		}
		priv->siul2[i].opads = (__iomem void *)addr;

		snprintf(temp, ARRAY_SIZE(temp), "ipads%d", i);
		addr = dev_read_addr_name(dev, temp);
		if (addr == FDT_ADDR_T_NONE) {
			dev_err(dev, "Error retrieving reg: %s\n", temp);
			return -EINVAL;
		}
		priv->siul2[i].ipads = (__iomem void *)addr;

		ret = dev_read_phandle_with_args(dev, "gpio-ranges", NULL, 3, i,
						 &args);
		if (ret < 0) {
			dev_err(dev,
				"gpio-ranges: property missing or invalid\n");
			return ret;
		}

		priv->siul2[i].range.start_gpio = args.args[0];
		priv->siul2[i].range.cnt = args.args[2];

		if (uc_priv->gpio_count < args.args[0] + args.args[2])
			uc_priv->gpio_count = args.args[0] + args.args[2];
	}

	return uclass_first_device_err(UCLASS_PINCTRL, &priv->pinctrl);
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
