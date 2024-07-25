/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2022-2024 NXP
 */

#ifndef __S32G399ARDB3_H__
#define __S32G399ARDB3_H__

#include <configs/s32g3.h>

#define EXTRA_BOOT_ARGS			""

#ifndef CONFIG_NXP_PFENG_SLAVE
#define FDT_FILE			"s32g399a-rdb3.dtb"
#else
#define FDT_FILE			"s32g399a-rdb3-pfems.dtb"
#endif

#endif
