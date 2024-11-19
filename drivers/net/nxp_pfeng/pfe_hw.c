// SPDX-License-Identifier: GPL 2.0
/*
 *  Copyright (c) 2019 Imagination Technologies Limited
 *  Copyright (c) 2020-2021 Imagination Technologies Limited
 *  Copyright 2018-2024 NXP
 */

#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <s32-cc/serdes_hwconfig.h>

#include "pfe_hw.h"
#include "internal/pfe_hw_priv.h"

#define PFE_HW_BLOCK_SLEEP_MS 100UL
#define PFE_HW_BLOCK_RESET_TIMEOUT_MS 2000UL
#define PFE_HW_G3_SOFT_RESET_SLEEP_MS 1000UL
#define PFE_HW_G3_SOFT_RESET_TIMEOUT_MS 1000000UL

static struct pfe_hw *_pfe;
u8 pfe_hw_get_platform_emac_mdio_div(void)
{
	if (_pfe)
		return _pfe->emac_mdio_div;
	return 0;
}

struct pfe_gpi_cfg {
	u32 pa_off;
	u32 alloc_retry_cycles;
	u32 gpi_tmlf_txthres;
	u32 gpi_dtx_aseq_len;
	bool emac_1588_ts_en;
	bool ingress;
};

struct pfe_bmu_cfg {
	u32 pa_off;
	u32 pool_pa;
	u16 max_buf_cnt;
	u32 buf_size;
	u32 bmu_ucast_thres;
	u32 bmu_mcast_thres;
	u32 int_mem_loc_cnt;
	u32 buf_mem_loc_cnt;
};

static inline phys_addr_t pfe_hw_block_pa_of(phys_addr_t addr, u32 pa_off)
{
	if (pa_off > 0 && (addr > (U64_MAX - pa_off)))
		return 0U;

	return addr + pa_off;
}

static inline phys_addr_t pfe_hw_block_pa(struct pfe_hw *pfe, u32 pa_off)
{
	return pfe_hw_block_pa_of(pfe->cbus_baseaddr, pa_off);
}

phys_addr_t pfe_hw_emac_get_iobase(phys_addr_t pfe_iobase, enum pfe_hw_emac_block block_id)
{
	phys_addr_t offset = 0;
	phys_addr_t addr = pfe_iobase;

	switch (block_id) {
	case PFENG_EMAC0:
		offset = CBUS_EMAC1_BASE_ADDR;
		break;

	case PFENG_EMAC1:
		offset = CBUS_EMAC2_BASE_ADDR;
		break;

	case PFENG_EMAC2:
		offset = CBUS_EMAC3_BASE_ADDR;
		break;

	default:
		return 0U;
	}

	if (offset > 0 && (addr > (U64_MAX - offset)))
		return 0U;

	return addr + offset;
}

static void __iomem *pfe_hw_get_bmu_cfg(struct pfe_hw *pfe, enum pfe_hw_bmu_blocks bmu_id,
					struct pfe_bmu_cfg *bmu_cfg)
{
	static const struct pfe_bmu_cfg cfg[PFENG_BMU_COUNT] = {
		{CBUS_BMU1_BASE_ADDR, PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_LMEM_BASE_ADDR,
		PFE_CFG_BMU1_BUF_COUNT, PFE_CFG_BMU1_BUF_SIZE,
		0x200U, 0x200U, 64U, 64U},
		{CBUS_BMU2_BASE_ADDR, 0, PFE_CFG_BMU2_BUF_COUNT, PFE_CFG_BMU2_BUF_SIZE,
		0x10U, 0x10U, 1024U, 1024U}
	};

	if (bmu_cfg)
		*bmu_cfg = cfg[bmu_id];

	return pfe_hw_phys_addr(pfe_hw_block_pa(pfe, cfg[bmu_id].pa_off));
}

static int pfe_hw_enable_bmu(struct pfe_hw *pfe)
{
	void __iomem *bmu_base;
	enum pfe_hw_bmu_blocks bmu_id;

	for (bmu_id = PFENG_BMU1; bmu_id < PFENG_BMUS_COUNT; bmu_id++) {
		bmu_base = pfe_hw_get_bmu_cfg(pfe, bmu_id, NULL);
		if (!bmu_base)
			return -EINVAL;

		pfe_hw_write_reg(bmu_base, BMU_CTRL, 0x1U);

		dev_dbg(pfe->cfg->dev, "Enabled the BMU%d block\n", bmu_id + 1);
	}

	return 0;
}

static int pfe_hw_init_bmu(struct pfe_hw *pfe)
{
	struct pfe_bmu_cfg bmu_cfg;
	void __iomem *bmu_base;
	u32 mem;
	enum pfe_hw_bmu_blocks bmu_id;

	pfe->bmu_buffers_size = PFE_CFG_BMU2_BUF_COUNT * (1U << PFE_CFG_BMU2_BUF_SIZE);
	if (pfe->bmu_buffers_size != pfe->cfg->bmu_addr_size) {
		dev_err(pfe->cfg->dev, "PFE: BMU expecteted mem size is: 0x%llX check dtb config\n",
			pfe->bmu_buffers_size);
		return -EINVAL;
	}

	pfe->bmu_buffers_va = pfe_hw_phys_addr(pfe->cfg->bmu_addr);
	if (!pfe->bmu_buffers_va) {
		dev_err(pfe->cfg->dev, "PFE: Unable to get BMU2 pool memory\n");
		return -ENOMEM;
	}

	for (bmu_id = PFENG_BMU1; bmu_id < PFENG_BMUS_COUNT; bmu_id++) {
		bmu_base = pfe_hw_get_bmu_cfg(pfe, bmu_id, &bmu_cfg);
		if (!bmu_base)
			return -EINVAL;

		if (bmu_id == PFENG_BMU2) {
			bmu_cfg.pool_pa = pfe_hw_dma_addr(pfe->bmu_buffers_va);

			/* PFE AXI MASTERs can only access range p0x00020000 - p0xbfffffff */
			if (bmu_cfg.pool_pa < 0x00020000U ||
			    (bmu_cfg.pool_pa + pfe->bmu_buffers_size) > 0xbfffffffU) {
				dev_err(pfe->cfg->dev, "BMU2 buffers not in required range: starts @ p0x%x\n",
					bmu_cfg.pool_pa);
				return -EINVAL;
			}
		}

		dev_dbg(pfe->cfg->dev, "BMU%d buffer base: p0x%x\n", bmu_id + 1, bmu_cfg.pool_pa);

		/* Disable */
		pfe_hw_write_reg(bmu_base, BMU_CTRL, 0x0U);

		/* Disable and clear BMU interrupts */
		pfe_hw_write_reg(bmu_base, BMU_INT_ENABLE, 0x0U);
		pfe_hw_write_reg(bmu_base, BMU_INT_SRC, 0xffffffffU);

		pfe_hw_write_reg(bmu_base, BMU_UCAST_BASEADDR, bmu_cfg.pool_pa & 0xffffffffU);
		pfe_hw_write_reg(bmu_base, BMU_UCAST_CONFIG, bmu_cfg.max_buf_cnt & 0xffffU);
		pfe_hw_write_reg(bmu_base, BMU_BUF_SIZE, bmu_cfg.buf_size & 0xffffU);

		/* Thresholds 75%  */
		pfe_hw_write_reg(bmu_base, BMU_THRES, (bmu_cfg.max_buf_cnt * 3U) / 4U);

		/* Clear internal memories */
		for (mem = 0U; mem < bmu_cfg.int_mem_loc_cnt; mem++) {
			pfe_hw_write_reg(bmu_base, BMU_INT_MEM_ACCESS_ADDR, mem);
			pfe_hw_write_reg(bmu_base, BMU_INT_MEM_ACCESS, 0U);
			pfe_hw_write_reg(bmu_base, BMU_INT_MEM_ACCESS2, 0U);
		}

		for (mem = 0U; mem < bmu_cfg.buf_mem_loc_cnt; mem++) {
			pfe_hw_write_reg(bmu_base, BMU_INT_MEM_ACCESS_ADDR, mem);
			pfe_hw_write_reg(bmu_base, BMU_INT_MEM_ACCESS, 0U);
			pfe_hw_write_reg(bmu_base, BMU_INT_MEM_ACCESS2, 0U);
		}

		/* Enable BMU interrupts except the global enable bit */
		pfe_hw_write_reg(bmu_base, BMU_INT_ENABLE, 0xffffffffU & ~(BMU_INT));
	}

	return 0;
}

static void pfe_hw_deinit_bmu(struct pfe_hw *pfe)
{
	void __iomem *bmu_base;
	enum pfe_hw_bmu_blocks bmu_id;

	for (bmu_id = PFENG_BMU1; bmu_id < PFENG_BMUS_COUNT; bmu_id++) {
		bmu_base = pfe_hw_get_bmu_cfg(pfe, bmu_id, NULL);
		if (!bmu_base)
			continue;

		pfe_hw_write_reg(bmu_base, BMU_CTRL, 0);
		/* Disable and clear BMU interrupts */
		pfe_hw_write_reg(bmu_base, BMU_INT_ENABLE, 0);
		pfe_hw_write_reg(bmu_base, BMU_INT_SRC, 0xffffffffU);
	}

	if (pfe->bmu_buffers_va)
		pfe->bmu_buffers_va = NULL;
}

static void __iomem *pfe_hw_get_gpi_cfg(struct pfe_hw *pfe, enum pfe_hw_gpi_blocks gpi_id,
					struct pfe_gpi_cfg *gpi_cfg)
{
	static const struct pfe_gpi_cfg cfg[PFENG_GPI_COUNT] = {
		{CBUS_EGPI1_BASE_ADDR, 0x200U, 0x178U, 0x40U, true, true},
		{CBUS_EGPI2_BASE_ADDR, 0x200U, 0x178U, 0x40U, true, true},
		{CBUS_EGPI3_BASE_ADDR, 0x200U, 0x178U, 0x40U, true, true},
		{CBUS_ETGPI1_BASE_ADDR, 0x200U, 0xbcU, 0x40U, true, false},
		{CBUS_ETGPI2_BASE_ADDR, 0x200U, 0xbcU, 0x40U, true, false},
		{CBUS_ETGPI3_BASE_ADDR, 0x200U, 0xbcU, 0x40U, true, false},
		{CBUS_HGPI_BASE_ADDR, 0x200U, 0x178U, 0x40U, false, false},
	};

	if (gpi_cfg)
		*gpi_cfg = cfg[gpi_id];

	return pfe_hw_phys_addr(pfe_hw_block_pa(pfe, cfg[gpi_id].pa_off));
}

static int pfe_hw_enable_gpi(struct pfe_hw *pfe)
{
	void __iomem *gpi_base;
	enum pfe_hw_gpi_blocks gpi_id;

	for (gpi_id = PFENG_EGPI1; gpi_id < PFENG_GPIS_COUNT; gpi_id++) {
		gpi_base = pfe_hw_get_gpi_cfg(pfe, gpi_id, NULL);
		if (!gpi_base)
			return -EINVAL;

		setbits_32(pfe_hw_reg_addr(gpi_base, GPI_CTRL), 0x1U);

		dev_dbg(pfe->cfg->dev, "Enabled the GPI%d block\n", gpi_id);
	}

	return 0;
}

static int pfe_hw_init_gpi(struct pfe_hw *pfe)
{
	struct pfe_gpi_cfg gpi_cfg;
	void __iomem *gpi_base;
	u32 reg, ii, lmem_hdr_size;
	enum pfe_hw_gpi_blocks gpi_id;

	if (pfe->on_g3)
		lmem_hdr_size = PFE_CFG_LMEM_HDR_SIZE_G3;
	else
		lmem_hdr_size = PFE_CFG_LMEM_HDR_SIZE_G2;

	for (gpi_id = PFENG_EGPI1; gpi_id < PFENG_GPIS_COUNT; gpi_id++) {
		gpi_base = pfe_hw_get_gpi_cfg(pfe, gpi_id, &gpi_cfg);
		if (!gpi_base)
			return -EINVAL;

		/* Reset GPI */
		setbits_32(pfe_hw_reg_addr(gpi_base, GPI_CTRL), 0x2U);

		if (read_poll_timeout(readl, pfe_hw_reg_addr(gpi_base, GPI_CTRL), reg,
				      !(reg & 0x2U),
				      PFE_HW_BLOCK_SLEEP_MS,
				      PFE_HW_BLOCK_RESET_TIMEOUT_MS) < 0) {
			dev_err(pfe->cfg->dev, "Init failed GPI%d\n", gpi_id);
			return -ETIMEDOUT;
		}

		/* Disable GPI */
		clrbits_32(pfe_hw_reg_addr(gpi_base, GPI_CTRL), 0x1U);

		/* INIT QOS */
		pfe_hw_write_reg(gpi_base, CSR_IGQOS_ENTRY_DATA_REG0, 0U);
		pfe_hw_write_reg(gpi_base, CSR_IGQOS_ENTRY_DATA_REG1, 0U);
		pfe_hw_write_reg(gpi_base, CSR_IGQOS_ENTRY_DATA_REG2, 0U);
		pfe_hw_write_reg(gpi_base, CSR_IGQOS_ENTRY_DATA_REG3, 0U);
		pfe_hw_write_reg(gpi_base, CSR_IGQOS_ENTRY_DATA_REG4, 0U);
		pfe_hw_write_reg(gpi_base, CSR_IGQOS_ENTRY_DATA_REG5, 0U);
		pfe_hw_write_reg(gpi_base, CSR_IGQOS_ENTRY_DATA_REG6, 0U);
		pfe_hw_write_reg(gpi_base, CSR_IGQOS_ENTRY_DATA_REG7, 0U);

		/*	Entry table */
		for (ii = 0U; gpi_cfg.ingress && ii < IGQOS_ENTRY_TABLE_LEN; ii++) {
			reg = CMDCNTRL_CMD_WRITE | CMDCNTRL_CMD_TAB_ADDR(ii);
			pfe_hw_write_reg(gpi_base, CSR_IGQOS_ENTRY_CMDCNTRL, reg);
			reg = CMDCNTRL_CMD_WRITE | CMDCNTRL_CMD_TAB_ADDR(ii) |
			      CMDCNTRL_CMD_TAB_SELECT_LRU;
			pfe_hw_write_reg(gpi_base, CSR_IGQOS_ENTRY_CMDCNTRL, reg);
		}

		/*	GPI_EMAC_1588_TIMESTAMP_EN */
		pfe_hw_write_reg(gpi_base, GPI_EMAC_1588_TIMESTAMP_EN, 0x0U);

		/*	GPI_EMAC_1588_TIMESTAMP_EN */
		if (gpi_cfg.emac_1588_ts_en)
			setbits_32(pfe_hw_reg_addr(gpi_base, GPI_EMAC_1588_TIMESTAMP_EN), 0xe01U);

		/*	GPI_RX_CONFIG */
		reg = (gpi_cfg.alloc_retry_cycles << 16) | GPI_DDR_BUF_EN | GPI_LMEM_BUF_EN;
		pfe_hw_write_reg(gpi_base, GPI_RX_CONFIG, reg);
		/*	GPI_HDR_SIZE */
		reg = (PFE_CFG_DDR_HDR_SIZE << 16) | lmem_hdr_size;
		pfe_hw_write_reg(gpi_base, GPI_HDR_SIZE, reg);
		/*	GPI_BUF_SIZE */
		reg = (PFE_CFG_DDR_BUF_SIZE << 16) | PFE_CFG_LMEM_BUF_SIZE;
		pfe_hw_write_reg(gpi_base, GPI_BUF_SIZE, reg);

		/*	DDR config */
		reg = PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU1_BASE_ADDR + BMU_ALLOC_CTRL;
		pfe_hw_write_reg(gpi_base, GPI_LMEM_ALLOC_ADDR, reg);
		reg = PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU1_BASE_ADDR + BMU_FREE_CTRL;
		pfe_hw_write_reg(gpi_base, GPI_LMEM_FREE_ADDR, reg);
		reg = PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU2_BASE_ADDR + BMU_ALLOC_CTRL;
		pfe_hw_write_reg(gpi_base, GPI_DDR_ALLOC_ADDR, reg);
		reg = PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU2_BASE_ADDR + BMU_FREE_CTRL;
		pfe_hw_write_reg(gpi_base, GPI_DDR_FREE_ADDR, reg);

		/*	GPI_CLASS_ADDR */
		reg = PFE_CFG_CBUS_PHYS_BASE_ADDR + CLASS_INQ_PKTPTR;
		pfe_hw_write_reg(gpi_base, GPI_CLASS_ADDR, reg);
		/*	GPI_DDR_DATA_OFFSET */
		pfe_hw_write_reg(gpi_base, GPI_DDR_DATA_OFFSET, PFE_CFG_DDR_HDR_SIZE);
		pfe_hw_write_reg(gpi_base, GPI_LMEM_DATA_OFFSET, 0x30U);
		pfe_hw_write_reg(gpi_base, GPI_LMEM_SEC_BUF_DATA_OFFSET, lmem_hdr_size);
		pfe_hw_write_reg(gpi_base, GPI_TMLF_TX, gpi_cfg.gpi_tmlf_txthres);
		pfe_hw_write_reg(gpi_base, GPI_DTX_ASEQ, gpi_cfg.gpi_dtx_aseq_len);

		/*	IP/TCP/UDP Checksum Offload */
		pfe_hw_write_reg(gpi_base, GPI_CSR_TOE_CHKSUM_EN, 1);

		dev_dbg(pfe->cfg->dev, "GPI%d block was initialized\n", gpi_id);
	}

	return 0;
}

static void pfe_hw_deinit_gpi(struct pfe_hw *pfe)
{
	void __iomem *gpi_base;
	u32 reg;
	enum pfe_hw_gpi_blocks gpi_id;

	for (gpi_id = PFENG_EGPI1; gpi_id < PFENG_GPIS_COUNT; gpi_id++) {
		gpi_base = pfe_hw_get_gpi_cfg(pfe, gpi_id, NULL);
		if (!gpi_base)
			continue;

		/* Disable and reset */
		reg = pfe_hw_read_reg(gpi_base, GPI_CTRL);
		reg |= 0x2U;
		pfe_hw_write_reg(gpi_base, GPI_CTRL, reg & ~0x1U);
		pfe_hw_write_reg(gpi_base, GPI_CTRL, reg);
	}
}

static int pfe_hw_init_tmu(struct pfe_hw *pfe)
{
	static const u32 phys[] = {
		PFE_PHY_IF_ID_EMAC0, PFE_PHY_IF_ID_EMAC1, PFE_PHY_IF_ID_EMAC2,
		PFE_PHY_IF_ID_HIF_NOCPY, PFE_PHY_IF_ID_HIF
	};
	void __iomem *qos_base;
	u32 queue;
	u32 reg;
	u32 i;
	enum pfe_hw_shp_blocks shp_id;
	enum pfe_hw_sch_blocks sch_id;

	pfe_hw_write(pfe, TMU_CTRL, 0x1U);

	if (read_poll_timeout(readl, pfe_hw_addr(pfe, TMU_CTRL), reg,
			      !(reg & 0x1U),
			      PFE_HW_BLOCK_SLEEP_MS,
			      PFE_HW_BLOCK_RESET_TIMEOUT_MS) < 0)
		dev_err(pfe->cfg->dev, "FATAL: TMU reset timed-out\n");

	pfe_hw_write(pfe, TMU_PHY0_TDQ_CTRL, 0x0U); /* EMAC0 */
	pfe_hw_write(pfe, TMU_PHY1_TDQ_CTRL, 0x0U); /* EMAC1 */
	pfe_hw_write(pfe, TMU_PHY2_TDQ_CTRL, 0x0U); /* EMAC2 */
	pfe_hw_write(pfe, TMU_PHY3_TDQ_CTRL, 0x0U); /* HIF */

	/*	Reset */
	pfe_hw_write(pfe, TMU_CTRL, 0x1U);

	if (read_poll_timeout(readl, pfe_hw_addr(pfe, TMU_CTRL), reg,
			      !(reg & 0x1U),
			      PFE_HW_BLOCK_SLEEP_MS,
			      PFE_HW_BLOCK_RESET_TIMEOUT_MS) < 0)
		dev_err(pfe->cfg->dev, "FATAL: TMU reset timed-out\n");

	/*	INQ */
	pfe_hw_write(pfe, TMU_PHY0_INQ_ADDR, PFE_CFG_CBUS_PHYS_BASE_ADDR +
		     CBUS_EGPI1_BASE_ADDR + GPI_INQ_PKTPTR); /* EGPI1 */
	pfe_hw_write(pfe, TMU_PHY1_INQ_ADDR, PFE_CFG_CBUS_PHYS_BASE_ADDR +
		     CBUS_EGPI2_BASE_ADDR + GPI_INQ_PKTPTR); /* EGPI2 */
	pfe_hw_write(pfe, TMU_PHY2_INQ_ADDR, PFE_CFG_CBUS_PHYS_BASE_ADDR +
		     CBUS_EGPI3_BASE_ADDR + GPI_INQ_PKTPTR); /* EGPI3 */
	pfe_hw_write(pfe, TMU_PHY3_INQ_ADDR, PFE_CFG_CBUS_PHYS_BASE_ADDR +
		     CBUS_HGPI_BASE_ADDR + GPI_INQ_PKTPTR); /* HGPI */

	/*	Context memory initialization */
	for (i = 0; i < ARRAY_SIZE(phys); i++) {
		/*	Initialize queues */
		for (queue = 0U; queue < TLITE_PHY_QUEUES_CNT; queue++) {
			/*	Set direct context memory access */
			pfe_hw_write(pfe, TMU_CNTX_ACCESS_CTRL, 0x1U);

			/*	Select PHY and QUEUE */
			pfe_hw_write(pfe, TMU_PHY_QUEUE_SEL,
				     ((phys[i] & 0x1fU) << 8) | (queue & 0x7U));
			dmb(); /* artificial delay */

			/*	Clear direct access registers */
			pfe_hw_write(pfe, TMU_CURQ_PTR, 0U);
			pfe_hw_write(pfe, TMU_CURQ_PKT_CNT, 0U);
			pfe_hw_write(pfe, TMU_CURQ_DROP_CNT, 0U);
			pfe_hw_write(pfe, TMU_CURQ_TRANS_CNT, 0U);
			pfe_hw_write(pfe, TMU_CURQ_QSTAT, 0U);
			pfe_hw_write(pfe, TMU_HW_PROB_CFG_TBL0, 0U);
			pfe_hw_write(pfe, TMU_HW_PROB_CFG_TBL1, 0U);
			pfe_hw_write(pfe, TMU_CURQ_DEBUG, 0U);
		}

		for (sch_id = PFENG_SCH1; sch_id < PFENG_SCHS_COUNT; sch_id++) {
			/*	Initialize HW schedulers/shapers */
			qos_base = pfe->base + TLITE_PHYN_SCHEDM_BASE_ADDR(phys[i], sch_id);
			pfe_hw_write_reg(qos_base, TMU_SCH_Q_ALLOC0, 0xffffffffU);
			pfe_hw_write_reg(qos_base, TMU_SCH_Q_ALLOC1, 0xffffffffU);
		}

		for (shp_id = PFENG_SHP1; shp_id < PFENG_SHPS_COUNT; shp_id++) {
			/*	Initialize HW schedulers/shapers */
			qos_base = pfe->base + TLITE_PHYN_SHPM_BASE_ADDR(phys[i], shp_id);
			clrbits_32(pfe_hw_reg_addr(qos_base, TMU_SHP_CTRL), 0x1U);

			/*	Set invalid position */
			pfe_hw_write_reg(qos_base, TMU_SHP_CTRL2, (0x1fU << 1));

			/*	Set default limits */
			pfe_hw_write_reg(qos_base, TMU_SHP_MAX_CREDIT, 0U);
			pfe_hw_write_reg(qos_base, TMU_SHP_MIN_CREDIT, 0U);
		}

		for (queue = 0U; queue < TLITE_PHY_QUEUES_CNT; queue++) {
			/*	Scheduler 0 */
			qos_base = pfe->base + TLITE_PHYN_SCHEDM_BASE_ADDR(phys[i], 0U);
			clrsetbits_32(pfe_hw_reg_addr(qos_base, TMU_SCH_Q_ALLOCN(queue / 4U)),
				      (0xffU << (8U * (queue % 4U))),
				      ((queue & 0x1fU) << (8U * (queue % 4U))));

			/*	Scheduler 1 */
			qos_base = pfe->base + TLITE_PHYN_SCHEDM_BASE_ADDR(phys[i], 1U);
			clrsetbits_32(pfe_hw_reg_addr(qos_base, TMU_SCH_Q_ALLOCN(0)),
				      0xffU, 0x1fU);
		}

		/*	Connect Scheduler 0 output to Scheduler 1 input 0 */
		pfe_hw_write(pfe, TLITE_PHYN_SCHEDM_BASE_ADDR(phys[i], 0U) + TMU_SCH_POS, 0);

		for (queue = 0U; queue < TLITE_PHY_QUEUES_CNT; queue++) {
			/* Configure Queue mode */
			pfe_hw_write(pfe, TMU_CNTX_ACCESS_CTRL, 0U);
			pfe_hw_write(pfe, TMU_CNTX_ADDR,
				     ((phys[i] & 0x1fU) << 16) | ((8U * queue) + 4U));
			pfe_hw_write(pfe, TMU_CNTX_DATA,
				     (255U << 11) | (0U << 2) | (0x1U << 0));
			pfe_hw_write(pfe, TMU_CNTX_CMD, 0x3U);
			/*	Wait until command has been finished */
			if (read_poll_timeout(readl, pfe_hw_addr(pfe, TMU_CNTX_CMD), reg,
					      (reg & 0x4U),
					      PFE_HW_BLOCK_SLEEP_MS,
					      PFE_HW_BLOCK_RESET_TIMEOUT_MS) < 0) {
				dev_err(pfe->cfg->dev, "FATAL: Set TMU Queue mode timed-out\n");
				return -ETIMEDOUT;
			}
		}
	}

	/*	BMU1 */
	pfe_hw_write(pfe, TMU_BMU_INQ_ADDR,
		     PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU1_BASE_ADDR + BMU_FREE_CTRL);

	/*	BMU2 */
	pfe_hw_write(pfe, TMU_BMU2_INQ_ADDR,
		     PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU2_BASE_ADDR + BMU_FREE_CTRL);

	/*	Thresholds */
	pfe_hw_write(pfe, TMU_AFULL_THRES, 0x100U);
	pfe_hw_write(pfe, TMU_INQ_WATERMARK, 0xfcU);

	/*	TDQ CTRL */
	pfe_hw_write(pfe, TMU_PHY0_TDQ_CTRL, 0xfU); /* EMAC0 */
	pfe_hw_write(pfe, TMU_PHY1_TDQ_CTRL, 0xfU); /* EMAC1 */
	pfe_hw_write(pfe, TMU_PHY2_TDQ_CTRL, 0xfU); /* EMAC2 */
	pfe_hw_write(pfe, TMU_PHY3_TDQ_CTRL, 0xfU); /* HIF */

	dev_dbg(pfe->cfg->dev, "TMU was initialized\n");
	return 0;
}

static int pfe_hw_soft_reset_g2(struct pfe_hw *pfe)
{
	void __iomem *wsp_base = pfe_hw_phys_addr(pfe_hw_block_pa(pfe, CBUS_GLOBAL_CSR_BASE_ADDR));

	if (!wsp_base)
		return -EINVAL;

	setbits_32(pfe_hw_reg_addr(wsp_base, WSP_GENERIC_CONTROL), SYS_GEN_SOFT_RST_BIT);

	/* 100ms (taken from reference code) */
	mdelay(100);

	clrbits_32(pfe_hw_reg_addr(wsp_base, WSP_GENERIC_CONTROL), SYS_GEN_SOFT_RST_BIT);

	return 0;
}

static int pfe_hw_soft_reset_g3(struct pfe_hw *pfe)
{
	void __iomem *wsp_base = pfe_hw_phys_addr(pfe_hw_block_pa(pfe, CBUS_GLOBAL_CSR_BASE_ADDR));
	u32 reg;

	if (!wsp_base)
		return -EINVAL;

	setbits_32(pfe_hw_reg_addr(wsp_base, WSP_GENERIC_CONTROL), SYS_GEN_SOFT_RST_BIT);

	mdelay(10);

	if (read_poll_timeout(readl, pfe_hw_reg_addr(wsp_base, WSP_DBUG_BUS1), reg,
			      (reg & (SOFT_RESET_DONE |
				      BMU1_SOFT_RESET_DONE |
				      BMU2_SOFT_RESET_DONE)),
			      PFE_HW_G3_SOFT_RESET_SLEEP_MS,
			      PFE_HW_G3_SOFT_RESET_TIMEOUT_MS) < 0)
		return -ETIME;

	setbits_32(pfe_hw_reg_addr(wsp_base, WSP_GENERIC_CONTROL),
		   SOFT_RESET_DONE_CLEAR | BMU1_SOFT_RESET_DONE_CLEAR | BMU2_SOFT_RESET_DONE_CLEAR);

	return 0;
}

static u8 pfe_hw_emac_mdio_div_decode(u64 csr_clk)
{
	u8 mdio_div = CSR_CLK_300_500_MHZ_MDC_CSR_DIV_204;

	if (csr_clk < 35 * MHZ)
		mdio_div = CSR_CLK_20_35_MHZ_MDC_CSR_DIV_16;
	else if ((csr_clk >= 35 * MHZ) && (csr_clk < 60 * MHZ))
		mdio_div = CSR_CLK_35_60_MHZ_MDC_CSR_DIV_26;
	else if ((csr_clk >= 60 * MHZ) && (csr_clk < 100 * MHZ))
		mdio_div = CSR_CLK_60_100_MHZ_MDC_CSR_DIV_42;
	else if ((csr_clk >= 100 * MHZ) && (csr_clk < 150 * MHZ))
		mdio_div = CSR_CLK_100_150_MHZ_MDC_CSR_DIV_62;
	else if ((csr_clk >= 150 * MHZ) && (csr_clk < 250 * MHZ))
		mdio_div = CSR_CLK_150_250_MHZ_MDC_CSR_DIV_102;
	else if ((csr_clk >= 250 * MHZ) && (csr_clk < 300 * MHZ))
		mdio_div = CSR_CLK_250_300_MHZ_MDC_CSR_DIV_124;
	else if ((csr_clk >= 300 * MHZ) && (csr_clk < 500 * MHZ))
		mdio_div = CSR_CLK_300_500_MHZ_MDC_CSR_DIV_204;
	else if ((csr_clk >= 500 * MHZ) && (csr_clk < 800 * MHZ))
		mdio_div = CSR_CLK_500_800_MHZ_MDC_CSR_DIV_324;
	else
		log_err("PFE: Invalid csr_clk");

	log_debug("DEB: csr_clock %llu, mdio_div %u\n", csr_clk, mdio_div);

	return mdio_div;
}

static void pfe_hw_init_lmem(struct pfe_hw *pfe)
{
	u32 i;

	for (i = 0; i < CBUS_LMEM_SIZE; i += 4)
		pfe_hw_write(pfe, CBUS_LMEM_BASE_ADDR + i, 0);
}

static int pfe_hw_safety_clear(struct pfe_hw *pfe)
{
	void __iomem *wsp_base = pfe_hw_phys_addr(pfe_hw_block_pa(pfe, CBUS_GLOBAL_CSR_BASE_ADDR));

	if (!wsp_base)
		return -EINVAL;

	/* Disable S32G3 safety */
	pfe_hw_write_reg(wsp_base, WSP_FAIL_STOP_MODE_INT_EN, 0x0);
	pfe_hw_write_reg(wsp_base, WSP_FAIL_STOP_MODE_EN, 0x0);
	pfe_hw_write_reg(wsp_base, WSP_ECC_ERR_INT_EN, 0x0);

	return 0;
}

/* external API */
void pfe_hw_hif_chnl_rings_attach(struct pfe_hw_ext *ext)
{
	pfe_hw_chnl_rings_attach(ext->hw_chnl);
}

int pfe_hw_init(struct pfe_hw_ext *ext, const struct pfe_hw_cfg *hw_cfg)
{
	struct pfe_hw *pfe;
	int ret = 0;
	enum pfe_hw_emac_block emac_id;

	pfe = kzalloc(sizeof(*pfe), GFP_KERNEL);
	if (!pfe)
		return -ENOMEM;

	ext->hw = pfe;
	_pfe = pfe;
	pfe->cfg = hw_cfg;
	pfe->on_g3 = hw_cfg->on_g3;

	/* Map CBUS address space */
	pfe->cbus_baseaddr = hw_cfg->cbus_base;
	pfe->base = pfe_hw_phys_addr(hw_cfg->cbus_base);
	if (!pfe->base) {
		dev_err(hw_cfg->dev, "Can't map PPFE CBUS\n");
		goto exit;
	}

	pfe->hif_base = pfe_hw_phys_addr(pfe_hw_block_pa(pfe, CBUS_HIF_BASE_ADDR));
	if (!pfe->hif_base) {
		ret = -EINVAL;
		goto exit;
	}

	/* Create HW components */
	pfe->hif_chnl = hw_cfg->hif_chnl_id;
	pfe->emac_mdio_div = pfe_hw_emac_mdio_div_decode(hw_cfg->csr_clk_f);

	/* clear safety on G3 */
	if (pfe->on_g3) {
		ret = pfe_hw_safety_clear(pfe);
		if (ret)
			goto exit;
	}

	/* Init LMEM */
	pfe_hw_init_lmem(pfe);

	/* BMU */
	ret = pfe_hw_init_bmu(pfe);
	if (ret)
		goto exit;

	/* GPI */
	ret = pfe_hw_init_gpi(pfe);
	if (ret)
		goto exit;

	/* TMU */
	ret = pfe_hw_init_tmu(pfe);
	if (ret)
		goto exit;

	/* Classifier */
	ret = pfe_hw_pe_init_class(&pfe->class, hw_cfg);
	if (ret)
		goto exit;

	/* EMAC */
	for (emac_id = PFENG_EMAC0; emac_id < PFENG_EMACS_COUNT; emac_id++) {
		struct pfe_hw_emac *hw_emac;

		if (!is_emac_active(hw_cfg, (u8)emac_id)) {
			dev_info(hw_cfg->dev, "EMAC%d not used, skipped\n", emac_id);
			continue;
		}

		hw_emac = devm_kzalloc(hw_cfg->dev, sizeof(*hw_emac), GFP_KERNEL);
		if (!hw_emac) {
			dev_err(hw_cfg->dev, "EMAC%d alloc failed\n", emac_id);
			ret = -ENOMEM;
			break;
		}

		ext->hw_emac[emac_id] = hw_emac;

		ret = pfe_hw_emac_init(hw_emac, emac_id, hw_cfg);
		if (ret) {
			dev_err(hw_cfg->dev, "EMAC%d init failed\n", emac_id);
			break;
		}
	}

	if (ret)
		goto exit;

	/* SOFT RESET */
	if (pfe->on_g3)
		ret = pfe_hw_soft_reset_g3(pfe);
	else
		ret = pfe_hw_soft_reset_g2(pfe);
	if (ret) {
		dev_err(hw_cfg->dev, "Platform reset failed\n");
		goto exit;
	}

	/* HIF (HIF DOES NOT LIKE SOFT RESET ONCE HAS BEEN CONFIGURED...) */
	ret = pfe_hw_hif_init(&pfe->hif, pfe->hif_base, pfe->hif_chnl, hw_cfg);
	if (ret)
		goto exit;

	/* Activate the classifier */
	dev_info(hw_cfg->dev, "Enabling the CLASS block\n");
	pfe_hw_pe_enable_class(&pfe->class);

	/* Wait to let classifier firmware to initialize */
	mdelay(50);

	/* Enable BMU */
	ret = pfe_hw_enable_bmu(pfe);
	if (ret)
		goto exit;

	/* Enable GPI */
	ret = pfe_hw_enable_gpi(pfe);
	if (ret)
		goto exit;

	dev_info(hw_cfg->dev, "PFE Platform started successfully (mask: %x)\n", hw_cfg->emac_mask);

	return 0;

exit:
	pfe_hw_remove(ext);
	return ret;
}

void pfe_hw_remove(struct pfe_hw_ext *ext)
{
	struct pfe_hw *pfe = ext->hw;
	void __iomem *wsp_base;
	u32 i;

	if (!pfe)
		return;

	if (!pfe->base)
		goto exit;

	/* Clear the generic control register */
	wsp_base = pfe_hw_phys_addr(pfe_hw_block_pa(pfe, CBUS_GLOBAL_CSR_BASE_ADDR));
	if (wsp_base)
		pfe_hw_write_reg(wsp_base, WSP_GENERIC_CONTROL, 0);

	pfe_hw_deinit_gpi(pfe);
	pfe_hw_deinit_bmu(pfe);

	pfe_hw_pe_deinit_class(&pfe->class);

	for (i = 0; i < PFENG_EMACS_COUNT; i++) {
		if (!ext->hw_emac[i]->base)
			continue;

		pfe_hw_emac_deinit(ext->hw_emac[i]);
	}

exit:
	pfe->cbus_baseaddr = 0x0ULL;
	kfree(pfe);
}

int pfe_hw_detect_version(phys_addr_t csr_base_addr, enum pfe_hw_ip_ver *pfe_ver)
{
	void __iomem *wsp_base = pfe_hw_phys_addr(pfe_hw_block_pa_of(csr_base_addr,
								     CBUS_GLOBAL_CSR_BASE_ADDR));
	const struct {
		u32 ver;
		const char *name;
		enum pfe_hw_ip_ver ip;
	} supported_vers[] = {
		 { WSP_VERSION_SILICON_G2, "S32G2", PFE_IP_S32G2 },
		 { WSP_VERSION_SILICON_G3, "S32G3", PFE_IP_S32G3 },
		 { 0 }
	};
	u32 val;
	u32 i;

	if (!wsp_base)
		return -EINVAL;

	/* read PFE IP version */
	val = pfe_hw_read_reg(wsp_base, PFE_GLOBAL_WSP_VERSION_OFF);
	if (val == 0xffffffff) {
		/* Special case -- no chip present */
		return -ENODEV;
	}

	*pfe_ver = PFE_IP_NONE;
	for (i = 0; i < ARRAY_SIZE(supported_vers); i++)
		if (!supported_vers[i].ver) {
			log_err("Invalid PFE version 0x%04x\n", val);
			return -EINVAL;
		} else if (val == supported_vers[i].ver) {
			log_info("\nFound PFE version 0x%04x (%s)\n", val,
				 supported_vers[i].name);

			*pfe_ver = supported_vers[i].ip;

			break;
		}

	return 0;
}

void pfe_hw_print_stats(struct pfe_hw_ext *ext)
{
	u32 i;

	printf("HIF statistics\n");
	pfe_hw_chnl_print_stats(ext->hw_chnl);

	for (i = 0; i < PFENG_EMACS_COUNT; i++)	{
		if (!ext->hw_emac[i] || !ext->hw_emac[i]->base)
			continue;

		printf("EMAC#%d statistics\n", i);
		pfe_hw_emac_print_stats(ext->hw_emac[i]);
	}
}
