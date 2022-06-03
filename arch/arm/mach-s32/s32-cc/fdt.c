// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017,2019-2022 NXP
 */

#include <common.h>
#include <fdt_support.h>
#include <fdtdec.h>
#include <hwconfig.h>
#include <malloc.h>
#include <misc.h>
#include <asm/arch-s32/s32-cc/serdes_hwconfig.h>
#include <dm/uclass.h>
#include <linux/ctype.h>
#include <linux/sizes.h>
#include <s32-cc/a53_gpr.h>
#include <s32-cc/fdt_wrapper.h>
#include <s32-cc/nvmem.h>
#include <dt-bindings/nvmem/s32cc-siul2-nvmem.h>
#include <dt-bindings/phy/phy.h>

DECLARE_GLOBAL_DATA_PTR;

#define PCIE_ALIAS_FMT		"pcie%d"
#define PCIE_ALIAS_SIZE		sizeof(PCIE_ALIAS_FMT)

#define SERDES_ALIAS_FMT	"serdes%d"
#define SERDES_ALIAS_SIZE	sizeof(SERDES_ALIAS_FMT)

#define SERDES_EXT_PATH_FMT		"/clocks/serdes_%d_ext"
#define SERDES_EXT_PATH_FMT_SIZE	sizeof(SERDES_EXT_PATH_FMT)
/* Add some space for SerDes ID */
#define SERDES_EXT_PATH_SIZE		(SERDES_EXT_PATH_FMT_SIZE + 2)

#define SERDES_EXT_CLK			"ext"
#define SERDES_EXT_SIZE			sizeof(SERDES_EXT_CLK)

#define SERDES_LINE_NAME_FMT	"serdes_lane%d"
#define SERDES_LINE_NAME_LEN	sizeof(SERDES_LINE_NAME_FMT)

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
	struct nvmem_cell cell;

	ret = uclass_get_device_by_name(UCLASS_MISC, dev_name,
					&siul2_nvmem);
	if (ret) {
		printf("%s: No SIUL21 NVMEM (err = %d)\n", __func__, ret);
		return ret;
	}

	ret = nvmem_cell_get_by_offset(siul2_nvmem,
				       S32CC_MAX_A53_CORES_PER_CLUSTER,
				       &cell);
	if (ret) {
		printf("%s: Failed to get A53 cores per cluster cell (err = %d)\n",
		       __func__, ret);
		return -ENODEV;
	}

	ret = nvmem_cell_read(&cell, max_cores_per_cluster,
			      sizeof(*max_cores_per_cluster));
	if (ret) {
		printf("%s: Failed to read A53 cores per cluster cell (err = %d)\n",
		       __func__, ret);
		return -EINVAL;
	}

	if (!(*max_cores_per_cluster)) {
		printf("%s: Number of max cores per cluster cannot be 0\n",
		       __func__);
		return -EINVAL;
	}

	ret = nvmem_cell_get_by_offset(siul2_nvmem,
				       S32CC_A53_CORES_MASK,
				       &cell);
	if (ret) {
		printf("%s: Failed to get A53 cores mask cell (err = %d)\n",
		       __func__, ret);
		return -ENODEV;
	}

	ret = nvmem_cell_read(&cell, cpu_mask, sizeof(*cpu_mask));
	if (ret) {
		printf("%s: Failed to read A53 cores mask cell (err = %d)\n",
		       __func__, ret);
		return -EINVAL;
	}

	return 0;
}

static bool is_lockstep_enabled(void)
{
	int ret, off;
	u32 lockstep_enabled = 0;
	struct udevice *s32cc_a53_gpr = NULL;
	const char *a53_compat = "nxp,s32cc-a53-gpr";

	off = fdt_node_offset_by_compatible(gd->fdt_blob, -1, a53_compat);
	if (off < 0) {
		printf("%s: Couldn't find \"%s\" node: %s\n", __func__,
		       a53_compat, fdt_strerror(off));
		return false;
	}

	ret = uclass_get_device_by_of_offset(UCLASS_MISC, off, &s32cc_a53_gpr);
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
	u32 max_cores_per_cluster = 0;
	u32 cpu_mask = 0;
	u64 core_mpidr, core_id;
	fdt32_t *reg;
	int off_prev;

	ret = get_cores_info(&max_cores_per_cluster, &cpu_mask);
	if (ret)
		return ret;

	off = fdt_path_offset(blob, "/cpus");
	if (off < 0) {
		puts("couldn't find /cpus node\n");
		return -ENODEV;
	}
	off_prev = off;

	fdt_support_default_count_cells(blob, off, &addr_cells, NULL);
	off = get_next_cpu(blob, off);

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
		off = get_next_cpu(blob, off);
	}

	return 0;
}

static int apply_memory_fixups(void *blob, bd_t *bd)
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

static int ft_fixup_memory(void *blob, bd_t *bd)
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
	ret = fdtdec_add_reserved_memory(new_blob, "atf", &carveout, NULL);
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

#ifdef CONFIG_PCIE_S32GEN1
static int fdt_alias2node(void *blob, const char *alias_fmt, int alias_id)
{
	const char *alias_path;
	char alias_name[strlen(alias_fmt) + 1];
	int nodeoff;

	sprintf(alias_name, alias_fmt, alias_id);

	alias_path = fdt_get_alias(blob, alias_name);
	if (!alias_path) {
		pr_err("Failed to get '%s' alias\n", alias_name);
		return -EINVAL;
	}

	nodeoff = fdt_path_offset(blob, alias_path);
	if (nodeoff < 0)
		pr_err("Failed to get offset of '%s' node\n", alias_path);

	return nodeoff;
}

static int set_pcie_mode(void *blob, int nodeoff, int id)
{
	int ret;
	const char *compatible;
	enum serdes_dev_type pcie_mode;

	pcie_mode = s32_serdes_get_mode_from_hwconfig(id);
	if (pcie_mode & PCIE_EP)
		compatible = "nxp,s32cc-pcie-ep";
	else
		compatible = "nxp,s32cc-pcie";

	ret = fdt_setprop(blob, nodeoff, "compatible", compatible,
			  strlen(compatible) + 1);
	if (ret) {
		pr_err("Failed to set PCIE compatible: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	return 0;
}

static int set_pcie_phy_mode(void *blob, int nodeoff, int id)
{
	int ret = 0;
	char mode[10] = "crns";
	enum serdes_phy_mode phy_mode;

	phy_mode = s32_serdes_get_phy_mode_from_hwconfig(id);
	if (phy_mode == PHY_MODE_INVALID) {
		pr_err("Invalid PCIe%d PHY mode", id);
		return -EINVAL;
	}

	switch (phy_mode) {
	case CRNS:
		break;
	case CRSS:
		strcpy(mode, "crss");
		break;
	case SRIS:
		strcpy(mode, "sris");
		break;
	default:
		pr_err("PCIe PHY mode not supported\n");
		return -EINVAL;
	}

	ret = fdt_setprop_string(blob, nodeoff, "nxp,phy-mode", mode);
	if (ret)
		pr_err("Failed to set 'nxp,phy-mode'\n");

	return ret;
}

static int add_serdes_lines(void *blob, int id, int lanes, uint32_t phandle)
{
	char serdes_lane[SERDES_LINE_NAME_LEN];
	int i, ret, nodeoff;

	nodeoff = fdt_alias2node(blob, PCIE_ALIAS_FMT, id);
	if (nodeoff < 0)
		return nodeoff;

	ret = fdt_setprop_u32(blob, nodeoff, "num-lanes", lanes);
	if (ret)
		pr_err("Failed to set 'num-lanes'\n");

	for (i = 0; i < lanes; i++) {
		sprintf(serdes_lane, SERDES_LINE_NAME_FMT, i);
		ret = fdt_appendprop_string(blob, nodeoff, "phy-names",
					    serdes_lane);
		if (ret) {
			pr_err("Failed to append serdes lane to 'phy-names': %s\n",
			       fdt_strerror(ret));
			return ret;
		}

		ret = fdt_appendprop_u32(blob, nodeoff, "phys", phandle);
		if (ret) {
			pr_err("Failed to append serdes phandle to 'phys': %s\n",
			       fdt_strerror(ret));
			return ret;
		}

		ret = fdt_appendprop_u32(blob, nodeoff, "phys", PHY_TYPE_PCIE);
		if (ret) {
			pr_err("Failed to append PHY type to 'phys': %s\n",
			       fdt_strerror(ret));
			return ret;
		}

		ret = fdt_appendprop_u32(blob, nodeoff, "phys", id);
		if (ret) {
			pr_err("Failed to append PCIE instance to 'phys': %s\n",
			       fdt_strerror(ret));
			return ret;
		}

		ret = fdt_appendprop_u32(blob, nodeoff, "phys", i);
		if (ret) {
			pr_err("Failed to append SerDes line to 'phys': %s\n",
			       fdt_strerror(ret));
			return ret;
		}
	}

	return 0;
}

static int set_serdes_lines(void *blob, int id)
{
	enum serdes_mode mode;
	u32 phandle;
	int serdes_off, ret, lanes = 0;

	mode = s32_serdes_get_op_mode_from_hwconfig(id);
	if (mode == SERDES_MODE_PCIE_PCIE)
		lanes = 2;

	if (mode == SERDES_MODE_PCIE_SGMII0 || mode == SERDES_MODE_PCIE_SGMII1)
		lanes = 1;

	if (!lanes) {
		pr_err("Invalid PCIe%d lanes config\n", id);
		return -EINVAL;
	}

	serdes_off = fdt_alias2node(blob, SERDES_ALIAS_FMT, id);
	if (serdes_off < 0)
		return serdes_off;

	phandle = fdt_get_phandle(blob, serdes_off);
	if (!phandle) {
		ret = fdt_generate_phandle(blob, &phandle);
		if (ret < 0) {
			pr_err("Failed to generate a new phandle for %s%d\n",
			       SERDES_ALIAS_FMT, id);
			return ret;
		}

		ret = fdtdec_set_phandle(blob, serdes_off, phandle);
		if (ret < 0) {
			pr_err("Failed to set phandle for node: %s%d\n",
			       SERDES_ALIAS_FMT, id);
			return ret;
		}
	}

	ret = add_serdes_lines(blob, id, lanes, phandle);
	if (ret)
		return ret;

	return 0;
}

static int prepare_pcie_node(void *blob, int id)
{
	int ret, nodeoff;

	nodeoff = fdt_alias2node(blob, PCIE_ALIAS_FMT, id);
	if (nodeoff < 0)
		return nodeoff;

	if (is_pcie_enabled_in_hwconfig(id)) {
		ret = fdt_status_okay(blob, nodeoff);
		if (ret) {
			pr_err("Failed to enable PCIe%d\n", id);
			return ret;
		}
	} else {
		ret = fdt_status_disabled(blob, nodeoff);
		if (ret) {
			pr_err("Failed to disable PCIe%d\n", id);
			return ret;
		}

		/* Skip rest of the configuration if not enabled */
		return 0;
	}

	ret = set_pcie_mode(blob, nodeoff, id);
	if (ret)
		return ret;

	ret = set_pcie_phy_mode(blob, nodeoff, id);
	if (ret)
		return ret;

	ret = set_serdes_lines(blob, id);
	if (ret)
		return ret;

	return ret;
}

static int rename_ext_clk(void *blob, int nodeoff, int prop_pos)
{
	int i, ret, length, str_pos;
	const char *list;
	char *propval;

	list = fdt_getprop(blob, nodeoff, "clock-names", &length);
	if (!list)
		return -EINVAL;

	propval = malloc(length);
	memcpy(propval, list, length);

	/* Jump over elements before 'ext' clock */
	for (str_pos = 0, i = 0; i < prop_pos; i++)
		str_pos += strlen(&propval[str_pos]) + 1;

	propval[str_pos] = toupper(propval[str_pos]);

	ret = fdt_setprop(blob, nodeoff, "clock-names", propval,
			  length);
	if (ret) {
		pr_err("Failed to rename 'ext' SerDes clock: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	free(propval);

	return 0;
}

static int get_ext_clk_phandle(void *blob, int id, uint32_t *phandle)
{
	enum serdes_clock_fmhz mhz;
	char ext_clk_path[SERDES_EXT_PATH_SIZE];
	int clk_mhz, ext_off, ret;

	mhz = s32_serdes_get_clock_fmhz_from_hwconfig(id);
	if (mhz == CLK_100MHZ)
		clk_mhz = 100;
	else
		clk_mhz = 125;

	sprintf(ext_clk_path, SERDES_EXT_PATH_FMT, clk_mhz);

	ext_off = fdt_path_offset(blob, ext_clk_path);
	if (ext_off < 0) {
		pr_err("Failed to get offset of '%s' node\n", ext_clk_path);
		return ext_off;
	}

	*phandle = fdt_get_phandle(blob, ext_off);
	if (!*phandle) {
		ret = fdt_generate_phandle(blob, phandle);
		if (ret < 0) {
			pr_err("Failed to generate a new phandle for %s\n",
			       ext_clk_path);
			return ret;
		}

		ret = fdtdec_set_phandle(blob, ext_off, *phandle);
		if (ret < 0) {
			pr_err("Failed to set phandle for node: %s\n",
			       ext_clk_path);
			return ret;
		}
	}

	return 0;
}

static int add_ext_clk(void *blob, int id)
{
	u32 phandle;
	int ret, nodeoff;

	ret = get_ext_clk_phandle(blob, id, &phandle);
	if (ret)
		return ret;

	nodeoff = fdt_alias2node(blob, SERDES_ALIAS_FMT, id);
	if (nodeoff < 0)
		return nodeoff;

	ret = fdt_appendprop_string(blob, nodeoff, "clock-names",
				    SERDES_EXT_CLK);
	if (ret) {
		pr_err("Failed to append ext clock to 'clock-names'\n");
		return ret;
	}

	ret = fdt_appendprop_u32(blob, nodeoff, "clocks", phandle);
	if (ret) {
		pr_err("Failed to append ext clock to 'clock-names'\n");
		return ret;
	}

	return ret;
}

static int set_serdes_clk(void *blob, int id)
{
	enum serdes_clock clk = s32_serdes_get_clock_from_hwconfig(id);
	int nodeoff, prop_pos;

	nodeoff = fdt_alias2node(blob, SERDES_ALIAS_FMT, id);
	if (nodeoff < 0)
		return nodeoff;

	prop_pos = fdt_stringlist_search(blob, nodeoff, "clock-names",
					 SERDES_EXT_CLK);

	if (clk == CLK_INT && prop_pos >= 0)
		return rename_ext_clk(blob, nodeoff, prop_pos);

	if (clk == CLK_EXT && prop_pos <= 0)
		return add_ext_clk(blob, id);

	return 0;
}

static int set_serdes_mode(void *blob, int id)
{
	int nodeoff, ret;
	enum serdes_mode mode;

	mode = s32_serdes_get_op_mode_from_hwconfig(id);
	if (mode == SERDES_MODE_INVAL) {
		pr_err("Invalid SerDes%d mode\n", id);
		return -EINVAL;
	}

	nodeoff = fdt_alias2node(blob, SERDES_ALIAS_FMT, id);
	if (nodeoff < 0)
		return nodeoff;

	ret = fdt_setprop_u32(blob, nodeoff, "fsl,sys-mode", mode);
	if (ret)
		pr_err("Failed to set 'fsl,sys-mode'\n");

	return ret;
}

static void ft_fixup_serdes(void *blob)
{
	int ret, id;

	/* Add some space for the following changes */
	ret = fdt_increase_size(blob, 512);
	if (ret < 0) {
		pr_err("Could not increase size of device tree: %s\n",
		       fdt_strerror(ret));
		return;
	}

	for (id = 0; id <= 1; id++) {
		ret = prepare_pcie_node(blob, id);
		if (ret)
			pr_err("Failed to set mode for PCIe%d\n", id);

		ret = set_serdes_clk(blob, id);
		if (ret)
			pr_err("Failed to set the clock for SerDes%d\n", id);

		ret = set_serdes_mode(blob, id);
		if (ret)
			pr_err("Failed to set mode for SerDes%d\n", id);
	}
}
#endif

void ft_cpu_setup(void *blob, bd_t *bd)
{
#ifdef CONFIG_PCIE_S32GEN1
	ft_fixup_serdes(blob);
#endif
}

int ft_system_setup(void *blob, bd_t *bd)
{
	int ret;

	/*
	 * Skip these fixups when reusing U-Boot dtb for Linux
	 * as they don't make sense.
	 *
	 * This block should be removed once the bindings and the dtbs
	 * used by Linux and U-Boot are fully compatible.
	 */
	if (IS_ENABLED(CONFIG_DISTRO_DEFAULTS)) {
		printf("Skipping %s ...\n", __func__);
		return 0;
	}

	ret = ft_fixup_cpu(blob);
	if (ret)
		goto exit;

	ret = ft_fixup_memory(blob, bd);
	if (ret)
		goto exit;

	ret = ft_fixup_atf(gd->fdt_blob, blob);

exit:
	return ret;
}
