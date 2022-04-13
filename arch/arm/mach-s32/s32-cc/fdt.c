// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017,2019-2022 NXP
 */

#include <common.h>
#include <env.h>
#include <fdt_support.h>
#include <misc.h>
#include <asm/global_data.h>
#include <dm/uclass.h>
#include <linux/libfdt.h>
#include <s32-cc/a53_gpr.h>
#include <s32-cc/siul2_nvram.h>

DECLARE_GLOBAL_DATA_PTR;

#define S32_DDR_LIMIT_VAR "ddr_limitX"

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

static int apply_memory_fixups(void *blob, struct bd_info *bd)
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

	return ret;
}

static void apply_ddr_limits(struct bd_info *bd)
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

static int ft_fixup_memory(void *blob, struct bd_info *bd)
{
	apply_ddr_limits(bd);

	return apply_memory_fixups(blob, bd);
}

static int add_atf_reserved_memory(const void *old_blob, void *new_blob)
{
	int ret, off;
	struct fdt_memory carveout;
	struct fdt_resource reg;

	/* Check FDT Headers */
	if (fdt_check_header(old_blob)) {
		pr_err("Invalid FDT Header for U-Boot DT Blob\n");
		return -EINVAL;
	}

	if (fdt_check_header(new_blob)) {
		pr_err("Invalid FDT Header for Linux DT Blob\n");
		return -EINVAL;
	}

	/* Get atf reserved-memory node offset */
	off = fdt_path_offset(old_blob, "/reserved-memory/atf");
	if (off < 0) {
		pr_err("Couldn't find 'atf' reserved-memory node\n");
		return off;
	}

	/* Get value of 'reg' prop */
	ret = fdt_get_resource(old_blob, off, "reg", 0, &reg);
	if (ret) {
		pr_err("Unable to get value of 'reg' prop of 'atf' node\n");
		return ret;
	}

	carveout.start = reg.start;
	carveout.end = reg.end;

	/* Increase Linux DT size before adding new node */
	ret = fdt_increase_size(new_blob, 512);
	if (ret < 0) {
		pr_err("Could not increase size of Linux DT: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	/* Add 'atf' node to Linux DT */
	ret = fdtdec_add_reserved_memory(new_blob, "atf", &carveout,
					 NULL, 0, NULL,
					 FDTDEC_RESERVED_MEMORY_NO_MAP);
	if (ret < 0) {
		pr_err("Unable to add 'atf' node to Linux DT\n");
		return ret;
	}

	return 0;
}

static int ft_fixup_atf(const void *old_blob, void *new_blob)
{
	int ret = add_atf_reserved_memory(old_blob, new_blob);

	if (ret)
		pr_err("Copying 'atf' node from U-Boot DT to Linux DT failed!\n");

	return ret;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	int ret;

	ret = ft_fixup_cpu(blob);
	if (ret)
		goto exit;

	ret = ft_fixup_ddr_polling(gd->fdt_blob, blob);
	if (ret)
		goto exit;

	ret = ft_fixup_memory(blob, bd);
	if (ret)
		goto exit;

	ret = ft_fixup_atf(gd->fdt_blob, blob);

exit:
	return ret;
}
