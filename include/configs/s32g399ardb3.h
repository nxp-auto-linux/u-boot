/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2022 NXP
 */
#ifndef __S32G399ARDB3_H__
#define __S32G399ARDB3_H__

#include <configs/s32g3.h>

#define EXTRA_BOOTCOMMAND		PFE_INIT_CMD
#define EXTRA_BOOT_ARGS			PFE_EXTRA_BOOT_ARGS
#define FDT_FILE			"s32g399a-rdb3.dtb"

#ifdef CONFIG_FSL_PFENG
#  define PFENG_MODE			"enable,sgmii,sgmii,rgmii"
#  define PFENG_EMAC			"0"
#endif

#endif
