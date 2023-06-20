// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2023 NXP
 */
#include <common.h>
#include <cpu_func.h>
#include <debug_uart.h>
#include <init.h>
#include <malloc.h>
#include <relocate.h>
#include <soc.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <dm/ofnode.h>
#include <dm/uclass.h>
#include <s32-cc/pcie.h>
#include <s32-cc/s32cc_soc.h>
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

#define S32CC_SRAM_6M	(6 * SZ_1M)
#define S32CC_SRAM_8M	(8 * SZ_1M)
#define S32CC_SRAM_15M	(15 * SZ_1M)
#define S32CC_SRAM_20M	(20 * SZ_1M)

struct s32cc_soc_sram_size {
	u32 sram_size;
};

static const struct soc_attr s32cc_soc_sram_size_data[] = {
	{
		.machine = SOC_MACHINE_S32G233A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_6M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G254A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_8M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G274A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_8M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G358A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_15M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G359A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_20M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G378A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_15M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G379A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_20M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G398A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_15M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G399A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_20M,
		},
	},
	{
		.machine = SOC_MACHINE_S32R455A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_8M,
		},
	},
	{
		.machine = SOC_MACHINE_S32R458A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_8M,
		},
	},
	{ /* sentinel */ }
};

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
		S32CC_SRAM_BASE, S32CC_SRAM_BASE, 0U,
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

static void mmu_disable_qspi_entry(void)
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

static int early_mmu_init(void)
{
	u64 pgtable_size = PGTABLE_SIZE;

	gd->arch.tlb_addr = (uintptr_t)memalign(pgtable_size, pgtable_size);
	if (!gd->arch.tlb_addr)
		return -ENOMEM;

	gd->arch.tlb_size = pgtable_size;
	icache_enable();
	dcache_enable();

	return 0;
}

static void clear_early_mmu_settings(void)
{
	/* Reset fillptr to allow reinit of page tables */
	gd->arch.tlb_fillptr = (uintptr_t)NULL;

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

static int get_sram_size(u32 *sram_size)
{
	const struct soc_attr *soc_match_data;
	const struct s32cc_soc_sram_size *s32cc_match_data;

	soc_match_data = soc_device_match(s32cc_soc_sram_size_data);
	if (!soc_match_data)
		return -EINVAL;

	s32cc_match_data = (struct s32cc_soc_sram_size *)soc_match_data->data;
	*sram_size = s32cc_match_data->sram_size;

	debug("%s: SRAM size: %u\n", __func__, *sram_size);

	return 0;
}

static void mmu_set_sram_size(void)
{
	struct mm_region *region;
	size_t i;
	u32 sram_size;
	int ret;

	ret = get_sram_size(&sram_size);
	if (ret)
		panic("Failed to get SRAM size (err=%d)\n", ret);

	for (i = 0; i < ARRAY_SIZE(s32_mem_map); i++) {
		region = &s32_mem_map[i];
		if (region->phys == S32CC_SRAM_BASE) {
			region->size = sram_size;
			break;
		}
	}
}

#else /* CONFIG_SYS_DCACHE_OFF */
static void mmu_disable_qspi_entry(void)
{
}

static void mmu_set_sram_size(void)
{
}

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

int arch_cpu_init_dm(void)
{
	mmu_disable_qspi_entry();
	mmu_set_sram_size();

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
