/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017-2021 NXP
 */

/*
 * Configuration settings for all the Freescale S32R45 boards.
 */

#ifndef __S32R45_H
#define __S32R45_H

#ifdef CONFIG_PCIE_S32GEN1
#define PCIE_MSIS_ENV_SETTINGS	\
	PCIE_SET_MSI_CONTROLLER \
	"fdt_pcie0_spis_fixup=pcie_addr=40400000; run fdt_pcie_set_gic; \0"
#endif

#include <configs/s32-gen1.h>

#endif
