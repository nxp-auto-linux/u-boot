/* SPDX-License-Identifier: GPL 2.0 */
/*
 *  Copyright (c) 2020 Imagination Technologies Limited
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

#define _ct_assert(param) _Static_assert(param, "Error in " __FILE__)
#define STRINGIFY(data) (# data)
#define TOSTRING(data) (STRINGIFY(data))

#endif /* __OAL_TYPES_UBOOT__ */
