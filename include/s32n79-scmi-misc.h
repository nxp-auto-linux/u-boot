/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2025-2026 NXP
 */
#ifndef S32N79_SCMI_MISC_H
#define S32N79_SCMI_MISC_H

#include <linux/types.h>

/* misc_call() message IDs for the NXP Misc SCMI driver. */
#define NXP_MISC_CMD_INIT_DDR_PARTIAL	0
#define NXP_MISC_CMD_GET_SOC_INFO	1

/*
 * Request payload for NXP_MISC_CMD_INIT_DDR_PARTIAL (passed as misc_call()
 * tx_msg). The driver marshals these fields into the SCMI wire format.
 */
struct nxp_misc_ddr_init_req {
	u64 addr;
	u64 size;
	u32 instance_id;
} __packed;

#endif /* S32N79_SCMI_MISC_H */

