// SPDX-License-Identifier: GPL 2.0
/*
 *  Copyright (c) 2019 Imagination Technologies Limited
 *  Copyright (c) 2020-2021 Imagination Technologies Limited
 *  Copyright 2018-2023 NXP
 */

#include <common.h>
#include <dm/device_compat.h>

#include "pfe_hw.h"
#include "internal/pfe_hw_priv.h"

#define RPC_PFE_IF_LOCK 0xBE
#define RPC_PFE_IF_UNLOCK 0xBF
#define RPC_PFE_IF_DISABLE 0x66

#define IHC_BUFFER_SIZE 128
#define MINIMAL_IHC_FRAME_SIZE 68

#define IDEX_RPC_RETRIES 3

#define FLUSH_COUNT_LIMIT (RING_LEN * 4)

enum pfe_idex_frame_type {
	IDEX_FRAME_CTRL_REQUEST,
	IDEX_FRAME_CTRL_RESPONSE,
};

struct pfe_idex_frame_header {
	u8 dst_phy_if;
	u8 type;
} __packed;

_ct_assert(sizeof(struct pfe_idex_frame_header) == 2);

enum pfe_idex_request_type {
	IDEX_MASTER_DISCOVERY,
	IDEX_RPC,
};

struct pfe_idex_request {
	u32 seqnum;
	u8 type;
	u8 dst_phy_id;
	u8 state;
	u8 padding[30U];
} __packed;

_ct_assert(sizeof(struct pfe_idex_request) == 37);

struct pfe_idex_msg_rpc {
	u32 rpc_id;
	int rpc_ret;
	u16 plen;
} __packed;

struct pfe_idex_rpc_req_hdr {
	struct pfe_idex_frame_header idex_hdr;
	struct pfe_idex_request idex_req;
	struct pfe_idex_msg_rpc idex_msg;
} __packed;

_ct_assert(sizeof(struct pfe_idex_rpc_req_hdr) == 49);

struct pfe_idex_rpc_req_hdr_if_mode {
	struct pfe_idex_rpc_req_hdr idex_rpc_req_hdr;
	u32 phy_if_id;
} __packed;

_ct_assert(sizeof(struct pfe_idex_rpc_req_hdr_if_mode) == 53);

struct pfe_idex_response {
	u32 seqnum;
	u8 type;
	u16 plen;
} __packed;

_ct_assert(sizeof(struct pfe_idex_response) == 7);

struct pfe_idex_resp {
	struct pfe_ct_hif_rx_hdr rx_hdr;
	struct pfe_idex_frame_header idex_hdr;
	struct pfe_idex_response idex_resp;
	struct pfe_idex_msg_rpc idex_msg;
} __packed;

_ct_assert(sizeof(struct pfe_idex_resp) == 35);

static inline phys_addr_t pfe_hw_block_pa_of(phys_addr_t addr, u32 pa_off)
{
	if (check_uptr_overflow(addr, (uintptr_t)pa_off))
		return 0U;

	return addr + pa_off;
}

static inline phys_addr_t pfe_hw_block_pa(struct pfe_hw *pfe, u32 pa_off)
{
	return pfe_hw_block_pa_of(pfe->cbus_baseaddr, pa_off);
}

/* external API */
int pfe_hw_hif_chnl_hw_init(struct pfe_hw_ext *ext, const struct pfe_hw_cfg *cfg)
{
	return pfe_hw_hif_init(&ext->hw->hif, ext->hw->hif_base, ext->hw->hif_chnl, cfg);
}

int pfe_hw_init(struct pfe_hw_ext *ext, const struct pfe_hw_cfg *hw_cfg)
{
	struct pfe_hw *pfe;
	int ret = 0;

	pfe = kzalloc(sizeof(*pfe), GFP_KERNEL);
	if (!pfe)
		return -ENOMEM;

	ext->hw = pfe;
	pfe->cfg = hw_cfg;

	/* Map CBUS address space */
	pfe->cbus_baseaddr = hw_cfg->cbus_base;
	pfe->base = pfe_hw_phys_addr(hw_cfg->cbus_base);
	if (!pfe->base) {
		dev_err(hw_cfg->dev, "Can't map PPFE CBUS\n");
		ret = -ENOMEM;
		goto exit;
	}

	pfe->hif_base = pfe_hw_phys_addr(pfe_hw_block_pa(pfe, CBUS_HIF_BASE_ADDR));
	if (!pfe->hif_base) {
		dev_err(hw_cfg->dev, "Can't get PFE HIF base address\n");
		ret = -EINVAL;
		goto exit;
	}

	pfe->bdr_buffers_va = pfe_hw_phys_addr(hw_cfg->bdrs_addr);
	pfe->bdr_buffers_size = hw_cfg->bdrs_size;

	/* Create HW components */
	pfe->hif_chnl = hw_cfg->hif_chnl_id;
	pfe->ihc_hif_id = hw_cfg->ihc_hif_id;
	pfe->master_hif_id = hw_cfg->master_hif_id;

	return 0;

exit:
	pfe_hw_remove(ext);
	return ret;
}

void pfe_hw_remove(struct pfe_hw_ext *ext)
{
	struct pfe_hw *pfe = ext->hw;

	if (!pfe)
		return;

	pfe->cbus_baseaddr = 0x0ULL;
	kfree(pfe);
	ext->hw = NULL;
}

void pfe_hw_print_stats(struct pfe_hw_ext *ext)
{
	printf("HIF statistics\n");

	if (ext->hw_chnl)
		pfe_hw_chnl_print_stats(ext->hw_chnl);
	else
		printf("Not available, HIF channel is not created yet\n");
}

int pfe_hw_grace_reset(struct pfe_hw_ext *ext)
{
	int ret = 0;

	printf("HIF graceful reset\n");

	return ret;
}
