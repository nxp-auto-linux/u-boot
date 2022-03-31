/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2022 NXP
 */
#ifndef __S32G_H__
#define __S32G_H__

#include <configs/s32-cc.h>

#ifdef CONFIG_FSL_PFENG
#  define PFE_EXTRA_BOOT_ARGS "nohz=off coherent_pool=64M"
#  define PFE_EXTRA_ENV_SETTINGS \
	"pfeng_mode=" PFENG_MODE "\0" \
	"pfeaddr=00:01:be:be:ef:11\0" \
	"pfe1addr=00:01:be:be:ef:22\0" \
	"pfe2addr=00:01:be:be:ef:33\0" \
	"ethact=eth_pfeng\0" \
	"pfengemac=" PFENG_EMAC "\0"
#  define PFE_INIT_CMD "pfeng stop; "
#else
#  define PFE_EXTRA_BOOT_ARGS
#  define PFE_EXTRA_ENV_SETTINGS
#  define PFE_INIT_CMD ""
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	S32CC_ENV_SETTINGS \
	PFE_EXTRA_ENV_SETTINGS \

#define GMAC1_ENABLE_VAR_VALUE ""

#endif
