// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2023 NXP
 */

#include <common.h>
#include <dm.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>
#include <asm/gpio.h>
#include <dm/pinctrl.h>

#include <malloc.h>
#include <linux/bitmap.h>
#include <linux/err.h>
#include <linux/list.h>

#include <sort.h>

#define SCMI_PINCTRL_NUM_RANGES_MASK 0xFFFF

#define SCMI_PINCTRL_PIN_FROM_PINMUX(v) ((v) >> 4)
#define SCMI_PINCTRL_FUNC_FROM_PINMUX(v) ((v) & 0XF)

/* 128 (channel size) - 28 (SMT header). */
#define SCMI_MAX_BUFFER_SIZE 100

/*
 * Remaining message size is 100. The longer messages that use a variable
 * number of pins are PINMUX_SET and PICONF_SET_*.
 *
 * PINMUX_SET would allow for maximum (100 - 4) / 4 = 24
 * PINCONF_SET_* would allow for maximum (100 - 4 - 4 - 4 - 8 * 4) / 2 = 28
 *
 * Because of ALB-10137 we cannot use the maximum size. Switch to 23
 * until it's fixed.
 *
 */
#define SCMI_MAX_PINS 23

#define PACK_CFG(p, a)		(((p) & 0xFF) | ((a) << 8))
#define UNPACK_PARAM(packed)	((packed) & 0xFF)
#define UNPACK_ARG(packed)	((packed) >> 8)

enum scmi_pinctrl_msg_id {
	SCMI_PINCTRL_PROTOCOL_ATTRIBUTES = 0x1,
	SCMI_PINCTRL_DESCRIBE = 0x3,
	SCMI_PINCTRL_PINMUX_GET = 0x4,
	SCMI_PINCTRL_PINMUX_SET = 0x5,
	SCMI_PINCTRL_PINCONF_GET = 0x6,
	SCMI_PINCTRL_PINCONF_SET_OVR = 0x7,
	SCMI_PINCTRL_PINCONF_SET_APP = 0x8,
};

struct scmi_pinctrl_range {
	u32 begin;
	u32 no_pins;
};

struct scmi_pinctrl_priv {
	struct scmi_pinctrl_range *ranges;
	struct list_head gpio_configs;
	unsigned int no_ranges;
};

struct scmi_pinctrl_pin_cfg {
	u8 no_configs;
	u8 allocated;
	u32 *configs;
};

struct scmi_pinctrl_saved_pin {
	u16 pin;
	u16 func;
	struct scmi_pinctrl_pin_cfg cfg;
	struct list_head list;
};

struct scmi_pinctrl_pinconf_resp {
	s32 status;
	u32 mask;
	u32 boolean_values;
	u32 multi_bit_values[];
};

static const struct pinconf_param scmi_pinctrl_pinconf_params[] = {
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

enum converted_pin_param {
	CONV_PIN_CONFIG_BIAS_BUS_HOLD = 0,
	CONV_PIN_CONFIG_BIAS_DISABLE,
	CONV_PIN_CONFIG_BIAS_HIGH_IMPEDANCE,
	CONV_PIN_CONFIG_BIAS_PULL_DOWN,
	CONV_PIN_CONFIG_BIAS_PULL_PIN_DEFAULT,
	CONV_PIN_CONFIG_BIAS_PULL_UP,
	CONV_PIN_CONFIG_DRIVE_OPEN_DRAIN,
	CONV_PIN_CONFIG_DRIVE_OPEN_SOURCE,
	CONV_PIN_CONFIG_DRIVE_PUSH_PULL,
	CONV_PIN_CONFIG_DRIVE_STRENGTH,
	CONV_PIN_CONFIG_DRIVE_STRENGTH_UA,
	CONV_PIN_CONFIG_INPUT_DEBOUNCE,
	CONV_PIN_CONFIG_INPUT_ENABLE,
	CONV_PIN_CONFIG_INPUT_SCHMITT,
	CONV_PIN_CONFIG_INPUT_SCHMITT_ENABLE,
	CONV_PIN_CONFIG_MODE_LOW_POWER,
	CONV_PIN_CONFIG_MODE_PWM,
	CONV_PIN_CONFIG_OUTPUT,
	CONV_PIN_CONFIG_OUTPUT_ENABLE,
	CONV_PIN_CONFIG_PERSIST_STATE,
	CONV_PIN_CONFIG_POWER_SOURCE,
	CONV_PIN_CONFIG_SKEW_DELAY,
	CONV_PIN_CONFIG_SLEEP_HARDWARE_STATE,
	CONV_PIN_CONFIG_SLEW_RATE,

	CONV_PIN_CONFIG_NUM_CONFIGS,

	CONV_PIN_CONFIG_ERROR,
};

static const u32 scmi_pinctrl_multi_bit_cfgs =
	BIT_32(CONV_PIN_CONFIG_SLEW_RATE) |
	BIT_32(CONV_PIN_CONFIG_SKEW_DELAY) |
	BIT_32(CONV_PIN_CONFIG_POWER_SOURCE) |
	BIT_32(CONV_PIN_CONFIG_MODE_LOW_POWER) |
	BIT_32(CONV_PIN_CONFIG_INPUT_SCHMITT) |
	BIT_32(CONV_PIN_CONFIG_INPUT_DEBOUNCE) |
	BIT_32(CONV_PIN_CONFIG_DRIVE_STRENGTH_UA) |
	BIT_32(CONV_PIN_CONFIG_DRIVE_STRENGTH);

static const enum converted_pin_param scmi_pinctrl_convert[] = {
	[PIN_CONFIG_BIAS_BUS_HOLD] = CONV_PIN_CONFIG_BIAS_BUS_HOLD,
	[PIN_CONFIG_BIAS_DISABLE] = CONV_PIN_CONFIG_BIAS_DISABLE,
	[PIN_CONFIG_BIAS_HIGH_IMPEDANCE] = CONV_PIN_CONFIG_BIAS_HIGH_IMPEDANCE,
	[PIN_CONFIG_BIAS_PULL_DOWN] = CONV_PIN_CONFIG_BIAS_PULL_DOWN,
	[PIN_CONFIG_BIAS_PULL_PIN_DEFAULT] =
		CONV_PIN_CONFIG_BIAS_PULL_PIN_DEFAULT,
	[PIN_CONFIG_BIAS_PULL_UP] = CONV_PIN_CONFIG_BIAS_PULL_UP,
	[PIN_CONFIG_DRIVE_OPEN_DRAIN] = CONV_PIN_CONFIG_DRIVE_OPEN_DRAIN,
	[PIN_CONFIG_DRIVE_OPEN_SOURCE] = CONV_PIN_CONFIG_DRIVE_OPEN_SOURCE,
	[PIN_CONFIG_DRIVE_PUSH_PULL] = CONV_PIN_CONFIG_DRIVE_PUSH_PULL,
	[PIN_CONFIG_DRIVE_STRENGTH] = CONV_PIN_CONFIG_DRIVE_STRENGTH,
	[PIN_CONFIG_DRIVE_STRENGTH_UA] = CONV_PIN_CONFIG_DRIVE_STRENGTH_UA,
	[PIN_CONFIG_INPUT_DEBOUNCE] = CONV_PIN_CONFIG_INPUT_DEBOUNCE,
	[PIN_CONFIG_INPUT_ENABLE] = CONV_PIN_CONFIG_INPUT_ENABLE,
	[PIN_CONFIG_INPUT_SCHMITT] = CONV_PIN_CONFIG_INPUT_SCHMITT,
	[PIN_CONFIG_INPUT_SCHMITT_ENABLE] =
		CONV_PIN_CONFIG_INPUT_SCHMITT_ENABLE,
	[PIN_CONFIG_LOW_POWER_MODE] = CONV_PIN_CONFIG_MODE_LOW_POWER,
	[PIN_CONFIG_OUTPUT_ENABLE] = CONV_PIN_CONFIG_OUTPUT_ENABLE,
	[PIN_CONFIG_OUTPUT] = CONV_PIN_CONFIG_OUTPUT,
	[PIN_CONFIG_POWER_SOURCE] = CONV_PIN_CONFIG_POWER_SOURCE,
	[PIN_CONFIG_SLEEP_HARDWARE_STATE] =
		CONV_PIN_CONFIG_SLEEP_HARDWARE_STATE,
	[PIN_CONFIG_SLEW_RATE] = CONV_PIN_CONFIG_SLEW_RATE,
	[PIN_CONFIG_SKEW_DELAY] = CONV_PIN_CONFIG_SKEW_DELAY,
};

/* This function allocates memory inside cfg->configs.
 * It should be freed by the caller.
 */
static int scmi_pinctrl_add_config(u32 config, struct scmi_pinctrl_pin_cfg *cfg)
{
	void *temp;

	if (cfg->no_configs > CONV_PIN_CONFIG_NUM_CONFIGS)
		return -EINVAL;

	if (cfg->allocated <= cfg->no_configs) {
		/* Reserve extra space. */
		cfg->allocated = (u8)2 * cfg->no_configs + 1;
		temp = realloc(cfg->configs, cfg->allocated * sizeof(u32));
		if (!temp)
			return -ENOMEM;
		cfg->configs = temp;
	}

	cfg->configs[cfg->no_configs++] = config;

	return 0;
}

static bool scmi_pinctrl_is_multi_bit_value(enum converted_pin_param p)
{
	return !!(BIT_32(p) & scmi_pinctrl_multi_bit_cfgs);
}

static u32 scmi_pinctrl_count_mb_configs(struct scmi_pinctrl_pin_cfg *cfg)
{
	enum converted_pin_param param;
	u32 count = 0, i;

	for (i = 0; i < cfg->no_configs; i++) {
		param = (enum converted_pin_param)UNPACK_PARAM(cfg->configs[i]);
		if (scmi_pinctrl_is_multi_bit_value(param))
			count++;
	}

	return count;
}

static int scmi_pinctrl_compare_cfgs(const void *a, const void *b)
{
	s32 pa, pb;

	pa = UNPACK_PARAM(*(s32 *)a);
	pb = UNPACK_PARAM(*(s32 *)b);

	return pb - pa;
}

static int scmi_pinctrl_set_configs_chunk(struct udevice *dev,
					  u16 no_pins,
					  u16 *pins,
					  struct scmi_pinctrl_pin_cfg *cfg,
					  bool override)
{
	u8 buffer[SCMI_MAX_BUFFER_SIZE];
	enum converted_pin_param p;
	struct {
		u32 no_pins;
		u16 pins[];
	} *r_pins = (void *)buffer;
	struct {
		u32 mask;
		u32 boolean_values;
		u32 multi_bit_values[];
	} *r_configs;
	struct {
		s32 status;
	} response;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_PINCTRL,
		.message_id	= SCMI_PINCTRL_PINCONF_SET_APP,
		.in_msg		= buffer,
		.in_msg_sz	= sizeof(*r_pins),
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response),
	};
	u32 *mb_iter, *mb_end;
	u32 param, arg;
	int ret, i;

	if (cfg->no_configs == 0)
		return 0;

	if (override)
		msg.message_id = SCMI_PINCTRL_PINCONF_SET_OVR;

	memset(buffer, 0, ARRAY_SIZE(buffer));

	r_pins->no_pins = no_pins;
	msg.in_msg_sz += no_pins * sizeof(r_pins->pins[0]);
	memcpy(r_pins->pins, pins, no_pins * sizeof(*pins));

	r_configs = (void *)(r_pins->pins + no_pins);
	msg.in_msg_sz += sizeof(*r_configs);
	mb_iter = &r_configs->multi_bit_values[0];
	mb_end = mb_iter + scmi_pinctrl_count_mb_configs(cfg);
	msg.in_msg_sz += scmi_pinctrl_count_mb_configs(cfg) *
			 sizeof(r_configs->multi_bit_values[0]);

	/* Sorting needs to be done in order to lay out
	 * the configs in descending order of their
	 * pinconf parameter value which matches
	 * the protocol specification.
	 */
	qsort(cfg->configs, cfg->no_configs, sizeof(cfg->configs[0]),
	      scmi_pinctrl_compare_cfgs);

	r_configs->mask = 0;
	r_configs->boolean_values = 0;

	for (i = 0; i < cfg->no_configs; ++i) {
		param = UNPACK_PARAM(cfg->configs[i]);
		arg = UNPACK_ARG(cfg->configs[i]);

		r_configs->mask |= BIT_32(param);

		if (param > CONV_PIN_CONFIG_NUM_CONFIGS)
			return -EINVAL;

		p = (enum converted_pin_param)param;

		if (scmi_pinctrl_is_multi_bit_value(p)) {
			if (mb_iter >= mb_end)
				return -EINVAL;

			*mb_iter = arg;
			mb_iter++;
		} else {
			r_configs->boolean_values |= arg ? BIT_32(param) : 0;
		}
	}

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret) {
		pr_err("Error setting pin_config: %d!\n", ret);
		return ret;
	}

	ret = scmi_to_linux_errno(response.status);
	if (ret) {
		pr_err("Error setting pin_config: %d!\n", ret);
		return ret;
	}

	return 0;
}

static int scmi_pinctrl_set_configs(struct udevice *sdev,
				    u16 no_pins,
				    u16 *pins,
				    struct scmi_pinctrl_pin_cfg *cfg)
{
	unsigned int i;
	int ret = 0;

	for (i = 0; i < no_pins / SCMI_MAX_PINS; i++) {
		ret = scmi_pinctrl_set_configs_chunk(sdev, SCMI_MAX_PINS, pins,
						     cfg, true);
		if (ret)
			return ret;

		pins += SCMI_MAX_PINS;
	}

	if (no_pins % SCMI_MAX_PINS != 0)
		ret = scmi_pinctrl_set_configs_chunk(sdev,
						     no_pins % SCMI_MAX_PINS,
						     pins, cfg, true);

	return ret;
}

static int scmi_pinctrl_append_conf(struct udevice *dev, unsigned int pin,
				    unsigned int param, unsigned int arg)
{
	struct scmi_pinctrl_pin_cfg cfg;
	u16 pin_id;
	int ret;

	if (pin > U16_MAX)
		return -EINVAL;

	pin_id = pin;

	cfg.no_configs = 0;
	cfg.allocated = 0;
	cfg.configs = NULL;

	ret = scmi_pinctrl_add_config(PACK_CFG(param, arg), &cfg);
	if (ret) {
		pr_err("Error appending pinconf %d for pin %d\n", param, pin);
		return ret;
	}

	ret = scmi_pinctrl_set_configs_chunk(dev, 1, &pin_id, &cfg, false);
	if (ret)
		pr_err("Error appending pinconf %d for pin %d\n", param, pin);

	free(cfg.configs);

	return ret;
}

static int scmi_pinctrl_set_mux_chunk(struct udevice *dev, u16 no_pins,
				      u16 *pins, u16 *funcs)
{
	u8 buffer[SCMI_MAX_BUFFER_SIZE];
	struct pin_function {
		u16 pin;
		u16 func;
	};
	struct {
		u32 no_pins;
		struct pin_function pf[];
	} *request = (void *)buffer;
	struct {
		s32 status;
	} response;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_PINCTRL,
		.message_id	= SCMI_PINCTRL_PINMUX_SET,
		.in_msg		= buffer,
		.in_msg_sz	= sizeof(*request),
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response),
	};
	int ret, i;

	if (no_pins > SCMI_MAX_PINS)
		return -EINVAL;

	request->no_pins = no_pins;
	msg.in_msg_sz += no_pins * sizeof(request->pf[0]);
	for (i = 0; i < no_pins; i++) {
		request->pf[i].pin = pins[i];
		request->pf[i].func = funcs[i];
	}

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret) {
		pr_err("Error getting gpio_mux: %d!\n", ret);
		return ret;
	}

	ret = scmi_to_linux_errno(response.status);
	if (ret)
		pr_err("Error getting gpio_mux: %d!\n", ret);

	return ret;
}

static int scmi_pinctrl_set_mux(struct udevice *sdev, u16 no_pins,
				u16 *pins, u16 *funcs)
{
	unsigned int i;
	int ret = 0;

	for (i = 0; i < no_pins / SCMI_MAX_PINS; i++) {
		ret = scmi_pinctrl_set_mux_chunk(sdev, SCMI_MAX_PINS, pins,
						 funcs);
		if (ret)
			return ret;

		pins += SCMI_MAX_PINS;
		funcs += SCMI_MAX_PINS;
	}

	if (no_pins % SCMI_MAX_PINS != 0)
		ret = scmi_pinctrl_set_mux_chunk(sdev,
						 no_pins % SCMI_MAX_PINS,
						 pins, funcs);

	return ret;
}

static int scmi_pinctrl_push_back_configs(u8 *buffer,
					  struct scmi_pinctrl_pin_cfg *cfg)
{
	struct scmi_pinctrl_pinconf_resp *r = (void *)buffer;
	unsigned int cfg_idx = 0, bit = sizeof(r->mask) * BITS_PER_BYTE - 1;
	enum converted_pin_param p;
	u32 current_cfg;
	int ret = 0;
	u32 val;

	do {
		if (!(BIT_32(bit) & r->mask))
			continue;

		p = (enum converted_pin_param)bit;
		if (p >= CONV_PIN_CONFIG_NUM_CONFIGS)
			return -EINVAL;

		if (scmi_pinctrl_is_multi_bit_value(p)) {
			if (cfg_idx >= hweight32(scmi_pinctrl_multi_bit_cfgs))
				return -EINVAL;

			val = r->multi_bit_values[cfg_idx++];
		} else {
			val = !!(r->boolean_values & BIT_32(bit));
		}

		current_cfg = PACK_CFG(bit, val);

		ret = scmi_pinctrl_add_config(current_cfg, cfg);
		if (ret)
			return ret;
	} while (bit-- != 0);

	return ret;
}

static int scmi_pinctrl_get_config(struct udevice *dev, u16 pin,
				   struct scmi_pinctrl_pin_cfg *cfg)
{
	u8 response_buffer[SCMI_MAX_BUFFER_SIZE];
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_PINCTRL,
		.message_id	= SCMI_PINCTRL_PINCONF_GET,
		.in_msg		= (u8 *)&pin,
		.in_msg_sz	= sizeof(pin),
		.out_msg	= (u8 *)response_buffer,
		.out_msg_sz	= ARRAY_SIZE(response_buffer),
	};
	struct scmi_pinctrl_pinconf_resp *r = (void *)response_buffer;
	int ret = 0;

	memset(response_buffer, 0, ARRAY_SIZE(response_buffer));

	cfg->allocated = 0;
	cfg->no_configs = 0;
	cfg->configs = NULL;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret) {
		pr_err("Error getting pin_config: %d!\n", ret);
		goto err;
	}

	ret = scmi_to_linux_errno(r->status);
	if (ret) {
		pr_err("Error getting pin_config: %d!\n", ret);
		goto err;
	}

	ret = scmi_pinctrl_push_back_configs(response_buffer,
					     cfg);
	if (ret)
		pr_err("Error getting pin_config: %d!\n", ret);

err:
	if (ret) {
		free(cfg->configs);
		cfg->configs = NULL;
	}

	return ret;
}

static int scmi_pinctrl_get_mux(struct udevice *dev, u16 pin, u16 *func)
{
	struct {
		u16 pin;
	} request;
	struct {
		s32 status;
		u16 function;
	} response;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_PINCTRL,
		.message_id	= SCMI_PINCTRL_PINMUX_GET,
		.in_msg		= (u8 *)&request,
		.in_msg_sz	= sizeof(request),
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response),
	};
	int ret;

	request.pin = pin;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret) {
		pr_err("Error getting gpio_mux: %d!\n", ret);
		return ret;
	}

	ret = scmi_to_linux_errno(response.status);
	if (ret) {
		pr_err("Error getting gpio_mux: %d!\n", ret);
		return ret;
	}

	*func = response.function;

	return 0;
}

static int scmi_pinctrl_app_pinconf_setting(struct udevice *dev,
					    struct ofprop property,
					    struct scmi_pinctrl_pin_cfg *cfg)
{
	enum converted_pin_param param;
	const struct pinconf_param *p;
	const char *pname = NULL;
	unsigned int arg, i;
	const void *value;
	int len = 0;

	value = dev_read_prop_by_prop(&property, &pname, &len);
	if (!value)
		return -EINVAL;

	if (len < 0)
		return -EINVAL;

	for (i = 0; i < ARRAY_SIZE(scmi_pinctrl_pinconf_params); ++i) {
		if (strcmp(pname, scmi_pinctrl_pinconf_params[i].property))
			continue;

		p = &scmi_pinctrl_pinconf_params[i];

		if (len == sizeof(fdt32_t)) {
			arg = fdt32_to_cpu(*(fdt32_t *)value);
		} else if (len == 0) {
			arg = p->default_value;
		} else {
			pr_err("Wrong argument size: %s %d\n", pname, len);
			return -EINVAL;
		}

		if (p->param >= ARRAY_SIZE(scmi_pinctrl_convert))
			return -EINVAL;

		param = scmi_pinctrl_convert[p->param];
		if (param == CONV_PIN_CONFIG_ERROR)
			return -EINVAL;

		return scmi_pinctrl_add_config(PACK_CFG(param, arg), cfg);
	}

	return 0;
}

static int scmi_pinctrl_parse_pinmux_len(struct udevice *dev,
					 struct udevice *config)
{
	int size;

	size = dev_read_size(config, "pinmux");
	if (size < 0)
		return size;
	size /= sizeof(fdt32_t);

	return size;
}

static int scmi_pinctrl_set_state_subnode(struct udevice *dev,
					  struct udevice *config)
{
	struct scmi_pinctrl_pin_cfg cfg;
	struct ofprop property;
	u32 pinmux_value = 0;
	int ret = 0, i, len;
	u16 *pins, *funcs;
	u32 pin, func;
	u16 no_pins;

	cfg.allocated = 0;
	cfg.no_configs = 0;
	cfg.configs = NULL;

	len = scmi_pinctrl_parse_pinmux_len(dev, config);
	if (len <= 0) {
		/* Not a pinmux node. Skip parsing this. */
		return 0;
	}

	dev_for_each_property(property, config) {
		ret = scmi_pinctrl_app_pinconf_setting(dev, property, &cfg);
		if (ret) {
			pr_err("Could not parse property for: %s!\n", config->name);
			break;
		}
	}

	if (ret)
		goto err;

	pins = malloc(len * sizeof(*pins));
	if (!pins) {
		ret = -ENOMEM;
		goto err;
	}

	funcs = malloc(len * sizeof(*funcs));
	if (!funcs) {
		ret = -ENOMEM;
		goto err_pins;
	}

	for (i = 0; i < len; ++i) {
		ret = dev_read_u32_index(config, "pinmux", i, &pinmux_value);
		if (ret) {
			pr_err("Error reading pinmux index: %d\n", i);
			goto err_funcs;
		}

		pin = SCMI_PINCTRL_PIN_FROM_PINMUX(pinmux_value);
		func = SCMI_PINCTRL_FUNC_FROM_PINMUX(pinmux_value);

		if (pin > U16_MAX || func > U16_MAX) {
			pr_err("Invalid pin or func: %u %u!\n", pin, func);
			ret = -EINVAL;
			goto err_funcs;
		}

		pins[i] = pin;
		funcs[i] = func;
	}

	no_pins = len;
	ret = scmi_pinctrl_set_mux(dev, no_pins, pins, funcs);
	if (ret) {
		pr_err("Error setting pinmux: %d!\n", ret);
		goto err_funcs;
	}

	ret = scmi_pinctrl_set_configs(dev, no_pins, pins, &cfg);
	if (ret)
		pr_err("Error setting pinconf: %d!\n", ret);

err_funcs:
	free(funcs);
err_pins:
	free(pins);
err:
	free(cfg.configs);
	return ret;
}

static int scmi_pinctrl_set_state(struct udevice *dev, struct udevice *config)
{
	struct udevice *child = NULL;
	int ret;

	ret = scmi_pinctrl_set_state_subnode(dev, config);
	if (ret) {
		pr_err("Error %d parsing: %s\n", ret, config->name);
		return ret;
	}

	device_foreach_child(child, config) {
		ret = scmi_pinctrl_set_state_subnode(dev, child);
		if (ret)
			return ret;
	}

	return 0;
}

static int scmi_pinctrl_pinmux_set(struct udevice *dev,
				   unsigned int pin_selector,
				   unsigned int func_selector)
{
	u16 pin, func;

	if (pin_selector > U16_MAX || func_selector > U16_MAX)
		return -EINVAL;

	pin = pin_selector;
	func = func_selector;

	return scmi_pinctrl_set_mux(dev, 1, &pin, &func);
}

static int scmi_pinctrl_pinconf_set(struct udevice *dev,
				    unsigned int pin_selector,
				    unsigned int p, unsigned int arg)
{
	if (p >= ARRAY_SIZE(scmi_pinctrl_convert))
		return -EINVAL;

	p = scmi_pinctrl_convert[(enum pin_config_param)p];
	if (p == CONV_PIN_CONFIG_ERROR)
		return -EINVAL;

	return scmi_pinctrl_append_conf(dev, pin_selector, p, arg);
}

static int scmi_pinctrl_gpio_request_enable(struct udevice *dev,
					    unsigned int pin_selector)
{
	struct scmi_pinctrl_priv *priv = dev_get_priv(dev);
	struct scmi_pinctrl_saved_pin *save;
	struct scmi_pinctrl_pin_cfg cfg;
	u16 pin, func;
	int ret;

	cfg.configs = NULL;

	if (pin_selector > U16_MAX)
		return -EINVAL;
	pin = pin_selector;

	save = malloc(sizeof(*save));
	if (!save)
		return -ENOMEM;

	ret = scmi_pinctrl_get_mux(dev, pin, &func);
	if (ret)
		goto err;

	ret = scmi_pinctrl_get_config(dev, pin, &cfg);
	if (ret)
		goto err;

	save->pin = pin_selector;
	save->func = func;
	save->cfg = cfg;

	func = 0;
	ret = scmi_pinctrl_set_mux(dev, 1, &pin, &func);
	if (ret)
		goto err;

	list_add(&save->list, &priv->gpio_configs);

err:
	if (ret) {
		free(cfg.configs);
		free(save);
	}

	return ret;
}

static int scmi_pinctrl_gpio_disable_free(struct udevice *dev,
					  unsigned int pin_selector)
{
	struct scmi_pinctrl_priv *priv = dev_get_priv(dev);
	struct scmi_pinctrl_saved_pin *save, *temp;
	int ret = -EINVAL;

	list_for_each_entry_safe(save, temp, &priv->gpio_configs, list) {
		if (save->pin != pin_selector)
			continue;

		ret = scmi_pinctrl_set_mux(dev, 1, &save->pin, &save->func);
		if (ret)
			break;

		ret = scmi_pinctrl_set_configs(dev, 1, &save->pin, &save->cfg);
		if (ret)
			break;

		list_del(&save->list);
		free(save->cfg.configs);
		free(save);
		return 0;
	}

	return ret;
}

static int scmi_pinctrl_get_gpio_mux(struct udevice *dev,
				     __maybe_unused int banknum,
				     int index)
{
	struct scmi_pinctrl_pin_cfg cfg;
	u16 function;
	int ret, i;
	u32 param, arg;
	bool output = false, input = false;

	if (index > U16_MAX || index < 0)
		return -EINVAL;

	ret = scmi_pinctrl_get_mux(dev, (u16)index, &function);
	if (ret)
		return ret;

	if (function != 0)
		return GPIOF_FUNC;

	ret = scmi_pinctrl_get_config(dev, (u16)index, &cfg);
	if (ret)
		return ret;

	for (i = 0; i < cfg.no_configs; ++i) {
		param = UNPACK_PARAM(cfg.configs[i]);
		arg = UNPACK_ARG(cfg.configs[i]);

		if (param == CONV_PIN_CONFIG_OUTPUT_ENABLE && arg == 1)
			output = true;
		else if (param == CONV_PIN_CONFIG_INPUT_ENABLE && arg == 1)
			input = true;
	}

	free(cfg.configs);

	if (!input)
		return GPIOF_UNKNOWN;

	if (output)
		return GPIOF_OUTPUT;

	return GPIOF_INPUT;
}

static int scmi_pinctrl_get_pins_count(struct udevice *dev)
{
	struct scmi_pinctrl_priv *priv = dev_get_priv(dev);
	u32 last = priv->no_ranges - 1;

	return priv->ranges[last].begin + priv->ranges[last].no_pins;
}

static struct scmi_pinctrl_range *scmi_pinctrl_get_range(struct udevice *dev,
							 unsigned int pin)
{
	struct scmi_pinctrl_priv *priv = dev_get_priv(dev);
	struct scmi_pinctrl_range *range;
	unsigned int i;

	for (i = 0; i < priv->no_ranges; i++) {
		range = &priv->ranges[i];
		if (pin >= range->begin && pin < range->begin + range->no_pins)
			return range;
	}

	return NULL;
}

static const char *scmi_pinctrl_get_pin_name(struct udevice *dev,
					     unsigned int pin)
{
	static char pin_name[PINNAME_SIZE];
	struct scmi_pinctrl_range *range;
	int ret;

	memset(pin_name, 0, PINNAME_SIZE);

	range = scmi_pinctrl_get_range(dev, pin);
	if (range) {
		ret = snprintf(pin_name, sizeof(pin_name), "pin%d", pin);
		if (ret >= sizeof(pin_name))
			return ERR_PTR(-EINVAL);
		return pin_name;
	}

	ret = snprintf(pin_name, sizeof(pin_name), "invalid");
	if (ret >= sizeof(pin_name))
		return ERR_PTR(-EINVAL);

	return ERR_PTR(-ENODEV);
}

static int scmi_pinctrl_get_pin_muxing(struct udevice *dev, unsigned int pin,
				       char *buf, int size)
{
	struct scmi_pinctrl_range *range;
	u16 func;
	int ret;

	if (pin > U16_MAX || size <= 0)
		return -EINVAL;

	memset(buf, 0, size);

	range = scmi_pinctrl_get_range(dev, pin);
	if (!range)
		return -ENODEV;

	ret = scmi_pinctrl_get_mux(dev, pin, &func);
	if (ret)
		return ret;

	ret = snprintf(buf, size, "function %hu", func);
	if (ret >= size)
		return -EINVAL;

	return 0;
}

static const struct pinctrl_ops scmi_pinctrl_ops = {
	.set_state		= scmi_pinctrl_set_state,
	.gpio_request_enable	= scmi_pinctrl_gpio_request_enable,
	.gpio_disable_free	= scmi_pinctrl_gpio_disable_free,
	.pinmux_set		= scmi_pinctrl_pinmux_set,
	.pinconf_set		= scmi_pinctrl_pinconf_set,
	.get_gpio_mux		= scmi_pinctrl_get_gpio_mux,
	.pinconf_num_params	= ARRAY_SIZE(scmi_pinctrl_pinconf_params),
	.pinconf_params		= scmi_pinctrl_pinconf_params,
	.get_pins_count		= scmi_pinctrl_get_pins_count,
	.get_pin_name		= scmi_pinctrl_get_pin_name,
	.get_pin_muxing		= scmi_pinctrl_get_pin_muxing,
};

static int scmi_pinctrl_get_proto_attr(struct udevice *dev)
{
	struct scmi_pinctrl_priv *priv = dev_get_priv(dev);
	struct {
		s32 status;
		u32 attributes;
	} response;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_PINCTRL,
		.message_id	= SCMI_PINCTRL_PROTOCOL_ATTRIBUTES,
		.in_msg		= NULL,
		.in_msg_sz	= 0,
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret) {
		if (ret != -EPROBE_DEFER)
			pr_err("Error getting proto attr: %d!\n", ret);
		return ret;
	}

	ret = scmi_to_linux_errno(response.status);
	if (ret) {
		pr_err("Error getting proto attr: %d!\n", ret);
		return ret;
	}

	priv->no_ranges = response.attributes & SCMI_PINCTRL_NUM_RANGES_MASK;

	return 0;
}

static int scmi_pinctrl_get_pin_ranges(struct udevice *dev)
{
	struct scmi_pinctrl_priv *priv = dev_get_priv(dev);
	u8 buffer[SCMI_MAX_BUFFER_SIZE];
	struct {
		u32 range_index;
	} request;
	struct {
		s32 status;
		u32 no_ranges;
		struct pr {
			u16 begin;
			u16 no_pins;
		} pin_ranges[];
	} *response = (void *)buffer;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_PINCTRL,
		.message_id	= SCMI_PINCTRL_DESCRIBE,
		.in_msg		= (u8 *)&request,
		.in_msg_sz	= sizeof(request),
		.out_msg	= buffer,
		.out_msg_sz	= ARRAY_SIZE(buffer),
	};
	u32 i, range_index = 0, tmp_idx;
	int ret;

	priv->ranges = malloc(priv->no_ranges * sizeof(*priv->ranges));
	if (!priv->ranges) {
		ret = -ENOMEM;
		goto err;
	}

	while (range_index < priv->no_ranges) {
		request.range_index = range_index;
		ret = devm_scmi_process_msg(dev, &msg);
		if (ret) {
			pr_err("Error getting pin ranges: %d!\n", ret);
			break;
		}

		ret = scmi_to_linux_errno(response->status);
		if (ret) {
			pr_err("Error getting pin ranges: %d!\n", ret);
			break;
		}

		for (i = 0; i < response->no_ranges; ++i) {
			tmp_idx = i + range_index;
			priv->ranges[tmp_idx].begin =
				response->pin_ranges[i].begin;
			priv->ranges[tmp_idx].no_pins =
				response->pin_ranges[i].no_pins;
		}

		range_index += response->no_ranges;
	}

err:
	return ret;
}

static int scmi_pinctrl_init(struct udevice *dev)
{
	int ret = 0;

	ret = scmi_pinctrl_get_proto_attr(dev);
	if (ret)
		return ret;

	return scmi_pinctrl_get_pin_ranges(dev);
}

static int scmi_pinctrl_probe(struct udevice *dev)
{
	struct scmi_pinctrl_priv *priv = dev_get_priv(dev);
	int ret;

	ret = scmi_pinctrl_init(dev);
	if (ret)
		return ret;

	INIT_LIST_HEAD(&priv->gpio_configs);

	return 0;
}

U_BOOT_DRIVER(scmi_pinctrl) = {
	.name = "scmi_pinctrl",
	.id = UCLASS_PINCTRL,
	.probe = scmi_pinctrl_probe,
	.priv_auto = sizeof(struct scmi_pinctrl_priv),
	.ops = &scmi_pinctrl_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
