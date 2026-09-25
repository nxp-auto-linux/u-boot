// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2025-2026 NXP
 */

#include <dm.h>
#include <misc.h>
#include <s32n79-scmi-misc.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>
#include <linux/types.h>

/*
 * Identifiers of the NXP Misc SCMI Protocol commands
 */
enum scmi_misc_message_id {
	MISC_SCMI_GET_SOC_INFO = 0x7,
};

struct scmi_nxp_misc_get_soc_info_p2a {
	s32 status;
	u32 prod_status_and_brand;
	u32 family;
	u32 series;
	u32 arm_cores_and_sram;
	u32 device_variant;
	u32 device_options;
	u32 security;
	u32 arm_core_speeds;
	u32 fab_and_mask_rev;
	u32 temp_range;
	u32 package_code;
	u32 shipping_method;
} __packed;

static int scmi_send_soc_info_msg(struct udevice *dev,
				  struct scmi_nxp_misc_get_soc_info_p2a *response)
{
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_NXP_MISC,
		.message_id	= MISC_SCMI_GET_SOC_INFO,
		.in_msg		= NULL,
		.in_msg_sz	= 0,
		.out_msg	= (u8 *)response,
		.out_msg_sz	= sizeof(*response),
	};
	int ret;

	/*
	 * Assumption is that 'devm_scmi_process_msg' searches for the related
	 * SCMI AGENT parent node.
	 */
	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	return scmi_to_linux_errno(response->status);
}

static int scmi_misc_call(struct udevice *dev, int msgid,
			  void *tx_msg, int tx_size,
			  void *rx_msg, int rx_size)
{
	switch (msgid) {
	case NXP_MISC_CMD_GET_SOC_INFO:
		if (!rx_msg ||
		    rx_size != sizeof(struct scmi_nxp_misc_get_soc_info_p2a))
			return -EINVAL;
		return scmi_send_soc_info_msg(dev, rx_msg);
	default:
		return -EOPNOTSUPP;
	}
}

static const struct misc_ops scmi_misc_ops = {
	.call = scmi_misc_call,
};

U_BOOT_DRIVER(scmi_misc) = {
	.name = "scmi_misc",
	.id = UCLASS_MISC,
	.ops = &scmi_misc_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
