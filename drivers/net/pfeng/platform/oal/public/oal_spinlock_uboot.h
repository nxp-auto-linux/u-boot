/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  2019 NXP
 */

/**
 * @addtogroup  dxgr_OAL_SYNC
 * @{
 * 
 * @file		oal_spinlock_uboot.h
 * @brief		The UBOOT-specific spinlock implementation.
 * @details		This file contains glue for UBOOT-specific spinlock implementation.
 *
 */

#ifndef PUBLIC_OAL_SPINLOCK_UBOOT_H_
#define PUBLIC_OAL_SPINLOCK_UBOOT_H_

typedef uint32_t oal_spinlock_t;

static inline errno_t oal_spinlock_init(oal_spinlock_t *spinlock)
{
	return 0;
}

static inline errno_t oal_spinlock_destroy(oal_spinlock_t *spinlock)
{
	return 0;
}

static inline errno_t oal_spinlock_lock(oal_spinlock_t *spinlock)
{
	return 0;
}

static inline errno_t oal_spinlock_unlock(oal_spinlock_t *spinlock)
{
	return 0;
}

#endif /* PUBLIC_OAL_SPINLOCK_UBOOT_H_ */

