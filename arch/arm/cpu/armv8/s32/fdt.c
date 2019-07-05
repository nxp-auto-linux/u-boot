// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017,2019 NXP
 */

#include <common.h>
#include <linux/libfdt.h>
#include <malloc.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <asm/io.h>
#include <asm/arch/siul.h>
#include <linux/sizes.h>
#include "mp.h"

#ifdef CONFIG_MP

#if CONFIG_RUN_AT_EL2
/* U-Boot at EL2, presumably because of a Trusted Firmware running at EL3 */
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
#else
/* Standalone U-Boot, no TF-A; default cpu enable method will be spin-table */
static void ft_fixup_enable_method(void *blob, int off, u64 reg)
{
	__maybe_unused u64 spin_tbl_addr = (u64)get_spin_tbl_addr();
	u64 val = spin_tbl_addr;
	const char *prop_method;
	const u64 *prop_addr;
	bool ovr;

#ifndef CONFIG_FSL_SMP_RELEASE_ALL
	val += id_to_core(fdt64_to_cpu(reg)) * SIZE_BOOT_ENTRY;
#endif
	val = cpu_to_fdt64(val);

	prop_method = fdt_getprop(blob, off, "enable-method", NULL);
	ovr = (prop_method == NULL);
	if (prop_method && strcmp(prop_method, "spin-table")) {
		printf("enable-method found: %s, overwriting with spin-table\n",
		       prop_method);
		ovr = true;
	}
	if (ovr)
		fdt_setprop_string(blob, off, "enable-method", "spin-table");

	prop_addr = fdt_getprop(blob, off, "cpu-release-addr", NULL);
	ovr = (prop_addr == NULL);
	if (prop_addr && (*prop_addr != val)) {
		ovr = true;
		printf("cpu-release-addr found: %llx, "
		       "overwriting with %llx\n",
		       fdt64_to_cpu(*prop_addr), fdt64_to_cpu(val));
	}
	if (ovr)
		fdt_setprop(blob, off, "cpu-release-addr", &val, sizeof(val));
}
#endif

#if CONFIG_RUN_AT_EL2
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
	int off;
	u64 *reg;

	off = fdt_node_offset_by_prop_value(blob, -1, "device_type", "cpu", 4);
	while (off != -FDT_ERR_NOTFOUND) {
		reg = (u64 *)fdt_getprop(blob, off, "reg", 0);
		if (!reg) {
			puts("cpu NULL\n");
			continue;
		}
		ft_fixup_enable_method(blob, off, *reg);
		off = fdt_node_offset_by_prop_value(blob, off, "device_type",
						    "cpu", 4);
	}

#if CONFIG_RUN_AT_EL2
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

/* Fixup the DDR node in order to reserve "pram" amount of KB at the top of
 * available physical memory. This would typically be used by TF-A as a secure
 * memory, and enforced through XRDC. Making it "invisible" to Linux is only a
 * defensive means of keeping software out of trouble.
 * The point is, u-boot may not be able to probe the whole DRAM (and may not
 * care about all of it anyway), so using "mem=" bootargs would not be enough.
 */
#if defined(CONFIG_S32G275) && defined(CONFIG_PRAM)
static void ft_fixup_ddr_pram(void *blob)
{
	int off, maxoff = -1;
	fdt32_t *reg;
	fdt_addr_t rambase, maxbase = 0;
	fdt_size_t ramsize, maxsize = 0;
	ulong pram_size;
	const void *val;
	void *newval;
	int len;
	int count = 0;

	off = fdt_node_offset_by_prop_value(blob, -1, "device_type",
					    "memory", 7);
	while (off != -FDT_ERR_NOTFOUND) {
		count++;
		reg = (fdt32_t *)fdt_getprop(blob, off, "reg", 0);
		if (!reg) {
			puts("Warning: memory node with no reg property\n");
			continue;
		}
		rambase = fdtdec_get_addr_size(blob, off, "reg", &ramsize);
		if (rambase == FDT_ADDR_T_NONE || !ramsize) {
			puts("Warning: Can't get baseaddr/size\n");
			continue;
		}
		if (rambase + ramsize > maxbase + maxsize) {
			maxbase = rambase;
			maxsize = ramsize;
			maxoff = off;
		}
		off = fdt_node_offset_by_prop_value(blob, off, "device_type",
						    "memory", 7);
	}

	if (maxoff == -1) {
		puts("Error finding top memory node, needed for PRAM\n");
		return;
	}

	pram_size = env_get_ulong("pram", 10, CONFIG_PRAM) * SZ_1K;
	if (pram_size >= maxsize) {
		printf("Warning: PRAM larger than phys mem @0x%llx " \
		       "which is 0x%llx\n",
		       maxbase, maxsize);
		return;
	}
	maxsize -= pram_size;
	val = fdt_getprop(blob, maxoff, "reg", &len);
	if (len < sizeof(maxsize)) {
		puts("Error: invalid memory size\n");
		return;
	}

	printf("Reserving %ldk off the top of ddr@0x%llx for protected RAM; " \
	       "new size=0x%llx\n",
	       pram_size >> 10, maxbase, maxsize);
	switch (sizeof(maxsize)) {
	case 8:
		maxsize = cpu_to_be64(maxsize);
		break;
	case 4:
		maxsize = cpu_to_be32(maxsize);
		break;
	default:
		printf("Unexpected fdt_size_t=%ld\n", sizeof(maxsize));
		return;
	}

	newval = malloc(len);
	if (!newval) {
		printf("Error allocating %d bytes for new reg property\n", len);
		return;
	}
	memcpy(newval, val, len);
	*(fdt_size_t *)((char *)newval + len - sizeof(maxsize)) = maxsize;
	fdt_setprop(blob, maxoff, "reg", newval, len);
	/* It's safe to free the buffer now that it's been copied to the blob */
	free(newval);
}
#endif /* CONFIG_S32G275 && CONFIG_PRAM */

void ft_cpu_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_MP
	ft_fixup_cpu(blob);
#endif

#ifdef CONFIG_S32V234
	ft_fixup_soc_revision(blob);
	ft_fixup_clock_frequency(blob);
#endif

#if defined(CONFIG_S32G275) && defined(CONFIG_PRAM)
	ft_fixup_ddr_pram(blob);
#endif
}
