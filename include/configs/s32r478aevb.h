/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2025 NXP
 */

#ifndef __S32R478AEVB_H__
#define __S32R478AEVB_H__

#include <configs/s32-cc.h>

#define CONFIG_EXTRA_ENV_SETTINGS	S32CC_ENV_SETTINGS
#define EXTRA_BOOT_ARGS
#define FDT_FILE		"s32r478a-evb.dtb"

#define S32CC_SRAM_BASE		0x33C00000

#endif
