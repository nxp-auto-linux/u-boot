// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  2019 NXP
 */

/**
 * @addtogroup  dxgr_OAL_UTIL
 * @{
 *
 * @file		oal_util_uboot.c
 * @brief		The oal_util module source file.
 * @details		This file contains utility management implementation.
 *
 */

#include <vsprintf.h>
#include "oal_types.h"
#include "oal_util.h"
#include "hal.h"

uint32_t oal_util_snprintf(char_t *buffer, size_t buf_len, const char_t *format, ...)
{
	uint32_t len = 0;
	va_list ap;
	char_t warn[] = "!BUF_LEN_ERR!\n";

	if(buf_len == 0)
	{
		NXP_LOG_ERROR(" Wrong buffer size (oal_util_snprintf)\n");
		return 0;
	}
	va_start(ap, format);
	len = vsnprintf(NULL, 0, format, ap);
	if (len < buf_len)
	{
		len = vsnprintf(buffer, buf_len, format, ap);
	}
	else
	{
		if (buf_len < sizeof(warn))
		{
			warn[buf_len] = '\0';
		}
		len = snprintf(buffer, buf_len, warn);
	}
	va_end(ap);
	return len;
}

/** @}*/
