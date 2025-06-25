// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2025 NXP
 */

#include <dm.h>
#include <misc.h>
#include <s32n79-scmi-misc.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>

/*
 * Identifiers of the NXP Misc SCMI Protocol commands
 */
enum scmi_misc_message_id {
	MISC_SCMI_GET_SOC_INFO = 0x7,
};

static int scmi_send_soc_info_msg(struct udevice *dev, void *buf)
{
	struct {
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
	} __packed response;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_NXP_MISC,
		.message_id	= MISC_SCMI_GET_SOC_INFO,
		.in_msg		= NULL,
		.in_msg_sz	= 0,
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response),
	};
	int ret;

	/*
	 * Assumption is that 'devm_scmi_process_msg' searches for the related
	 * SCMI AGENT parent node.
	 */
	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	ret = scmi_to_linux_errno(response.status);
	if (ret)
		return ret;

	memcpy(buf, &response, sizeof(response));

	return response.status;
}

static int scmi_misc_read(struct udevice *dev,
			  int offset, void *buf, int size)
{
	if (offset == S32N79_SOC_INFO_OFFSET &&
	    size == S32N79_SOC_INFO_SIZE)
		return scmi_send_soc_info_msg(dev, buf);

	return -EOPNOTSUPP;
}

static const struct misc_ops scmi_misc_ops = {
	.read = scmi_misc_read,
};

U_BOOT_DRIVER(scmi_misc) = {
	.name = "scmi_misc",
	.id = UCLASS_MISC,
	.ops = &scmi_misc_ops,
	.flags = DM_FLAG_PRE_RELOC,
};
