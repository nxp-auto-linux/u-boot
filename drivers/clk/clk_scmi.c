// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019-2020 Linaro Limited
 */

#define LOG_CATEGORY UCLASS_CLK

#include <common.h>
#include <clk-uclass.h>
#include <command.h>
#include <dm.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>
#include <asm/types.h>
#include <linux/clk-provider.h>

struct clk_scmi {
	struct clk clk;
	u32 ctrl_flags;
};

struct scmi_clock_priv {
	u32 version;
};

static int scmi_clk_get_permissions(struct udevice *dev, int clkid, u32 *perm)
{
	struct scmi_clock_priv *priv = dev_get_priv(dev);
	int ret;

	struct scmi_clk_get_permissions_in in = {
		.clock_id = clkid,
	};
	struct scmi_clk_get_permissions_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_CLOCK,
		.message_id = SCMI_CLOCK_GET_PERMISSIONS,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};

	if (priv->version < CLOCK_PROTOCOL_VERSION_3_0) {
		log_debug("%s: SCMI clock management protocol version is less than 3.0.\n", __func__);
		return -EINVAL;
	}

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret) {
		log_debug("%s: get SCMI clock management protocol permissions failed\n", __func__);
		return ret;
	}

	ret = scmi_to_linux_errno(out.status);
	if (ret < 0) {
		log_debug("%s: the status code of getting permissions: %d\n", __func__, ret);
		return ret;
	}

	*perm = out.permissions;
	return 0;
}

static int scmi_clk_get_num_clock(struct udevice *dev, size_t *num_clocks)
{
	struct scmi_clk_protocol_attr_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_CLOCK,
		.message_id = SCMI_PROTOCOL_ATTRIBUTES,
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	*num_clocks = out.attributes & SCMI_CLK_PROTO_ATTR_COUNT_MASK;

	return 0;
}

static int scmi_clk_get_attibute(struct udevice *dev, int clkid, char **name,
				 u32 *attr)
{
	struct scmi_clock_priv *priv = dev_get_priv(dev);
	struct scmi_clk_attribute_in in = {
		.clock_id = clkid,
	};
	int ret;

	if (priv->version >= 0x20000) {
		struct scmi_clk_attribute_out_v2 out;
		struct scmi_msg msg = {
			.protocol_id = SCMI_PROTOCOL_ID_CLOCK,
			.message_id = SCMI_CLOCK_ATTRIBUTES,
			.in_msg = (u8 *)&in,
			.in_msg_sz = sizeof(in),
			.out_msg = (u8 *)&out,
			.out_msg_sz = sizeof(out),
		};

		ret = devm_scmi_process_msg(dev, &msg);
		if (ret)
			return ret;

		*name = strdup(out.clock_name);
		*attr = out.attributes;
	} else {
		struct scmi_clk_attribute_out out;
		struct scmi_msg msg = {
			.protocol_id = SCMI_PROTOCOL_ID_CLOCK,
			.message_id = SCMI_CLOCK_ATTRIBUTES,
			.in_msg = (u8 *)&in,
			.in_msg_sz = sizeof(in),
			.out_msg = (u8 *)&out,
			.out_msg_sz = sizeof(out),
		};

		ret = devm_scmi_process_msg(dev, &msg);
		if (ret)
			return ret;

		*name = strdup(out.clock_name);
		*attr = out.attributes;
	}

	return 0;
}

static int scmi_clk_gate(struct clk *clk, int enable)
{
	struct scmi_clk_state_in in = {
		.clock_id = clk->id,
		.attributes = enable,
	};
	struct scmi_clk_state_out out;
	struct scmi_msg msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_CLOCK,
					  SCMI_CLOCK_CONFIG_SET,
					  in, out);
	int ret;

	ret = devm_scmi_process_msg(clk->dev, &msg);
	if (ret)
		return ret;

	return scmi_to_linux_errno(out.status);
}

static int scmi_clk_enable(struct clk *clk)
{
	struct clk_scmi *clkscmi;
	struct clk *c;
	int ret;

	if (!CONFIG_IS_ENABLED(CLK_CCF))
		return scmi_clk_gate(clk, 1);

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	clkscmi = container_of(c, struct clk_scmi, clk);

	if (clkscmi->ctrl_flags & SUPPORT_CLK_STAT_CONTROL)
		return scmi_clk_gate(clk, 1);

	/* Following Linux drivers/clk/clk-scmi.c, directly return 0 if agent has no permission. */
	log_debug("%s: SCMI CLOCK: the clock cannot be enabled by the agent.\n", __func__);
	return 0;
}

static int scmi_clk_disable(struct clk *clk)
{
	struct clk_scmi *clkscmi;
	struct clk *c;
	int ret;

	if (!CONFIG_IS_ENABLED(CLK_CCF))
		return scmi_clk_gate(clk, 0);

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	clkscmi = container_of(c, struct clk_scmi, clk);

	if (clkscmi->ctrl_flags & SUPPORT_CLK_STAT_CONTROL)
		return scmi_clk_gate(clk, 0);

	/* Following Linux drivers/clk/clk-scmi.c, directly return 0 if agent has no permission. */
	log_debug("%s: SCMI CLOCK: the clock cannot be disabled by the agent.\n", __func__);
	return 0;
}

static ulong scmi_clk_get_rate(struct clk *clk)
{
	struct scmi_clk_rate_get_in in = {
		.clock_id = clk->id,
	};
	struct scmi_clk_rate_get_out out;
	struct scmi_msg msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_CLOCK,
					  SCMI_CLOCK_RATE_GET,
					  in, out);
	int ret;

	ret = devm_scmi_process_msg(clk->dev, &msg);
	if (ret < 0)
		return ret;

	ret = scmi_to_linux_errno(out.status);
	if (ret < 0)
		return ret;

	return (ulong)(((u64)out.rate_msb << 32) | out.rate_lsb);
}

static ulong __scmi_clk_set_rate(struct clk *clk, ulong rate)
{
	struct scmi_clk_rate_set_in in = {
		.clock_id = clk->id,
		.flags = SCMI_CLK_RATE_ROUND_CLOSEST,
		.rate_lsb = (u32)rate,
		.rate_msb = (u32)((u64)rate >> 32),
	};
	struct scmi_clk_rate_set_out out;
	struct scmi_msg msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_CLOCK,
					  SCMI_CLOCK_RATE_SET,
					  in, out);
	int ret;

	ret = devm_scmi_process_msg(clk->dev, &msg);
	if (ret < 0)
		return ret;

	ret = scmi_to_linux_errno(out.status);
	if (ret < 0)
		return ret;

	return scmi_clk_get_rate(clk);
}

static ulong scmi_clk_set_rate(struct clk *clk, ulong rate)
{
	struct clk_scmi *clkscmi;
	struct clk *c;
	int ret;

	if (!CONFIG_IS_ENABLED(CLK_CCF))
		return __scmi_clk_set_rate(clk, rate);

	ret = clk_get_by_id(clk->id, &c);
	if (ret)
		return ret;

	clkscmi = container_of(c, struct clk_scmi, clk);

	if (clkscmi->ctrl_flags & SUPPORT_CLK_RATE_CONTROL)
		return __scmi_clk_set_rate(clk, rate);

	/* Following Linux drivers/clk/clk-scmi.c, directly return 0 if agent has no permission. */
	log_debug("%s: SCMI CLOCK: the clock rate cannot be changed by the agent.\n", __func__);
	return 0;
}

static int scmi_clk_probe(struct udevice *dev)
{
	struct clk_scmi *clk_scmi;
	struct scmi_clock_priv *priv = dev_get_priv(dev);
	size_t num_clocks, i;
	int ret;

	if (!CONFIG_IS_ENABLED(CLK_CCF))
		return 0;

	ret = scmi_generic_protocol_version(dev, SCMI_PROTOCOL_ID_CLOCK,
					    &priv->version);

	/* register CCF children: CLK UCLASS, no probed again */
	if (device_get_uclass_id(dev->parent) == UCLASS_CLK)
		return 0;

	ret = scmi_clk_get_num_clock(dev, &num_clocks);
	if (ret)
		return ret;

	ret = scmi_generic_protocol_version(dev, SCMI_PROTOCOL_ID_CLOCK, &priv->version);
	if (ret) {
		log_debug("%s: get SCMI clock management protocol version failed\n", __func__);
		return ret;
	}

	for (i = 0; i < num_clocks; i++) {
		char *clock_name;
		u32 attributes;

		if (!scmi_clk_get_attibute(dev, i, &clock_name, &attributes)) {
			clk_scmi = kzalloc(sizeof(*clk_scmi), GFP_KERNEL);
			if (!clk_scmi || !clock_name)
				ret = -ENOMEM;
			else
				ret = clk_register(&clk_scmi->clk, dev->driver->name,
						   clock_name, dev->name);

			if (ret) {
				free(clk_scmi);
				free(clock_name);
				return ret;
			}

			clk_dm(i, &clk_scmi->clk);

			if (CLK_HAS_RESTRICTIONS(attributes)) {
				u32 perm;

				ret = scmi_clk_get_permissions(dev, i, &perm);
				if (ret < 0)
					clk_scmi->ctrl_flags = 0;
				else
					clk_scmi->ctrl_flags = perm;
			} else {
				clk_scmi->ctrl_flags = SUPPORT_CLK_STAT_CONTROL | SUPPORT_CLK_PARENT_CONTROL |
						       SUPPORT_CLK_RATE_CONTROL;
			}
		}
	}

	return 0;
}

static const struct clk_ops scmi_clk_ops = {
	.enable = scmi_clk_enable,
	.disable = scmi_clk_disable,
	.get_rate = scmi_clk_get_rate,
	.set_rate = scmi_clk_set_rate,
};

U_BOOT_DRIVER(scmi_clock) = {
	.name = "scmi_clk",
	.id = UCLASS_CLK,
	.ops = &scmi_clk_ops,
	.probe = &scmi_clk_probe,
	.priv_auto = sizeof(struct scmi_clock_priv),
};

static int gate_scmi_clk_id(struct udevice *dev, unsigned long clk_id,
			    int enable)
{
	struct clk clk;

	memset(&clk, 0, sizeof(clk));

	clk.dev = dev;
	clk.id = clk_id;

	return scmi_clk_gate(&clk, enable);
}

static int process_clocks_by_name(struct udevice *dev, const char *name_part, bool enable)
{
	size_t num_clocks, i;
	char *name, *res;
	u32 attributes;
	int ret;

	ret = scmi_clk_get_num_clock(dev, &num_clocks);
	if (ret) {
		printf("Failed to get the number of clocks\n");
		return CMD_RET_FAILURE;
	}

	/* Look for clocks containing the given string */
	for (i = 0; i < num_clocks; i++) {
		if (scmi_clk_get_attibute(dev, i, &name, &attributes))
			continue;

		res = strstr(name, name_part);
		if (res)
			printf("## Setting %s clock ...\n", name);

		free(name);

		if (!res)
			continue;

		if (gate_scmi_clk_id(dev, i, enable)) {
			printf("Failed to control the clock %lu\n", i);
			return CMD_RET_FAILURE;
		}
	}

	return CMD_RET_SUCCESS;
}

static int do_scmi_clk_gate(struct cmd_tbl *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct udevice *dev;
	unsigned long id;
	int ret, enable;

	if (argc < 3 || argc > 4)
		return CMD_RET_USAGE;

	enable = simple_strtoul(argv[argc - 1], NULL, 10) ? 1 : 0;

	if (argc == 4) {
		ret = uclass_get_device_by_name(UCLASS_CLK, argv[1], &dev);
		if (ret) {
			printf("Failed to get device '%s'\n", argv[1]);
			return CMD_RET_FAILURE;
		}
	} else {
		ret = uclass_get_device_by_driver(UCLASS_CLK, DM_DRIVER_GET(scmi_clock), &dev);
		if (ret) {
			printf("Failed to get the SCMI clock device\n");
			return CMD_RET_FAILURE;
		}
	}

	/* Is it a number ?*/
	if (!strict_strtoul(argv[argc - 2], 10, &id)) {
			if (gate_scmi_clk_id(dev, id, enable)) {
				printf("Failed to control the clock %lu\n", id);
				return CMD_RET_FAILURE;
			}
			printf("## Setting %lu clock ...\n", id);
			return CMD_RET_SUCCESS;
	}
	
	return process_clocks_by_name(dev, argv[argc - 2], enable);
}

static struct cmd_tbl cmd_clk_sub[] = {
	U_BOOT_CMD_MKENT(gate, 3, 1, do_scmi_clk_gate, "", ""),
};

static int do_scmi_clk(struct cmd_tbl *cmdtp, int flag, int argc,
		       char *const argv[])
{
	struct cmd_tbl *c;

	if (argc < 2)
		return CMD_RET_USAGE;

	/* Strip off leading 'scmi_clk' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_clk_sub[0], ARRAY_SIZE(cmd_clk_sub));

	if (c)
		return c->cmd(cmdtp, flag, argc, argv);

	return CMD_RET_USAGE;
}

#ifdef CONFIG_SYS_LONGHELP
static char scmi_clk_help_text[] =
	"gate [device_name] [clk] [1/0] - Turn on/off a clock\n"
	"\tThe argument 'device_name' is optional\n"
	"\tThe argument 'clk' specifies the name of the clock or SCMI clock ID\n";
#endif

U_BOOT_CMD(scmi_clk, 5, 1, do_scmi_clk, "SCMI CLK sub-system",
	   scmi_clk_help_text);
