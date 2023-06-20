// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022,2023 NXP
 */
#include <common.h>
#include <debug_uart.h>
#include <init.h>
#include <soc.h>
#include <asm/global_data.h>
#include <asm/sections.h>
#include <asm/system.h>
#include <asm/armv8/mmu.h>
#include <dm/ofnode.h>
#include <s32-cc/s32cc_soc.h>
#include <s32-cc/serdes_hwconfig.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_SYS_DCACHE_OFF
#define PERIPH_BASE      0x40000000
#define PERIPH_SIZE      0x20000000

#define CONFIG_SYS_PCIE0_PHYS_ADDR_HI       0x5800000000ULL
#define CONFIG_SYS_PCIE1_PHYS_ADDR_HI       0x4800000000ULL
#define CONFIG_SYS_PCIE0_PHYS_SIZE_HI       0x0800000000ULL
#define CONFIG_SYS_PCIE1_PHYS_SIZE_HI       0x0800000000ULL
#endif

#define DTB_SIZE	(CONFIG_S32CC_MAX_DTB_SIZE)
#define DTB_ADDR	(CONFIG_SYS_TEXT_BASE - DTB_SIZE)

#define DDR_ATTRS	(PTE_BLOCK_MEMTYPE(MT_NORMAL) | \
			 PTE_BLOCK_OUTER_SHARE | \
			 PTE_BLOCK_NS)

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
		PHYS_SDRAM_1, PHYS_SDRAM_1, PHYS_SDRAM_1_SIZE, DDR_ATTRS
	},
	{
		DTB_ADDR, DTB_ADDR, DTB_SIZE,
		DDR_ATTRS | PTE_BLOCK_AP_RO | PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#ifdef PHYS_SDRAM_2
	{
		PHYS_SDRAM_2, PHYS_SDRAM_2, PHYS_SDRAM_2_SIZE, DDR_ATTRS
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
#endif

int arch_cpu_init(void)
{
	gd->flags |= GD_FLG_SKIP_RELOC;

	if (IS_ENABLED(CONFIG_DEBUG_UART))
		debug_uart_init();

	return 0;
}

int arch_cpu_init_dm(void)
{
	mmu_disable_qspi_entry();
	mmu_set_sram_size();

	return 0;
}

int cpu_secondary_init_r(void)
{
	int ret;

	/*
	 * This is the only place where the environment is available
	 * and PCIe initialization didn't happen yet.
	 */
	if (IS_ENABLED(CONFIG_OF_LIVE)) {
		ret = apply_dm_hwconfig_fixups();
		if (ret) {
			pr_err("Failed to apply HWCONFIG fixups\n");
			return ret;
		}
	}

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
