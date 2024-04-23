/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2024 NXP
 */
#ifndef __S32N79_H
#define __S32N79_H

#include <linux/sizes.h>
#include <generated/autoconf.h>

#define CONFIG_SYS_INIT_SP_OFFSET	(SZ_16K)
#define CONFIG_SYS_INIT_SP_ADDR		(CONFIG_SYS_DATA_BASE + \
					 CONFIG_SYS_MALLOC_F_LEN + \
					 GENERATED_GBL_DATA_SIZE + \
					 CONFIG_SYS_INIT_SP_OFFSET)

/* Increase max gunzip size */
#define CONFIG_SYS_BOOTM_LEN	(SZ_64M)

#define CONFIG_SYS_CBSIZE		(SZ_512)
#endif
