// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2019 NXP
 */

#include <common.h>
#include <asm/arch/soc.h>

#include "board_common.h"

void setup_iomux_uart(void)
{
#if (CONFIG_FSL_LINFLEX_MODULE == 1)

	/* Muxing for linflex1 */
	setup_iomux_uart1_pb09_pb10();

#else
#error "Unsupported UART pinmuxing configuration"
#endif
}

void setup_iomux_i2c(void) {}

#ifdef CONFIG_FSL_DSPI
void setup_iomux_dspi(void) {}
#endif

void setup_iomux_sdhc(void) {}
