// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021-2023 NXP
 */

#include <common.h>
#include <dm.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>

#include <linux/compat.h>
#include <linux/list.h>

#include <malloc.h>

#define S32_PAD(pin)	((pin) * 4)

#define SIUL2_MSCR_PUS	BIT(12)
#define SIUL2_MSCR_PUE	BIT(13)
#define SIUL2_MSCR_IBE	BIT(19)
#define SIUL2_MSCR_OBE	BIT(21)
#define SIUL2_MSCR_ODE	BIT(20)

#define SIUL2_MSCR_SSS_MASK	0x7
#define SIUL2_MSCR_SRE_SHIFT	14
#define SIUL2_MSCR_SRE_MASK	GENMASK(16, 14)

#define SIUL2_PIN_FROM_PINMUX(v) ((v) >> 4)
#define SIUL2_FUNC_FROM_PINMUX(v) ((v) & 0XF)

#define SIUL2_IMCR_OFFSET 512

#define UPTR(a) ((uintptr_t)(a))

#define SIUL2_NXP_PINS "nxp,pins"

struct s32_range {
	void __iomem *base_addr;
	u32 begin;
	u32 end;
};

struct s32_pin {
	u32 pin;
	u32 config;
	struct list_head list;
};

struct s32_pinctrl {
	struct s32_range *ranges;
	int num_ranges;
	struct list_head gpio_configs;
};

static const struct pinconf_param siul2_pinconf_params[] = {
	{ "bias-pull-up", PIN_CONFIG_BIAS_PULL_UP, 1 },
	{ "bias-pull-down", PIN_CONFIG_BIAS_PULL_DOWN, 1 },
	{ "bias-disable", PIN_CONFIG_BIAS_DISABLE, 1 },
	{ "input-enable", PIN_CONFIG_INPUT_ENABLE, 1 },
	{ "input-disable", PIN_CONFIG_INPUT_ENABLE, 0 },
	{ "output-enable", PIN_CONFIG_OUTPUT_ENABLE, 1 },
	{ "output-disable", PIN_CONFIG_OUTPUT_ENABLE, 0 },
	{ "slew-rate", PIN_CONFIG_SLEW_RATE, 4 },
	{ "drive-open-drain", PIN_CONFIG_DRIVE_OPEN_DRAIN, 1 },
	{ "drive-push-pull", PIN_CONFIG_DRIVE_PUSH_PULL, 1 },
};

static struct s32_range *s32_get_pin_range(struct s32_pinctrl *ctlr, u32 pin)
{
	int i;
	struct s32_range *range = NULL;
	u32 pin_off;

	for (i = 0; i < ctlr->num_ranges; ++i) {
		range = &ctlr->ranges[i];
		if (pin >= range->begin && pin <= range->end) {
			pin_off = pin - range->begin;

			if (pin_off > __UINT32_MAX__ / 4)
				return NULL;

			if (UPTR(range->base_addr) + S32_PAD(pin_off) >
			    __UINT32_MAX__)
				return NULL;

			return range;
		}
	}

	return NULL;
}

static int s32_get_mscr_setting_from_param(unsigned int param,
					   unsigned int argument,
					   u32 *mscr_value)
{
	enum pin_config_param cfg = (enum pin_config_param)param;

	switch (cfg) {
	case PIN_CONFIG_OUTPUT_ENABLE:
		*mscr_value &= ~SIUL2_MSCR_OBE;
		if (argument)
			*mscr_value |= SIUL2_MSCR_OBE;
		break;
	case PIN_CONFIG_INPUT_ENABLE:
		*mscr_value &= ~SIUL2_MSCR_IBE;
		if (argument)
			*mscr_value |= SIUL2_MSCR_IBE;
		break;
	case PIN_CONFIG_DRIVE_OPEN_DRAIN:
		*mscr_value |= SIUL2_MSCR_ODE;
		break;
	case PIN_CONFIG_DRIVE_PUSH_PULL:
		*mscr_value &= ~SIUL2_MSCR_ODE;
		break;
	case PIN_CONFIG_SLEW_RATE:
		argument = (((uint8_t)argument) << SIUL2_MSCR_SRE_SHIFT) &
			   SIUL2_MSCR_SRE_MASK;
		*mscr_value &= ~SIUL2_MSCR_SRE_MASK;
		*mscr_value |= argument;
		break;
	case PIN_CONFIG_BIAS_PULL_UP:
		*mscr_value |= SIUL2_MSCR_PUE | SIUL2_MSCR_PUS;
		break;
	case PIN_CONFIG_BIAS_PULL_DOWN:
		*mscr_value |= SIUL2_MSCR_PUE;
		*mscr_value &= ~SIUL2_MSCR_PUS;
		break;
	case PIN_CONFIG_BIAS_DISABLE:
		*mscr_value &= ~SIUL2_MSCR_PUE;
		break;
	default:
		return -ENOTSUPP;
	}

	return 0;
}

static int s32_get_pinconf_setting(struct udevice *dev, struct ofprop property,
				   u32 *mscr_value)
{
	const struct pinconf_param *p;
	const char *propname;
	const void *value;
	unsigned int arg;
	int i, len;

	value = dev_read_prop_by_prop(&property, &propname, &len);
	if (!value)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(siul2_pinconf_params); ++i) {
		if (!strcmp(propname, siul2_pinconf_params[i].property)) {
			p = &siul2_pinconf_params[i];

			if (len == sizeof(fdt32_t)) {
				arg = fdt32_to_cpu(*(fdt32_t *)value);
			} else if (len == 0) {
				arg = p->default_value;
			} else {
				dev_err(dev,
					"Wrong argument size: %s\n", propname);
				return -EINVAL;
			}

			return s32_get_mscr_setting_from_param(p->param, arg,
							       mscr_value);
		}
	}

	return 0;
}

static int s32_parse_pinmux_len(struct udevice *dev, struct udevice *config)
{
	int size;

	size = dev_read_size(config, "pinmux");
	if (size < 0)
		return size;
	size /= sizeof(fdt32_t);

	return size;
}

static int s32_set_state_subnode(struct udevice *dev, struct udevice *config)
{
	struct s32_pinctrl *priv = dev_get_priv(dev);
	struct s32_range *range;
	struct ofprop property;
	u32 mscr_value = 0;
	u32 pinmux_value, pin, func;
	int ret, i, len;

	len = s32_parse_pinmux_len(dev, config);
	if (len <= 0) {
		/* Not a pinmux node. Skip parsing this. */
		return 0;
	}

	dev_for_each_property(property, config) {
		ret = s32_get_pinconf_setting(dev, property, &mscr_value);
		if (ret) {
			dev_err(dev,
				"Could not parse property for: %s!\n",
				config->name);
			return ret;
		}
	}

	for (i = 0; i < len; ++i) {
		ret = dev_read_u32_index(config, "pinmux", i, &pinmux_value);
		if (ret) {
			dev_err(dev, "Error reading pinmux index: %d\n", i);
			return ret;
		}

		pin = SIUL2_PIN_FROM_PINMUX(pinmux_value);
		func = SIUL2_FUNC_FROM_PINMUX(pinmux_value);

		range = s32_get_pin_range(priv, pin);
		if (!range) {
			dev_err(dev, "Invalid pin: %d\n", pin);
			return -EINVAL;
		}

		pin -= range->begin;
		writel(mscr_value | func, UPTR(range->base_addr) + S32_PAD(pin));
	}

	return 0;
}

static int s32_set_state(struct udevice *dev, struct udevice *config)
{
	struct udevice *child;
	int ret;

	ret = s32_set_state_subnode(dev, config);
	if (ret) {
		dev_err(dev, "Error %d parsing: %s\n", ret, config->name);
		return ret;
	}

	for (device_find_first_child(config, &child);
	     child;
	     device_find_next_child(&child)) {
		ret = s32_set_state_subnode(dev, child);
		if (ret)
			return ret;
	}

	return 0;
}

static int s32_pinmux_set(struct udevice *dev, unsigned int pin_selector,
			  unsigned int func_selector)
{
	struct s32_pinctrl *priv = dev_get_priv(dev);
	struct s32_range *range;
	u32 mscr_value;

	if (!priv)
		return -EINVAL;

	range = s32_get_pin_range(priv, pin_selector);
	if (!range)
		return -ENOENT;

	pin_selector -= range->begin;
	mscr_value = readl(UPTR(range->base_addr) + S32_PAD(pin_selector));
	mscr_value &= ~SIUL2_MSCR_SSS_MASK;
	mscr_value |= (func_selector & SIUL2_MSCR_SSS_MASK);

	writel(mscr_value, UPTR(range->base_addr) + S32_PAD(pin_selector));

	return 0;
}

static int s32_pinconf_set(struct udevice *dev, unsigned int pin_selector,
			   unsigned int param, unsigned int argument)
{
	struct s32_pinctrl *priv = dev_get_priv(dev);
	struct s32_range *range;
	u32 mscr_value;
	int ret;

	if (!priv)
		return -EINVAL;

	range = s32_get_pin_range(priv, pin_selector);
	if (!range)
		return -ENOENT;

	pin_selector -= range->begin;
	mscr_value = readl(UPTR(range->base_addr) + S32_PAD(pin_selector));

	ret = s32_get_mscr_setting_from_param(param, argument, &mscr_value);
	if (ret)
		return ret;

	writel(mscr_value, UPTR(range->base_addr) + S32_PAD(pin_selector));

	return 0;
}

static int s32_gpio_request_enable(struct udevice *dev, unsigned int pin_selector)
{
	struct s32_pinctrl *priv = dev_get_priv(dev);
	struct s32_range *range;
	u32 mscr_value;
	struct s32_pin *pin;

	if (!priv)
		return -EINVAL;

	range = s32_get_pin_range(priv, pin_selector);
	if (!range)
		return -ENOENT;

	pin = malloc(sizeof(*pin));
	if (!pin)
		return -ENOMEM;

	pin->pin = pin_selector;

	pin_selector -= range->begin;
	mscr_value = readl(UPTR(range->base_addr) + S32_PAD(pin_selector));

	pin->config = mscr_value;

	mscr_value &= ~SIUL2_MSCR_SSS_MASK;

	writel(mscr_value, UPTR(range->base_addr) + S32_PAD(pin_selector));

	list_add(&pin->list, &priv->gpio_configs);

	return 0;
}

static int s32_gpio_disable_free(struct udevice *dev, unsigned int pin_selector)
{
	struct s32_pinctrl *priv = dev_get_priv(dev);
	struct s32_range *range;
	struct list_head *pos, *temp;

	if (!priv)
		return -EINVAL;

	range = s32_get_pin_range(priv, pin_selector);
	if (!range)
		return -ENOENT;

	list_for_each_safe(pos, temp, &priv->gpio_configs) {
		struct s32_pin *pin = list_entry(pos, struct s32_pin, list);

		if (pin->pin == pin_selector) {
			pin_selector -= range->begin;

			writel(pin->config,
			       UPTR(range->base_addr) + S32_PAD(pin_selector));

			list_del(pos);
			kfree(pin);
			break;
		}
	}

	return 0;
}

static int s32_pinmux_get(struct udevice *dev, unsigned int pin,
			  unsigned int *raw_function,
			  unsigned int *gpio_function)
{
	struct s32_pinctrl *priv = dev_get_priv(dev);
	struct s32_range *range;
	u32 mscr_value;
	u32 sss_value;

	*raw_function = 0;
	*gpio_function = GPIOF_UNKNOWN;

	if (!priv)
		return -EINVAL;

	range = s32_get_pin_range(priv, pin);
	if (!range)
		return -ENOENT;

	pin -= range->begin;
	mscr_value = readl(UPTR(range->base_addr) + S32_PAD(pin));
	sss_value = mscr_value & SIUL2_MSCR_SSS_MASK;
	*raw_function = sss_value;

	if (sss_value != 0) {
		*gpio_function = GPIOF_FUNC;
		return 0;
	}

	if (mscr_value & SIUL2_MSCR_OBE)
		*gpio_function = GPIOF_OUTPUT;
	else if (mscr_value & SIUL2_MSCR_IBE)
		*gpio_function = GPIOF_INPUT;

	return 0;
}

static int s32_get_gpio_mux(struct udevice *dev, __maybe_unused int banknum,
			    int index)
{
	unsigned int function, gpio_function;
	int ret;

	if (index < 0)
		return -EINVAL;

	ret = s32_pinmux_get(dev, index, &function, &gpio_function);
	if (ret)
		return ret;

	return gpio_function;
}

static int s32_pinctrl_get_pins_count(struct udevice *dev)
{
	struct s32_pinctrl *priv = dev_get_priv(dev);

	return priv->ranges[priv->num_ranges - 1].end + 1;
}

static const char *s32_pinctrl_get_pin_name(struct udevice *dev,
					    unsigned int pin)
{
	struct s32_pinctrl *priv = dev_get_priv(dev);
	static char pin_name[PINNAME_SIZE];
	struct s32_range *range;
	int ret;

	memset(pin_name, 0, PINNAME_SIZE);

	range = s32_get_pin_range(priv, pin);
	if (!range)
		return ERR_PTR(-ENODEV);

	if (pin < SIUL2_IMCR_OFFSET)
		ret = snprintf(pin_name, sizeof(pin_name), "mscr%d", pin);
	else
		ret = snprintf(pin_name, sizeof(pin_name), "imcr%d",
			       pin - SIUL2_IMCR_OFFSET);

	if (ret >= sizeof(pin_name))
		return ERR_PTR(-EINVAL);

	return pin_name;
}

static int s32_pinctrl_get_pin_muxing(struct udevice *dev, unsigned int pin,
				      char *buf, int size)
{
	unsigned int function, gpio_function;
	int ret;

	if (size <= 0)
		return -EINVAL;

	ret = s32_pinmux_get(dev, pin, &function, &gpio_function);
	if (ret < 0)
		return ret;

	ret = snprintf(buf, size, "function %hu", function);
	if (ret >= size)
		return -EINVAL;

	return 0;
}

static const struct pinctrl_ops s32_pinctrl_ops = {
	.set_state		= s32_set_state,
	.gpio_request_enable	= s32_gpio_request_enable,
	.gpio_disable_free	= s32_gpio_disable_free,
	.pinmux_set		= s32_pinmux_set,
	.pinconf_set		= s32_pinconf_set,
	.get_gpio_mux		= s32_get_gpio_mux,
	.pinconf_num_params	= ARRAY_SIZE(siul2_pinconf_params),
	.pinconf_params		= siul2_pinconf_params,
	.get_pins_count		= s32_pinctrl_get_pins_count,
	.get_pin_name		= s32_pinctrl_get_pin_name,
	.get_pin_muxing		= s32_pinctrl_get_pin_muxing,
};

static int s32_pinctrl_probe(struct udevice *dev)
{
	struct s32_pinctrl *priv = dev_get_priv(dev);
	int ret, i, num_ranges;
	const void *pins_prop;
	fdt_addr_t addr;
	u32 begin, end;

	pins_prop = dev_read_prop(dev, SIUL2_NXP_PINS, &num_ranges);
	if (!pins_prop || !num_ranges || (num_ranges % sizeof(u32) != 0)) {
		dev_err(dev, "Error retrieving %s!\n", SIUL2_NXP_PINS);
		return -EINVAL;
	}

	/* For each range we have a start and an end. */
	num_ranges /= (sizeof(u32) * 2);
	priv->num_ranges = num_ranges;

	priv->ranges = malloc(num_ranges * sizeof(*priv->ranges));
	if (!priv->ranges)
		return -ENOMEM;

	for (i = 0; i < num_ranges; ++i) {
		addr = dev_read_addr_index(dev, i);
		if (addr == FDT_ADDR_T_NONE) {
			dev_err(dev, "Error retrieving reg: %d\n", i);
			return -EINVAL;
		}

		priv->ranges[i].base_addr = (__iomem void *)addr;

		ret = dev_read_u32_index(dev, SIUL2_NXP_PINS, i * 2, &begin);
		if (ret) {
			dev_err(dev, "Error reading %s start: %d\n",
				SIUL2_NXP_PINS,
				ret);
			return ret;
		}

		ret = dev_read_u32_index(dev, SIUL2_NXP_PINS, i * 2 + 1, &end);
		if (ret) {
			dev_err(dev, "Error reading %s no. GPIOs: %d\n",
				SIUL2_NXP_PINS,
				ret);
			return ret;
		}

		priv->ranges[i].begin = begin;
		priv->ranges[i].end = end;
	}

	INIT_LIST_HEAD(&priv->gpio_configs);

	return 0;
}

static const struct udevice_id s32_pinctrl_ids[] = {
	{ .compatible = "nxp,s32g-siul2-pinctrl" },
	{ .compatible = "nxp,s32r45-siul2-pinctrl" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(s32_pinctrl) = {
	.name = "s32_pinctrl",
	.id = UCLASS_PINCTRL,
	.of_match = of_match_ptr(s32_pinctrl_ids),
	.probe = s32_pinctrl_probe,
	.priv_auto = sizeof(struct s32_pinctrl),
	.ops = &s32_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
