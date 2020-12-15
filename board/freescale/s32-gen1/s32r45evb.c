// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2021 NXP
 */

#include "asm/arch-s32/siul-s32r45.h"
#include "board_common.h"
#include <asm/arch/soc.h>
#include <common.h>

void setup_iomux_uart(void)
{
#if (CONFIG_FSL_LINFLEX_MODULE == 0)

	/* Muxing for linflex0 */
	setup_iomux_uart0_pc09_pc10();

#else
#error "Unsupported UART pinmuxing configuration"
#endif
}
