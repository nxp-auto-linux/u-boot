// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2020 NXP
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/siul.h>
#include "mp.h"
#include "dma_mem.h"
#include <asm/arch/soc.h>
#include <asm/arch/s32-gen1/a53_cluster_gpr.h>
#include <asm/arch/s32-gen1/ncore.h>
#ifdef CONFIG_S32_SKIP_RELOC
#include <asm-generic/sections.h>
#endif

#define S32_MMU_TABLE(BASE, IDX) ((BASE) + (IDX) * PGTABLE_SIZE)

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
static int s32_gentimer_init(void);
#endif

static struct mm_region s32_mem_map[] = {
	{
		/* List terminator */
		0,
	}
};

struct mm_region *mem_map = s32_mem_map;

#ifndef CONFIG_SYS_DCACHE_OFF

static void set_pgtable_section(u64 *page_table, u64 index, u64 section,
				u64 memory_type, u64 attribute)
{
	u64 value;

	value = section | PTE_TYPE_BLOCK | PTE_BLOCK_AF;
	value |= PMD_ATTRINDX(memory_type);
	value |= attribute;
	page_table[index] = value;
}

static void set_pgtable_table(u64 *page_table, u64 index, u64 *table_addr)
{
	u64 value;

	value = (u64)table_addr | PTE_TYPE_TABLE;
	page_table[index] = value;
}

/*
 * Set the block entries according to the information of the table.
 */
static int set_block_entry(const struct sys_mmu_table *list,
			   struct table_info *table)
{
	u64 block_size = 0, block_shift = 0;
	u64 block_addr, index;
	int j;

	if (table->entry_size == BLOCK_SIZE_L1) {
		block_size = BLOCK_SIZE_L1;
		block_shift = SECTION_SHIFT_L1;
	} else if (table->entry_size == BLOCK_SIZE_L2) {
		block_size = BLOCK_SIZE_L2;
		block_shift = SECTION_SHIFT_L2;
	} else {
		printf("Wrong size\n");
	}

	block_addr = list->phys_addr;
	index = (list->virt_addr - table->table_base) >> block_shift;

	for (j = 0; j < (list->size >> block_shift); j++) {
		set_pgtable_section(table->ptr,
				    index,
				    block_addr,
				    list->memory_type,
				    list->share);
		block_addr += block_size;
		index++;
	}

	return 0;
}

/*
 * Find the corresponding table entry for the list.
 */
static int find_table(const struct sys_mmu_table *list,
		      struct table_info *table, u64 *level0_table)
{
	u64 index = 0, level = 0;
	u64 *level_table = level0_table;
	u64 temp_base = 0, block_size = 0, block_shift = 0;
	while (level < 3) {
		if (level == 0) {
			block_size = BLOCK_SIZE_L0;
			block_shift = SECTION_SHIFT_L0;
		} else if (level == 1) {
			block_size = BLOCK_SIZE_L1;
			block_shift = SECTION_SHIFT_L1;
		} else if (level == 2) {
			block_size = BLOCK_SIZE_L2;
			block_shift = SECTION_SHIFT_L2;
		}

		index = 0;
		while (list->virt_addr >= temp_base) {
			index++;
			temp_base += block_size;
		}
		temp_base -= block_size;
		if ((level_table[index - 1] & PTE_TYPE_MASK) ==
		    PTE_TYPE_TABLE) {
			level_table = (u64 *)(level_table[index - 1] &
				~PTE_TYPE_MASK);
			level++;
			continue;
		} else {
			if (level == 0)
				return -1;

			if ((list->phys_addr + list->size) >
			(temp_base + block_size * NUM_OF_ENTRY))
				return -1;

			/*
			* Check the address and size of the list member is
			* aligned with the block size.
			*/
			if (((list->phys_addr & (block_size - 1)) != 0) ||
				((list->size & (block_size - 1)) != 0))
				return -1;
			table->ptr = level_table;
			table->table_base = temp_base -
					    ((index - 1) << block_shift);
			table->entry_size = block_size;

			return 0;
		}
	}
	return -1;
}

/*
 * To start MMU before DDR is available, we create MMU table in SRAM.
 * The base address of SRAM is IRAM_BASE_ADDR. We use three
 * levels of translation tables here to cover 40-bit address space.
 * We use 4KB granule size, with 40 bits physical address, T0SZ=24
 */
static inline void early_mmu_setup(void)
{
	unsigned int el, i;
#ifdef CONFIG_S32V234
	volatile struct ccsr_cci400 *cci = (struct ccsr_cci400 *)CCI400_BASE_ADDR;
#endif
	u64 *level0_table =  (u64 *)S32_MMU_TABLE(S32_IRAM_MMU_TABLES_BASE, 0);
	u64 *level1_table0 = (u64 *)S32_MMU_TABLE(S32_IRAM_MMU_TABLES_BASE, 1);
	u64 *level1_table1 = (u64 *)S32_MMU_TABLE(S32_IRAM_MMU_TABLES_BASE, 2);
	u64 *level2_table0 = (u64 *)S32_MMU_TABLE(S32_IRAM_MMU_TABLES_BASE, 3);
	u64 *level2_table1 = (u64 *)S32_MMU_TABLE(S32_IRAM_MMU_TABLES_BASE, 4);
	u64 *level2_table2 = (u64 *)S32_MMU_TABLE(S32_IRAM_MMU_TABLES_BASE, 5);
	struct table_info table = {level0_table, 0, BLOCK_SIZE_L0};

	dma_mem_clr((uintptr_t)level0_table,
		    ((uintptr_t)level2_table2) + PGTABLE_SIZE -
			((uintptr_t)level0_table));


	/* Fill in the table entries */
	set_pgtable_table(level0_table, 0, level1_table0);
	set_pgtable_table(level0_table, 1, level1_table1);

	set_pgtable_table(level1_table0,
			  CONFIG_SYS_FSL_IRAM_BASE >> SECTION_SHIFT_L1,
			  level2_table0);
	set_pgtable_table(level1_table0,
			  CONFIG_SYS_FSL_PERIPH_BASE >> SECTION_SHIFT_L1,
			  level2_table1);
	set_pgtable_table(level1_table0,
			  CONFIG_SYS_FSL_DRAM_BASE2 >> SECTION_SHIFT_L1,
			  level2_table2);

	/* Find the table and fill in the block entries */
	for (i = 0; i < ARRAY_SIZE(s32_early_mmu_table); i++) {
		if (find_table(&s32_early_mmu_table[i],
			&table, level0_table) == 0) {
			/*
			 * If find_table() returns error, it cannot be dealt
			 * with here. Breakpoint can be added for debugging.
			 */
			set_block_entry(&s32_early_mmu_table[i], &table);
			/*
			 * If set_block_entry() returns error, it cannot be
			 * dealt with here too.
			 */
		}
	}
#ifdef CONFIG_S32V234
	out_le32(&cci->slave[3].snoop_ctrl,
			 CCI400_DVM_MESSAGE_REQ_EN | CCI400_SNOOP_REQ_EN);

	out_le32(&cci->slave[4].snoop_ctrl,
			 CCI400_DVM_MESSAGE_REQ_EN | CCI400_SNOOP_REQ_EN);
#endif
	el = current_el();
	set_ttbr_tcr_mair(el, (u64)level0_table, S32V_TCR, MEMORY_ATTRIBUTES);
	set_sctlr(get_sctlr() | CR_M);
	set_sctlr(get_sctlr() | CR_C);

}

/*
 * The final tables look similar to early tables, but different in detail.
 * These tables are in DRAM using the area reserved by dtb for spintable area.
 *
 * Level 1 table 0 contains 512 entries for each 1GB from 0 to 512GB.
 * Level 1 table 1 contains 512 entries for each 1GB from 512GB to 1TB.
 * Level 2 table 0 contains 512 entries for each 2MB from 0 to 1GB.
 * Level 2 table 1 contains 512 entries for each 2MB from 1GB to 2GB.
 * Level 2 table 2 contains 512 entries for each 2MB from 3GB to 4GB.
 */
static inline void final_mmu_setup(void)
{
	unsigned int el, i;
	u64 *level0_table =  (u64 *)S32_MMU_TABLE(S32_SDRAM_MMU_TABLES_BASE, 0);
	u64 *level1_table0 = (u64 *)S32_MMU_TABLE(S32_SDRAM_MMU_TABLES_BASE, 1);
	u64 *level1_table1 = (u64 *)S32_MMU_TABLE(S32_SDRAM_MMU_TABLES_BASE, 2);
	u64 *level2_table0 = (u64 *)S32_MMU_TABLE(S32_SDRAM_MMU_TABLES_BASE, 3);
	u64 *level2_table1 = (u64 *)S32_MMU_TABLE(S32_SDRAM_MMU_TABLES_BASE, 4);
	u64 *level2_table2 = (u64 *)S32_MMU_TABLE(S32_SDRAM_MMU_TABLES_BASE, 5);
	struct table_info table = {level0_table, 0, BLOCK_SIZE_L0};

	/* Invalidate all table entries */

#ifdef CONFIG_S32_SKIP_RELOC
	dma_mem_clr((uintptr_t)level0_table,
		    ((uintptr_t)level2_table2) + PGTABLE_SIZE -
			((uintptr_t)level0_table));
#else
	memset(level0_table, 0, PGTABLE_SIZE);
	memset(level1_table0, 0, PGTABLE_SIZE);
	memset(level1_table1, 0, PGTABLE_SIZE);
	memset(level2_table0, 0, PGTABLE_SIZE);
	memset(level2_table1, 0, PGTABLE_SIZE);
	memset(level2_table2, 0, PGTABLE_SIZE);
#endif

	/* Fill in the table entries */
	set_pgtable_table(level0_table, 0, level1_table0);
	set_pgtable_table(level0_table, 1, level1_table1);
	set_pgtable_table(level1_table0,
			 CONFIG_SYS_FSL_IRAM_BASE >> SECTION_SHIFT_L1,
			 level2_table0);
	set_pgtable_table(level1_table0,
			  CONFIG_SYS_FSL_PERIPH_BASE >> SECTION_SHIFT_L1,
			  level2_table1);
	set_pgtable_table(level1_table0,
			  CONFIG_SYS_FSL_DRAM_BASE2 >> SECTION_SHIFT_L1,
			  level2_table2);


	/* Find the table and fill in the block entries */
	for (i = 0; i < ARRAY_SIZE(s32_final_mmu_table); i++) {
		if (find_table(&s32_final_mmu_table[i],
			&table, level0_table) == 0) {
			if (set_block_entry(&s32_final_mmu_table[i],
					&table) != 0) {
				printf("MMU error: could not set block entry for %p\n",
				&s32_final_mmu_table[i]);
			}
			} else {
				printf("MMU error: could not find the table for %p\n",
				&s32_final_mmu_table[i]);
			}
	}

	/* flush new MMU table */

	/* Disable cache and MMU */
	dcache_disable();   /* TLBs are invalidated */
	invalidate_icache_all();

	/* point TTBR to the new table */
	el = current_el();
	set_ttbr_tcr_mair(el, (u64)level0_table, S32V_TCR_FINAL, MEMORY_ATTRIBUTES);
	/*
	 * MMU is already enabled, just need to invalidate TLB to load the
	 * new table. The new table is compatible with the current table, if
	 * MMU somehow walks through the new table before invalidation TLB,
	 * it still works. So we don't need to turn off MMU here.
	 */
	set_sctlr(get_sctlr() | CR_M);
}

int arch_cpu_init(void)
{
#ifdef CONFIG_S32_SKIP_RELOC
	int ret;
	int base, size;

	gd->flags |= GD_FLG_SKIP_RELOC;

	/*
	 * Assumption: lowlevel.S will clear at least [__bss_start - __bss_end]
	 */
	base = (uintptr_t)&__bss_end;
	size = IRAM_SIZE + IRAM_BASE_ADDR - base;
	ret = dma_mem_clr(base, size);
	if (!ret)
		return ret;
#endif

#if defined(CONFIG_S32_GEN1) && defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
	/* Platforms with Concerto/Ncore have to explicitly initialize
	 * the interconnect before any cache operations are performed.
	 * Also, ensure that clocks are initialized before the interconnect.
	 *
	 * Note: TF-A has already initialized these, so don't do it again if
	 * we're running at EL2.
	 */
	clock_init();
	ncore_init(0x1);
#endif
	icache_enable();
	__asm_invalidate_dcache_all();
	__asm_invalidate_tlb_all();
	early_mmu_setup();

#if defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
	s32_gentimer_init();
#endif
	return 0;
}

/*
 * This function is called from lib/board.c.
 * It recreates MMU table in main memory. MMU and d-cache are enabled earlier.
 * There is no need to disable d-cache for this operation.
 */
void enable_caches(void)
{
	final_mmu_setup();
	__asm_invalidate_tlb_all();
#ifdef CONFIG_S32_GEN1
	dcache_enable();
#endif
}

#endif

#if defined(CONFIG_ARCH_EARLY_INIT_R)
#if !defined(CONFIG_S32_ATF_BOOT_FLOW)
int arch_early_init_r(void)
{
	int rv;
	asm volatile("dsb sy");
	rv = fsl_s32_wake_secondary_cores();

	if (rv)
		printf("Did not wake secondary cores\n");

#ifdef CONFIG_S32_GEN1
	/* Reconfigure Concerto before actually waking the cores */
	ncore_init(0xf);
#endif

	asm volatile("sev");

	return 0;
}
#else
/* With TF-A, practically we should do nothing of the above; define an empty
 * stub to appease the compiler.
 */
int arch_early_init_r(void)
{
	return 0;
}
#endif
#endif

/* For configurations with U-Boot *not* at EL3, it is presumed that
 * the EL3 software (e.g. the TF-A) will initialize the generic timer.
 */
#if defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
#ifdef CONFIG_S32V234
static int s32_gentimer_init(void)
{
	if (get_siul2_midr1_major() >= 1)
		return 0;

	/* For CUT1 chip version, update with accurate clock frequency for all cores. */

	/* update for secondary cores */
	__real_cntfrq = COUNTER_FREQUENCY_CUT1;
	flush_dcache_range((unsigned long)&__real_cntfrq,
			   (unsigned long)&__real_cntfrq + 8);


	/* Update made for main core. */
	asm volatile("msr cntfrq_el0, %0" : : "r" (__real_cntfrq) : "memory");
	return 0;
}
#elif defined(CONFIG_S32_GEN1) && defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
/* The base counter frequency (FXOSC on the S32G) is actually board-dependent.
 * Moreoever, only software running at the highest implemented Exception level
 * can write to CNTFRQ_EL0, so we won't even define this function if we are
 * running with TF-A.
 */
static int s32_gentimer_init(void)
{
	u32 clk_div;

	clk_div = readl(A53_CLUSTER_GPR_GPR(0)) & GPR00_CA53_COUNTER_CLK_DIV_VAL_MASK;
	clk_div = (clk_div >> GPR00_CA53_COUNTER_CLK_DIV_VAL_SHIFT) + 1;

	__real_cntfrq = COUNTER_FREQUENCY / clk_div;
	flush_dcache_range((unsigned long)&__real_cntfrq,
			   (unsigned long)&__real_cntfrq +
			   sizeof(__real_cntfrq));

	/* Primary core updated here, secondaries in start_slave_cores */
	asm volatile("msr cntfrq_el0, %0" : : "r" (__real_cntfrq) : "memory");

	return 0;
}
#else
#error "S32 platform should provide ARMv8 generic timer initialization"
#endif
#endif /* CONFIG_S32_STANDALONE_BOOT_FLOW */
