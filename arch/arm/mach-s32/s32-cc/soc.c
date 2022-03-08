// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */
#include <common.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <debug_uart.h>

DECLARE_GLOBAL_DATA_PTR;

#define PERIPH_BASE      0x40000000
#define PERIPH_SIZE      0x20000000

static struct mm_region s32_mem_map[] = {
	{
		PHYS_SDRAM_1, PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE,
		PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE |
		    PTE_BLOCK_NS
	},
#ifdef PHYS_SDRAM_2
	{
		PHYS_SDRAM_2, PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE,
		PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE |
		    PTE_BLOCK_NS
	},
#endif
	{
		PERIPH_BASE, PERIPH_BASE, PERIPH_SIZE,
		PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
		    PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},

	/* list terminator */
	{},
};

struct mm_region *mem_map = s32_mem_map;

int arch_cpu_init(void)
{
	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	gd->flags |= GD_FLG_SKIP_RELOC;

	if (IS_ENABLED(CONFIG_DEBUG_UART))
		debug_uart_init();

	return 0;
}

