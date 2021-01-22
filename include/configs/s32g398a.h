/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2021 NXP
 */

/*
 * Configuration settings for all the Freescale S32G398A boards.
 */

#ifndef __S32G398A_H
#define __S32G398A_H

#include <configs/s32-gen1.h>

#if !defined(CONFIG_PRAM) && !defined(CONFIG_S32_SKIP_RELOC)

/* 24 MB covering the following:
 *  - 22 MB for optee_os + shared memory between optee_os and linux kernel
 *  - 2 MB for the Secure Monitor
 */
#define CONFIG_PRAM	24576	/* 24MB */
#endif

#endif
