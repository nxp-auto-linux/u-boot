// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2019-2020 NXP
 */

/**
 * @addtogroup  dxgr_OAL_IRQ
 * @{
 *
 * @file		oal_irq_uboot.c
 * @brief		The oal_irq module source file (UBOOT).
 * @details		This file contains empty implementation of oal_irq API for UBOOT.
 *
 */

#include "oal.h"
#include "oal_irq.h"
#include "linked_list.h"

/**
 * @brief	The IRQ instance representation
 */
typedef uint32_t __oal_irq_tag;

static __oal_irq_tag singl;

oal_irq_t * oal_irq_create(int32_t id, oal_irq_flags_t flags, char_t *name)
{
	return (oal_irq_t *)&singl;
}

errno_t oal_irq_add_handler(oal_irq_t *irq, oal_irq_handler_t handler, void *data, oal_irq_isr_handle_t *handle)
{
	return 0;
}

errno_t oal_irq_mask(oal_irq_t *irq)
{
	return 0;
}

errno_t oal_irq_del_handler(oal_irq_t *irq, oal_irq_isr_handle_t handle)
{
	return 0;
}

void oal_irq_destroy(oal_irq_t *irq)
{
	return;
}

errno_t oal_irq_unmask(oal_irq_t *irq)
{
	return 0;
}

int32_t oal_irq_get_id(oal_irq_t *irq)
{
	return 0;
}

errno_t oal_irq_get_flags(oal_irq_t *irq, oal_irq_flags_t *flags)
{
	return 0;
}

/** @}*/
