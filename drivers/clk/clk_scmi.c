// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019-2020 Linaro Limited
 * Copyright 2021, 2023 NXP
 */
#include <common.h>
#include <clk-uclass.h>
#include <dm.h>
#include <scmi.h>
#include <asm/types.h>

enum scmi_clock_message_id {
	SCMI_CLOCK_RATE_SET = 0x5,
	SCMI_CLOCK_RATE_GET = 0x6,
	SCMI_CLOCK_CONFIG_SET = 0x7,
};

#define SCMI_CLK_RATE_ASYNC_NOTIFY	BIT(0)
#define SCMI_CLK_RATE_ASYNC_NORESP	(BIT(0) | BIT(1))
#define SCMI_CLK_RATE_ROUND_DOWN	0
#define SCMI_CLK_RATE_ROUND_UP		BIT(2)
#define SCMI_CLK_RATE_ROUND_CLOSEST	BIT(3)

struct scmi_clk_state_in {
	u32 clock_id;
	u32 attributes;
};

struct scmi_clk_state_out {
	s32 status;
};

static int scmi_clk_gate(struct clk *clk, int enable)
{
	struct scmi_clk_state_in in = {
		.clock_id = clk->id,
		.attributes = enable,
	};
	struct scmi_clk_state_out out;
	struct scmi_msg scmi_msg = {
		.protocol_id = SCMI_PROTOCOL_ID_CLOCK,
		.message_id = SCMI_CLOCK_CONFIG_SET,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int rc;

	rc = scmi_send_and_process_msg(clk->dev->parent, &scmi_msg);
	if (rc)
		return rc;

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

struct scmi_clk_rate_get_in {
	u32 clock_id;
};

struct scmi_clk_rate_get_out {
	s32 status;
	u32 rate_lsb;
	u32 rate_msb;
};

static ulong scmi_clk_get_rate(struct clk *clk)
{
	struct scmi_clk_rate_get_in in = {
		.clock_id = clk->id,
	};
	struct scmi_clk_rate_get_out out;
	struct scmi_msg scmi_msg = {
		.protocol_id = SCMI_PROTOCOL_ID_CLOCK,
		.message_id = SCMI_CLOCK_RATE_GET,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int rc;

	rc = scmi_send_and_process_msg(clk->dev->parent, &scmi_msg);
	if (rc)
		return 0;

	rc = scmi_to_linux_errno(out.status);
	if (rc)
		return 0;

	return (ulong)(((u64)out.rate_msb << 32) | out.rate_lsb);
}

struct scmi_clk_rate_set_in {
	u32 flags;
	u32 clock_id;
	u32 rate_lsb;
	u32 rate_msb;
};

struct scmi_clk_rate_set_out {
	s32 status;
};

static ulong scmi_clk_set_rate(struct clk *clk, ulong rate)
{
	struct scmi_clk_rate_set_in in = {
		.clock_id = clk->id,
		.flags = SCMI_CLK_RATE_ROUND_CLOSEST,
		.rate_lsb = (u32)rate,
		.rate_msb = (u32)((u64)rate >> 32),
	};
	struct scmi_clk_rate_set_out out;
	struct scmi_msg scmi_msg = {
		.protocol_id = SCMI_PROTOCOL_ID_CLOCK,
		.message_id = SCMI_CLOCK_RATE_SET,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int rc;

	rc = scmi_send_and_process_msg(clk->dev->parent, &scmi_msg);
	if (rc)
		return 0;

	if (scmi_to_linux_errno(out.status))
		return 0;

	return rate;
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
};

static int do_scmi_clk_gate(cmd_tbl_t *cmdtp, int flag, int argc,
			    char *const argv[])
{
	struct clk clk;
	int ret, enable;

	if (argc < 4)
		return CMD_RET_USAGE;

	memset(&clk, 0, sizeof(clk));

	ret = uclass_get_device_by_name(UCLASS_CLK, argv[1], &clk.dev);
	if (ret) {
		printf("Failed to get device '%s'\n", argv[1]);
		return CMD_RET_FAILURE;
	}

	clk.id = simple_strtoul(argv[2], NULL, 10);
	if (clk.id > INT_MAX)
		return CMD_RET_FAILURE;

	if (simple_strtoul(argv[3], NULL, 10))
		enable = 1;
	else
		enable = 0;

	ret = scmi_clk_gate(&clk, enable);
	if (ret) {
		printf("scmi_clk_enable failed: %d\n", ret);
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

static cmd_tbl_t cmd_clk_sub[] = {
	U_BOOT_CMD_MKENT(gate, 3, 1, do_scmi_clk_gate, "", ""),
};

static int do_scmi_clk(cmd_tbl_t *cmdtp, int flag, int argc,
		       char *const argv[])
{
	cmd_tbl_t *c;

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
	"gate [device_name] [clk] [enable/disable]- Turn on/off a clock";
#endif

U_BOOT_CMD(scmi_clk, 5, 1, do_scmi_clk, "SCMI CLK sub-system",
	   scmi_clk_help_text);
