/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2020 NXP
 *
 */

/**
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 *
 * @defgroup    dxgr_PFE_WDT WDT
 * @brief		The watchdog timer unit
 * @details     This is the watchdog timer unit.
 *
 * @addtogroup  dxgr_PFE_WDT
 * @{
 *
 * @file		pfe_wdt.h
 * @brief		The WDT module header file.
 * @details		This file contains WDT-related API.
 *
 */

#ifndef PUBLIC_PFE_WDT_H_
#define PUBLIC_PFE_WDT_H_

typedef struct __pfe_wdt_tag pfe_wdt_t;

typedef struct {
	void *pool_pa; /* f.e. which irq enable	not valid */
	void *pool_va; /* f.e. timer settings  not valid*/
} pfe_wdt_cfg_t;

pfe_wdt_t *pfe_wdt_create(void *cbus_base_va, void *wdt_base);
void pfe_wdt_destroy(pfe_wdt_t *wdt);
errno_t pfe_wdt_isr(pfe_wdt_t *wdt);
void pfe_wdt_irq_mask(pfe_wdt_t *wdt);
void pfe_wdt_irq_unmask(pfe_wdt_t *wdt);
uint32_t pfe_wdt_get_text_statistics(pfe_wdt_t *wdt, char_t *buf,
				     u32 buf_len, uint8_t verb_level);

#endif /* PUBLIC_PFE_WDT_H_ */

/** @}*/
