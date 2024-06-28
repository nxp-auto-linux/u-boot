// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2024 NXP
 */
#include <common.h>
#include <init.h>
#include <asm/global_data.h>
#include <asm/armv8/mmu.h>
#include <dm/ofnode.h>
#include <linux/ioport.h>
#include <s32/soc.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_DCACHE_OFF

#define UART_BASE	0x4A030000
#define UART_SIZE	0x10000
#define UFS_BASE	0x4ac80000
#define UFS_SIZE	0x1000

static struct mm_region s32n_mem_map[] = {
	{
		CONFIG_BOARD_DRAM_BASE, CONFIG_BOARD_DRAM_BASE,
		CONFIG_BOARD_DRAM_SIZE, DDR_ATTRS
	},
	{
		DTB_ADDR, DTB_ADDR, DTB_SIZE,
		DDR_ATTRS | PTE_BLOCK_AP_RO | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		UART_BASE, UART_BASE, UART_SIZE,
		PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
		    PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
		UFS_BASE, UFS_BASE, UFS_SIZE,
		PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
		    PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	/* list terminator */
	{},
};

struct mm_region *mem_map = s32n_mem_map;

struct mm_region *get_mm_region(u64 phys_base)
{
	struct mm_region *region;
	size_t i;

	for (i = 0; i < ARRAY_SIZE(s32n_mem_map); i++) {
		region = &s32n_mem_map[i];

		if (region->phys == phys_base)
			return region;
	}

	return NULL;
}

#endif

int arch_cpu_init_dm(void)
{
	/* RO access for device tree */
	set_dtb_wr_access(false);

	return 0;
}

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

	return reg.start < CONFIG_SYS_DATA_BASE ? gd->ram_top : reg.start;
}
