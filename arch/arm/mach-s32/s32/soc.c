// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022-2025 NXP
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
#include <dm/ofnode.h>
#include <linux/kernel.h>
#include <s32/soc.h>

#ifndef CONFIG_SYS_DCACHE_OFF
/*
 * Without forcing the ".data" section, this would get saved in ".bss". BSS
 * will be cleared soon after, so it's not suitable.
 */
static u64 nt_fw_dtb __section(".data");

/*
 * Save the FDT address provided by TF-A in r0 at boot time. This function is
 * called from start.S.
 */
void save_boot_params(u64 r0, u64 r1, u64 r2, u64 r3)
{
	nt_fw_dtb = r0;

	save_boot_params_ret();
}

static inline u64 get_dtb_addr(void)
{
	return nt_fw_dtb;
}

extern struct mm_region static_mem_map[];
extern struct mm_region dyn_mem_map[];
struct mm_region *mem_map = static_mem_map;

void *board_fdt_blob_setup(int *err)
{
	void *dtb = (void *)get_dtb_addr();

	*err = 0;
	if (fdt_magic(dtb) != FDT_MAGIC)
		*err = -EFAULT;

	return dtb;
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
	ofnode node;

	/* Get atf reserved-memory node offset if not just exit. */
	node = ofnode_path("/reserved-memory/atf");
	if (!ofnode_valid(node)) {
		log_err("Unable to find 'atf' node\n");
		return -EINVAL;
	}

	*addr = ofnode_get_addr_size(node, "reg", size);
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
	stack_addr = ALIGN((u64)&__bss_start + CONFIG_SYS_INIT_SP_BSS_OFFSET,
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
	struct mm_region *region_ptr;
	fdt_addr_t tfa_addr;
	fdt_size_t tfa_size;
	ulong fdt_size;
	size_t i;
	int ret;

	/*
	 * Since some data was relocated at the end of the ram, map the entire
	 * first memory bank.
	 */
	region_ptr = get_mm_region(static_mem_map, (uintptr_t)_start);
	if (!region_ptr)
		panic("Text memory region not found in static_mem_map\n");

	region_ptr->virt = gd->ram_base;
	region_ptr->phys = gd->ram_base;
	region_ptr->size = get_effective_memsize();

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

	/* Map the rest of the memory banks. */
	for (i = 1; i < CONFIG_NR_DRAM_BANKS; i++)
		mmu_map_region_attrs(gd->bd->bi_dram[i].start,
				     gd->bd->bi_dram[i].size,
				     DDR_ATTRS, false);

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
