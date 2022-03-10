// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2017-2022 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <mmc.h>
#include <div64.h>
#include <errno.h>
#include <hang.h>
#include <board_common.h>
#include <fdtdec.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <generic-phy.h>

DECLARE_GLOBAL_DATA_PTR;

__weak int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}
