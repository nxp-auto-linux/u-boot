// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022-2025 NXP
 */
#include <common.h>
#include <fdtdec.h>
#include <image.h>

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	return 0;
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}
