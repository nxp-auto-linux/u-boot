/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2017-2022 NXP
 */

/*
 * Configuration settings for all the Freescale S32G274A boards.
 */

#ifndef __S32G274A_H
#define __S32G274A_H

#include <configs/s32-gen1.h>

#if defined(CONFIG_TARGET_S32G2XXAEVB)
#define FDT_FILE fsl-s32g2xxa-evb.dtb

#if defined(CONFIG_USB)
#define CONFIG_USB_EHCI_MX6
#define CONFIG_MXC_USB_PORTSC        PORT_PTS_ULPI
#endif

#elif defined(CONFIG_TARGET_S32G274ARDB2)
#define FDT_FILE fsl-s32g274a-rdb2.dtb

#elif defined(CONFIG_TARGET_S32G274ABLUEBOX3)
#define FDT_FILE fsl-s32g274a-bluebox3.dtb
#endif


#endif
