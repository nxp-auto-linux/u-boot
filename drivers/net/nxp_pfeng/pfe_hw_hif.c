// SPDX-License-Identifier: GPL 2.0
/*
 *  Copyright (c) 2019 Imagination Technologies Limited
 *  Copyright (c) 2020-2021 Imagination Technologies Limited
 *  Copyright 2018-2023 NXP
 */

#include <common.h>
#include <net.h>
#include <asm/system.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#include "pfe_hw.h"
#include "internal/pfe_hw_priv.h"

#define PFE_HW_BD_TIMEOUT_US 1000UL
#define HIF_SOFT_RESET_TIMEOUT_US 1000000UL

static struct pfe_ct_hif_tx_hdr tx_header;

static const struct pfe_hw_stats pfe_stats_hif[] = {
	{"HIF_TX_STATE", HIF_TX_STATE },
	{"HIF_TX_ACTV", HIF_TX_ACTV },
	{"HIF_TX_CURR_CH_NO", HIF_TX_CURR_CH_NO },
	{"HIF_DXR_TX_FIFO_CNT", HIF_DXR_TX_FIFO_CNT },
	{"HIF_TX_CTRL_WORD_FIFO_CNT1", HIF_TX_CTRL_WORD_FIFO_CNT1 },
	{"HIF_TX_CTRL_WORD_FIFO_CNT2", HIF_TX_CTRL_WORD_FIFO_CNT2 },
	{"HIF_TX_BVALID_FIFO_CNT", HIF_TX_BVALID_FIFO_CNT },
	{"HIF_TX_PKT_CNT1", HIF_TX_PKT_CNT1 },
	{"HIF_TX_PKT_CNT2", HIF_TX_PKT_CNT2 },
	{"HIF_RX_STATE", HIF_RX_STATE },
	{"HIF_RX_ACTV", HIF_RX_ACTV },
	{"HIF_RX_CURR_CH_NO", HIF_RX_CURR_CH_NO },
	{"HIF_DXR_RX_FIFO_CNT", HIF_DXR_RX_FIFO_CNT },
	{"HIF_RX_CTRL_WORD_FIFO_CNT", HIF_RX_CTRL_WORD_FIFO_CNT },
	{"HIF_RX_BVALID_FIFO_CNT", HIF_RX_BVALID_FIFO_CNT },
	{"HIF_RX_PKT_CNT1", HIF_RX_PKT_CNT1 },
	{"HIF_RX_PKT_CNT2", HIF_RX_PKT_CNT2 },
};

void pfe_hw_chnl_print_stats(struct pfe_hw_chnl *chnl)
{
	u32 reg;
	size_t i;

	for (i = 0; i < ARRAY_SIZE(pfe_stats_hif); i++) {
		const struct pfe_hw_stats *stat = &pfe_stats_hif[i];

		reg = pfe_hw_read(chnl, stat->reg_off);

		printf("%-40s : 0x%08x\n", stat->reg_name, reg);
	}
}

static void *pfe_hw_dma_alloc(size_t size, size_t align)
{
	return memalign(align, size);
}

static void pfe_hw_dma_free(void *addr)
{
	free(addr);
}

static void pfe_hif_set_bd_data(struct pfe_hif_bd *bd, void *addr)
{
	bd->data = (u32)(pfe_hw_dma_addr(addr) & U32_MAX);
}

static void *pfe_hif_get_bd_data(struct pfe_hif_bd *bd)
{
	return pfe_hw_phys_addr(bd->data + HIF_HEADER_SIZE);
}

static inline int pfe_hw_check_mem_bounds(void *dat, u32 len)
{
	if (len > 0 && ((u64)dat > (U64_MAX - len)))
		return -EINVAL;

	return 0;
}

static void pfe_hw_flush_d(void *dat, u32 len)
{
	if (pfe_hw_check_mem_bounds(dat, len))
		return;

	flush_dcache_range(rounddown((u64)dat, ARCH_DMA_MINALIGN),
			   roundup((u64)dat + len, ARCH_DMA_MINALIGN));
}

static void pfe_hw_inval_d(void *dat, u32 len)
{
	if (pfe_hw_check_mem_bounds(dat, len))
		return;

	invalidate_dcache_range(rounddown((u64)dat, ARCH_DMA_MINALIGN),
				roundup((u64)dat + len, ARCH_DMA_MINALIGN));
}

static void pfe_hw_chnl_rings_attach(struct pfe_hw_chnl *chnl)
{
	dma_addr_t txr = pfe_hw_dma_addr(chnl->tx_ring->bd);
	dma_addr_t txr_wb = pfe_hw_dma_addr(chnl->tx_ring->wb_bd);
	dma_addr_t rxr = pfe_hw_dma_addr(chnl->rx_ring->bd);
	dma_addr_t rxr_wb = pfe_hw_dma_addr(chnl->rx_ring->wb_bd);
	u8 id = chnl->id;

	/* Set TX BD ring */
	pfe_hw_write(chnl, HIF_TX_BDP_RD_LOW_ADDR_CHN(id), (txr & U32_MAX));
	pfe_hw_write(chnl, HIF_TX_BDP_RD_HIGH_ADDR_CHN(id), 0);

	/* Set TX WB BD ring */
	pfe_hw_write(chnl, HIF_TX_BDP_WR_LOW_ADDR_CHN(id), (txr_wb & U32_MAX));
	pfe_hw_write(chnl, HIF_TX_BDP_WR_HIGH_ADDR_CHN(id), 0);
	pfe_hw_write(chnl, HIF_TX_WRBK_BD_CHN_BUFFER_SIZE(id), RING_LEN);

	/* Set RX BD ring */
	pfe_hw_write(chnl, HIF_RX_BDP_RD_LOW_ADDR_CHN(id), (rxr & U32_MAX));
	pfe_hw_write(chnl, HIF_RX_BDP_RD_HIGH_ADDR_CHN(id), 0);

	/* Set RX WB BD ring */
	pfe_hw_write(chnl, HIF_RX_BDP_WR_LOW_ADDR_CHN(id), (rxr_wb & U32_MAX));
	pfe_hw_write(chnl, HIF_RX_BDP_WR_HIGH_ADDR_CHN(id), 0);
	pfe_hw_write(chnl, HIF_RX_WRBK_BD_CHN_BUFFER_SIZE(id), RING_LEN);
}

static struct pfe_hif_ring *pfe_hw_chnl_rings_create(bool is_rx)
{
	struct pfe_hif_ring *ring;
	size_t size;
	u32 i;

	ring = kzalloc(sizeof(*ring), GFP_KERNEL);
	if (!ring)
		return NULL;

	size = RING_LEN * sizeof(*ring->bd);
	ring->bd = pfe_hw_dma_alloc(size, RING_BD_ALIGN);
	if (!ring->bd) {
		log_warning("WARN: HIF ring couldn't be allocated.\n");
		goto err_with_ring;
	}

	memset_io(ring->bd, 0, size);

	size = RING_LEN * sizeof(*ring->wb_bd);
	ring->wb_bd = pfe_hw_dma_alloc(size, RING_BD_ALIGN);
	if (!ring->wb_bd) {
		log_warning("WARN: HIF ring couldn't be allocated.\n");
		goto err_with_bd;
	}

	memset_io(ring->wb_bd, 0, size);

	ring->is_rx = is_rx;
	ring->write_idx = 0;
	ring->read_idx = 0;

	/* flush cache to update MMU mappings */
	flush_dcache_all();

	for (i = 0; i < RING_LEN; i++) {
		if (ring->is_rx) {
			/* mark BD as RX */
			ring->bd[i].dir = 1;
			/* add buffer to rx descriptor */
			ring->bd[i].buflen = PKTSIZE_ALIGN;
			ring->bd[i].lifm = 1;
			ring->bd[i].desc_en = 1;
		}

		ring->bd[i].next = (u32)(pfe_hw_dma_addr(pfe_hif_get_bd(ring, i + 1)) & U32_MAX);
		if (i == RING_LEN - 1)
			ring->bd[i].last_bd = 1;

		/* enable BD interrupt */
		ring->bd[i].cbd_int_en = 1;

		pfe_hw_flush_d(&ring->bd[i], sizeof(*ring->bd));
	}

	for (i = 0; i < RING_LEN; i++) {
		ring->wb_bd[i].seqnum = BD_INITIAL_SEQ_NUM;
		ring->wb_bd[i].desc_en = 1;

		pfe_hw_flush_d(&ring->wb_bd[i], sizeof(*ring->wb_bd));
	}

	log_debug("BD ring 0x%p\nWB ring 0x%p\n", ring->bd, ring->wb_bd);

	return ring;

err_with_bd:
	pfe_hw_dma_free(ring->bd);
err_with_ring:
	kfree(ring);
	return NULL;
}

static void pfe_hw_chnl_rings_destroy(struct pfe_hif_ring *ring)
{
	if (!ring)
		return;

	if (ring->wb_bd)
		pfe_hw_dma_free(ring->wb_bd);
	if (ring->bd)
		pfe_hw_dma_free(ring->bd);

	ring->wb_bd = NULL;
	ring->bd = NULL;

	kfree(ring);
}

/* HIF channel external API*/
int pfe_hw_hif_chnl_create(struct pfe_hw_ext *ext)
{
	struct pfe_hw_chnl *chnl;
	int ret;

	if (!ext->hw->hif_base)
		return -EINVAL;

	chnl = kzalloc(sizeof(*chnl), GFP_KERNEL);
	if (!chnl)
		return -ENOMEM;

	chnl->base = ext->hw->hif_base;
	chnl->id = ext->hw->hif_chnl;
	ext->hw_chnl = chnl;
	chnl->tx_ring = NULL;
	chnl->rx_ring = NULL;

	/* init TX ring */
	chnl->tx_ring = pfe_hw_chnl_rings_create(false);
	if (!chnl->tx_ring) {
		ret = -ENODEV;
		goto err;
	}

	/* init RX ring */
	chnl->rx_ring = pfe_hw_chnl_rings_create(true);
	if (!chnl->rx_ring) {
		ret = -ENODEV;
		goto err;
	}

	/* register rings to pfe HW */
	pfe_hw_chnl_rings_attach(chnl);

	return 0;

err:
	pfe_hw_hif_chnl_destroy(ext);
	return ret;
}

void pfe_hw_hif_chnl_destroy(struct pfe_hw_ext *ext)
{
	struct pfe_hw_chnl *chnl;

	if (!ext || !ext->hw_chnl)
		return;

	chnl = ext->hw_chnl;

	if (chnl->rx_ring) {
		pfe_hw_chnl_rings_destroy(chnl->rx_ring);
		chnl->rx_ring = NULL;
	}

	if (chnl->tx_ring) {
		pfe_hw_chnl_rings_destroy(chnl->tx_ring);
		chnl->tx_ring = NULL;
	}

	kfree(chnl);
	ext->hw_chnl = NULL;
}

void pfe_hw_hif_chnl_enable(struct pfe_hw_chnl *chnl)
{
	struct pfe_hif_ring *ring = chnl->rx_ring;
	int i;

	for (i = 0; i < RING_LEN; i++) {
		pfe_hif_set_bd_data(&ring->bd[i], net_rx_packets[i]);
		pfe_hw_flush_d(&ring->bd[i], sizeof(*ring->bd));
	}
	/* Enable RX & TX DMA engine and polling */
	setbits_32(pfe_hw_addr(chnl, HIF_CTRL_CHN(chnl->id)),
		   RX_BDP_POLL_CNTR_EN | RX_DMA_ENABLE | TX_BDP_POLL_CNTR_EN | TX_DMA_ENABLE);
}

void pfe_hw_hif_chnl_disable(struct pfe_hw_chnl *chnl)
{
	/* Disable RX & TX DMA engine and polling */
	clrbits_32(pfe_hw_addr(chnl, HIF_CTRL_CHN(chnl->id)),
		   RX_BDP_POLL_CNTR_EN | RX_DMA_ENABLE | TX_BDP_POLL_CNTR_EN | TX_DMA_ENABLE);
}

int pfe_hw_chnl_xmit(struct pfe_hw_chnl *chnl, u8 phyif, void *packet, int length)
{
	struct pfe_hif_ring *ring = chnl->tx_ring;
	struct pfe_hif_bd *bd_hd, *bd_pkt, *bp_rd;
	struct pfe_hif_wb_bd *wb_bd_hd, *wb_bd_pkt, *wb_bp_rd;
	u32 wr_idx, wr_idx_1, rd_idx;
	int ret;

	if (length < 0)
		return -EINVAL;

	wr_idx = pfe_hif_get_buffer_idx(ring->write_idx);
	wr_idx_1 = pfe_hif_get_buffer_idx(wr_idx + 1);

	/* Get descriptor for header */
	bd_hd = pfe_hif_get_bd(ring, wr_idx);
	wb_bd_hd = pfe_hif_get_wb_bd(ring, wr_idx);

	/* Get descriptor for packet */
	bd_pkt = pfe_hif_get_bd(ring, wr_idx_1);
	wb_bd_pkt = pfe_hif_get_wb_bd(ring, wr_idx_1);

	pfe_hw_inval_d(bd_hd, sizeof(struct pfe_hif_bd));
	pfe_hw_inval_d(bd_pkt, sizeof(struct pfe_hif_bd));

	if (pfe_hif_get_bd_desc_en(bd_hd))
		log_debug("Invalid Tx desc state (%u)\n", wr_idx);
	else if (pfe_hif_get_bd_desc_en(bd_pkt))
		log_debug("Invalid Tx desc state (%u)\n", wr_idx_1);

	/* Flush the data buffer */
	pfe_hw_flush_d(packet, length);

	/* Fill header */
	memset(&tx_header, 0, HIF_HEADER_SIZE);
	tx_header.flags = HIF_TX_INJECT;
	tx_header.chid = chnl->id;
	tx_header.e_phy_ifs = htonl(1U << phyif);
	pfe_hw_flush_d(&tx_header, HIF_HEADER_SIZE);

	pfe_hif_set_bd_data(bd_hd, &tx_header);
	bd_hd->buflen = HIF_HEADER_SIZE;
	bd_hd->status = 0;
	bd_hd->lifm = 0;
	wb_bd_hd->desc_en = 1;
	dmb();
	bd_hd->desc_en = 1;
	pfe_hw_flush_d(wb_bd_hd, sizeof(*wb_bd_hd));
	pfe_hw_flush_d(bd_hd, sizeof(*bd_hd));

	/* Fill packet */
	pfe_hif_set_bd_data(bd_pkt, packet);
	bd_pkt->buflen = (uint16_t)length;
	bd_pkt->status = 0;
	bd_pkt->lifm = 1;
	wb_bd_pkt->desc_en = 1;
	dmb();
	bd_pkt->desc_en = 1;
	pfe_hw_flush_d(wb_bd_pkt, sizeof(*wb_bd_pkt));
	pfe_hw_flush_d(bd_pkt, sizeof(*bd_pkt));

	/* Increment index for next buffer descriptor */
	wr_idx = pfe_hif_get_buffer_idx(wr_idx + 2);
	ring->write_idx = wr_idx;
	rd_idx = pfe_hif_get_buffer_idx(ring->read_idx);

	/* Tx Confirmation */
	while (rd_idx != wr_idx) {
		u32 wb_ctrl = 0;

		bp_rd = pfe_hif_get_bd(ring, rd_idx);
		wb_bp_rd = pfe_hif_get_wb_bd(ring, rd_idx);

		pfe_hw_inval_d(bp_rd, sizeof(struct pfe_hif_bd));
		pfe_hw_inval_d(wb_bp_rd, sizeof(struct pfe_hif_wb_bd));

		ret = readl_poll_timeout(&wb_bp_rd->ctrl, wb_ctrl,
					 !(wb_ctrl & RING_WBBD_DESC_EN),
					 PFE_HW_BD_TIMEOUT_US);
		if (ret < 0)
			log_debug("Tx BD timeout (%d)\n", ret);

		bp_rd->desc_en = 0;
		wb_bp_rd->desc_en = 0;
		dmb();
		rd_idx = pfe_hif_get_buffer_idx(rd_idx + 1);
		ring->read_idx = rd_idx;
	}

	return 0;
}

int pfe_hw_chnl_receive(struct pfe_hw_chnl *chnl, int flags, uchar **packetp)
{
	struct pfe_hif_bd *bd_pkt;
	struct pfe_hif_wb_bd *wb_bd_pkt;
	struct pfe_hif_ring *ring = chnl->rx_ring;
	u32 wb_ctrl = 0;
	u32 rd_idx;
	int plen = 0;

	rd_idx = pfe_hif_get_buffer_idx(ring->read_idx);
	bd_pkt = pfe_hif_get_bd(ring, rd_idx);
	wb_bd_pkt = pfe_hif_get_wb_bd(ring, rd_idx);

	pfe_hw_inval_d(bd_pkt, sizeof(struct pfe_hif_bd));
	pfe_hw_inval_d(wb_bd_pkt, sizeof(struct pfe_hif_wb_bd));

	/* check if we received data */
	if (readl_poll_timeout(&wb_bd_pkt->ctrl, wb_ctrl,
			       !(wb_ctrl & RING_WBBD_DESC_EN),
			       PFE_HW_BD_TIMEOUT_US) < 0)
		return -EAGAIN;

	/* Give the data to u-boot stack */
	bd_pkt->desc_en = 0;
	wb_bd_pkt->desc_en = 1;
	pfe_hw_flush_d(wb_bd_pkt, sizeof(*wb_bd_pkt));
	pfe_hw_flush_d(bd_pkt, sizeof(*bd_pkt));
	dmb();
	*packetp = pfe_hif_get_bd_data(bd_pkt);
	if (wb_bd_pkt->buflen >= HIF_HEADER_SIZE)
		plen = wb_bd_pkt->buflen - HIF_HEADER_SIZE;

	/* Advance read buffer */
	rd_idx = pfe_hif_get_buffer_idx(rd_idx + 1);
	ring->read_idx = rd_idx;

	/* Invalidate the buffer */
	pfe_hw_inval_d(*packetp, plen);

	if (wb_bd_pkt->lifm != 1) {
		log_warning("Multi buffer packets not supported, discarding.\n");
		/* Return EOK so the stack frees the buffer */
		return 0;
	}

	return plen;
}

int pfe_hw_chnl_free_pkt(struct pfe_hw_chnl *chnl, uchar *packet, int length)
{
	struct pfe_hif_ring *ring = chnl->rx_ring;
	struct pfe_hif_bd *bd_pkt;
	struct pfe_hif_wb_bd *wb_bd_pkt;
	u32 wr_idx;

	if (length < 0)
		return -EINVAL;

	wr_idx = pfe_hif_get_buffer_idx(ring->write_idx);
	bd_pkt = pfe_hif_get_bd(ring, wr_idx);
	wb_bd_pkt = pfe_hif_get_wb_bd(ring, wr_idx);

	pfe_hw_inval_d(bd_pkt, sizeof(struct pfe_hif_bd));
	pfe_hw_inval_d(wb_bd_pkt, sizeof(struct pfe_hif_wb_bd));

	if (bd_pkt->desc_en) {
		log_err("ERR: Can't free buffer since the BD entry is used\n");
		return -EIO;
	}

	/* Free buffer */
	bd_pkt->buflen = PKTSIZE_ALIGN;
	bd_pkt->status = 0;
	bd_pkt->lifm = 1;
	wb_bd_pkt->desc_en = 1;
	dmb();
	bd_pkt->desc_en = 1;

	/* This has to be here for correct HW functionality */
	pfe_hw_flush_d(packet, length);
	pfe_hw_inval_d(packet, length);

	/* Advance free pointer */
	wr_idx = pfe_hif_get_buffer_idx(wr_idx + 1);
	ring->write_idx = wr_idx;

	return 0;
}

/* HIF external API */
int pfe_hw_hif_init(struct pfe_hw_hif *hif, void __iomem *base, u8 hif_chnl,
		    const struct pfe_hw_cfg *config)
{
	struct pfe_hw_chnl chnl = { .base = base, .id = hif_chnl };
	u32 reg;

	hif->base = base;
	hif->hif_chnl = hif_chnl;

	/* Disable and clear HIF interrupts */
	pfe_hw_write(hif, HIF_ERR_INT_EN, 0);
	pfe_hw_write(hif, HIF_TX_FIFO_ERR_INT_EN, 0);
	pfe_hw_write(hif, HIF_RX_FIFO_ERR_INT_EN, 0);
	pfe_hw_write(hif, HIF_ERR_INT_SRC, 0xffffffff);
	pfe_hw_write(hif, HIF_TX_FIFO_ERR_INT_SRC, 0xffffffff);
	pfe_hw_write(hif, HIF_RX_FIFO_ERR_INT_SRC, 0xffffffff);

	if (config->on_g3) {
		log_debug("Skipping HIF soft reset\n");
	} else {
		/* SOFT RESET */
		pfe_hw_write(hif, HIF_SOFT_RESET, HIF_SOFT_RESET_CMD);
		if (read_poll_timeout(readl, pfe_hw_addr(hif, HIF_SOFT_RESET), reg,
				      !(reg & HIF_SOFT_RESET_CMD),
				      1000, HIF_SOFT_RESET_TIMEOUT_US) < 0) {
			dev_err(config->dev, "HIF reset timed out.\n");
			return -ETIMEDOUT;
		}
		pfe_hw_write(hif, HIF_SOFT_RESET, 0);
		log_debug("HIF soft reset done\n");
	}

	/* # of poll cycles */
	pfe_hw_write(hif, HIF_TX_POLL_CTRL, (0xff << 16) | (0xff));
	pfe_hw_write(hif, HIF_RX_POLL_CTRL, (0xff << 16) | (0xff));

	/* misc */
	pfe_hw_write(hif, HIF_MISC, HIF_TIMEOUT_EN | BD_START_SEQ_NUM(0));

	/* Timeout in cycles of sys_clk */
	pfe_hw_write(hif, HIF_TIMEOUT_REG, 100000000);

	/* TMU queue mapping. 0,1->ch.0, 2,3->ch.1, 4,5->ch.2, 6,7->ch.3 */
	pfe_hw_write(hif, HIF_RX_QUEUE_MAP_CH_NO_ADDR, 0x33221100);

	/* DMA burst size */
	pfe_hw_write(hif, HIF_DMA_BURST_SIZE_ADDR, 0);

	/* DMA base address */
	pfe_hw_write(hif, HIF_DMA_BASE_ADDR, 0);

	/* Disable channel interrupts */
	pfe_hw_write(hif, HIF_CHN_INT_EN(hif_chnl), 0);
	pfe_hw_write(hif, HIF_CHN_INT_SRC(hif_chnl), 0xffffffff);

	/* Disable RX & TX DMA engine and polling */
	pfe_hw_hif_chnl_disable(&chnl);

	/* Disable interrupt coalescing */
	pfe_hw_write(hif, HIF_INT_COAL_EN_CHN(hif_chnl), 0);
	pfe_hw_write(hif, HIF_ABS_INT_TIMER_CHN(hif_chnl), 0);
	pfe_hw_write(hif, HIF_ABS_FRAME_COUNT_CHN(hif_chnl), 0);

	/* LTC reset */
	pfe_hw_write(hif, HIF_LTC_PKT_CTRL_ADDR, 0);
	pfe_hw_write(hif, HIF_LTC_MAX_PKT_CHN_ADDR(hif_chnl), 0);

	return 0;
}