// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2018-2019 NXP
 */

/**
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 * 
 * @defgroup    dxgr_PFE_HIF HIF
 * @brief		The Host Interface
 * @details     This is the software representation of the HIF block.
 * 
 * @addtogroup  dxgr_PFE_HIF
 * @{
 * 
 * @file		pfe_hif.h
 * @brief		The HIF module header file.
 * @details		This file contains HIF-related API.
 *
 */

#ifndef PUBLIC_PFE_HIF_H_
#define PUBLIC_PFE_HIF_H_

#include "pfe_hif_ring.h"
#include "pfe_hif_chnl.h"

typedef enum
{
	HIF_CHNL_0 = (1 << 0),
	HIF_CHNL_1 = (1 << 1),
	HIF_CHNL_2 = (1 << 2),
	HIF_CHNL_3 = (1 << 3)
} pfe_hif_chnl_id_t;

typedef struct __pfe_hif_tag pfe_hif_t;

pfe_hif_t *pfe_hif_create(void *base_va, pfe_hif_chnl_id_t channels);
pfe_hif_chnl_t *pfe_hif_get_channel(pfe_hif_t *hif, pfe_hif_chnl_id_t channel_id);
void pfe_hif_destroy(pfe_hif_t *hif);

#ifdef GLOBAL_CFG_PFE_MASTER
errno_t pfe_hif_isr(pfe_hif_t *hif);
void pfe_hif_irq_mask(pfe_hif_t *hif);
void pfe_hif_irq_unmask(pfe_hif_t *hif);
uint32_t pfe_hif_get_text_statistics(pfe_hif_t *hif, char_t *buf, uint32_t buf_len, uint8_t verb_level);
#endif /* GLOBAL_CFG_PFE_MASTER */

#endif /* PUBLIC_PFE_HIF_H_ */

/** @}*/
/** @}*/
