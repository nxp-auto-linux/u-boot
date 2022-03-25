// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017,2019-2022 NXP
 */

#include <common.h>
#include <fdt_support.h>
#include <misc.h>
#include <asm/global_data.h>
#include <dm/uclass.h>
#include <linux/libfdt.h>
#include <s32-cc/a53_gpr.h>
#include <s32-cc/siul2_nvram.h>

DECLARE_GLOBAL_DATA_PTR;

static int get_core_id(u32 core_mpidr, u32 max_cores_per_cluster)
{
	u32 cluster_id = (core_mpidr >> 8) & 0xFF;

	return (core_mpidr & 0xf) + cluster_id * max_cores_per_cluster;
}

static int get_cores_info(u32 *max_cores_per_cluster,
			  u32 *cpu_mask)
{
	int ret;
	const char *dev_name = "siul2_0_nvram";
	struct udevice *siul2_nvmem = NULL;

	ret = uclass_get_device_by_name(UCLASS_MISC, dev_name,
					&siul2_nvmem);
	if (ret) {
		printf("%s: No SIUL21 NVMEM (err = %d)\n", __func__, ret);
		return ret;
	}

	ret = misc_read(siul2_nvmem, S32CC_MAX_A53_CORES_PER_CLUSTER,
			max_cores_per_cluster, sizeof(*max_cores_per_cluster));
	if (ret != sizeof(*max_cores_per_cluster)) {
		printf("%s: Failed to read SoC's Part Number (err = %d)\n",
		       __func__, ret);
		return ret;
	}
	if (!(*max_cores_per_cluster)) {
		printf("%s: Number of max cores per cluster cannot be 0\n",
		       __func__);
		return -EINVAL;
	}

	ret = misc_read(siul2_nvmem, S32CC_A53_CORES_MASK,
			cpu_mask, sizeof(*cpu_mask));
	if (ret != sizeof(*cpu_mask)) {
		printf("%s: Failed to read SoC's Part Number (err = %d)\n",
		       __func__, ret);
		return ret;
	}

	return 0;
}

static bool is_lockstep_enabled(void)
{
	int ret;
	u32 lockstep_enabled = 0;
	const char *dev_name = "a53_gpr";
	struct udevice *s32cc_a53_gpr = NULL;

	ret = uclass_get_device_by_name(UCLASS_MISC, dev_name,
					&s32cc_a53_gpr);
	if (ret) {
		printf("%s: No A53 GPR (err = %d)\n", __func__, ret);
		return false;
	}

	ret = misc_read(s32cc_a53_gpr, S32CC_A53_GPR_LOCKSTEP_EN,
			&lockstep_enabled, sizeof(lockstep_enabled));
	if (ret != sizeof(lockstep_enabled)) {
		printf("%s: Failed to read if Lockstep Enabled (err = %d)\n",
		       __func__, ret);
		return false;
	}

	return !!lockstep_enabled;
}

static int ft_fixup_cpu(void *blob)
{
	int ret, off, addr_cells = 0;
	int off_prev = -1;
	u32 max_cores_per_cluster = 0;
	u32 cpu_mask = 0;
	u64 core_mpidr, core_id;
	fdt32_t *reg;

	ret = get_cores_info(&max_cores_per_cluster, &cpu_mask);
	if (ret)
		return ret;

	off = fdt_path_offset(blob, "/cpus");
	if (off < 0) {
		puts("couldn't find /cpus node\n");
		return -ENODEV;
	}

	fdt_support_default_count_cells(blob, off, &addr_cells, NULL);
	off = fdt_node_offset_by_prop_value(blob, off_prev, "device_type",
					    "cpu", 4);

	if (is_lockstep_enabled()) {
		/* Disable secondary cluster */
		cpu_mask &= ~GENMASK(max_cores_per_cluster * 2 - 1,
							 max_cores_per_cluster);
	}

	while (off != -FDT_ERR_NOTFOUND) {
		reg = (fdt32_t *)fdt_getprop(blob, off, "reg", 0);
		if (!reg)
			continue;

		core_mpidr = fdt_read_number(reg, addr_cells);
		core_id = get_core_id(core_mpidr, max_cores_per_cluster);

		if (!test_bit(core_id, &cpu_mask)) {
			/* Disable lockstep or defeatured
			 * cores on derivatives
			 */
			fdt_del_node(blob, off);
			off = off_prev;
		}

		off_prev = off;
		off = fdt_node_offset_by_prop_value(blob, off_prev,
						    "device_type", "cpu", 4);
	}

	return 0;
}

static int ft_fixup_ddr_polling(const void *old_blob, void *new_blob)
{
	int off, ret;
	const char *status;
	const char *exp_compatible = "nxp,s32cc-ddr";

	/* Get node offset in U-Boot DT */
	off = fdt_node_offset_by_compatible(old_blob, -1, exp_compatible);
	if (off < 0) {
		printf("%s: Couldn't find \"%s\" node: %s\n", __func__,
		       exp_compatible, fdt_strerror(off));
		return -ENODEV;
	}

	/* Check "status" property */
	status = fdt_getprop(old_blob, off, "status", NULL);
	if (!status) {
		printf("%s: Node \"%s\" does not have \"status\" set",
		       __func__, exp_compatible);
		return -EINVAL;
	}

	if (!strncmp("disabled", status, 8))
		return 0;

	/* Get node offset in Linux DT */
	off = fdt_node_offset_by_compatible(new_blob, -1, exp_compatible);
	if (off < 0) {
		printf("%s: Couldn't find \"%s\" node: %s\n", __func__,
		       exp_compatible, fdt_strerror(off));
		return -ENODEV;
	}

	/* Copy the status from the U-Boot DT */
	ret = fdt_setprop_string(new_blob, off, "status", status);
	if (ret) {
		printf("WARNING: Could not fix up the Linux DT, err=%s\n",
		       fdt_strerror(ret));
		return ret;
	}

	return 0;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	int ret;

	ret = ft_fixup_cpu(blob);
	if (ret)
		return ret;

	return ft_fixup_ddr_polling(gd->fdt_blob, blob);
}
