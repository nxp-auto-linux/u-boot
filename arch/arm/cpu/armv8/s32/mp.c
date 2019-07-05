// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2015 Freescale Semiconductor, Inc.
 * (C) Copyright 2017-2019 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/mc_rgm_regs.h>
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

#ifdef CONFIG_S32V234
int fsl_s32_wake_secondary_cores(void)
{
	void *boot_loc = (void *)SECONDARY_CPU_BOOT_PAGE;
	size_t *boot_page_size = &(__secondary_boot_page_size);
	u64 *table = get_spin_tbl_addr();

	/* Clear spin table so that secondary processors
	 * observe the correct value after waking up from wfe.
	 */
	memset(table, 0, CONFIG_MAX_CPUS * ENTRY_SIZE);
	flush_dcache_range((unsigned long)boot_loc,
			   (unsigned long)boot_loc + *boot_page_size);

	/* program the cores possible running modes */
	writew(MC_ME_CCTL_DEASSERT_CORE, MC_ME_CCTL2);
	writew(MC_ME_CCTL_DEASSERT_CORE, MC_ME_CCTL3);
	writew(MC_ME_CCTL_DEASSERT_CORE, MC_ME_CCTL4);

	/* write the cores' start address */
	writel(CONFIG_SYS_TEXT_BASE | MC_ME_CADDRn_ADDR_EN, MC_ME_CADDR2);
	writel(CONFIG_SYS_TEXT_BASE | MC_ME_CADDRn_ADDR_EN, MC_ME_CADDR3);
	writel(CONFIG_SYS_TEXT_BASE | MC_ME_CADDRn_ADDR_EN, MC_ME_CADDR4);

	writel( MC_ME_MCTL_RUN0 | MC_ME_MCTL_KEY, MC_ME_MCTL );
	writel( MC_ME_MCTL_RUN0 | MC_ME_MCTL_INVERTEDKEY, MC_ME_MCTL );

	while( (readl(MC_ME_GS) & MC_ME_GS_S_MTRANS) != 0x00000000 );

	smp_kick_all_cpus();

	printf("All (%d) cores are up.\n", cpu_numcores());

	return 0;
}

#elif defined(CONFIG_S32_GEN1)
#define GPR06_CA53_LCKSTEP_EN (0x1 << 0)
static void fsl_s32_wake_secondary_core(int prtn, int core)
{
	u32 reset;
	u32 gpr_06;

	/* Write start_address in MC_ME_PRTN_N_CORE_M_ADDR register. */
	writel(gd->relocaddr, MC_ME_PRTN_N_CORE_M_ADDR(prtn, core));

	reset = readl(RGM_PRST(RGM_CORES_RESET_GROUP));
	reset &= ~(RGM_CORE_RST(core));

	writel(reset, RGM_PRST(RGM_CORES_RESET_GROUP));

	/* if Lockstep configuration for CA53 is not enabled,
	 * enable all 4 cores
	 */

	gpr_06 = readl(S32_A53_GPR_BASE_ADDR + S32_A53_GP06_OFF);
	if (!(gpr_06 & GPR06_CA53_LCKSTEP_EN)) {
		printf("performance mode for CA53 clusters\n");
		/* Set core clock enable bit. */
		writel(MC_ME_PRTN_N_CORE_M_PCONF_CCE,
		       MC_ME_PRTN_N_CORE_M_PCONF(prtn, core & ~1));

		/* Enable core clock triggering to update on writing
		 * CTRL key sequence.
		 */
		writel(MC_ME_PRTN_N_CORE_M_PUPD_CCUPD,
		       MC_ME_PRTN_N_CORE_M_PUPD(prtn, core & ~1));

		/* Write valid key sequence to trigger the update. */
		writel(MC_ME_CTL_KEY_KEY, MC_ME_CTL_KEY);
		writel(MC_ME_CTL_KEY_INVERTEDKEY, MC_ME_CTL_KEY);

		/* Wait until hardware process to enable core is completed. */
		while (readl(MC_ME_PRTN_N_CORE_M_STAT(prtn, core & ~1)) !=
		       MC_ME_PRTN_N_CORE_M_PCONF_CCE)
			;
	}
	while (readl(RGM_PSTAT(RGM_CORES_RESET_GROUP)) != reset)
		;
}

int fsl_s32_wake_secondary_cores(void)
{
	void *boot_loc = (void *)SECONDARY_CPU_BOOT_PAGE;
	size_t *boot_page_size = &(__secondary_boot_page_size);
	u64 *table = get_spin_tbl_addr();

	/* Clear spin table so that secondary processors
	 * observe the correct value after waking up from wfe.
	 */
	memset(table, 0, CONFIG_MAX_CPUS * ENTRY_SIZE);
	flush_dcache_range((unsigned long)boot_loc,
			   (unsigned long)boot_loc + *boot_page_size);

	/* Enable partition clock */
	writel(MC_ME_PRTN_N_PCE, MC_ME_PRTN_N_PCONF(MC_ME_CORES_PRTN));
	writel(MC_ME_PRTN_N_PCE, MC_ME_PRTN_N_PUPD(MC_ME_CORES_PRTN));

	/* Write valid key sequence to trigger the update. */
	writel(MC_ME_CTL_KEY_KEY, MC_ME_CTL_KEY);
	writel(MC_ME_CTL_KEY_INVERTEDKEY, MC_ME_CTL_KEY);

	/* Cluster 0, core 0 is already enabled by BootROM.
	 * We should enable core 1 from cluster 0 and
	 * core 0 and 1 from cluster 1. All are in prtn 1.
	 * The procedure can be found in
	 * "MC_ME application core enable", S32R RM Rev1 DraftC.
	 */
	fsl_s32_wake_secondary_core(1, 1);
	fsl_s32_wake_secondary_core(1, 2);
	fsl_s32_wake_secondary_core(1, 3);

	smp_kick_all_cpus();

	printf("All (%d) cores are up.\n", cpu_numcores());

	return 0;
}
#else
#error "Incomplete platform definition"
#endif

int is_core_valid(unsigned int core)
{
	if (core == 0)
		return 0;

	return !!((1 << core) & cpu_mask());
}

int cpu_reset(u32 nr)
{
	puts("Feature is not implemented.\n");

	return 0;
}

int cpu_disable(u32 nr)
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

int cpu_status(u32 nr)
{
	u64 *table;
	int valid;

	if (nr == 0) {
		table = (u64 *)get_spin_tbl_addr();
		printf("table base @ 0x%p\n", table);
	} else {
		valid = is_core_valid(nr);
		if (!valid)
			return -1;
		table = (u64 *)get_spin_tbl_addr() + nr * NUM_BOOT_ENTRY;
		printf("table @ 0x%p\n", table);
		printf("   addr - 0x%016llx\n", table[BOOT_ENTRY_ADDR]);
		printf("   r3   - 0x%016llx\n", table[BOOT_ENTRY_R3]);
		printf("   pir  - 0x%016llx\n", table[BOOT_ENTRY_PIR]);
	}

	return 0;
}

int cpu_release(u32 nr, int argc, char * const argv[])
{
	u64 boot_addr;
	u64 *table = (u64 *)get_spin_tbl_addr();
#ifndef CONFIG_FSL_SMP_RELEASE_ALL
	int valid;

	valid = is_core_valid(nr);
	if (!valid)
		return 0;

	table += nr * NUM_BOOT_ENTRY;
#endif
	boot_addr = simple_strtoull(argv[0], NULL, 16);
	table[BOOT_ENTRY_ADDR] = boot_addr;
	flush_dcache_range((unsigned long)table,
			   (unsigned long)table + SIZE_BOOT_ENTRY);
	asm volatile("dsb st");
	smp_kick_all_cpus();	/* only those with entry addr set will run */

	return 0;
}
