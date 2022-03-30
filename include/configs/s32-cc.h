/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 NXP
 */
#ifndef __S32CC_H__
#define __S32CC_H__

#include <linux/sizes.h>

#define PHYS_SDRAM_1			0x80000000UL
#define PHYS_SDRAM_1_SIZE		(SZ_2G)
#define PHYS_SDRAM_2			0x880000000UL
#define PHYS_SDRAM_2_SIZE		(SZ_2G)

#define CONFIG_SYS_INIT_SP_OFFSET	(SZ_16K)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_DATA_BASE + CONFIG_SYS_INIT_SP_OFFSET)

#endif
