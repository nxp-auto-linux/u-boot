/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2021-2022 NXP
 */

/*
 * Configuration settings for all the Freescale S32G399A boards.
 */

#ifndef __S32G399A_H
#define __S32G399A_H

#include <configs/s32-gen1.h>

#if defined(CONFIG_TARGET_S32G3XXAEVB)
#define FDT_FILE			fsl-s32g3xxa-evb.dtb

#if defined(CONFIG_USB)
#define CONFIG_USB_EHCI_MX6
#define CONFIG_MXC_USB_PORTSC		PORT_PTS_ULPI
#endif

#elif defined(CONFIG_TARGET_S32G399ARDB3)
#define FDT_FILE			'fsl-s32g399a-rdb3.dtb'

#endif

#endif
