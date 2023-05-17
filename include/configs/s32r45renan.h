/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2023 NXP
 */
#ifndef __S32R45RENAN_H__
#define __S32R45RENAN_H__

#include <configs/s32r45.h>

/* Renan board has only 1GB RAM. Remap the memory layout. */
#undef PHYS_SDRAM_1_SIZE
#define PHYS_SDRAM_1_SIZE   (SZ_1G)
#undef PHYS_SDRAM_2

#define CONFIG_EXTRA_ENV_SETTINGS	S32CC_ENV_SETTINGS

#define EXTRA_BOOT_ARGS		""
#define FDT_FILE		"s32r45-renan.dtb"

#endif
