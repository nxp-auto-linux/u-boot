/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2019 NXP
 */

/**
 * @addtogroup  dxgr_PFE_SAFETY
 * @{
 *
 * @file		pfe_safety_csr.h
 * @brief		The safety module registers definition file (s32g).
 * @details
 *
 */

#ifndef PFE_SAFETY_CSR_H_
#define PFE_SAFETY_CSR_H_

#include "pfe_safety.h"

errno_t pfe_safety_cfg_isr(void *base_va);
void pfe_safety_cfg_irq_mask(void *base_va);
void pfe_safety_cfg_irq_unmask(void *base_va);
void pfe_safety_cfg_irq_unmask_all(void *base_va);

#endif /* PFE_SAFETY_CSR_H_ */

/** @}*/
