// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2018-2019 NXP
 */

/**
 * @addtogroup  dxgr_PFE_HIF_NOCPY
 * @{
 *
 * @file		pfe_hif_nocpy_csr.h
 * @brief		The HIF_NOCOPY module registers definition file (s32g).
 * @details		This is the HW-specific part of the HIF_NOCOPY module.
 *				Applicable for IP versions listed below.
 *
 */

#ifndef PFE_HIF_NOCPY_CSR_H_
#define PFE_HIF_NOCPY_CSR_H_

#include "pfe_hif_nocpy.h"

#ifndef PFE_CBUS_H_
#error Missing cbus.h
#endif /* PFE_CBUS_H_ */

/*	Supported IPs. Defines are validated within pfe_cbus.h. */
#if (GLOBAL_CFG_IP_VERSION != IP_VERSION_FPGA_5_0_4) && (GLOBAL_CFG_IP_VERSION != IP_VERSION_NPU_7_14)
#error Unsupported IP version
#endif /* GLOBAL_CFG_IP_VERSION */

#define HIF_NOCPY_VERSION			(0x00U)
#define HIF_NOCPY_TX_CTRL			(0x04U)
#define HIF_NOCPY_TX_CURR_BD_ADDR	(0x08U)
#define HIF_NOCPY_TX_ALLOC			(0x0cU)
#define HIF_NOCPY_TX_BDP_ADDR		(0x10U)
#define HIF_NOCPY_TX_STATUS			(0x14U)
#define HIF_NOCPY_RX_CTRL			(0x20U)
#define HIF_NOCPY_RX_BDP_ADDR		(0x24U)
#define HIF_NOCPY_RX_STATUS			(0x30U)
#define HIF_NOCPY_INT_SRC			(0x34U)
#define HIF_NOCPY_INT_EN			(0x38U)
#define HIF_NOCPY_POLL_CTRL			(0x3cU)
#define HIF_NOCPY_RX_CURR_BD_ADDR	(0x40U)
#define HIF_NOCPY_RX_ALLOC			(0x44U)
#define HIF_NOCPY_TX_DMA_STATUS		(0x48U)
#define HIF_NOCPY_RX_DMA_STATUS		(0x4cU)
#define HIF_NOCPY_RX_INQ0_PKTPTR	(0x50U)
#define HIF_NOCPY_RX_INQ1_PKTPTR	(0x54U)
#define HIF_NOCPY_TX_PORT_NO		(0x60U)
#define HIF_NOCPY_LMEM_ALLOC_ADDR	(0x64U)
#define HIF_NOCPY_CLASS_ADDR		(0x68U)
#define HIF_NOCPY_TMU_PORT0_ADDR	(0x70U)
#define HIF_NOCPY_TMU_PORT1_ADDR	(0x74U)
#define HIF_NOCPY_TMU_PORT2_ADDR	(0x7cU)
#define HIF_NOCPY_TMU_PORT3_ADDR	(0x80U)
#define HIF_NOCPY_TMU_PORT4_ADDR	(0x84U)
#define HIF_NOCPY_INT_COAL_ADDR		(0x90U)
#define HIF_NOCPY_CSR_AXI_WAIT_DONE	(0x94U)
#define HIF_NOCPY_ABS_FRAME_CNT		(0x98U)

/*	HIF_NOCPY_INT_SRC, HIF_NOCPY_INT_EN bits */
#define HIF_NOCPY_INT				(1U << 0)
#define BDP_CSR_RX_CBD_INT			(1U << 1)
#define BDP_CSR_RX_PKT_INT			(1U << 2)
#define BDP_CSR_TX_CBD_INT			(1U << 3)
#define BDP_CSR_TX_PKT_INT			(1U << 4)

errno_t pfe_hif_nocpy_cfg_isr(void *base_va);
void pfe_hif_nocpy_cfg_irq_mask(void *base_va);
void pfe_hif_nocpy_cfg_irq_unmask(void *base_va);
errno_t pfe_hif_nocpy_cfg_set_cbk(pfe_hif_chnl_event_t event, pfe_hif_chnl_cbk_t cbk, void *arg);
errno_t pfe_hif_nocpy_cfg_init(void *base_va);
void pfe_hif_nocpy_cfg_fini(void *base_va);
void pfe_hif_nocpy_cfg_tx_enable(void *base_va);
void pfe_hif_nocpy_cfg_tx_disable(void *base_va);
void pfe_hif_nocpy_cfg_rx_enable(void *base_va);
void pfe_hif_nocpy_cfg_rx_disable(void *base_va);
void pfe_hif_nocpy_cfg_rx_dma_start(void *base_va);
void pfe_hif_nocpy_cfg_tx_dma_start(void *base_va);
void pfe_hif_nocpy_cfg_rx_irq_mask(void *base_va);
void pfe_hif_nocpy_cfg_rx_irq_unmask(void *base_va);
void pfe_hif_nocpy_cfg_tx_irq_mask(void *base_va);
void pfe_hif_nocpy_cfg_tx_irq_unmask(void *base_va);
void pfe_hif_nocpy_cfg_set_rx_bd_ring_addr(void *base_va, void *rx_ring_pa);
void pfe_hif_nocpy_cfg_set_tx_bd_ring_addr(void *base_va, void *tx_ring_pa);
bool_t pfe_hif_nocpy_cfg_is_rx_dma_active(void *base_va);
bool_t pfe_hif_nocpy_cfg_is_tx_dma_active(void *base_va);
uint32_t pfe_hif_nocpy_chnl_cfg_get_text_stat(void *base_va, char_t *buf, uint32_t size, uint8_t verb_level);
uint32_t pfe_hif_nocpy_cfg_get_text_stat(void *base_va, char_t *buf, uint32_t size, uint8_t verb_level);

#endif /* PFE_HIF_NOCPY_CSR_H_ */