/* SPDX-License-Identifier: GPL 2.0 */
/*
 *  Copyright (c) 2019 Imagination Technologies Limited
 *  Copyright (c) 2020-2021 Imagination Technologies Limited
 *  Copyright 2018-2023 NXP
 */

#ifndef PFE_HIF_RING_H_
#define PFE_HIF_RING_H_

#include <linux/bitops.h>

#include "pfe_ct.h"

#define RING_LEN	CONFIG_SYS_RX_ETH_BUFFER
#define RING_BD_ALIGN	ARCH_DMA_MINALIGN

#define RING_WBBD_DESC_EN BIT_32(9)

#define HIF_HEADER_SIZE sizeof(struct pfe_ct_hif_tx_hdr) /* same for Rx */

struct pfe_hif_bd {
	u16 seqnum;
	union {
		u16 ctrl;
		struct {
			u16 pkt_int_en : 1; /* LSB */
			u16 cbd_int_en : 1;
			u16 lifm : 1;
			u16 last_bd : 1;
			u16 dir : 1;
			u16 reserved : 10;
			u16 desc_en : 1; /* MSB */
		};
	};
	u16 buflen;
	union {
		u16 rsvd;
		u16 status; /* Due to backwards compatibility */
	};
	u32 data;
	u32 next;
} __packed;

struct pfe_hif_wb_bd {
	union {
		u32 ctrl;
		struct {
			u32 reserved : 4;
			u32 cbd_int_en : 1;
			u32 pkt_int_en : 1;
			u32 lifm : 1;
			u32 last_bd : 1;
			u32 dir : 1;
			u32 desc_en : 1;
			u32 reserved1 : 1;
			u32 reserved2 : 21;
		};
	};
	u16 buflen;
	u16 seqnum;
} __packed;

struct pfe_hif_ring {
	struct pfe_hif_bd __iomem *bd;
	struct pfe_hif_wb_bd __iomem *wb_bd;
	u32 write_idx;
	u32 read_idx;
	bool is_rx;
};

static inline u32 pfe_hif_get_buffer_idx(u32 idx)
{
	return idx % RING_LEN;
}

static inline struct pfe_hif_bd *pfe_hif_get_bd(struct pfe_hif_ring *ring, u32 idx)
{
	return &ring->bd[idx];
}

static inline struct pfe_hif_wb_bd *pfe_hif_get_wb_bd(struct pfe_hif_ring *ring, u32 idx)
{
	return &ring->wb_bd[idx];
}

static inline u16 pfe_hif_get_bd_desc_en(struct pfe_hif_bd __iomem *bd)
{
	return readw(&bd->ctrl) & (u16)BIT_32(15);
}

#endif /* PFE_HIF_RING_H_ */
