/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2018-2019 NXP
 */

/**
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 *
 * @defgroup    dxgr_PFE_HIF_NOCPY HIF_NOCPY
 * @brief		The Host Interface (NO COPY)
 * @details     This is the software representation of the HIF_NOCPY block.
 *
 * @addtogroup  dxgr_PFE_HIF_NOCPY
 * @{
 *
 * @file		pfe_hif_nocpy.h
 * @brief		The HIF module header file.
 * @details		This file contains HIF_NOCPY-related API.
 *
 */

#ifndef PUBLIC_PFE_HIF_NOCPY_H_
#define PUBLIC_PFE_HIF_NOCPY_H_

#include "pfe_hif_ring.h"
#include "pfe_hif_chnl.h"
#include "pfe_bmu.h"

typedef struct
{
	uint32_t nothing; /* Some compilers don't support empty structs */
} pfe_hif_nocpy_cfg_t;

typedef struct __pfe_hif_nocpy_tag pfe_hif_nocpy_t;

pfe_hif_nocpy_t *pfe_hif_nocpy_create(void *base_va, pfe_bmu_t *bmu);
pfe_hif_chnl_t *pfe_hif_nocpy_get_channel(pfe_hif_nocpy_t *hif, uint32_t channel_id);
uint32_t pfe_hif_nocpy_get_text_statistics(pfe_hif_nocpy_t *hif, char_t *buf, uint32_t buf_len, uint8_t verb_level);
void pfe_hif_nocpy_destroy(pfe_hif_nocpy_t *hif);

#endif /* PUBLIC_PFE_HIF_NOCPY_H_ */

/** @}*/
/** @}*/
