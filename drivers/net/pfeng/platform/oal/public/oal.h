/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2018-2020 NXP
 */

/**
 * @defgroup	dxgrOAL OAL
 * @brief		The OS Abstraction Layer
 * @details		OAL is intended to provide OS abstraction. To write portable SW one shall use
 * 				OAL calls instead of OS-specific ones. This OAL module incorporates following
 * 				functionality:
 * 				
 * 				- oal_irq - Interrupt management
 * 				- oal_mbox - Message-based IPC
 * 				- oal_mm - Memory management
 * 				- oal_types - Abstraction of standard types
 * 				- oal_thread - Threading support
 * 				- oal_sync - Thread synchronization
 * 				- oal_util - Simplification utility
 * 				- oaj_job - Job context abstraction
 * 				
 * 
 * @addtogroup	dxgrOAL
 * @{
 * 
 * @file		oal.h
 * @brief		The main OAL header file
 * @details		Use this header to include all the OAL-provided functionality
 *
 */

#ifndef __OAL__
#define __OAL__

#include "oal_types.h"
#include "oal_mbox.h"
#include "oal_irq.h"
#include "oal_mm.h"
#include "oal_thread.h"
#include "oal_sync.h"
#include "oal_time.h"
#include "oal_util.h"
#include "oal_job.h"

/*
 * Compile-time assert
 */
#define OAL_ASSERT_CONCAT_(a, b) a##b
#define OAL_ASSERT_CONCAT(a, b) OAL_ASSERT_CONCAT_(a, b)
#define oal_ct_assert(e) enum { OAL_ASSERT_CONCAT(precompile_assert_, __LINE__) = 1/(!!(e)) }

#endif /* __OAL__ */

/** @}*/
