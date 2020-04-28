/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2018-2019 NXP
 */

/**
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 * 
 * @defgroup    dxgr_PFE_GPI GPI
 * @brief		The Generic Packet Interface
 * @details     This is the software representation of the GPI block.
 * 
 * @addtogroup  dxgr_PFE_GPI
 * @{
 * 
 * @file		pfe_gpi.h
 * @brief		The GPI module header file.
 * @details		This file contains GPI-related API.
 *
 */

#ifndef PUBLIC_PFE_GPI_H_
#define PUBLIC_PFE_GPI_H_

typedef struct __pfe_gpi_tag pfe_gpi_t;

typedef struct
{
	uint32_t alloc_retry_cycles;		/* Number of system clock cycles, the state machine has to wait before retrying in case the buffers are full at the buffer manager */
	uint32_t gpi_tmlf_txthres;		/* */
	uint32_t gpi_dtx_aseq_len;		/* */
	bool_t emac_1588_ts_en;		/* If TRUE then the 1588 time-stamping is enabled */
} pfe_gpi_cfg_t;

pfe_gpi_t *pfe_gpi_create(void *cbus_base_va, void *gpi_base, pfe_gpi_cfg_t *cfg);
void pfe_gpi_enable(pfe_gpi_t *gpi);
void pfe_gpi_reset(pfe_gpi_t *gpi);
void pfe_gpi_disable(pfe_gpi_t *gpi);
uint32_t pfe_gpi_get_text_statistics(pfe_gpi_t *gpi, char_t *buf, uint32_t buf_len, uint8_t verb_level);
void pfe_gpi_destroy(pfe_gpi_t *gpi);

#endif /* PUBLIC_PFE_GPI_H_ */

/** @}*/
/** @}*/
