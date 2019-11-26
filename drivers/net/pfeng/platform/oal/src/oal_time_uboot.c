// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  2019 NXP
 */

/**
 * @addtogroup  dxgr_OAL_TIME
 * @{
 * 
 * @file        oal_time_autosar.c
 * @brief       The oal_time module source file (UBOOT variant).
 * @details     This file contains UBOOT-specific time management implementation.
 *
 */

#include <linux/delay.h>

#include "oal_types.h"

void oal_time_usleep(uint32_t usec)
{
	ndelay(usec * 1000);
}

void oal_time_mdelay(uint32_t msec)
{
	mdelay(msec);
}

/** @}*/
