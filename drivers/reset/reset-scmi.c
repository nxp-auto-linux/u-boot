// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2019-2020 Linaro Limited
 */
#include <common.h>
#include <dm.h>
#include <errno.h>
#include <reset-uclass.h>
#include <scmi.h>
#include <asm/types.h>

/* Helper macro to match a message on input/output array references */
#define SCMI_MSG_IN(_protocol, _message, _in_array, _out_array) \
	(struct scmi_msg){			\
		.protocol_id = (_protocol),	\
		.message_id = (_message),	\
		.in_msg = (uint8_t *)&(_in_array),	\
		.in_msg_sz = sizeof(_in_array),	\
		.out_msg = (uint8_t *)&(_out_array),	\
		.out_msg_sz = sizeof(_out_array),	\
	}

/**
 * struct scmi_clk_state_in - Message payload for CLOCK_RATE_SET command
 * @flags:	Flags for the clock rate set request
 * @clock_id:	SCMI clock ID
 * @rate_lsb:	32bit LSB of the clock rate in Hertz
 * @rate_msb:	32bit MSB of the clock rate in Hertz
 */
struct scmi_clk_rate_set_in {
	u32 flags;
	u32 clock_id;
	u32 rate_lsb;
	u32 rate_msb;
};

/**
 * struct scmi_rd_reset_in - Message payload for RESET command
 * @domain_id:		SCMI reset domain ID
 * @flags:		Flags for the reset request
 * @reset_state:	Reset target state
 */
struct scmi_rd_reset_in {
	u32 domain_id;
	u32 flags;
	u32 reset_state;
};

/**
 * struct scmi_rd_reset_out - Response payload for RESET command
 * @status:	SCMI command status
 */
struct scmi_rd_reset_out {
	s32 status;
};

/*
 * SCMI Reset Domain Protocol
 */

#define SCMI_RD_RESET_FLAG_ASSERT	BIT(1)
#define SCMI_RD_NAME_LEN		16

enum scmi_reset_domain_message_id {
	SCMI_RESET_DOMAIN_ATTRIBUTES = 0x3,
	SCMI_RESET_DOMAIN_RESET = 0x4,
};

/**
 * struct scmi_rd_attr_in - Payload for RESET_DOMAIN_ATTRIBUTES message
 * @domain_id:	SCMI reset domain ID
 */
struct scmi_rd_attr_in {
	u32 domain_id;
};

/**
 * struct scmi_rd_attr_out - Payload for RESET_DOMAIN_ATTRIBUTES response
 * @status:	SCMI command status
 * @attributes:	Retrieved attributes of the reset domain
 * @latency:	Reset cycle max lantency
 * @name:	Reset domain name
 */
struct scmi_rd_attr_out {
	s32 status;
	u32 attributes;
	u32 latency;
	char name[SCMI_RD_NAME_LEN];
};

static int scmi_reset_set_level(struct reset_ctl *rst, bool assert_not_deassert)
{
	struct scmi_rd_reset_in in = {
		.domain_id = rst->id,
		.flags = assert_not_deassert ? SCMI_RD_RESET_FLAG_ASSERT : 0,
		.reset_state = 0,
	};
	struct scmi_rd_reset_out out;
	struct scmi_msg msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_RESET_DOMAIN,
					  SCMI_RESET_DOMAIN_RESET,
					  in, out);
	int ret;

	ret = scmi_send_and_process_msg(rst->dev->parent, &msg);
	if (ret)
		return ret;

	return scmi_to_linux_errno(out.status);
}

static int scmi_reset_assert(struct reset_ctl *rst)
{
	return scmi_reset_set_level(rst, true);
}

static int scmi_reset_deassert(struct reset_ctl *rst)
{
	return scmi_reset_set_level(rst, false);
}

static int scmi_reset_request(struct reset_ctl *rst)
{
	struct scmi_rd_attr_in in = {
		.domain_id = rst->id,
	};
	struct scmi_rd_attr_out out;
	struct scmi_msg msg = SCMI_MSG_IN(SCMI_PROTOCOL_ID_RESET_DOMAIN,
					  SCMI_RESET_DOMAIN_ATTRIBUTES,
					  in, out);
	int ret;

	/*
	 * We don't really care about the attribute, just check
	 * the reset domain exists.
	 */
	ret = scmi_send_and_process_msg(rst->dev->parent, &msg);
	if (ret)
		return ret;

	return scmi_to_linux_errno(out.status);
}

static int scmi_reset_rfree(struct reset_ctl *rst)
{
	return 0;
}

static const struct reset_ops scmi_reset_domain_ops = {
	.request	= scmi_reset_request,
	.rfree		= scmi_reset_rfree,
	.rst_assert	= scmi_reset_assert,
	.rst_deassert	= scmi_reset_deassert,
};

U_BOOT_DRIVER(scmi_reset_domain) = {
	.name = "scmi_reset_domain",
	.id = UCLASS_RESET,
	.ops = &scmi_reset_domain_ops,
};
