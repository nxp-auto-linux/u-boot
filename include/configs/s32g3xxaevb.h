/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2022 NXP
 */
#ifndef __S32G3XXAEVB_H__
#define __S32G3XXAEVB_H__

#include <configs/s32g3.h>

#define EXTRA_BOOT_ARGS			""
#define FDT_FILE			"s32g3xxa-evb.dtb"

#if defined(CONFIG_USB)
#	define CONFIG_MXC_USB_PORTSC	PORT_PTS_ULPI
#	define CONFIG_USB_EHCI_MX6
#endif

#endif
