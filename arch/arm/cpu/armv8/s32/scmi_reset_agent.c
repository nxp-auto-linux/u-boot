// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2020 NXP
 */
#include <common.h>
#include <scmi.h>
#include <asm/types.h>
#include <dm/uclass.h>
#include "scmi_reset_agent.h"

#define SCMI_BASE_RESET_AGENT_CONFIG	0xB

#define SCMI_RESET_AGENT_FLAG		BIT(0)

/**
 * BASE_RESET_AGENT_CONFIGURATION
 */
struct scmi_base_reset_agent_in {
       uint32_t agent_id;
       uint32_t flags;
};

struct scmi_base_reset_agent_out {
       int32_t status;
};

static struct udevice *get_scmi_dev(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_FIRMWARE, "scmi", &dev);
	if (ret) {
		pr_err("Failed to get 'scmi' device\n");
		return NULL;
	}

	return dev;
}

int scmi_reset_agent(void)
{
	struct udevice *dev;
	const struct scmi_base_reset_agent_in in = {
		.agent_id = 0,
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
		return -EIO;

	rc = scmi_send_and_process_msg(dev, &scmi_msg);
	if (rc)
		return 0;

	return scmi_to_linux_errno(out.status);
}
