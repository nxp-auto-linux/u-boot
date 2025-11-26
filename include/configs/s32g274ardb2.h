/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022,2025 NXP
 */

#ifndef __S32G274ARDB2_H__
#define __S32G274ARDB2_H__

#include <configs/s32g2.h>

#define EXTRA_BOOT_ARGS			""

#ifndef CONFIG_NXP_PFENG_SLAVE
#define FDT_FILE			"s32g274a-rdb2.dtb"
#else
#define FDT_FILE			"s32g274a-rdb2-pfems.dtb"
#endif

#endif
