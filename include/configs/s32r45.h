/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017-2023 NXP
 */
#ifndef __S32R45_H__
#define __S32R45_H__

#include <configs/s32-cc.h>

#define CONFIG_EXTRA_ENV_SETTINGS \
	S32CC_ENV_SETTINGS \

#define GMAC1_ENABLE_VAR_VALUE		"s32cc_gmac1_mode=enable\0"

#define EXTRA_BOOTCOMMAND		""

#endif
