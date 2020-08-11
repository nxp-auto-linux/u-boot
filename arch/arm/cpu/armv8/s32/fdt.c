// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017,2019-2020 NXP
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
	int off;
	u64 *reg;

	off = fdt_node_offset_by_prop_value(blob, -1, "device_type", "cpu", 4);
	while (off != -FDT_ERR_NOTFOUND) {
		reg = (u64 *)fdt_getprop(blob, off, "reg", 0);
		if (!reg) {
			puts("cpu NULL\n");
			continue;
		}
#if CONFIG_S32_ATF_BOOT_FLOW
		ft_fixup_enable_method(blob, off, *reg);
#endif
		off = fdt_node_offset_by_prop_value(blob, off, "device_type",
						    "cpu", 4);
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

/* Fixup the DDR node in order to reserve "pram" amount of KB somewhere in the
 * available physical memory. This would typically be used by TF-A as a secure
 * memory, and enforced through XRDC. Making it "invisible" to Linux is only a
 * defensive means of keeping software out of trouble.
 * The point is, u-boot may not be able to probe the whole DRAM (and may not
 * care about all of it anyway), so using "mem=" bootargs would not be enough.
 */
#if defined(CONFIG_S32G274A) && defined(CONFIG_PRAM)
static void ft_fixup_ddr_pram(void *blob)
{
	int off = -1, maxoff = -1;
	fdt32_t *reg;
	fdt_addr_t rambase, maxbase = 0;
	fdt_size_t ramsize, maxsize = 0;
	ulong pram_size;
	const void *val;
	void *newval;
	int len;

	while (1) {
		off = fdt_node_offset_by_prop_value(blob, off, "device_type",
						    "memory", 7);
		if (off == -FDT_ERR_NOTFOUND)
			break;

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
		/* Only take into account nodes that declare memory below the
		 * 2GB mark. In the SoC's memory map, these are guaranteed to
		 * reside in the 32-bit physical address space, so all we need
		 * to check is the start address of the region.
		 */
		if (rambase >> 32)
			continue;
		if (rambase + ramsize > maxbase + maxsize) {
			maxbase = rambase;
			maxsize = ramsize;
			maxoff = off;
		}
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

	printf("Reserving %ldk off the top of [%llx-%llx] for protected RAM\n",
	       pram_size >> 10, maxbase, maxbase + maxsize + pram_size - 1);
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
#endif /* CONFIG_S32G274A && CONFIG_PRAM */

void ft_cpu_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_MP
	ft_fixup_cpu(blob);
#endif

#ifdef CONFIG_S32V234
	ft_fixup_soc_revision(blob);
	ft_fixup_clock_frequency(blob);
#endif

#if defined(CONFIG_S32G274A) && defined(CONFIG_PRAM)
	ft_fixup_ddr_pram(blob);
#endif
}
