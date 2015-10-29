/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/arch-fsl-lsch2/immap_lsch2.h>
#include "mp.h"

DECLARE_GLOBAL_DATA_PTR;

void *get_spin_tbl_addr(void)
{
	void *ptr = (void *)SECONDARY_CPU_BOOT_PAGE;

	/*
	 * Spin table is at the beginning of secondary boot page. It is
	 * copied to SECONDARY_CPU_BOOT_PAGE.
	 */
	ptr += (u64)&__spin_table - (u64)&secondary_boot_page;

	return ptr;
}

phys_addr_t determine_mp_bootpg(void)
{
	return (phys_addr_t)SECONDARY_CPU_BOOT_PAGE;
}

int fsl_lsch2_wake_seconday_cores(void)
{
	struct ccsr_gur __iomem *gur = (void *)(CONFIG_SYS_FSL_GUTS_ADDR);
	struct ccsr_scfg __iomem *scfg = (void *)(CONFIG_SYS_FSL_SCFG_ADDR);
	void *boot_loc = (void *)SECONDARY_CPU_BOOT_PAGE;
	size_t *boot_page_size = &(__secondary_boot_page_size);
	u32 cores;
	u32 corebcr;
	int timeout = 10;
	u64 *table = get_spin_tbl_addr();

	cores = cpu_mask();
	memcpy(boot_loc, &secondary_boot_page, *boot_page_size);
	/* Clear spin table so that secondary processors
	 * observe the correct value after waking up from wfe.
	 */
	memset(table, 0, CONFIG_MAX_CPUS * ENTRY_SIZE);
	flush_dcache_range((unsigned long)boot_loc,
			   (unsigned long)boot_loc + *boot_page_size);

	printf("Waking secondary cores to start from %lx\n", gd->relocaddr);
	out_be32(&scfg->scratchrw[0], (u32)(gd->relocaddr >> 32));
	out_be32(&scfg->scratchrw[1], (u32)gd->relocaddr);

	asm volatile("dsb st" : : : "memory");
	out_be32(&gur->brrl, cores);
	asm volatile("dsb st" : : : "memory");

	/* Bootup online cores */
	out_be32(&scfg->corebcr, cores);

	do {
		asm volatile("sev");
		udelay(10);
		corebcr = in_be32(&scfg->corebcr);
		if (hweight32(~corebcr) == hweight32(cores))
			break;
	} while (timeout--);
	if (timeout <= 0) {
		printf("Not all cores (0x%x) are up (0x%x)\n",
		       cores, corebcr);
		return 1;
	}

	printf("All (%d) cores are up.\n", hweight32(cores));

	return 0;
}

int is_core_valid(unsigned int core)
{
	return !!((1 << core) & cpu_mask());
}

int cpu_reset(int nr)
{
	puts("Feature is not implemented.\n");

	return 0;
}

int cpu_disable(int nr)
{
	puts("Feature is not implemented.\n");

	return 0;
}

int core_to_pos(int nr)
{
	u32 cores = cpu_mask();
	int i, count = 0;

	if (nr == 0) {
		return 0;
	} else if (nr >= hweight32(cores)) {
		puts("Not a valid core number.\n");
		return -1;
	}

	for (i = 1; i < 32; i++) {
		if (is_core_valid(i)) {
			count++;
			if (count == nr)
				break;
		}
	}

	return count;
}

int cpu_status(int nr)
{
	u64 *table;
	int pos;

	if (nr == 0) {
		table = (u64 *)get_spin_tbl_addr();
		printf("table base @ 0x%p\n", table);
	} else {
		pos = core_to_pos(nr);
		if (pos < 0)
			return -1;
		table = (u64 *)get_spin_tbl_addr() + pos * NUM_BOOT_ENTRY;
		printf("table @ 0x%p\n", table);
		printf("   addr - 0x%016llx\n", table[BOOT_ENTRY_ADDR]);
		printf("   r3   - 0x%016llx\n", table[BOOT_ENTRY_R3]);
		printf("   pir  - 0x%016llx\n", table[BOOT_ENTRY_PIR]);
	}

	return 0;
}

int cpu_release(int nr, int argc, char * const argv[])
{
	u64 boot_addr;
	u64 *table = (u64 *)get_spin_tbl_addr();
#ifndef CONFIG_FSL_SMP_RELEASE_ALL
	int pos;

	pos = core_to_pos(nr);
	if (pos <= 0)
		return -1;

	table += pos * NUM_BOOT_ENTRY;
#endif
	boot_addr = simple_strtoull(argv[0], NULL, 16);
	table[BOOT_ENTRY_ADDR] = boot_addr;
	flush_dcache_range((unsigned long)table,
			   (unsigned long)table + SIZE_BOOT_ENTRY);
	asm volatile("dsb st");
	smp_kick_all_cpus();	/* only those with entry addr set will run */

	return 0;
}
