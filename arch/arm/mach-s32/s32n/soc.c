// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2024-2025 NXP
 */
#include <common.h>
#include <init.h>
#include <asm-generic/sections.h>
#include <asm/global_data.h>
#include <asm/armv8/mmu.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <dm/ofnode.h>
#include <linux/ioport.h>
#include <s32/soc.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_DCACHE_OFF

#define UART_SIZE	0x1000

#define ENETC0_BASE	0x48800000
#define ENETC0_SIZE	0x540000

#define CONFIG_SYS_IEPRC_PHYS_ADDR 0x240000000ULL
#define CONFIG_SYS_IEPRC_PHYS_SIZE 0x2000000ULL

#define HIGHEST_ADDR	(CONFIG_SYS_IEPRC_PHYS_ADDR + \
		CONFIG_SYS_IEPRC_PHYS_SIZE - PAGE_SIZE)

struct mm_region static_mem_map[] = {
	{
		/* Will be updated before dcache_enable call. */
		(uintptr_t)_start, (uintptr_t)_start, PAGE_SIZE, DDR_ATTRS
	},
	{
		/*
		 * This is necessary since get_tcr rely on the static memory
		 * map when computing the largest possible address.
		 */
		HIGHEST_ADDR, HIGHEST_ADDR, PAGE_SIZE,
		PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN | PTE_BLOCK_AP_RO
	},
	{ /* sentinel */ }
};

struct mm_region dyn_mem_map[] = {
	{
		ENETC0_BASE, ENETC0_BASE, ENETC0_SIZE,
		PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
		    PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#ifdef CONFIG_DEBUG_UART
	{
		CONFIG_DEBUG_UART_BASE, CONFIG_DEBUG_UART_BASE, UART_SIZE,
		PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
		    PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#endif
	{
		CONFIG_SYS_IEPRC_PHYS_ADDR, CONFIG_SYS_IEPRC_PHYS_ADDR,
		CONFIG_SYS_IEPRC_PHYS_SIZE,
		PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
		PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{ /* sentinel */ }
};

u64 get_page_table_size(void)
{
	/*
	 * The default implementation can't be used since the maximum
	 * size is computed based on the mem_map array, which in this case,
	 * doesn't contain the regions which will be dynamically added.
	 */
	return SZ_64K;
}
#endif

/* The top of the DRAM is reserved for the Secure Monitor. */
ulong board_get_usable_ram_top(ulong total_size)
{
	struct resource reg;
	ofnode node;
	int ret;

	node = ofnode_path("/reserved-memory/atf");
	if (!ofnode_valid(node)) {
		log_err("Couldn't find 'atf' reserved-memory node\n");
		return gd->ram_top;
	}

	ret = ofnode_read_resource(node, 0, &reg);
	if (ret)
		return gd->ram_top;

	return reg.start < (u64)_start ? gd->ram_top : reg.start;
}

int arch_cpu_init_dm(void)
{
	return 0;
}

