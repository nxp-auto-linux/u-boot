// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017,2019-2021 NXP
 */

#include <common.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch/siul.h>
#include <linux/sizes.h>
#include <errno.h>
#include "mp.h"

#define ID_TO_CORE(ID)	(((ID) & 3) | ((ID) >> 7))

#if defined(CONFIG_TARGET_S32G274AEVB) || defined(CONFIG_TARGET_S32G274ARDB)
#include <dt-bindings/clock/s32gen1-clock-freq.h>
#endif

#define S32_DDR_LIMIT_VAR "ddr_limitX"

#ifdef CONFIG_SYS_ERRATUM_ERR050543
extern uint8_t polling_needed;
#endif

#ifdef CONFIG_MP

#if CONFIG_S32_ATF_BOOT_FLOW
static void ft_fixup_enable_method(void *blob, int off, u64 __always_unused reg)
{
	const char *prop = fdt_getprop(blob, off, "enable-method", NULL);
	bool ovr = (prop == NULL);

	if (prop && strcmp(prop, "psci")) {
		printf("enable-method found: %s, overwriting with psci\n",
		       prop);
		ovr = true;
	}
	if (ovr)
		fdt_setprop_string(blob, off, "enable-method", "psci");
}
#endif

#if CONFIG_S32_ATF_BOOT_FLOW
/* Add a "psci" node at the top-level of the devide-tree,
 * if it does not already exist
 */
static void ft_fixup_psci_node(void *blob)
{
	int off;
	const char *prop;
	const char *exp_compatible = "arm,psci-1.0";
	const char *exp_method = "smc";
	bool ovr;

	off = fdt_path_offset(blob, "/psci");
	if (off >= 0) {
		/* Node exists, but we'll want to check it has
		 * the correct properties
		 */
		goto set_psci_prop;
	}
	if (off != -FDT_ERR_NOTFOUND)
		goto fdt_error;

	/* psci node did not exist, create one now */
	off = fdt_add_subnode(blob, 0, "psci");
	if (off < 0)
		goto fdt_error;

set_psci_prop:
	prop = fdt_getprop(blob, off, "compatible", NULL);
	ovr = (prop == NULL);
	if (prop && strcmp(prop, exp_compatible)) {
		printf("psci/compatible prop found: %s; replacing with %s\n",
		       prop, exp_compatible);
		ovr = true;
	}
	if (ovr)
		fdt_setprop_string(blob, off, "compatible", exp_compatible);

	prop = fdt_getprop(blob, off, "method", NULL);
	ovr = (prop == NULL);
	if (prop && strcmp(prop, exp_method)) {
		printf("psci/method prop found: %s; replacing with %s\n",
		       prop, exp_method);
		ovr = true;
	}
	if (ovr)
		fdt_setprop_string(blob, off, "method", exp_method);

	return;

fdt_error:
	printf("%s: error at \"psci\" node: %s\n", __func__, fdt_strerror(off));
}
#endif

void ft_fixup_cpu(void *blob)
{
	int off, addr_cells;
	u64 core_id;
	fdt32_t *reg;
	u32 mask = cpu_pos_mask();
	int off_prev = -1;

	off = fdt_path_offset(blob, "/cpus");
	if (off < 0) {
		puts("couldn't find /cpus node\n");
		return;
	}

	fdt_support_default_count_cells(blob, off, &addr_cells, NULL);

	off = fdt_node_offset_by_prop_value(blob, off_prev, "device_type",
					    "cpu", 4);
	while (off != -FDT_ERR_NOTFOUND) {
		reg = (fdt32_t *)fdt_getprop(blob, off, "reg", 0);
		if (!reg) {
			continue;
		}

		core_id = fdt_read_number(reg, addr_cells);
		if (!test_bit(ID_TO_CORE(core_id), &mask)) {
			fdt_del_node(blob, off);
			off = off_prev;
		} else {
#if CONFIG_S32_ATF_BOOT_FLOW
			ft_fixup_enable_method(blob, off, *reg);
#endif
		}

		off_prev = off;
		off = fdt_node_offset_by_prop_value(blob, off_prev,
						    "device_type", "cpu", 4);
	}

#if CONFIG_S32_ATF_BOOT_FLOW
	/* Check if a "psci" node should be added */
	ft_fixup_psci_node(blob);
#endif

	/*
	 * Boot page and spin table can be reserved here if not done statically
	 * in device tree.
	 *
	 * fdt_add_mem_rsv(blob, bootpg,
	 *		   *((u64 *)&(__secondary_boot_page_size)));
	 * If defined CONFIG_FSL_SMP_RELEASE_ALL, the release address should
	 * also be reserved.
	 */
}
#endif /* CONFIG_MP */

#ifdef CONFIG_S32V234
void ft_fixup_soc_revision(void *blob)
{
	const u32 socmask_info = readl(SIUL2_MIDR1) &
		(SIUL2_MIDR1_MINOR_MASK | SIUL2_MIDR1_MAJOR_MASK);
	const char *path = "/chosen";
	int ret;

	/* The booting guest may implement its own fixups based on the chip
	 * revision. One such example is PCIe erratum ERR009852, which can be
	 * safely ignored iff the chip is newer than revision 0.
	 * So pass this piece of info along in the FDT.
	 */
	ret = fdt_find_and_setprop(blob, path, "soc_revision", &socmask_info,
			sizeof(u32), 1);
	if (ret)
		printf("WARNING: Could not fix up the S32V234 device-tree, err=%s\n",
			fdt_strerror(ret));
}

void ft_fixup_clock_frequency(void *blob)
{
	const u32 cntfrq_be = cpu_to_be32(get_siul2_midr1_major() < 1 ?
			COUNTER_FREQUENCY_CUT1 : COUNTER_FREQUENCY);
	const char *path = "/timer";
	int ret;

	/* Update system clock_frequency according to the cpu detected version.
	 */
	ret = fdt_find_and_setprop(blob, path, "clock-frequency", &cntfrq_be,
			sizeof(u32), 1);
	if (ret)
		printf("WARNING: Could not fix up the S32V234 device-tree clock frequency, err=%s\n",
			fdt_strerror(ret));
}
#endif

#ifdef CONFIG_SYS_ERRATUM_ERR050543
static void ft_fixup_ddr_polling(void *blob)
{
	int off, ret;

	if (polling_needed != 1)
		return;

	off = fdt_path_offset(blob, "/ddr");
	if (off < 0) {
		printf("%s: error at \"ddr\" node: %s\n", __func__,
				fdt_strerror(off));
		return;
	}

	ret = fdt_set_node_status(blob, off, FDT_STATUS_OKAY, 0);
	if (ret)
		printf("WARNING: Could not fix up the S32GEN1 device-tree ddr, err=%s\n",
				fdt_strerror(ret));
}
#endif

static void hide_sram(bd_t *bd)
{
	int bank;

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		if (bd->bi_dram[bank].start == S32_SRAM_BASE) {
			bd->bi_dram[bank].start = 0;
			bd->bi_dram[bank].size = 0;
			break;
		}
	}
}

#if defined(CONFIG_NXP_S32G2XX) && defined(CONFIG_PRAM)
/* Fixup the DDR node in order to reserve "pram" amount of KB somewhere in the
 * available physical memory. This would typically be used by TF-A as a secure
 * memory, and enforced through XRDC. Making it "invisible" to Linux is only a
 * defensive means of keeping software out of trouble.
 * The point is, u-boot may not be able to probe the whole DRAM (and may not
 * care about all of it anyway), so using "mem=" bootargs would not be enough.
 */
static void exclude_pram(bd_t *bd)
{
	int bank;

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		if (bd->bi_dram[bank].start == CONFIG_SYS_FSL_DRAM_BASE1) {
			bd->bi_dram[bank].size -= CONFIG_PRAM * SZ_1K;
			break;
		}
	}
}
#endif

static void apply_memory_fixups(void *blob, bd_t *bd)
{
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	int ret, bank, banks = 0;

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		if (!bd->bi_dram[bank].start && !bd->bi_dram[bank].size)
			continue;

		start[banks] = bd->bi_dram[bank].start;
		size[banks] = bd->bi_dram[bank].size;
		banks++;
	}

	ret = fdt_fixup_memory_banks(blob, start, size, banks);
	if (ret)
		pr_err("s32-fdt: Failed to set memory banks\n");
}

static void apply_ddr_limits(bd_t *bd)
{
	u64 start, end, limit;
	static const size_t var_len = sizeof(S32_DDR_LIMIT_VAR);
	static const size_t digit_pos = var_len - 2;
	char ddr_limit[var_len];
	char *var_val;
	int bank;

	memcpy(ddr_limit, S32_DDR_LIMIT_VAR, var_len);

	ddr_limit[digit_pos] = '0';
	while ((var_val = env_get(ddr_limit))) {
		limit = simple_strtoull(var_val, NULL, 16);

		for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
			start = bd->bi_dram[bank].start;
			end = start + bd->bi_dram[bank].size;

			if (limit >= start && limit < end)
				bd->bi_dram[bank].size = limit - start;
		}

		if (ddr_limit[digit_pos] >= '9')
			break;

		ddr_limit[digit_pos]++;
	};
}

#if defined(CONFIG_TARGET_S32G274AEVB) || defined(CONFIG_TARGET_S32G274ARDB)
static void ft_fixup_qspi_frequency(void *blob)
{
	const u32 qspi_freq_be = cpu_to_be32(is_s32gen1_soc_rev1() ?
			S32G274A_REV1_QSPI_MAX_FREQ : S32GEN1_QSPI_MAX_FREQ);
	const char *path = "/spi@40134000/mx25uw51245g@0";
	int ret;

	/* Update QSPI max frequency according to the SOC detected rev.
	 */
	ret = fdt_find_and_setprop(blob, path, "spi-max-frequency",
				   &qspi_freq_be, sizeof(u32), 1);
	if (ret)
		printf("WARNING: Could not fix up QSPI device-tree max frequency, err=%s\n",
		       fdt_strerror(ret));
}
#endif

static void ft_fixup_memory(void *blob, bd_t *bd)
{
	hide_sram(bd);

#if defined(CONFIG_NXP_S32G2XX) && defined(CONFIG_PRAM)
	exclude_pram(bd);
#endif
	apply_ddr_limits(bd);

	apply_memory_fixups(blob, bd);

}

#ifdef CONFIG_S32_ATF_BOOT_FLOW
static int disable_clk_node(void *blob, uint32_t *phandle)
{
	const char *clk_path;
	int nodeoff, ret;

	clk_path = fdt_get_alias(blob, "clks");
	if (!clk_path) {
		pr_err("Failed to get path of 'clks' alias\n");
		return -EIO;
	}

	nodeoff = fdt_path_offset(blob, clk_path);
	if (nodeoff < 0) {
		pr_err("Failed to get offset of '%s' node\n", clk_path);
		return nodeoff;
	}

	*phandle = fdt_get_phandle(blob, nodeoff);
	if (*phandle < 0) {
		pr_err("Failed to get phandle of '%s' node\n", clk_path);
		return *phandle;
	}

	ret = fdt_set_node_status(blob, nodeoff, FDT_STATUS_DISABLED, 0);
	if (ret) {
		pr_err("Failed to disable '%s' node\n", clk_path);
		return ret;
	}

	ret = fdt_delprop(blob, nodeoff, "phandle");
	if (ret) {
		pr_err("Failed to remove phandle property of '%s' node\n",
		       clk_path);
		return ret;
	}

	return 0;
}

static int enable_scmi_clk_node(void *blob, uint32_t phandle)
{
	int nodeoff, ret;

	nodeoff = fdt_path_offset(blob, "/firmware/scmi/protocol@14");
	if (nodeoff < 0) {
		pr_err("Failed to get offset of '/firmware/scmi/protocol@14' node\n");
		return nodeoff;
	}

	ret = fdt_set_phandle(blob, nodeoff, phandle);
	if (ret) {
		pr_err("Failed to set phandle property of '/firmware/scmi/protocol@14' node\n");
		return ret;
	}

	return 0;
}

static int enable_scmi_mbox(void *blob)
{
	int nodeoff, ret;

	nodeoff = fdt_node_offset_by_compatible(blob, -1, "arm,smc-mbox");
	if (nodeoff < 0) {
		pr_debug("Failed to get offset of 'arm,smc-mbox' node\n");
		return -ENODEV;
	}

	ret = fdt_set_node_status(blob, nodeoff, FDT_STATUS_OKAY, 0);
	if (ret) {
		pr_err("Failed to enable 'arm,smc-mbox' node\n");
		return -ENXIO;
	}

	return 0;
}

static int enable_scmi_smc(void *blob)
{
	int nodeoff, ret;

	nodeoff = fdt_node_offset_by_compatible(blob, -1, "arm,scmi-smc");
	if (nodeoff < 0) {
		pr_debug("Failed to get offset of 'arm,scmi-smc' node\n");
		return -ENODEV;
	}

	ret = fdt_set_node_status(blob, nodeoff, FDT_STATUS_OKAY, 0);
	if (ret) {
		pr_err("Failed to enable 'arm,scmi-smc' node\n");
		return -ENXIO;
	}

	return 0;
}

static void ft_fixup_scmi_clks(void *blob)
{
	u32 phandle;

	if (disable_clk_node(blob, &phandle))
		return;

	if (enable_scmi_clk_node(blob, phandle))
		return;

	/* As of Linux Kernel version 5.10, the 'arm,smc-mbox'
	 * dts node is no longer used
	 */
	if (enable_scmi_mbox(blob) == -ENODEV)
		if (enable_scmi_smc(blob))
			pr_err("Failed to enable 'arm,smc-mbox' or 'arm,scmi-smc' node\n");
}
#endif

void ft_cpu_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_MP
	ft_fixup_cpu(blob);
#endif

#ifdef CONFIG_S32V234
	ft_fixup_soc_revision(blob);
	ft_fixup_clock_frequency(blob);
#endif
	ft_fixup_memory(blob, bd);
#if defined(CONFIG_TARGET_S32G274AEVB) || defined(CONFIG_TARGET_S32G274ARDB)
	ft_fixup_qspi_frequency(blob);
#endif
#ifdef CONFIG_SYS_ERRATUM_ERR050543
	ft_fixup_ddr_polling(blob);
#endif
#ifdef CONFIG_S32_ATF_BOOT_FLOW
	ft_fixup_scmi_clks(blob);
#endif
}
