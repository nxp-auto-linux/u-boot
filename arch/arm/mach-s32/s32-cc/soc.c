// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2022 NXP
 */
#include <common.h>
#include <debug_uart.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <asm/armv8/mmu.h>
#include <dm/ofnode.h>
#include <dm/uclass.h>
#include <s32-cc/pcie.h>
#include <s32-cc/serdes_hwconfig.h>

DECLARE_GLOBAL_DATA_PTR;

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

#ifndef CONFIG_SYS_DCACHE_OFF
static struct mm_region s32_mem_map[] = {
	{
		PHYS_SDRAM_1, PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE,
		PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE |
		    PTE_BLOCK_NS
	},
#ifdef PHYS_SDRAM_2
	{
		PHYS_SDRAM_2, PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE,
		PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE |
		    PTE_BLOCK_NS
	},
#endif
	{
		S32CC_SRAM_BASE, S32CC_SRAM_BASE,
		S32CC_SRAM_SIZE,
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
	ofnode node;
	size_t i;

	node = ofnode_by_compatible(ofnode_null(), "nxp,s32cc-qspi");
	if (ofnode_valid(node) && ofnode_is_available(node))
		return;

	/* Skip AHB mapping by setting its size to 0 */
	for (i = 0; i < ARRAY_SIZE(s32_mem_map); i++) {
		region = &s32_mem_map[i];
		if (region->phys == CONFIG_SYS_FLASH_BASE) {
			region->size = 0U;
			break;
		}
	}
}
#else /* CONFIG_SYS_DCACHE_OFF */
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

__weak void show_pcie_devices(void)
{
}

int initr_pci(void)
{
	int ret;

	debug("%s\n", __func__);

	if (IS_ENABLED(CONFIG_OF_LIVE)) {
		ret = apply_dm_hwconfig_fixups();
		if (ret) {
			pr_err("Failed to apply HWCONFIG fixups\n");
			return ret;
		}
	}

	/*
	 * Enumerate all known PCIe controller devices. Enumeration has
	 * the side-effect of probing them, so PCIe devices will be
	 * enumerated too.
	 * This is inspired from commands `pci` and `dm tree`.
	 */

	pci_init();

	/* now show the devices */
	show_pcie_devices();

	return 0;
}
