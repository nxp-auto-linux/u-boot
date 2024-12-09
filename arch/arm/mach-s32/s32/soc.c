// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022-2024 NXP
 */
#include <common.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <init.h>
#include <malloc.h>
#include <relocate.h>
#include <soc.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <s32/soc.h>

#ifndef CONFIG_SYS_DCACHE_OFF

static inline u64 get_dtb_addr(void)
{
	return (u64)_start - CONFIG_S32_MAX_DTB_SIZE;
}

static int early_mmu_init(void)
{
	u64 pgtable_size = PGTABLE_SIZE;
	ulong fdt_size;

	gd->arch.tlb_addr = (uintptr_t)memalign(pgtable_size, pgtable_size);
	if (!gd->arch.tlb_addr)
		return -ENOMEM;

	gd->arch.tlb_size = pgtable_size;
	icache_enable();
	dcache_enable();

	fdt_size = fdt_totalsize(get_dtb_addr());
	fdt_size = ALIGN(fdt_size, PAGE_SIZE);
	/* Map DTB RW to allow device tree fixups. */
	mmu_map_region_attrs(get_dtb_addr(), fdt_size,
			     DDR_ATTRS | PTE_BLOCK_PXN | PTE_BLOCK_UXN,
			     false);

	return 0;
}

static void clear_early_mmu_settings(void)
{
	/* Reset fillptr to allow reinit of page tables */
	gd->new_gd->arch.tlb_fillptr = (uintptr_t)NULL;

	icache_disable();
	dcache_disable();
}

/*
 * Assumption: Called at the end of init_sequence_f to clean-up
 * before initr_caches().
 */
int clear_bss(void)
{
	clear_early_mmu_settings();

	return 0;
}

void enable_caches(void)
{
	ulong fdt_size;

	icache_enable();
	dcache_enable();

	fdt_size = fdt_totalsize(get_dtb_addr());
	fdt_size = ALIGN(fdt_size, PAGE_SIZE);
	/* Map DTB read only. */
	mmu_map_region_attrs(get_dtb_addr(), fdt_size,
			     DDR_ATTRS | PTE_BLOCK_AP_RO | PTE_BLOCK_PXN |
			     PTE_BLOCK_UXN, false);
}

#else /* CONFIG_SYS_DCACHE_OFF */

static int early_mmu_init(void)
{
	return 0;
}
#endif

int arch_cpu_init(void)
{
	gd->flags |= GD_FLG_SKIP_RELOC;

	if (IS_ENABLED(CONFIG_DEBUG_UART))
		debug_uart_init();

	/* Enable MMU and caches early to speed-up boot process */
	return early_mmu_init();
}

int print_socinfo(void)
{
	struct udevice *soc;
	char str[SOC_MAX_STR_SIZE];
	int ret;

	printf("SoC:   ");

	ret = soc_get(&soc);
	if (ret) {
		printf("Unknown\n");
		return 0;
	}

	ret = soc_get_family(soc, str, sizeof(str));
	if (!ret)
		printf("%s", str);

	ret = soc_get_machine(soc, str, sizeof(str));
	if (!ret)
		printf("%s", str);

	ret = soc_get_revision(soc, str, sizeof(str));
	if (!ret)
		printf(" rev. %s", str);

	printf("\n");

	return 0;
}
