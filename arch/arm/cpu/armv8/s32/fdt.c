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
#include <hwconfig.h>
#include <asm/arch-s32/s32-gen1/serdes_hwconfig.h>
#include <dt-bindings/phy/phy.h>
#include <linux/ctype.h>
#include "mp.h"
#ifdef CONFIG_SYS_ERRATUM_ERR050543
#include <ddr_s32gen1_err050543.h>
#endif

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
		if (!test_bit(fdt_to_cpu_id(core_id), &mask)) {
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

static void ft_fixup_memory(void *blob, bd_t *bd)
{
	hide_sram(bd);

	apply_ddr_limits(bd);

	apply_memory_fixups(blob, bd);

}

#ifdef CONFIG_S32_ATF_BOOT_FLOW
static int disable_node_alias(void *blob, const char *alias, uint32_t *phandle)
{
	const char *alias_path;
	int nodeoff, ret;

	alias_path = fdt_get_alias(blob, alias);
	if (!alias_path) {
		pr_err("Failed to get path of '%s' alias\n", alias);
		return -EIO;
	}

	nodeoff = fdt_path_offset(blob, alias_path);
	if (nodeoff < 0) {
		pr_err("Failed to get offset of '%s' node\n", alias_path);
		return nodeoff;
	}

	*phandle = fdt_get_phandle(blob, nodeoff);
	if (*phandle < 0) {
		pr_err("Failed to get phandle of '%s' node\n", alias_path);
		return *phandle;
	}

	ret = fdt_set_node_status(blob, nodeoff, FDT_STATUS_DISABLED, 0);
	if (ret) {
		pr_err("Failed to disable '%s' node\n", alias_path);
		return ret;
	}

	ret = fdt_delprop(blob, nodeoff, "phandle");
	if (ret) {
		pr_err("Failed to remove phandle property of '%s' node: %s\n",
		       alias_path, fdt_strerror(ret));
		return ret;
	}

	return 0;
}

static int disable_clk_node(void *blob, uint32_t *phandle)
{
	return disable_node_alias(blob, "clks", phandle);
}

static int disable_reset_node(void *blob, uint32_t *phandle)
{
	return disable_node_alias(blob, "reset", phandle);
}

static int enable_scmi_protocol(void *blob, const char *path, uint32_t phandle)
{
	int nodeoff, ret;

	nodeoff = fdt_path_offset(blob, path);
	if (nodeoff < 0) {
		pr_err("Failed to get offset of '%s' node\n", path);
		return nodeoff;
	}

	ret = fdt_set_phandle(blob, nodeoff, phandle);
	if (ret) {
		pr_err("Failed to set phandle property of '%s' node\n", path);
		return ret;
	}

	return 0;
}

static int enable_scmi_clk_node(void *blob, uint32_t phandle)
{
	const char *path = "/firmware/scmi/protocol@14";

	return enable_scmi_protocol(blob, path, phandle);
}

static int enable_scmi_reset_node(void *blob, uint32_t phandle)
{
	const char *path = "/firmware/scmi/protocol@16";

	return enable_scmi_protocol(blob, path, phandle);
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

static int enable_scmi_clks(void *blob)
{
	u32 phandle;
	int ret;

	ret = disable_clk_node(blob, &phandle);
	if (ret)
		return ret;

	ret = enable_scmi_clk_node(blob, phandle);
	if (ret)
		return ret;

	return 0;
}

static int enable_scmi_reset(void *blob)
{
	u32 phandle;
	int ret;

	ret = disable_reset_node(blob, &phandle);
	if (ret)
		return ret;

	ret = enable_scmi_reset_node(blob, phandle);
	if (ret)
		return ret;

	return 0;
}

static void ft_fixup_scmi(void *blob)
{
	if (enable_scmi_clks(blob))
		return;

	if (enable_scmi_reset(blob))
		return;

	/* As of Linux Kernel version 5.10, the 'arm,smc-mbox'
	 * dts node is no longer used
	 */
	if (enable_scmi_mbox(blob) == -ENODEV)
		if (enable_scmi_smc(blob))
			pr_err("Failed to enable 'arm,smc-mbox' or 'arm,scmi-smc' node\n");
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

static void ft_fixup_atf(const void *old_blob, void *new_blob)
{
	if (add_atf_reserved_memory(old_blob, new_blob))
		pr_err("Copying 'atf' node from U-Boot DT to Linux DT failed!\n");
}
#endif

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
		compatible = "fsl,s32gen1-pcie-ep";
	else
		compatible = "fsl,s32gen1-pcie";

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
#ifdef CONFIG_MP
	ft_fixup_cpu(blob);
#endif

	ft_fixup_memory(blob, bd);
#ifdef CONFIG_SYS_ERRATUM_ERR050543
	ft_fixup_ddr_polling(blob);
#endif
#ifdef CONFIG_S32_ATF_BOOT_FLOW
	ft_fixup_scmi(blob);
	ft_fixup_atf(gd->fdt_blob, blob);
#endif
#ifdef CONFIG_PCIE_S32GEN1
	ft_fixup_serdes(blob);
#endif
}
