/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2019-2020 NXP
 */

/**
 * @addtogroup	dxgrOAL
 * @{
 *
 * @defgroup    dxgr_OAL_UTIL UTIL
 * @brief		Advanced utilities
 * @details		TODO
 *
 *
 * @addtogroup	dxgr_OAL_UTIL
 * @{
 *
 * @file		oal_util.h
 * @brief		The oal_util module header file.
 * @details		This file contains utility management-related API.
 *
 */

#ifndef OAL_UTIL_H_
#define OAL_UTIL_H_

#ifdef PFE_CFG_TARGET_OS_AUTOSAR
#include "oal_util_autosar.h"
#endif /* PFE_CFG_TARGET_OS_AUTOSAR */

/**
 * @brief		Modified snprintf function
 * @details		Function return real number of written data into buffer
 * @details		in case of lack of space fills buffer with warming message.
 * @param[in]	*buffer buffer to write data
 * @param[in]	buf_len buffer length
 * @param[in]	Format input data format (same as printf format)
 * @param[in]	... variable arguments according to Format
 *
 * @return		Number of bytes written to the buffer
 */
uint32_t oal_util_snprintf(char_t *buffer, size_t buf_len,
			   const char_t *format, ...);

#endif /* OAL_UTIL_H_ */

/** @}*/
/** @}*/
