/* SPDX-License-Identifier: GPL 2.0 */
/*
 *  Copyright (c) 2019 Imagination Technologies Limited
 *  Copyright 2018-2023 NXP
 */

/* Common types (s32g).
 * This header contains data types shared by host as well
 * as PFE firmware.
 */

#ifndef PFE_CT_H_
#define PFE_CT_H_

#include <linux/bitops.h>
#include <linux/types.h>

#define _ct_assert(param) _Static_assert(param, "Error in " __FILE__)

enum pfe_ct_phy_if_id {
	/*	HW interfaces */
	PFE_PHY_IF_ID_EMAC0 = 0U,
	PFE_PHY_IF_ID_EMAC1 = 1U,
	PFE_PHY_IF_ID_EMAC2 = 2U,
	PFE_PHY_IF_ID_HIF = 3U,
	PFE_PHY_IF_ID_HIF_NOCPY = 4U,
	/*	UTIL PE - FW internal use */
	PFE_PHY_IF_ID_UTIL = 5U,
	/*	Synthetic interfaces */
	PFE_PHY_IF_ID_HIF0 = 6U,
	PFE_PHY_IF_ID_HIF1 = 7U,
	PFE_PHY_IF_ID_HIF2 = 8U,
	PFE_PHY_IF_ID_HIF3 = 9U,
	/*	Internals */
	PFE_PHY_IF_ID_MAX = PFE_PHY_IF_ID_HIF3,
	PFE_PHY_IF_ID_INVALID
};

/* Tx header flags*/
#define HIF_TX_INJECT		BIT_32(6)
#define HIF_TX_IHC		BIT_32(7)

/* HIF TX packet header */
struct pfe_ct_hif_tx_hdr {
	u8 flags;
	u8 queue;
	u8 chid;
	u8 reserved1;
	u16 reserved2;
	u16 refnum;
	u32 e_phy_ifs;
	u32 cookie;
} __aligned(4) __packed;

/* Structure has to be 16B in length for correct HW functionality */
_ct_assert(sizeof(struct pfe_ct_hif_tx_hdr) == 16);

/* Rx header flags */
#define HIF_RX_IHC		BIT_32(8)

struct pfe_ct_hif_rx_hdr {
	u32 flags;
	u8 i_phy_if;
	u8 i_log_if;
	u8 queue;
	u8 reserved;
	u32 rx_timestamp_ns;
	u32 rx_timestamp_s;
} __packed;

/* Structure has to be 16B in length for correct HW functionality */
_ct_assert(sizeof(struct pfe_ct_hif_rx_hdr) == 16);

#endif /* PFE_CT_H_ */
