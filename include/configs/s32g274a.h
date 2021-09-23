/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2017-2021 NXP
 */

/*
 * Configuration settings for all the Freescale S32G274A boards.
 */

#ifndef __S32G274A_H
#define __S32G274A_H

#ifdef CONFIG_PCIE_S32GEN1
#define PCIE_MSIS_ENV_SETTINGS	\
	PCIE_SET_MSI_CONTROLLER \
	"fdt_pcie0_spis_fixup=pcie_addr=40400000; run fdt_pcie_set_gic; \0" \
	"fdt_pcie1_spis_fixup=pcie_addr=44100000; run fdt_pcie_set_gic; \0"
#endif

#include <configs/s32-gen1.h>

#if defined(CONFIG_TARGET_S32G2XXAEVB)
#define FDT_FILE fsl-s32g2xxa-evb.dtb

#if defined(CONFIG_USB)
#define CONFIG_USB_EHCI_MX6
#define CONFIG_MXC_USB_PORTSC        PORT_PTS_ULPI
#endif

#elif defined(CONFIG_TARGET_S32G274ARDB)
#ifdef CONFIG_S32G274ARDB
#define FDT_FILE fsl-s32g274a-rdb.dtb
#else
#define FDT_FILE fsl-s32g274a-rdb2.dtb
#endif /* CONFIG_TARGET_S32G274ARDB */

#elif defined(CONFIG_TARGET_S32G274ABLUEBOX3)
#define FDT_FILE fsl-s32g274a-bluebox3.dtb
#endif


#endif
