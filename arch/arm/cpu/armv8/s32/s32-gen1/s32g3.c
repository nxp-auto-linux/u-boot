// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2021 NXP
 */
#include <asm/arch/cpu.h>

u64 fdt_to_cpu_id(u64 fdt_id)
{
	return (fdt_id & 0x3u) | (fdt_id >> 0x6u);
}
