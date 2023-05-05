// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2020,2022-2023 NXP
 */
#include <common.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>
#include <dm/uclass.h>
#include <s32-cc/scmi_reset_agent.h>

#define SCMI_BASE_RESET_AGENT_CONFIG	0xB

#define SCMI_RESET_AGENT_FLAG		BIT(0)

/**
 * BASE_RESET_AGENT_CONFIGURATION
 */
struct scmi_base_reset_agent_in {
	u32 agent_id;
	u32 flags;
};

struct scmi_base_reset_agent_out {
	s32 status;
};

static struct udevice *get_scmi_dev(void)
{
	struct udevice *dev = NULL;
	ofnode node;
	int ret;

	node = ofnode_by_compatible(ofnode_null(), "arm,scmi-smc");
	if (!ofnode_valid(node)) {
		pr_err("Failed to get 'arm,scmi-smc' node\n");
		return NULL;
	}

	ret = uclass_get_device_by_ofnode(UCLASS_SCMI_AGENT, node, &dev);
	if (ret) {
		pr_err("Failed to get SCMI device\n");
		return NULL;
	}

	return dev;
}

int scmi_reset_agent(void)
{
	struct udevice *dev;
	const struct scmi_base_reset_agent_in in = {
		.agent_id = S32_SCMI_AGENT_OSPM,
		.flags = SCMI_RESET_AGENT_FLAG,
	};
	struct scmi_base_reset_agent_out out;
	struct scmi_msg scmi_msg = {
		.protocol_id = SCMI_PROTOCOL_ID_BASE,
		.message_id = SCMI_BASE_RESET_AGENT_CONFIG,
		.in_msg = (u8 *)&in,
		.in_msg_sz = sizeof(in),
		.out_msg = (u8 *)&out,
		.out_msg_sz = sizeof(out),
	};
	int rc;

	dev = get_scmi_dev();
	if (!dev)
		return -ENODEV;

	rc = devm_scmi_process_msg(dev, &scmi_msg);
	if (rc)
		return rc;

	return scmi_to_linux_errno(out.status);
}
