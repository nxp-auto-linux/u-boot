/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2019 NXP
 */

/**
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 *
 * @defgroup    dxgr_PFE_SAFETY SAFETY
 * @brief		The safety interrupts unit
 * @details     This is the safety interrupts unit.
 *
 * @addtogroup  dxgr_PFE_SAFETY
 * @{
 *
 * @file		pfe_safety.h
 * @brief		The SAFETY module header file.
 * @details		This file contains SAFETY-related API.
 *
 */

#ifndef PUBLIC_PFE_SAFETY_H_
#define PUBLIC_PFE_SAFETY_H_

typedef struct __pfe_safety_tag pfe_safety_t;

pfe_safety_t *pfe_safety_create(void *cbus_base_va, void *safety_base);
void pfe_safety_destroy(pfe_safety_t *safety);
errno_t pfe_safety_isr(pfe_safety_t *safety);
void pfe_safety_irq_mask(pfe_safety_t *safety);
void pfe_safety_irq_unmask(pfe_safety_t *safety);

#endif /* PUBLIC_PFE_SAFETY_H_ */

/** @}*/
