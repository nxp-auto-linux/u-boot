// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2023 NXP
 */

#include <dm.h>
#include <log.h>
#include <misc.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>
#include <dm/device.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/kernel.h>

enum scmi_nvmem_message_id {
	SCMI_NVMEM_PROTOCOL_ATTRIBUTES = 0x1,
	SCMI_NVMEM_PROTOCOL_MESSAGE_ATTRIBUTES = 0x2,
	SCMI_NVMEM_READ_CELL = 0x3,
};

static int scmi_nvmem_read(struct udevice *dev, int offset,
			   void *buf, int size)
{
	struct {
		s32 status;
		u32 value;
		u32 bytes_read;
	} response;
	struct {
		u32 offset;
		u32 bytes;
	} request;
	struct scmi_msg msg = {
		.protocol_id = SCMI_PROTOCOL_ID_NVMEM,
		.message_id = SCMI_NVMEM_READ_CELL,
		.in_msg = (u8 *)&request,
		.in_msg_sz = sizeof(request),
		.out_msg = (u8 *)&response,
		.out_msg_sz = sizeof(response),
	};
	int ret;

	request.offset = (u32)offset;
	request.bytes = (u32)size;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret) {
		pr_err("Error reading cell %d, size %d: %d!\n", offset, size,
		       ret);
		return ret;
	}

	ret = scmi_to_linux_errno(response.status);
	if (ret) {
		pr_err("Error reading cell %d, size %d: %d!\n", offset, size,
		       ret);
		return -EIO;
	}

	memcpy(buf, &response.value, response.bytes_read);

	return response.bytes_read;
}

static const struct misc_ops scmi_nvmem_ops = {
	.read = scmi_nvmem_read,
};

static int scmi_nvmem_get_num_cells(struct udevice *scmi_dev,
				    u32 *num_cells)
{
	struct {
		s32 status;
		u32 attributes;
	} response;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_NVMEM,
		.message_id	= SCMI_NVMEM_PROTOCOL_ATTRIBUTES,
		.in_msg		= NULL,
		.in_msg_sz	= 0,
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response),
	};
	int ret;

	ret = devm_scmi_process_msg(scmi_dev, &msg);
	if (ret) {
		pr_err("Error getting number of cells: %d!\n", ret);
		return ret;
	}

	*num_cells = response.attributes;

	return scmi_to_linux_errno(response.status);
}

static int scmi_nvmem_probe(struct udevice *dev)
{
	u32 num_cells = 0;
	int ret;

	ret = scmi_nvmem_get_num_cells(dev, &num_cells);
	if (ret)
		return ret;

	if (!num_cells) {
		pr_err("Zero NVMEM cells exposed by SCMI Platform!\n");
		return -EINVAL;
	}

	return 0;
}

U_BOOT_DRIVER(scmi_nvmem) = {
	.name = "scmi_nvmem",
	.id = UCLASS_MISC,
	.ops = &scmi_nvmem_ops,
	.probe = scmi_nvmem_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
