// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022-2026 NXP
 */
#include <config.h>
#include <errno.h>
#include <linux/types.h>
#include <linux/string.h>
#include <stdio.h>
#include <vsprintf.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <fdtdec.h>
#include <init.h>
#include <malloc.h>
#include <relocate.h>
#include <soc.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <linux/kernel.h>
#include <s32/soc.h>

#ifndef CONFIG_SYS_DCACHE_OFF

enum {
	S32_MMU_HIGH_ADDR_PERIPH_INDEX,
#ifdef CONFIG_DEBUG_UART
	/*
	 * Map the debug UART in the static map, which is live at MMU enable.
	 * dyn_mem_map is applied too late (after the MMU is on), so early
	 * console access would otherwise fault. Placed before the DDR region
	 * so the post-relocation DRAM-bank fill never overwrites it.
	 */
	S32_MMU_DEBUG_UART_INDEX,
#endif
	S32_MMU_POST_RELOC_DDR_INDEX,
	/*
	 * Number of fixed regions: the highest peripheral addr region, the
	 * DDR/_start region and, when enabled, the debug UART region.
	 */
	S32_STATIC_NR_FIXED_REGIONS,
};

/* +1 for the sentinel entry. */
#define S32_STATIC_MEM_MAP_MAX (CONFIG_NR_DRAM_BANKS + S32_STATIC_NR_FIXED_REGIONS + 1)

/*
 * Reserve space for DRAM regions that will be added by mem_map_from_dram_banks().
 * The `_start` entry is updated in two phases:
 *  - Before relocation: early_mmu_init adjusts its size to cover U-Boot text and data.
 *  - After relocation: mem_map_from_dram_banks replaces it with the first DRAM region,
 *    then appends the remaining regions.
 *
 * Array size calculation:
 *    CONFIG_NR_DRAM_BANKS + NUMBER_OF_STATIC_REGIONS + 1 (sentinel)
 */
struct mm_region static_mem_map[S32_STATIC_MEM_MAP_MAX] = {
	[S32_MMU_HIGH_ADDR_PERIPH_INDEX] = {
		/*
		 * This is necessary since get_tcr rely on the static memory
		 * map when computing the largest possible address.
		 */
		HIGHEST_PER_ADDR, HIGHEST_PER_ADDR, PAGE_SIZE,
		PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN | PTE_BLOCK_AP_RO
	},
#ifdef CONFIG_DEBUG_UART
	[S32_MMU_DEBUG_UART_INDEX] = {
		CONFIG_DEBUG_UART_BASE, CONFIG_DEBUG_UART_BASE, PAGE_SIZE,
		PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
			PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#endif
	[S32_MMU_POST_RELOC_DDR_INDEX] = {
		(uintptr_t)_start, (uintptr_t)_start, PAGE_SIZE, DDR_ATTRS
	},
	{ /* sentinel */ }
};

/*
 * FDT (NT_FW_CONFIG) address that TF-A passes in x0 at reset. It is written by
 * the assembly save_boot_params() (save_boot_params.S) before the stack exists,
 * and read back via get_dtb_addr() once U-Boot is running. Forced into ".data"
 * (not ".bss"): .bss is cleared soon after save_boot_params runs. Non-static so
 * the assembly routine can reach it.
 */
u64 nt_fw_dtb __section(".data");

static inline u64 get_dtb_addr(void)
{
	return nt_fw_dtb;
}

extern struct mm_region dyn_mem_map[];
struct mm_region *mem_map = static_mem_map;

int board_fdt_blob_setup(void **fdtp)
{
	void *dtb = (void *)get_dtb_addr();

	if (fdt_magic(dtb) != FDT_MAGIC)
		return -EFAULT;

	*fdtp = dtb;

	return 0;
}

struct mm_region *get_mm_region(struct mm_region *mem_map_array, u64 phys_base)
{
	struct mm_region *region;
	size_t i;

	for (i = 0; mem_map_array[i].size || mem_map_array[i].attrs; i++) {
		region = &mem_map_array[i];

		if (region->phys == phys_base)
			return region;
	}

	return NULL;
}

static int parse_tfa_mem(fdt_addr_t *addr, fdt_size_t *size)
{
	const void *fdt = (const void *)get_dtb_addr();
	int node;

	/*
	 * Use raw FDT calls instead of ofnode API because this function is
	 * called from enable_caches() via initr_caches(), which runs before
	 * initr_dm() initializes the ofnode infrastructure (oftree_reset).
	 * With OFNODE_MULTI_TREE, ofnode_to_fdt() returns NULL at this point.
	 */
	node = fdt_path_offset(fdt, "/reserved-memory/atf");
	if (node < 0) {
		log_err("Unable to find 'atf' node\n");
		return -EINVAL;
	}

	*addr = fdtdec_get_addr_size(fdt, node, "reg", size);
	if (*addr == FDT_ADDR_T_NONE) {
		log_err("Unable to get value of 'reg' prop of 'atf' node\n");
		return -EINVAL;
	}

	return 0;
}

static int early_mmu_init(void)
{
	u64 stack_addr, pgtable_size = PGTABLE_SIZE;
	struct mm_region *region;
	ulong fdt_size;
	size_t i;

	/*
	 * Read DTB totalsize before enabling the MMU, since, at that point,
	 * the area where DTB resides will no longer be accessible.
	 */
	fdt_size = fdt_totalsize(get_dtb_addr());
	fdt_size = ALIGN(fdt_size, PAGE_SIZE);

	if (check_uptr_overflow((phys_addr_t)&__bss_start,
				(phys_addr_t)CONFIG_SYS_INIT_SP_BSS_OFFSET))
		return -ENOMEM;

	/*
	 * We need to map the region starting with the text and ending with
	 * the stack address.
	 */
	stack_addr = ALIGN((u64)&__bss_end + CONFIG_SYS_INIT_SP_BSS_OFFSET,
			   PAGE_SIZE);

	region = get_mm_region(static_mem_map, (uintptr_t)_start);
	if (!region)
		panic("Text memory region not defined in static_mem_map\n");

	region->size = stack_addr - (u64)_start;

	/* Compute TLB address. */
	gd->arch.tlb_addr = (uintptr_t)memalign(pgtable_size, pgtable_size);
	if (!gd->arch.tlb_addr)
		return -ENOMEM;

	gd->arch.tlb_size = pgtable_size;
	icache_enable();
	dcache_enable();

	/* Map the rest of the regions. */
	for (i = 0; dyn_mem_map[i].size || dyn_mem_map[i].attrs; i++)
		mmu_map_region_attrs(dyn_mem_map[i].virt, dyn_mem_map[i].size,
				     dyn_mem_map[i].attrs, false);

	/* Map DTB RW to allow device tree fixups. */
	mmu_map_region_attrs(get_dtb_addr(), fdt_size,
			     DDR_ATTRS | PTE_BLOCK_PXN | PTE_BLOCK_UXN,
			     false);

	return 0;
}

/*
 * Assumption: Called at the end of init_sequence_f to clean-up
 * before initr_caches().
 */
int clear_bss(void)
{
	/* Reset fillptr to allow reinit of page tables */
	gd->new_gd->arch.tlb_fillptr = (uintptr_t)NULL;
	dcache_disable();

	return 0;
}

/*
 * Recreate MMU table in main memory.
 */
void enable_caches(void)
{
	fdt_addr_t tfa_addr;
	fdt_size_t tfa_size;
	ulong fdt_size;
	size_t i;
	int ret;

	/* Enable data cache and MMU. */
	dcache_enable();

	/* Map the rest of the regions. */
	for (i = 0; dyn_mem_map[i].size || dyn_mem_map[i].attrs; i++)
		mmu_map_region_attrs(dyn_mem_map[i].virt, dyn_mem_map[i].size,
				     dyn_mem_map[i].attrs, false);

	/* Compute the DTB address. */
	fdt_size = fdt_totalsize(get_dtb_addr());
	fdt_size = ALIGN(fdt_size, PAGE_SIZE);

	/* Map DTB read only. */
	mmu_map_region_attrs(get_dtb_addr(), fdt_size,
			     DDR_ATTRS | PTE_BLOCK_AP_RO | PTE_BLOCK_PXN |
			     PTE_BLOCK_UXN, false);

	ret = parse_tfa_mem(&tfa_addr, &tfa_size);
	if (ret)
		return;

	if (!IS_ALIGNED(tfa_addr, PAGE_SIZE) ||
	    !IS_ALIGNED(tfa_size, PAGE_SIZE)) {
		log_err("BL31 address/size not aligned to PAGE_SIZE!\n");
		tfa_addr = ALIGN_DOWN(tfa_addr, PAGE_SIZE);
		tfa_size = ALIGN(tfa_size, PAGE_SIZE);
	}

	mmu_change_region_attr(tfa_addr, tfa_size, PTE_TYPE_FAULT);
}
#else /* CONFIG_SYS_DCACHE_OFF */

static int early_mmu_init(void)
{
	return 0;
}
#endif

int arch_cpu_init(void)
{
	int ret;

	/*
	 * In the 2026.04 linker script, .rela.dyn overlaps .bss (OVERLAY),
	 * so BSS contains relocation data after pie_fixup runs. Clear it
	 * here before any code relies on zero-initialized globals.
	 * The crt0_64.S BSS clear only runs after board_init_f() returns.
	 */
	memset(__bss_start, 0, __bss_end - __bss_start);

	gd->flags |= GD_FLG_SKIP_RELOC;

	if (IS_ENABLED(CONFIG_DEBUG_UART))
		debug_uart_init();

	/* Enable MMU and caches early to speed-up boot process */
	ret = early_mmu_init();
	if (ret)
		return ret;

	ret = fdtdec_setup_mem_size_base();
	if (ret)
		return ret;

	if (!IS_ENABLED(CONFIG_SYS_DCACHE_OFF))
		mmu_map_region_attrs(gd->ram_base, get_effective_memsize(),
				     DDR_ATTRS, true);
	return 0;
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

int dram_init_banksize(void)
{
	int ret;

	/* static_mem_map should have space for CONFIG_NR_DRAM_BANKS +
	 * sentinel.
	 */
	assert(S32_MMU_POST_RELOC_DDR_INDEX + CONFIG_NR_DRAM_BANKS + 1 <=
	       ARRAY_SIZE(static_mem_map));

	ret = fdtdec_setup_memory_banksize();
	if (ret) {
		log_err("%s: Failed to setup dram banks\n", __func__);
		return ret;
	}

	ret = mem_map_from_dram_banks(S32_MMU_POST_RELOC_DDR_INDEX,
				      ARRAY_SIZE(static_mem_map), DDR_ATTRS);
	if (ret)
		log_err("%s: Failed to setup dram banks\n", __func__);

	return ret;
}
