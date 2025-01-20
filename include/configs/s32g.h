/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2023, 2025 NXP
 */
#ifndef __S32G_H__
#define __S32G_H__

#include <configs/s32-cc.h>

#define CONFIG_EXTRA_ENV_SETTINGS S32CC_ENV_SETTINGS

/* Ethernet */
#define CONFIG_SYS_RX_ETH_BUFFER	256

#define S32CC_SRAM_BASE		0x34000000

#endif
