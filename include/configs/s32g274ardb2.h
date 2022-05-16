/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2022 NXP
 */
#ifndef __S32G274ARDB2_H__
#define __S32G274ARDB2_H__

#include <configs/s32g2.h>

#define EXTRA_BOOTCOMMAND		PFE_INIT_CMD
#define EXTRA_BOOT_ARGS			PFE_EXTRA_BOOT_ARGS
#define FDT_FILE			"s32g274a-rdb2.dtb"

#ifdef CONFIG_FSL_PFENG
#  define PFENG_EMAC			"0"
#  define PFENG_MODE			"enable,sgmii,none,rgmii"
#endif

#endif
