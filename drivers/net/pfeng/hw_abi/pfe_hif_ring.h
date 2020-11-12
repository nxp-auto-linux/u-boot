/* SPDX-License-Identifier: GPL 2.0 */
/*
 *  Copyright (c) 2020 Imagination Technologies Limited
 *  Copyright 2018-2020 NXP
 */

#ifndef PUBLIC_PFE_HIF_RING_H_
#define PUBLIC_PFE_HIF_RING_H_

#include <asm/io.h>

#define RING_LEN      PFE_HIF_RING_CFG_LENGTH
#define RING_LEN_MASK (PFE_HIF_RING_CFG_LENGTH - 1U)
#define RING_BD_ALIGN (ARCH_DMA_MINALIGN)
#define PFE_BUF_SIZE (2048U)

#define RING_BD_DESC_EN(ctrl) (readw(&(ctrl)) & BIT(15))
#define RING_WBBD_DESC_EN(ctrl) (readl(&(ctrl)) & BIT(9))

__packed struct pfe_hif_bd {
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
};

__packed struct pfe_hif_wb_bd {
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
};

__packed struct pfe_hif_ring {
	struct pfe_hif_bd *bd;
	struct pfe_hif_wb_bd *wb_bd;
	u32 write_idx;
	u32 read_idx;
	bool is_rx;
	void *mem;
};

#endif /* PUBLIC_PFE_HIF_RING_H_ */

/** @}*/
/** @}*/
