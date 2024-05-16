// SPDX-License-Identifier: GPL 2.0
/*
 *  Copyright (c) 2019 Imagination Technologies Limited
 *  Copyright (c) 2020-2021 Imagination Technologies Limited
 *  Copyright 2018-2024 NXP
 */

#include <common.h>
#include <dm/device_compat.h>
#include <linux/delay.h>

#include "pfe_hw.h"
#include "internal/pfe_hw_priv.h"

#define RPC_PFE_IF_LOCK 0xBE
#define RPC_PFE_IF_UNLOCK 0xBF
#define RPC_PFE_IF_ENABLE 0x65
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

static u32 idex_seqnum;

static inline u32 get_idex_seqnum(void)
{
	return idex_seqnum;
}

static inline void advance_idex_seqnum(void)
{
	idex_seqnum++;
}

static int idex_rpc(struct pfe_hw_ext *ext, void *ihc_frame, u32 cmd_id, u8 length, int *rpc_ret)
{
	struct pfe_idex_rpc_req_hdr *rpc_req = (void *)ihc_frame;
	struct pfe_idex_resp *rpc_resp;
	uchar *rec_buf = NULL;
	int retry = 1500;
	int ret;
	int rec_cnt;
	bool valid_rpc_resp = false;
	u32 frame_size;
	u32 seqnum = get_idex_seqnum();
	u8 master_hif_id = ext->hw->master_hif_id;
	u8 ihc_hif_id = ext->hw->ihc_hif_id;

	rpc_req->idex_hdr.dst_phy_if = master_hif_id;
	rpc_req->idex_hdr.type = IDEX_FRAME_CTRL_REQUEST;

	rpc_req->idex_req.seqnum = htonl(seqnum);
	rpc_req->idex_req.type = IDEX_RPC;
	rpc_req->idex_req.dst_phy_id = master_hif_id;

	rpc_req->idex_msg.rpc_id = htonl(cmd_id);
	rpc_req->idex_msg.rpc_ret = htonl(0);
	rpc_req->idex_msg.plen = htons(length);

	frame_size = length;
	if (frame_size < MINIMAL_IHC_FRAME_SIZE)
		frame_size = MINIMAL_IHC_FRAME_SIZE;

	ret = pfe_hw_chnl_xmit(ext->hw_chnl, true, master_hif_id, ihc_frame, frame_size);
	if (ret)
		return ret;

	/* Wait when submitted RPC command to the Master driver */
	mdelay(500);

	while (retry-- > 0) {
		/* Unused parameters flags and phyif, call with zeroes */
		ret = pfe_hw_chnl_receive(ext->hw_chnl, 0, false, 0, &rec_buf);
		if (ret < 0 || !rec_buf)
			continue;

		rec_cnt = ret;
		if (rec_cnt >= sizeof(struct pfe_idex_resp)) {
			rpc_resp = (void *)rec_buf;

			if (ntohl(rpc_resp->rx_hdr.flags) & HIF_RX_IHC &&
			    rpc_resp->rx_hdr.i_phy_if == master_hif_id &&
			    rpc_resp->idex_hdr.dst_phy_if == ihc_hif_id &&
			    rpc_resp->idex_hdr.type == IDEX_FRAME_CTRL_RESPONSE &&
			    ntohl(rpc_resp->idex_resp.seqnum) == seqnum &&
			    rpc_resp->idex_resp.type == IDEX_RPC &&
			    ntohs(rpc_resp->idex_resp.plen) == sizeof(struct pfe_idex_msg_rpc) &&
			    ntohl(rpc_resp->idex_msg.rpc_id) == cmd_id) {
				valid_rpc_resp = true;
				*rpc_ret = ntohs(rpc_resp->idex_msg.rpc_ret);
			}
		}

		ret = pfe_hw_chnl_free_pkt(ext->hw_chnl, rec_buf, rec_cnt);
		if (ret)
			return ret;

		if (valid_rpc_resp)
			return 0;
	}

	return -EAGAIN;
}

static int do_idex_rpc(struct pfe_hw_ext *ext, void *ihc_frame, u32 cmd_id, u8 length)
{
	int rpc_ret;
	int ret;
	int retry = IDEX_RPC_RETRIES;

	advance_idex_seqnum();
	while (retry-- > 0) {
		ret = idex_rpc(ext, ihc_frame, cmd_id, length, &rpc_ret);
		if (!ret) {
			if (!rpc_ret)
				return 0;

			dev_warn(ext->hw->cfg->dev,
				 "RPC command %u execution failed with error %d\n",
				 cmd_id, rpc_ret);
		}
	}

	return ret;
}

static int send_idex_rpc_req(struct pfe_hw_ext *ext, void *ihc_frame, u32 cmd_id)
{
	u8 length = sizeof(struct pfe_idex_rpc_req_hdr);

	memset(ihc_frame, 0, length);

	return do_idex_rpc(ext, ihc_frame, cmd_id, length);
}

static int send_idex_rpc_req_if_mode(struct pfe_hw_ext *ext, void *ihc_frame, u32 cmd_id)
{
	u8 length = sizeof(struct pfe_idex_rpc_req_hdr_if_mode);

	struct pfe_idex_rpc_req_hdr_if_mode *rpc_req_hdr_if_mode = ihc_frame;

	memset(ihc_frame, 0, length);
	rpc_req_hdr_if_mode->phy_if_id = htonl(ext->hw->ihc_hif_id);

	return do_idex_rpc(ext, ihc_frame, cmd_id, length);
}

static int send_idex_if_cmd(struct pfe_hw_ext *ext, u32 cmd_id)
{
	void *ihc_frame;
	int ret, ret_unlock;

	ihc_frame = kzalloc(IHC_BUFFER_SIZE, GFP_KERNEL);
	if (!ihc_frame)
		return -ENOMEM;

	/* Send interface database lock command */
	ret = send_idex_rpc_req(ext, ihc_frame, RPC_PFE_IF_LOCK);
	if (ret) {
		dev_err(ext->hw->cfg->dev,
			"Failed to execute Lock RPC command: %d\n", ret);
		goto exit;
	}

	/* Send command to the master */
	ret = send_idex_rpc_req_if_mode(ext, ihc_frame, cmd_id);
	if (ret)
		dev_err(ext->hw->cfg->dev,
			"Failed to execute IF RPC command (%u): %d\n", cmd_id, ret);
		/* Once RPC_PFE_IF_LOCK is executed successfully then
		 * the RPC_PFE_IF_UNLOCK must be used, do not exit here.
		 */

	/* Send interface database unlock command */
	ret_unlock = send_idex_rpc_req(ext, ihc_frame, RPC_PFE_IF_UNLOCK);
	if (ret_unlock)
		dev_err(ext->hw->cfg->dev,
			"Failed to execute Unlock RPC command: %d\n", ret_unlock);

	if (!ret && ret_unlock)
		ret = ret_unlock;
exit:
	kfree(ihc_frame);

	return ret;
}

static int hif_channel_grace_reset(struct pfe_hw_ext *ext)
{
	uchar *rec_buf;
	u32 rx_bdp_fifo_len;
	int flush_count = 0;
	int ret;

	pfe_hw_hif_chnl_enable(ext->hw_chnl);

	ret = send_idex_if_cmd(ext, RPC_PFE_IF_DISABLE);
	if (ret)
		goto exit;

	do {
		rx_bdp_fifo_len = pfe_hw_chnl_rx_bdp_fifo_len(ext->hw_chnl);
		if (!rx_bdp_fifo_len)
			break;

		ret = pfe_hw_chnl_xmit_dummy(ext->hw_chnl);
		if (ret) {
			dev_err(ext->hw->cfg->dev,
				"Failed to send dummy frame to IHC channel: %d\n", ret);
			goto exit;
		}

		udelay(500);

		rec_buf = NULL;
		/* Unused parameters flags and phyif, call with zeroes */
		ret = pfe_hw_chnl_receive(ext->hw_chnl, 0, false, 0, &rec_buf);
		if (ret < 0 || !rec_buf)
			break;
	} while (flush_count < FLUSH_COUNT_LIMIT);

	if (rx_bdp_fifo_len) {
		dev_err(ext->hw->cfg->dev,
			"Failed to flush IHC channel within %d ierations\n", FLUSH_COUNT_LIMIT);
		ret = -EINVAL;
	} else {
		ret = 0;
		ext->in_grace_reset = true;
	}

exit:
	pfe_hw_hif_chnl_disable(ext->hw_chnl);

	return ret;
}

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

	if (ext->hw_chnl) {
		if (ext->in_grace_reset) {
			printf("PFE is in the grace reset state already\n");
			return ret;
		}

		ret = hif_channel_grace_reset(ext);
		if (!ret)
			printf("Performed graceful PFE reset\n");
		else
			printf("Graceful PFE reset failed with error code %d\n", ret);
	} else {
		printf("No action taken, HIF channel is not created yet\n");
	}

	return ret;
}

int pfe_hw_hif_enable(struct pfe_hw_ext *ext)
{
	int ret;

	if (ext->hw_chnl_enabled)
		return 0;

	ret = send_idex_if_cmd(ext, RPC_PFE_IF_ENABLE);
	if (!ret)
		ext->hw_chnl_enabled = true;
	else
		dev_err(ext->hw->cfg->dev, "Failed to enable HIF channel: %d\n", ret);

	return ret;
}
