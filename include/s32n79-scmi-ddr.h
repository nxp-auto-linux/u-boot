/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2026 NXP
 */
#ifndef S32N79_SCMI_DDR_H
#define S32N79_SCMI_DDR_H

#include <linux/types.h>

/* misc_call() message IDs */
#define NXP_DDR_CMD_DISCOVER_REGIONS	0
#define NXP_DDR_CMD_PARTIAL_INIT	1
#define NXP_DDR_CMD_LAST_INIT_STATUS	2

#define NXP_DDR_MAX_INSTANCES		2

/* Regions that fit in one DDR_DRAM_DISCOVERY reply on the SMC transport */
#define NXP_DDR_MAX_REGIONS		4

#define NXP_DDR_REQ_STATUS_SUCCESS	0

/**
 * struct nxp_ddr_region - A region reported by DDR_DRAM_DISCOVERY
 * @start: Physical start address.
 * @size: Size in bytes.
 * @instance: DDR (SMMS) instance owning the region.
 * @ecc_enabled: ECC is on, so the region needs initializing.
 */
struct nxp_ddr_region {
	u64  start;
	u64  size;
	u32  instance;
	bool ecc_enabled;
};

/* tx payload of NXP_DDR_CMD_PARTIAL_INIT */
struct nxp_ddr_init_req {
	u64 addr;
	u64 size;
};

/* rx payload of NXP_DDR_CMD_DISCOVER_REGIONS */
struct nxp_ddr_discovery {
	struct nxp_ddr_region regions[NXP_DDR_MAX_REGIONS];
	unsigned int count;
};

/**
 * struct nxp_ddr_instance_status - Our requests on one instance
 * @start: Start of the oldest queued request, or of the last completed one.
 * @size: Size of that request.
 * @pending: Number of requests still queued.
 * @status: NXP_DDR_REQ_STATUS_SUCCESS if the last request succeeded.
 */
struct nxp_ddr_instance_status {
	u64 start;
	u64 size;
	u8  pending;
	u8  status;
};

/* rx payload of NXP_DDR_CMD_LAST_INIT_STATUS */
struct nxp_ddr_init_status {
	struct nxp_ddr_instance_status inst[NXP_DDR_MAX_INSTANCES];
	unsigned int count;
};

#endif /* S32N79_SCMI_DDR_H */
