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

static int scmi_clk_get_attibute(struct udevice *dev, int clkid, char **name)
{
	struct scmi_clk_attribute_in in = {
		.clock_id = clkid,
	};
	struct scmi_clk_attribute_out out;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_CLOCK,
		.message_id = SCMI_CLOCK_ATTRIBUTES,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	*name = strdup(out.clock_name);

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
	return scmi_clk_gate(clk, 1);
}

static int scmi_clk_disable(struct clk *clk)
{
	return scmi_clk_gate(clk, 0);
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

static ulong scmi_clk_set_rate(struct clk *clk, ulong rate)
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

static int scmi_clk_probe(struct udevice *dev)
{
	struct clk *clk;
	size_t num_clocks, i;
	int ret;

	if (!CONFIG_IS_ENABLED(CLK_CCF))
		return 0;

	/* register CCF children: CLK UCLASS, no probed again */
	if (device_get_uclass_id(dev->parent) == UCLASS_CLK)
		return 0;

	ret = scmi_clk_get_num_clock(dev, &num_clocks);
	if (ret)
		return ret;

	for (i = 0; i < num_clocks; i++) {
		char *clock_name;

		if (!scmi_clk_get_attibute(dev, i, &clock_name)) {
			clk = kzalloc(sizeof(*clk), GFP_KERNEL);
			if (!clk || !clock_name)
				ret = -ENOMEM;
			else
				ret = clk_register(clk, dev->driver->name,
						   clock_name, dev->name);

			if (ret) {
				free(clk);
				free(clock_name);
				return ret;
			}

			clk_dm(i, clk);
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
	int ret;

	ret = scmi_clk_get_num_clock(dev, &num_clocks);
	if (ret) {
		printf("Failed to get the number of clocks\n");
		return CMD_RET_FAILURE;
	}

	/* Look for clocks containing the given string */
	for (i = 0; i < num_clocks; i++) {
		if (scmi_clk_get_attibute(dev, i, &name))
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
