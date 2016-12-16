/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2016 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Configuration settings for the Freescale S32V234 mini Bluebox board
 */

#ifndef __CONFIG_H
#define __CONFIG_H

/* The configurations of this board depend on the definitions in this file and
* the ones in the header included at the end, configs/s32v234evb_2016q4.h */

#define	FDT_FILE s32v234-bbmini.dtb

#define CONFIG_CMD_PCI
#ifdef CONFIG_CMD_PCI
#define CONFIG_PCIE_EP_MODE
#endif

/* we include this file here because it depends on the above definitions */
#include <configs/s32v234evb_2016q4.h>

#endif
