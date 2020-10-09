// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2020 NXP
 */

#include <common.h>
#include <cpu_func.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/arch/cpu.h>
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/siul.h>
#include <asm-generic/sections.h>
#include "mp.h"
#include "dma_mem.h"
#include <asm/arch/soc.h>
#include <asm/arch/s32-gen1/a53_cluster_gpr.h>
#include <asm/arch/s32-gen1/ncore.h>
#include <s32gen1_clk_utils.h>
#include <asm-generic/sections.h>
#include <clk.h>
#include <dm/uclass.h>
#include <linux/sizes.h>
#include <power/pmic.h>
#include <power/vr5510.h>

DECLARE_GLOBAL_DATA_PTR;

#if defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
static int s32_gentimer_init(void);
#endif

void mmu_setup(void);

#ifndef CONFIG_SYS_DCACHE_OFF

static struct mm_region early_map[] = {
#ifndef CONFIG_S32_SKIP_RELOC
#ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
	{
	  CONFIG_SYS_FSL_DRAM_BASE1, CONFIG_SYS_FSL_DRAM_BASE1,
	  CONFIG_SYS_FSL_DRAM_SIZE1,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE | PTE_BLOCK_NS
	},
#else
	/* DRAM_SIZE1 is configurable via defconfig, but there are both
	 * address and size alignment restrictions in the MMU table lookup code
	 */
	{
	  CONFIG_SYS_FSL_DRAM_BASE1, CONFIG_SYS_FSL_DRAM_BASE1,
	  CONFIG_SYS_FSL_DRAM_SIZE1,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL_NC) | PTE_BLOCK_OUTER_SHARE
	},
#endif
#endif
	{
	  S32_SRAM_BASE, S32_SRAM_BASE,
	  S32_SRAM_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE
	},
#ifdef CONFIG_S32_GEN1
	{
	  CONFIG_SYS_FSL_PERIPH_BASE, CONFIG_SYS_FSL_PERIPH_BASE,
	  CONFIG_SYS_FSL_PERIPH_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
	  PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#else	/* S32V234 */
	{
	  CONFIG_SYS_FSL_PERIPH_BASE, CONFIG_SYS_FSL_PERIPH_BASE,
	  CONFIG_SYS_FSL_PERIPH_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE
	},
#endif /* CONFIG_S32_GEN1 */
#ifndef CONFIG_S32_SKIP_RELOC
	{
	  CONFIG_SYS_FSL_DRAM_BASE2, CONFIG_SYS_FSL_DRAM_BASE2,
	  CONFIG_SYS_FSL_DRAM_SIZE2,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL_NC) | PTE_BLOCK_OUTER_SHARE
	},
#endif
	{
	  CONFIG_SYS_FSL_FLASH0_BASE, CONFIG_SYS_FSL_FLASH0_BASE,
	  CONFIG_SYS_FSL_FLASH0_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL_NC) | PTE_BLOCK_OUTER_SHARE
	},
#ifdef CONFIG_SYS_FSL_FLASH1_BASE
	{
	  CONFIG_SYS_FSL_FLASH1_BASE, CONFIG_SYS_FSL_FLASH1_BASE,
	  CONFIG_SYS_FSL_FLASH1_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL_NC) | PTE_BLOCK_OUTER_SHARE
	},
#endif
	/* list terminator */
	{},
};

static struct mm_region final_map[] = {
#ifndef CONFIG_S32_SKIP_RELOC
#ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
	{
	  CONFIG_SYS_FSL_DRAM_BASE1, CONFIG_SYS_FSL_DRAM_BASE1,
	  CONFIG_SYS_FSL_DRAM_SIZE1,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE
	},
#else
	{
	  CONFIG_SYS_FSL_DRAM_BASE1, CONFIG_SYS_FSL_DRAM_BASE1,
	  CONFIG_SYS_FSL_DRAM_SIZE1,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE
	},
#endif
#endif
	{
	  S32_SRAM_BASE, S32_SRAM_BASE,
	  S32_SRAM_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE
	},
#ifdef CONFIG_S32_GEN1
	{
	  CONFIG_SYS_FSL_PERIPH_BASE, CONFIG_SYS_FSL_PERIPH_BASE,
	  CONFIG_SYS_FSL_PERIPH_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE |
	  PTE_BLOCK_PXN | PTE_BLOCK_UXN
	},
#else
	{
	  CONFIG_SYS_FSL_PERIPH_BASE, CONFIG_SYS_FSL_PERIPH_BASE,
	  CONFIG_SYS_FSL_PERIPH_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) | PTE_BLOCK_NON_SHARE
	},
#endif /* CONFIG_S32_GEN1 */
#ifndef CONFIG_S32_SKIP_RELOC
	{
	  CONFIG_SYS_FSL_DRAM_BASE2, CONFIG_SYS_FSL_DRAM_BASE2,
	  CONFIG_SYS_FSL_DRAM_SIZE2,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE
	},
#endif
	{
	  CONFIG_SYS_FSL_FLASH0_BASE, CONFIG_SYS_FSL_FLASH0_BASE,
	  CONFIG_SYS_FSL_FLASH0_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE
	},
#ifdef CONFIG_SYS_FSL_FLASH1_BASE
	{
	  CONFIG_SYS_FSL_FLASH1_BASE, CONFIG_SYS_FSL_FLASH1_BASE,
	  CONFIG_SYS_FSL_FLASH1_SIZE,
	  PTE_BLOCK_MEMTYPE(MT_NORMAL) | PTE_BLOCK_OUTER_SHARE
	},
#endif
#if defined(CONFIG_PCIE_S32GEN1)
	/* TODO: for CONFIG_DM_PCI, we should get address/size from
	 * device tree
	 */
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

struct mm_region *mem_map = early_map;

#ifdef CONFIG_S32V234
static void enable_snooping(void)
{
	struct ccsr_cci400 *cci = (struct ccsr_cci400 *)CCI400_BASE_ADDR;

	out_le32(&cci->slave[3].snoop_ctrl,
		 CCI400_DVM_MESSAGE_REQ_EN | CCI400_SNOOP_REQ_EN);
	out_le32(&cci->slave[4].snoop_ctrl,
		 CCI400_DVM_MESSAGE_REQ_EN | CCI400_SNOOP_REQ_EN);
}
#endif

static inline void early_mmu_setup(void)
{
	/* global data is already setup, no allocation yet */
	gd->arch.tlb_addr = S32_IRAM_MMU_TABLES_BASE;
	gd->arch.tlb_size = CONFIG_SYS_TEXT_BASE - S32_IRAM_MMU_TABLES_BASE;

#if defined(CONFIG_S32_SKIP_RELOC) && !defined(CONFIG_S32_ATF_BOOT_FLOW)
	dma_mem_clr(gd->arch.tlb_addr, gd->arch.tlb_size);
#endif

#ifdef CONFIG_S32V234
	enable_snooping();
#endif
	mmu_setup();
	set_sctlr(get_sctlr() | CR_C);
}

/* Saved TLB settings for secondaries */
uintptr_t s32_tlb_addr;
u64 s32_tcr;

static inline void save_tlb(void)
{
	s32_tlb_addr = gd->arch.tlb_addr;
	s32_tcr = get_tcr(current_el(), NULL, NULL);

	flush_dcache_range((unsigned long)&s32_tlb_addr,
			   (unsigned long)&s32_tlb_addr + sizeof(s32_tlb_addr));

	flush_dcache_range((unsigned long)&s32_tcr,
			   (unsigned long)&s32_tcr + sizeof(s32_tcr));
}

static inline void final_mmu_setup(void)
{
	unsigned int el = current_el();

	mem_map = final_map;

	/* global data is already setup, no allocation yet */
	gd->arch.tlb_addr = S32_SDRAM_MMU_TABLES_BASE;
	gd->arch.tlb_fillptr = gd->arch.tlb_addr;
	gd->arch.tlb_size = CONFIG_SYS_TEXT_BASE - S32_IRAM_MMU_TABLES_BASE;

#if defined(CONFIG_S32_SKIP_RELOC) && !defined(CONFIG_S32_ATF_BOOT_FLOW)
	dma_mem_clr(gd->arch.tlb_addr, gd->arch.tlb_size);
#endif
	setup_pgtables();

	/* flush new MMU table */
	/* Disable cache and MMU */
	dcache_disable();   /* TLBs are invalidated */
	invalidate_icache_all();

	/* point TTBR to the new table */
	set_ttbr_tcr_mair(el, gd->arch.tlb_addr, get_tcr(el, NULL, NULL),
			  MEMORY_ATTRIBUTES);
	__asm_invalidate_tlb_all();

	/* gd->arch.tlb_emerg is used by mmu_set_region_dcache_behaviour */
	gd->arch.tlb_emerg = gd->arch.tlb_addr;

	save_tlb();

	/*
	 * MMU is already enabled, just need to invalidate TLB to load the
	 * new table. The new table is compatible with the current table, if
	 * MMU somehow walks through the new table before invalidation TLB,
	 * it still works. So we don't need to turn off MMU here.
	 */
	set_sctlr(get_sctlr() | CR_M);
}

#ifdef CONFIG_S32_GEN1
/*
 * This function is a temporary fix for drivers without clock bindings.
 *
 * It should be removed once Linux kernel is able to enable all
 * needed clocks and all U-Boot drivers have clock bindings.
 *
 * E.g. QSPI, FTM, etc.
 */
static int enable_periph_clocks(void)
{
	struct udevice *dev;
	struct clk_bulk bulk;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_CLK, "clks", &dev);
	if (ret) {
		pr_err("Failed to get 'clk' device\n");
		return ret;
	}

	ret = clk_get_bulk(dev, &bulk);
	if (ret == -EINVAL)
		return 0;

	if (ret)
		return ret;

	ret = clk_enable_bulk(&bulk);
	if (ret) {
		clk_release_bulk(&bulk);
		return ret;
	}

	return 0;
}
#endif

#if defined(CONFIG_S32_SKIP_RELOC) && !defined(CONFIG_S32_ATF_BOOT_FLOW)
static inline int clear_after_bss(void)
{
	int base, size, ret;
	/*
	 * Assumption: lowlevel.S will clear at least [__bss_start - __bss_end]
	 */
	base = (uintptr_t)&__bss_end;
	size = S32_SRAM_BASE + S32_SRAM_SIZE - base;
	ret = dma_mem_clr(base, size);
	if (!ret)
		return ret;

	return 0;
}
#endif

#if defined(CONFIG_DM_PMIC_VR5510) && !defined(CONFIG_S32_ATF_BOOT_FLOW)
static int watchdog_refresh(struct udevice *pmic)
{
	uint seed, wd_cfg;
	int ret;

	seed = pmic_reg_read(pmic, VR5510_FS_WD_SEED);

	/* Challenger watchdog refresh */
	seed = ((~(seed * 4 + 2) & 0xFFFFFFFFU) / 4) & 0xFFFFU;

	ret = pmic_reg_write(pmic, VR5510_FS_WD_ANSWER, seed);
	if (ret) {
		pr_err("Failed to write VR5510 WD answer\n");
		return ret;
	}

	wd_cfg = pmic_reg_read(pmic, VR5510_FS_I_WD_CFG);
	if (VR5510_ERR_CNT(wd_cfg)) {
		pr_err("Failed to refresh watchdog\n");
		return -EIO;
	}

	return 0;
}

static int disable_vr5510_wdg(void)
{
	struct udevice *pmic;
	uint wd_window, safe_input, fs_states, diag;
	int ret;

	ret = pmic_get(VR5510_FSU_NAME, &pmic);
	if (ret)
		return ret;

	fs_states = pmic_reg_read(pmic, VR5510_FS_STATES);
	if (VR5510_STATE(fs_states) != INIT_FS) {
		pr_err("VR5510 is not in INIT_FS state\n");
		return -EIO;
	}

	/* Disable watchdog */
	wd_window = pmic_reg_read(pmic, VR5510_FS_WD_WINDOW);
	wd_window &= ~VR5510_WD_WINDOW_MASK;
	ret = pmic_reg_write(pmic, VR5510_FS_WD_WINDOW, wd_window);
	if (ret) {
		pr_err("Failed write watchdog window\n");
		return ret;
	}

	wd_window = ~wd_window & 0xFFFFU;
	ret = pmic_reg_write(pmic, VR5510_FS_NOT_WD_WINDOW, wd_window);
	if (ret) {
		pr_err("Failed write watchdog window\n");
		return ret;
	}

	diag = pmic_reg_read(pmic, VR5510_FS_DIAG_SAFETY);
	if (!VR5510_ABIST1_OK(diag)) {
		pr_err("VR5510 is not in ABIST1 state\n");
		return ret;
	}

	/* Disable FCCU monitoring */
	safe_input = pmic_reg_read(pmic, VR5510_FS_I_SAFE_INPUTS);
	safe_input &= ~VR5510_FCCU_CFG_MASK;
	ret = pmic_reg_write(pmic, VR5510_FS_I_SAFE_INPUTS, safe_input);
	if (ret) {
		pr_err("Failed to disable FCCU\n");
		return ret;
	}

	safe_input = ~safe_input & 0xFFFFU;
	ret = pmic_reg_write(pmic, VR5510_FS_I_NOT_SAFE_INPUTS, safe_input);
	if (ret) {
		pr_err("Failed to disable FCCU\n");
		return ret;
	}

	return watchdog_refresh(pmic);
}
#endif

int arch_cpu_init(void)
{
	int ret = 0;

#ifdef CONFIG_S32_ATF_BOOT_FLOW
	/* Clear the BSS. */
	memset(__bss_start, 0, __bss_end - __bss_start);
#endif

#ifdef CONFIG_S32_SKIP_RELOC
	gd->flags |= GD_FLG_SKIP_RELOC;
#endif

#if defined(CONFIG_S32_SKIP_RELOC) && !defined(CONFIG_S32_ATF_BOOT_FLOW)
	ret = clear_after_bss();
	if (ret)
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
	ret = enable_early_clocks();
	if (ret)
		return ret;

	ncore_init(0x1);
#endif

	icache_enable();
	__asm_invalidate_dcache_all();
	__asm_invalidate_tlb_all();
	early_mmu_setup();

#if defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
	s32_gentimer_init();
#endif
	return ret;
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
int arch_early_init_r(void)
{
	int rv = 0;

#if defined(CONFIG_DM_PMIC_VR5510) && !defined(CONFIG_S32_ATF_BOOT_FLOW)
	rv = disable_vr5510_wdg();
	if (rv)
		return rv;
#endif

#if !defined(CONFIG_S32_ATF_BOOT_FLOW)
	asm volatile("dsb sy");
	rv = fsl_s32_wake_secondary_cores();

	if (rv)
		printf("Did not wake secondary cores\n");

#ifdef CONFIG_S32_GEN1
	/* Reconfigure Concerto before actually waking the cores */
	ncore_init(0xf);
#endif
	asm volatile("sev");
#endif

#ifdef CONFIG_S32_GEN1
	rv = enable_periph_clocks();

	if (rv)
		return rv;
#endif

	return rv;
}
#endif /* CONFIG_ARCH_EARLY_INIT_R */

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

		/* Don't advertise SRAM */
		if (start == S32_SRAM_BASE)
			continue;

		if (!start && !size)
			continue;

		gd->ram_size += get_ram_size((long *)start, size);
	}
}

int dram_init_banksize(void)
{
#if defined(CONFIG_S32_SKIP_RELOC) && !defined(CONFIG_S32_ATF_BOOT_FLOW)
	gd->bd->bi_dram[0].start = S32_SRAM_BASE;
	gd->bd->bi_dram[0].size = S32_SRAM_SIZE;

	gd->bd->bi_dram[1].start = 0x0;
	gd->bd->bi_dram[1].size = 0x0;

	gd->bd->bi_dram[2].start = 0x0;
	gd->bd->bi_dram[2].size = 0x0;
#else
#ifdef CONFIG_S32_GEN1
	int ret;

	ret = fdtdec_setup_memory_banksize();
	if (ret)
		return ret;
#else
	gd->bd->bi_dram[0].start = CONFIG_SYS_FSL_DRAM_BASE1;
	gd->bd->bi_dram[0].size = CONFIG_SYS_FSL_DRAM_SIZE1;

	gd->bd->bi_dram[1].start = CONFIG_SYS_FSL_DRAM_BASE2;
	gd->bd->bi_dram[1].size = CONFIG_SYS_FSL_DRAM_SIZE2;

	gd->bd->bi_dram[2].start = S32_SRAM_BASE;
	gd->bd->bi_dram[2].size = S32_SRAM_SIZE;
#endif
#endif
	s32_init_ram_size();
	return 0;
}

phys_size_t __weak get_effective_memsize(void)
{
	phys_size_t size;
	/*
	 * Restrict U-Boot area to the first bank of the DDR memory.
	 * Note: gd->bd isn't initialized yet
	 */
#if defined(CONFIG_S32_SKIP_RELOC) && !defined(CONFIG_S32_ATF_BOOT_FLOW)
	size = S32_SRAM_SIZE;
#else
	size = CONFIG_SYS_FSL_DRAM_SIZE1;
#ifdef CONFIG_PRAM
	/* ATF space */
	size -= CONFIG_PRAM * SZ_1K;
#endif
#endif
	return size;
}
