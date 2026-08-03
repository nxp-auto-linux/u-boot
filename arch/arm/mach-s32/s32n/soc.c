// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2024-2026 NXP
 */
#include <common.h>
#include <env.h>
#include <init.h>
#include <asm-generic/sections.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <dm/ofnode.h>
#include <linux/ioport.h>
#include <s32/soc.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_DCACHE_OFF

struct mm_region dyn_mem_map[] = {
	{ /* sentinel */ }
};

u64 get_page_table_size(void)
{
	/*
	 * The default implementation can't be used since the maximum
	 * size is computed based on the mem_map array, which in this case,
	 * doesn't contain the regions which will be dynamically added.
	 */
	return SZ_128K;
}
#endif

/*
 * The top of the DRAM is reserved for the Secure Monitor and part
 * of BL2. Use the base address of the reserved BL2 region needed
 * on the resume path to determine the top of usable RAM,
 * since it is deployed at lower addresses than the BL31 stage.
 */
ulong board_get_usable_ram_top(ulong total_size)
{
	struct resource reg;
	ofnode node;
	int ret;

	node = ofnode_path("/reserved-memory/bl2-resume");
	if (!ofnode_valid(node)) {
		log_err("Couldn't find 'bl2-resume' reserved-memory node\n");
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

/*
 * Sets environment variable which refers to an OS image
 * and which is computed based on an offset in memory.
 */
static void s32n_set_image_addr(const char *env, u64 offset)
{
	phys_addr_t addr;
	int ret;

	/* Check if the env variable is defined and not 0. */
	if (env_get_hex(env, 0))
		return;

	if (check_uptr_overflow(gd->ram_base, offset)) {
		pr_err("\"%s\" environment variable overflows\n", env);
		return;
	}

	addr = gd->ram_base + offset;
	if (addr >= gd->ram_top) {
		pr_err("\"%s\" environment variable out of DRAM memory range\n",
		       env);
		return;
	}

	ret = env_set_hex(env, addr);
	if (ret)
		pr_warn("\"%s\" environment variable was not set correctly\n",
			env);
}

int arch_misc_init(void)
{
	s32n_set_image_addr("loadaddr", CONFIG_SYS_LOAD_RAM_OFFSET);
	s32n_set_image_addr("fdt_addr", CONFIG_DTB_RAM_OFFSET);
	s32n_set_image_addr("ramdisk_addr", CONFIG_RAMDISK_RAM_OFFSET);

	return 0;
}

