/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2022 NXP
 */
#ifndef __S32G3XXAEVB_H__
#define __S32G3XXAEVB_H__

#include <configs/s32g3.h>

#define EXTRA_BOOTCOMMAND		PFE_INIT_CMD
#define EXTRA_BOOT_ARGS			PFE_EXTRA_BOOT_ARGS
#define FDT_FILE			"s32g3xxa-evb.dtb"

#ifdef CONFIG_FSL_PFENG
#  define PFENG_MODE			"enable,sgmii,sgmii,rgmii"
#  define PFENG_EMAC			"0"
#endif

#if defined(CONFIG_USB)
#  define CONFIG_USB_EHCI_MX6
#  define CONFIG_MXC_USB_PORTSC		PORT_PTS_ULPI
#endif

#endif
