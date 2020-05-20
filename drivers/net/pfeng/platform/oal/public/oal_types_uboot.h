/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 *  Copyright 2019-2020 NXP
 */

#ifndef __OAL_TYPES_UBOOT__
#define __OAL_TYPES_UBOOT__

#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <vsprintf.h>
#include <errno.h>
#include <stddef.h>
#include <linux/string.h>
#include <asm/types.h>
#include <asm/byteorder.h>

#ifndef EOK
#define EOK 0
#endif

/* Boolean values definition*/
#ifndef TRUE
#define TRUE 1
#endif /* TRUE */

#ifndef FALSE
#define FALSE 0
#endif /* FALSE */

typedef bool bool_t;
typedef int errno_t;
typedef char char_t;
typedef int
	int_t; /* For use within printf like functions that require "int" regardless its size */
typedef unsigned int uint_t; /* For use within printf like functions */

#define oal_htons(x) htons(x)
#define oal_ntohs(x) ntohs(x)
#define oal_htonl(x) htonl(x)
#define oal_ntohl(x) ntohl(x)

typedef unsigned int uint32_t;
typedef signed int int32_t;

#if defined(PFE_CFG_TARGET_ARCH_aarch64)
typedef unsigned long long addr_t;
#define MAX_ADDR_T_VAL ((unsigned long long)(~0ULL))
#define PRINT64	       "l"
#define PRINTADDR_T    "llu"
#else
#error Unsupported or no platform defined
#endif

#if defined(PFE_CFG_BUILD_PROFILE_DEBUG)
#define NXP_LOG_ERROR(format, ...) \
	printf("ERR[%s:%d]: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#define NXP_LOG_WARNING(format, ...) \
	printf("WRN[%s:%d]: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#define NXP_LOG_INFO(format, ...) \
	printf("INF[%s:%d]: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#define NXP_LOG_DEBUG(format, ...) \
	printf("DBG[%s:%d]: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#elif defined(PFE_CFG_BUILD_PROFILE_RELEASE) || \
	defined(BUILD_PROFILE_PROFILE) || defined(BUILD_PROFILE_COVERAGE)
#define NXP_LOG_ERROR(format, ...) \
	printf("ERR[%s:%d]: " format, __FILE__, __LINE__, ##__VA_ARGS__)
#define NXP_LOG_WARNING(format, ...) \
	{ /* be quiet */             \
	}
#define NXP_LOG_INFO(format, ...) \
	{ /* be quiet */          \
	}
#define NXP_LOG_DEBUG(format, ...) \
	{ /* be quiet */           \
	}
#endif /* PFE_CFG_BUILD_PROFILE */

#endif /* __OAL_TYPES_UBOOT__ */
