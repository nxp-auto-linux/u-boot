// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2020,2022-2023,2026 NXP
 */
#include <errno.h>
#include <log.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>
#include <dm/uclass.h>
#include <linux/printk.h>
#include <s32-cc/scmi_reset_agent.h>

int scmi_reset_agent(void)
{
	struct udevice *agent, *base;
	int ret;

	/* Only one SCMI agent can exist, see scmi_bind_protocols() */
	ret = uclass_first_device_err(UCLASS_SCMI_AGENT, &agent);
	if (ret) {
		pr_err("SCMI agent not found (%d)\n", ret);
		return ret;
	}

	base = scmi_get_protocol(agent, SCMI_PROTOCOL_ID_BASE);
	if (!base) {
		pr_err("SCMI base protocol not found\n");
		return -ENODEV;
	}

	return scmi_base_reset_agent_configuration(base, S32_SCMI_AGENT_OSPM,
					SCMI_BASE_RESET_ALL_ACCESS_PERMISSIONS);
}
