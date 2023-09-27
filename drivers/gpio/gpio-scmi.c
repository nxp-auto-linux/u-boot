// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022-2023 NXP
 */
#include <common.h>
#include <dm.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>
#include <asm/gpio.h>
#include <dm/devres.h>
#include <dm/pinctrl.h>
#include <dm/uclass-internal.h>
#include <linux/bitmap.h>
#include <dt-bindings/gpio/gpio.h>

#define SCMI_GPIO_PINS_PER_SLOT		(sizeof(u32) * BITS_PER_BYTE)
#define SCMI_NUM_GPIO_MASK		(0xFFu)

enum scmi_gpio_message_id {
	SCMI_GPIO_PROTOCOL_ATTRIBUTES = 0x1,
	SCMI_GPIO_DESCRIBE = 0x3,
	SCMI_GPIO_REQUEST = 0x4,
	SCMI_GPIO_FREE = 0x5,
	SCMI_GPIO_SET_VALUE = 0x6,
	SCMI_GPIO_GET_VALUE = 0x7,
};

struct pinctrl_ranges {
	struct udevice *pinctrl;
	unsigned int gpio_off;
	unsigned int pin_off;
	unsigned int npins;
};

struct scmi_gpio {
	struct pinctrl_ranges *pinctrl;
	unsigned int nranges;
	unsigned long *availability;
	unsigned long *requested;
};

struct gpio_describe_reply {
	s32 status;
	u32 base_id;
	u32 availab[0];
};

static bool is_pin_available(struct scmi_gpio *gpio_dev, unsigned int offset)
{
	if (!gpio_dev)
		return false;

	if (offset > INT_MAX)
		return false;

	return test_bit(offset, gpio_dev->availability);
}

static bool is_pin_requested(struct scmi_gpio *gpio_dev, unsigned int offset)
{
	if (!gpio_dev)
		return false;

	if (offset > INT_MAX)
		return false;

	return test_bit(offset, gpio_dev->requested);
}

static int get_num_gpios(struct udevice *scmi_dev, unsigned int *num_gpios)
{
	struct {
		s32 status;
		u32 attributes;
	} response;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_GPIO,
		.message_id	= SCMI_GPIO_PROTOCOL_ATTRIBUTES,
		.in_msg		= NULL,
		.in_msg_sz	= 0,
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response)
	};
	int ret;

	ret = devm_scmi_process_msg(scmi_dev, &msg);
	if (ret)
		return ret;

	*num_gpios = response.attributes & SCMI_NUM_GPIO_MASK;

	return scmi_to_linux_errno(response.status);
}

static int init_gpio_base_available(struct udevice *dev, unsigned int ngpios,
				    unsigned int *gpio_base)
{
	struct scmi_gpio *gpio_dev = dev_get_priv(dev);
	struct gpio_describe_reply *response = NULL;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_GPIO,
		.message_id	= SCMI_GPIO_DESCRIBE,
		.in_msg		= NULL,
		.in_msg_sz	= 0,
	};
	int ret;
	size_t i, j, pos, out_size;
	u32 slot;

	if (!gpio_dev)
		return -EINVAL;

	out_size = sizeof(struct gpio_describe_reply);
	out_size += (ngpios / (sizeof(u32) * BITS_PER_BYTE) + 1) * sizeof(u32);
	response = kzalloc(out_size, GFP_KERNEL);
	if (!response)
		return -ENOMEM;

	msg.out_msg = (u8 *)response;
	msg.out_msg_sz = out_size;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		goto free_response;

	*gpio_base = response->base_id;

	ret = scmi_to_linux_errno(response->status);
	if (ret)
		goto free_response;

	for (i = 0; i < out_size / sizeof(response->availab[0]); i++) {
		slot = response->availab[i];
		for (j = 0; j < SCMI_GPIO_PINS_PER_SLOT; j++) {
			if (slot & BIT(j)) {
				pos = i * SCMI_GPIO_PINS_PER_SLOT + j;
				bitmap_set(gpio_dev->availability, pos, 1);
			}
		}
	}

free_response:
	kfree(response);

	return ret;
}

static int scmi_gpio_request(struct udevice *dev, unsigned int offset,
			     const char *label)
{
	struct scmi_gpio *gpio_dev = dev_get_priv(dev);
	struct {
		s32 status;
	} response;
	struct {
		u32 pin;
	} request;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_GPIO,
		.message_id	= SCMI_GPIO_REQUEST,
		.in_msg		= (u8 *)&request,
		.in_msg_sz	= sizeof(request),
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response)
	};
	int ret;

	if (!gpio_dev)
		return -EINVAL;

	if (!is_pin_available(gpio_dev, offset))
		return -EINVAL;

	request.pin = offset;

	ret = pinctrl_gpio_request(dev, offset);
	if (ret)
		return ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		goto release_pinctrl;

	ret = scmi_to_linux_errno(response.status);

release_pinctrl:
	if (ret)
		(void)pinctrl_gpio_free(dev, offset);
	else
		bitmap_set(gpio_dev->requested, offset, 1);

	return ret;
}

static int scmi_gpio_free(struct udevice *dev, unsigned int offset)
{
	struct scmi_gpio *gpio_dev = dev_get_priv(dev);
	struct {
		s32 status;
	} response;
	struct {
		u32 pin;
	} request;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_GPIO,
		.message_id	= SCMI_GPIO_FREE,
		.in_msg		= (u8 *)&request,
		.in_msg_sz	= sizeof(request),
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response)
	};
	int ret;

	if (!gpio_dev)
		return -EINVAL;

	if (!is_pin_requested(gpio_dev, offset))
		return -EINVAL;

	request.pin = offset;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	ret = scmi_to_linux_errno(response.status);
	if (ret)
		return ret;

	ret = pinctrl_gpio_free(dev, offset);
	if (ret)
		return ret;

	bitmap_clear(gpio_dev->requested, offset, 1);

	return ret;
}

static int scmi_gpio_get_value(struct udevice *dev, unsigned int offset)
{
	struct scmi_gpio *gpio_dev = dev_get_priv(dev);
	struct {
		s32 status;
		u32 value;
	} response;
	struct {
		u32 gpio;
	} request;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_GPIO,
		.message_id	= SCMI_GPIO_GET_VALUE,
		.in_msg		= (u8 *)&request,
		.in_msg_sz	= sizeof(request),
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response)
	};
	int ret, err;
	bool requested_pin = is_pin_requested(gpio_dev, offset);

	if (!gpio_dev)
		return -EINVAL;

	if (!is_pin_available(gpio_dev, offset))
		return -EINVAL;

	if (!requested_pin) {
		/**
		 * 'gpio status -a' checks the value on all pins regardless of
		 * their request status
		 */
		ret = scmi_gpio_request(dev, offset, NULL);
		if (ret)
			return ret;
	}

	request.gpio = offset;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		goto release_pin;

	ret = scmi_to_linux_errno(response.status);
	if (ret)
		goto release_pin;

release_pin:
	if (!requested_pin) {
		err = scmi_gpio_free(dev, offset);
		if (err)
			return err;
	}

	if (ret)
		return ret;

	return response.value ? 1 : 0;
}

static int scmi_gpio_set_value(struct udevice *dev, unsigned int offset,
			       int value)
{
	struct scmi_gpio *gpio_dev = dev_get_priv(dev);
	struct {
		s32 status;
	} response;
	struct {
		u32 gpio;
		u32 value;
	} request;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_GPIO,
		.message_id	= SCMI_GPIO_SET_VALUE,
		.in_msg		= (u8 *)&request,
		.in_msg_sz	= sizeof(request),
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response)
	};
	int ret;

	if (!gpio_dev)
		return -EINVAL;

	if (!is_pin_requested(gpio_dev, offset))
		return -EINVAL;

	request.gpio = offset;
	request.value = value ? 1u : 0u;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	return scmi_to_linux_errno(response.status);
}

static int scmi_gpio_direction_input(struct udevice *dev, unsigned int offset)
{
	struct scmi_gpio *gpio_dev = dev_get_priv(dev);
	int ret;

	if (!gpio_dev)
		return -EINVAL;

	if (!is_pin_requested(gpio_dev, offset))
		return -EINVAL;

	ret = pinctrl_gpio_set_config(dev, offset, PIN_CONFIG_INPUT_ENABLE, 1);
	if (ret)
		return ret;

	return pinctrl_gpio_set_config(dev, offset, PIN_CONFIG_OUTPUT_ENABLE, 0);
}

static int scmi_gpio_direction_output(struct udevice *dev, unsigned int offset,
				      int value)
{
	struct scmi_gpio *gpio_dev = dev_get_priv(dev);
	int ret;

	if (!gpio_dev)
		return -EINVAL;

	if (!is_pin_requested(gpio_dev, offset))
		return -EINVAL;

	ret = scmi_gpio_set_value(dev, offset, value);
	if (ret)
		return ret;

	ret = pinctrl_gpio_set_config(dev, offset, PIN_CONFIG_OUTPUT_ENABLE, 1);
	if (ret)
		return ret;

	ret = pinctrl_gpio_set_config(dev, offset, PIN_CONFIG_INPUT_ENABLE, 1);
	if (ret)
		return ret;

	return ret;
}

static struct udevice *get_pinctrl(struct scmi_gpio *gpio, unsigned int offset)
{
	unsigned int i;
	struct pinctrl_ranges *range;

	if (!gpio)
		return NULL;

	for (i = 0; i < gpio->nranges; i++) {
		range = &gpio->pinctrl[i];

		if (offset >= range->gpio_off &&
		    offset < range->gpio_off + range->npins)
			return range->pinctrl;
	}

	return NULL;
}

static int scmi_gpio_get_function(struct udevice *dev, unsigned int offset)
{
	struct scmi_gpio *gpio_dev = dev_get_priv(dev);
	struct udevice *pinctrl_dev = get_pinctrl(gpio_dev, offset);

	if (!gpio_dev)
		return -EINVAL;

	if (!is_pin_available(gpio_dev, offset))
		return GPIOF_UNKNOWN;

	if (!pinctrl_dev)
		return -EINVAL;

	return pinctrl_get_gpio_mux(pinctrl_dev, 0, offset);
}

static int scmi_gpio_get_xlate(struct udevice *dev, struct gpio_desc *desc,
			       struct ofnode_phandle_args *args)
{
	struct scmi_gpio *gpio_dev = dev_get_priv(dev);

	if (!gpio_dev)
		return -EINVAL;

	if (args->args_count < 1)
		return -EINVAL;

	if (!is_pin_available(gpio_dev, args->args[0]))
		return -EINVAL;

	desc->offset = args->args[0];

	if (args->args_count < 2)
		return 0;

	if (args->args[1] & GPIO_ACTIVE_LOW)
		desc->flags = GPIOD_ACTIVE_LOW;

	return 0;
}

static int scmi_gpio_probe(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);
	struct ofnode_phandle_args args = { .args = {0} };
	struct scmi_gpio *gpio_dev = dev_get_priv(dev);
	int ret = 0, i, prop_size;
	size_t avail_size;

	if (!gpio_dev || !uc_priv)
		return -EINVAL;

	prop_size = dev_read_size(dev, "gpio-ranges");
	if (prop_size < 0)
		return prop_size;

	/* 'gpio-ranges' has 4 cells per range */
	gpio_dev->nranges = prop_size / sizeof(u32) / 4;

	gpio_dev->pinctrl = devm_kzalloc(dev,
					 gpio_dev->nranges * sizeof(*gpio_dev->pinctrl),
					 GFP_KERNEL);
	if (!gpio_dev->pinctrl)
		return -ENOMEM;

	for (i = 0; i < gpio_dev->nranges; i++) {
		ret = dev_read_phandle_with_args(dev, "gpio-ranges", NULL, 3, i,
						 &args);
		if (ret < 0) {
			pr_err("gpio-ranges: property missing or invalid\n");
			return ret;
		}

		ret = uclass_find_device_by_ofnode(UCLASS_PINCTRL, args.node,
						   &gpio_dev->pinctrl[i].pinctrl);
		if (ret)
			return ret;

		gpio_dev->pinctrl[i].gpio_off = args.args[0];
		gpio_dev->pinctrl[i].pin_off = args.args[1];
		gpio_dev->pinctrl[i].npins = args.args[2];
	}

	uc_priv->bank_name = dev->name;
	ret = get_num_gpios(dev, &uc_priv->gpio_count);
	if (ret)
		return ret;

	avail_size = uc_priv->gpio_count / sizeof(*gpio_dev->availability) + 1;

	gpio_dev->availability = devm_kzalloc(dev, avail_size, GFP_KERNEL);
	if (!gpio_dev->availability)
		return -ENOMEM;

	gpio_dev->requested = devm_kzalloc(dev, avail_size, GFP_KERNEL);
	if (!gpio_dev->requested)
		return -ENOMEM;

	ret = init_gpio_base_available(dev, uc_priv->gpio_count,
				       &uc_priv->gpio_base);
	if (ret)
		return ret;

	return 0;
}

static const struct dm_gpio_ops scmi_gpio_ops = {
	.request = scmi_gpio_request,
	.rfree = scmi_gpio_free,
	.direction_input = scmi_gpio_direction_input,
	.direction_output = scmi_gpio_direction_output,
	.get_value = scmi_gpio_get_value,
	.set_value = scmi_gpio_set_value,
	.get_function = scmi_gpio_get_function,
	.xlate = scmi_gpio_get_xlate,
};

U_BOOT_DRIVER(scmi_gpio) = {
	.name = "scmi_gpio",
	.id = UCLASS_GPIO,
	.ops = &scmi_gpio_ops,
	.probe = scmi_gpio_probe,
	.priv_auto = sizeof(struct scmi_gpio),
};
