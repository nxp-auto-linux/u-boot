/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  2019 NXP
 */

/**
 * @addtogroup  dxgr_OAL_SYNC
 * @{
 *
 * @file		oal_mutex_uboot.h
 * @brief		The UBOOT-specific mutex implementation.
 * @details		This file contains empty implementation of oal_mutex API for UBOOT.
 *
 */

#ifndef __OAL_MUTEX_UBOOT_H__
#define __OAL_MUTEX_UBOOT_H__

#include "hal.h"

typedef uint32_t oal_mutex_t;

static inline errno_t oal_mutex_init(oal_mutex_t *mutex)
{
	return 0;
}

static inline errno_t oal_mutex_destroy(oal_mutex_t *mutex)
{
	return 0;
}

static inline errno_t oal_mutex_lock(oal_mutex_t *mutex)
{
	return 0;
}

static inline errno_t oal_mutex_unlock(oal_mutex_t *mutex)
{
	return 0;
}

#endif /* __OAL_MUTEX_UBOOT_H__ */
