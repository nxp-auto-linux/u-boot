// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2026 NXP
 *
 * NXP DDR SCMI vendor protocol (0x8B) driver, DDR API v0.3. U-Boot only uses
 * the synchronous form of DDR_PARTIAL_DRAM_INIT.
 */

#include <dm.h>
#include <misc.h>
#include <s32n79-scmi-ddr.h>
#include <scmi_agent.h>
#include <scmi_protocols.h>
#include <dm/device_compat.h>
#include <linux/bitfield.h>
#include <linux/bitops.h>
#include <linux/kernel.h>
#include <linux/types.h>

enum scmi_nxp_ddr_message_id {
	DDR_SCMI_DRAM_DISCOVERY			= 0x3,
	DDR_SCMI_PARTIAL_DRAM_INIT		= 0x4,
	DDR_SCMI_GET_LAST_INIT_REQ_STATUS	= 0x5,
};

/* PROTOCOL_VERSION reply: [31:16] major, [15:0] minor. */
#define DDR_VERSION_MAJOR		GENMASK(31, 16)
#define DDR_VERSION_MINOR		GENMASK(15, 0)

#define DDR_SUPPORTED_VERSION_MAJOR	1

struct scmi_nxp_ddr_discovery_a2p {
	u32 region_index;
} __packed;

struct scmi_nxp_ddr_region_entry {
	u32 start_lo;
	u32 start_hi;
	u32 end_lo;
	u32 end_hi;
	u32 instance_id;
} __packed;

struct scmi_nxp_ddr_discovery_p2a {
	s32 status;
	u32 ddr_cfg_info;	/* bit N: ECC enabled on instance N */
	u32 number_region;	/* [7:0] returned, [15:8] remaining */
	struct scmi_nxp_ddr_region_entry regions[NXP_DDR_MAX_REGIONS];
} __packed;

#define DDR_DISC_NUM_RETURNED		GENMASK(7, 0)
#define DDR_DISC_NUM_REMAINING		GENMASK(15, 8)
#define DDR_DISC_INSTANCE_MASK		GENMASK(1, 0)

struct scmi_nxp_ddr_partial_init_a2p {
	u32 flags;
	u32 start_low;
	u32 start_high;
	u32 size_low;
	u32 size_high;
} __packed;

struct scmi_nxp_ddr_partial_init_p2a {
	s32 status;
} __packed;

struct scmi_nxp_ddr_inst_status_entry {
	u32 hdr;	/* [7:0] pending requests, [15:8] request status */
	u32 start_lo;
	u32 start_hi;
	u32 size_lo;
	u32 size_hi;
} __packed;

struct scmi_nxp_ddr_last_status_p2a {
	s32 status;
	u32 num_instances;
	struct scmi_nxp_ddr_inst_status_entry inst[NXP_DDR_MAX_INSTANCES];
} __packed;

#define DDR_LAST_STATUS_PENDING		GENMASK(7, 0)
#define DDR_LAST_STATUS_REQ_STATUS	GENMASK(15, 8)

static int scmi_ddr_discover(struct udevice *dev,
			     struct nxp_ddr_discovery *out)
{
	struct scmi_nxp_ddr_discovery_a2p request = { .region_index = 0 };
	struct scmi_nxp_ddr_discovery_p2a response = { 0 };
	const struct scmi_nxp_ddr_region_entry *e;
	struct nxp_ddr_region *r;
	unsigned int i, returned;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_NXP_DDR,
		.message_id	= DDR_SCMI_DRAM_DISCOVERY,
		.in_msg		= (u8 *)&request,
		.in_msg_sz	= sizeof(request),
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response),
	};
	u64 start, end;
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	ret = scmi_to_linux_errno(response.status);
	if (ret)
		return ret;

	returned = FIELD_GET(DDR_DISC_NUM_RETURNED, response.number_region);
	if (returned > NXP_DDR_MAX_REGIONS ||
	    msg.out_msg_sz < offsetof(struct scmi_nxp_ddr_discovery_p2a,
				      regions[returned])) {
		dev_err(dev, "malformed DDR discovery reply (%u regions)\n",
			returned);
		return -EPROTO;
	}

	/* RegionIndex pages per instance, so the rest cannot be fetched. */
	if (FIELD_GET(DDR_DISC_NUM_REMAINING, response.number_region)) {
		dev_err(dev, "DDR discovery has more regions than fit in one reply\n");
		return -E2BIG;
	}

	for (i = 0; i < returned; i++) {
		e = &response.regions[i];
		r = &out->regions[i];
		start = ((u64)e->start_hi << 32) | e->start_lo;
		end = ((u64)e->end_hi << 32) | e->end_lo;

		r->instance = e->instance_id & DDR_DISC_INSTANCE_MASK;
		if (end < start || r->instance >= NXP_DDR_MAX_INSTANCES) {
			dev_err(dev, "malformed DDR discovery region %u\n", i);
			return -EPROTO;
		}

		r->start = start;
		r->size = end - start + 1;
		r->ecc_enabled = response.ddr_cfg_info & BIT(r->instance);
	}
	out->count = returned;

	return 0;
}

static int scmi_ddr_partial_init(struct udevice *dev,
				 const struct nxp_ddr_init_req *req)
{
	struct scmi_nxp_ddr_partial_init_a2p request = {
		.flags      = 0,
		.start_low  = lower_32_bits(req->addr),
		.start_high = upper_32_bits(req->addr),
		.size_low   = lower_32_bits(req->size),
		.size_high  = upper_32_bits(req->size),
	};
	struct scmi_nxp_ddr_partial_init_p2a response = { 0 };
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_NXP_DDR,
		.message_id	= DDR_SCMI_PARTIAL_DRAM_INIT,
		.in_msg		= (u8 *)&request,
		.in_msg_sz	= sizeof(request),
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response),
	};
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	/* IN_USE: the instance's queue is full, which clears on its own. */
	ret = scmi_to_linux_errno(response.status);
	if (ret == -EADDRINUSE)
		return -EBUSY;

	return ret;
}

static int scmi_ddr_last_status(struct udevice *dev,
				struct nxp_ddr_init_status *out)
{
	struct scmi_nxp_ddr_last_status_p2a response = { 0 };
	const struct scmi_nxp_ddr_inst_status_entry *e;
	struct nxp_ddr_instance_status *s;
	struct scmi_msg msg = {
		.protocol_id	= SCMI_PROTOCOL_ID_NXP_DDR,
		.message_id	= DDR_SCMI_GET_LAST_INIT_REQ_STATUS,
		.in_msg		= NULL,
		.in_msg_sz	= 0,
		.out_msg	= (u8 *)&response,
		.out_msg_sz	= sizeof(response),
	};
	unsigned int i, count;
	int ret;

	ret = devm_scmi_process_msg(dev, &msg);
	if (ret)
		return ret;

	ret = scmi_to_linux_errno(response.status);
	if (ret)
		return ret;

	count = response.num_instances;
	if (count > NXP_DDR_MAX_INSTANCES ||
	    msg.out_msg_sz < offsetof(struct scmi_nxp_ddr_last_status_p2a,
				      inst[count])) {
		dev_err(dev, "malformed DDR init status reply (%u instances)\n",
			count);
		return -EPROTO;
	}

	for (i = 0; i < count; i++) {
		e = &response.inst[i];
		s = &out->inst[i];

		s->pending = FIELD_GET(DDR_LAST_STATUS_PENDING, e->hdr);
		s->status = FIELD_GET(DDR_LAST_STATUS_REQ_STATUS, e->hdr);
		s->start = ((u64)e->start_hi << 32) | e->start_lo;
		s->size = ((u64)e->size_hi << 32) | e->size_lo;
	}
	out->count = count;

	return 0;
}

static int scmi_ddr_call(struct udevice *dev, int msgid,
			 void *tx_msg, int tx_size,
			 void *rx_msg, int rx_size)
{
	switch (msgid) {
	case NXP_DDR_CMD_DISCOVER_REGIONS:
		if (!rx_msg || rx_size != sizeof(struct nxp_ddr_discovery))
			return -EINVAL;
		return scmi_ddr_discover(dev, rx_msg);
	case NXP_DDR_CMD_PARTIAL_INIT:
		if (!tx_msg || tx_size != sizeof(struct nxp_ddr_init_req))
			return -EINVAL;
		return scmi_ddr_partial_init(dev, tx_msg);
	case NXP_DDR_CMD_LAST_INIT_STATUS:
		if (!rx_msg || rx_size != sizeof(struct nxp_ddr_init_status))
			return -EINVAL;
		return scmi_ddr_last_status(dev, rx_msg);
	default:
		return -EOPNOTSUPP;
	}
}

static const struct misc_ops scmi_ddr_ops = {
	.call = scmi_ddr_call,
};

static int scmi_ddr_probe(struct udevice *dev)
{
	u32 version, major, minor;
	int ret;

	ret = scmi_generic_protocol_version(dev, SCMI_PROTOCOL_ID_NXP_DDR,
					    &version);
	if (ret)
		return ret;

	major = FIELD_GET(DDR_VERSION_MAJOR, version);
	minor = FIELD_GET(DDR_VERSION_MINOR, version);

	/* Minor revisions stay compatible; a new major does not. */
	if (major != DDR_SUPPORTED_VERSION_MAJOR) {
		dev_err(dev, "unsupported NXP DDR protocol version %u.%u\n",
			major, minor);
		return -EPROTONOSUPPORT;
	}

	return 0;
}

U_BOOT_DRIVER(scmi_ddr) = {
	.name = "scmi_ddr",
	.id = UCLASS_MISC,
	.ops = &scmi_ddr_ops,
	.probe = scmi_ddr_probe,
	.flags = DM_FLAG_PRE_RELOC,
};
