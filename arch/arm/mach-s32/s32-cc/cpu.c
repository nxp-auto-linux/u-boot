// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2022 NXP
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/armv8/mmu.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm-generic/sections.h>
#include "scmi_reset_agent.h"
#include <asm-generic/sections.h>
#include <linux/sizes.h>
#include <debug_uart.h>

#define S32GEN1_DRAM_STD_ADDR	0x80000000ULL
#define S32GEN1_DRAM_EXT_ADDR	0x800000000ULL

#ifndef CONFIG_SYS_DCACHE_OFF
#define PERIPH_BASE      0x40000000
#define PERIPH_SIZE      0x20000000

#ifdef CONFIG_PCIE_S32GEN1
#define CONFIG_SYS_PCIE0_PHYS_ADDR_HI       0x5800000000ULL
#define CONFIG_SYS_PCIE1_PHYS_ADDR_HI       0x4800000000ULL
#define CONFIG_SYS_PCIE0_PHYS_SIZE_HI       0x0800000000ULL
#define CONFIG_SYS_PCIE1_PHYS_SIZE_HI       0x0800000000ULL
#endif
#endif

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_DCACHE_OFF

static struct mm_region s32_mem_map[] = {
	{
	  PHYS_SDRAM_1, PHYS_SDRAM_1,
	  PHYS_SDRAM_1_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
#ifdef PHYS_SDRAM_2
	{
	  PHYS_SDRAM_2, PHYS_SDRAM_2,
	  PHYS_SDRAM_2_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
#endif
	{
	  S32_SRAM_BASE, S32_SRAM_BASE,
	  S32_SRAM_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE
	},
	{
	  PERIPH_BASE, PERIPH_BASE, PERIPH_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
	  PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
	  CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE,
	  CONFIG_SYS_FLASH_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE
	},
#if defined(CONFIG_PCIE_S32GEN1)
	{
	  CONFIG_SYS_PCIE0_PHYS_ADDR_HI, CONFIG_SYS_PCIE0_PHYS_ADDR_HI,
	  CONFIG_SYS_PCIE0_PHYS_SIZE_HI,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
	{
	  CONFIG_SYS_PCIE1_PHYS_ADDR_HI, CONFIG_SYS_PCIE1_PHYS_ADDR_HI,
	  CONFIG_SYS_PCIE1_PHYS_SIZE_HI,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
	  PTE_BLOCK_NON_SHARE | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#endif
	/* list terminator */
	{},
};

struct mm_region *mem_map = s32_mem_map;

static void disable_qspi_mmu_entry(void)
{
	struct mm_region *region;
	int offset;
	size_t i;

	offset = fdt_node_offset_by_compatible(gd->fdt_blob, -1,
					       "fsl,s32gen1-qspi");
	if (offset > 0) {
		if (fdtdec_get_is_enabled(gd->fdt_blob, offset))
			return;
	}

	/* Skip AHB mapping by setting its size to 0 */
	for (i = 0; i < ARRAY_SIZE(s32_mem_map); i++) {
		region = &s32_mem_map[i];
		if (region->phys == CONFIG_SYS_FLASH_BASE) {
			region->size = 0U;
			break;
		}
	}
}
#else
static void disable_qspi_mmu_entry(void)
{
}
#endif

int arch_cpu_init(void)
{
	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);

	disable_qspi_mmu_entry();

	gd->flags |= GD_FLG_SKIP_RELOC;

	if (IS_ENABLED(CONFIG_DEBUG_UART))
		debug_uart_init();

	return 0;
}

static void s32_init_ram_size(void)
{
	int i;
	unsigned long start, size;

	if (!gd->bd) {
		pr_err("gd->bd isn't initialized\n");
		return;
	}

	gd->ram_size = 0;
	for (i = 0; i < CONFIG_NR_DRAM_BANKS; i++) {
		start = gd->bd->bi_dram[i].start;
		size = gd->bd->bi_dram[i].size;

		if (!start && !size)
			continue;

		gd->ram_size += get_ram_size((long *)start, size);
	}
}

int dram_init_banksize(void)
{
	int ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		return ret;

	s32_init_ram_size();

	return 0;
}

phys_size_t __weak get_effective_memsize(void)
{
	unsigned long size;
	int nodeoff = -1, ret;
	struct fdt_resource res = {.start = 0, .end = 0};

	/*
	 * Restrict U-Boot area to the first bank of the DDR memory.
	 * Note: gd->bd isn't initialized yet
	 */
	size = PHYS_SDRAM_1_SIZE;

	/* Get first DDR bank size from DT 'memory' node */
	while ((nodeoff = fdt_node_offset_by_prop_value(gd->fdt_blob, nodeoff,
							"device_type",
							"memory", 7)) >= 0) {
		ret = fdt_get_resource(gd->fdt_blob, nodeoff, "reg", 0, &res);
		if (ret) {
			pr_err("Unable to get 'reg' values of memory node\n");
			return ret;
		}
		if (res.start == PHYS_SDRAM_1) {
			size = res.end - res.start + 1;
			break;
		}
	}

	return size;
}

void board_prep_linux(bootm_headers_t *images)
{
	int ret;

	ret = scmi_reset_agent();
	if (ret)
		pr_err("Failed to reset SCMI agent's settings\n");
}

void *board_fdt_blob_setup(void)
{
	void *dtb;

	dtb = (void *)(CONFIG_SYS_TEXT_BASE - CONFIG_S32GEN1_MAX_DTB_SIZE);

	if (fdt_magic(dtb) != FDT_MAGIC)
		panic("DTB is not passed via %p\n", dtb);

	return dtb;
}
