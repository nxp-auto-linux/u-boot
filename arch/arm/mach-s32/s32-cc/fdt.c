// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017,2019-2024 NXP
 */

#include <common.h>
#include <env.h>
#include <fdt_support.h>
#include <misc.h>
#include <phy.h>
#include <soc.h>
#include <asm/global_data.h>
#include <dm/uclass.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/libfdt.h>
#include <s32-cc/dts_fixups_utils.h>
#include <s32-cc/fdt_wrapper.h>
#include <s32-cc/s32cc_soc.h>
#include <s32/fdt.h>

#define S32_DDR_LIMIT_VAR	"ddr_limitX"
#define FDT_CLUSTER1_PATH	"/cpus/cpu-map/cluster1"

#define SOC_CPUMASK_S32G2			GENMASK(3, 0)
#define SOC_CPUMASK_S32G2_DERIVATIVE		(BIT(0) | BIT(2))
#define SOC_CPUMASK_S32G3			GENMASK(7, 0)
#define SOC_CPUMASK_S32G37X_DERIVATIVE		(GENMASK(5, 4) | GENMASK(1, 0))
#define SOC_CPUMASK_S32G35X_DERIVATIVE		(BIT(4) | BIT(0))
#define SOC_CPUMASK_S32R			GENMASK(3, 0)
#define SOC_MAX_CORES_PER_CLUSTER_S32G2		2
#define SOC_MAX_CORES_PER_CLUSTER_S32G3		4
#define SOC_MAX_CORES_PER_CLUSTER_S32R		2

#define S32CC_MAX_NVMEM_CELLS_PER_NODE		0x10

static const char *s32cc_gpio_compatible = "nxp,s32cc-siul2-gpio";
static const char *scmi_gpio_node_path = "/firmware/scmi/protocol@81";
static const char *scmi_nvmem_node_path = "/firmware/scmi/protocol@82";
static const char *s32g_pfe_compatible = "nxp,s32g-pfe-netif";

struct s32cc_soc_cores_info {
	u32 max_cores_per_cluster;
	u32 cpu_mask;
};

static const struct soc_attr s32cc_soc_cores_info_data[] = {
	{
		.machine = SOC_MACHINE_S32G233A,
		.data = &(struct s32cc_soc_cores_info) {
			.max_cores_per_cluster = SOC_MAX_CORES_PER_CLUSTER_S32G2,
			.cpu_mask = SOC_CPUMASK_S32G2_DERIVATIVE,
		},
	},
	{
		.machine = SOC_MACHINE_S32G254A,
		.data = &(struct s32cc_soc_cores_info) {
			.max_cores_per_cluster = SOC_MAX_CORES_PER_CLUSTER_S32G2,
			.cpu_mask = SOC_CPUMASK_S32G2_DERIVATIVE,
		},
	},
	{
		.machine = SOC_MACHINE_S32G274A,
		.data = &(struct s32cc_soc_cores_info) {
			.max_cores_per_cluster = SOC_MAX_CORES_PER_CLUSTER_S32G2,
			.cpu_mask = SOC_CPUMASK_S32G2,
		},
	},
	{
		.machine = SOC_MACHINE_S32G358A,
		.data = &(struct s32cc_soc_cores_info) {
			.max_cores_per_cluster = SOC_MAX_CORES_PER_CLUSTER_S32G3,
			.cpu_mask = SOC_CPUMASK_S32G35X_DERIVATIVE,
		},
	},
	{
		.machine = SOC_MACHINE_S32G359A,
		.data = &(struct s32cc_soc_cores_info) {
			.max_cores_per_cluster = SOC_MAX_CORES_PER_CLUSTER_S32G3,
			.cpu_mask = SOC_CPUMASK_S32G35X_DERIVATIVE,
		},
	},
	{
		.machine = SOC_MACHINE_S32G378A,
		.data = &(struct s32cc_soc_cores_info) {
			.max_cores_per_cluster = SOC_MAX_CORES_PER_CLUSTER_S32G3,
			.cpu_mask = SOC_CPUMASK_S32G37X_DERIVATIVE,
		},
	},
	{
		.machine = SOC_MACHINE_S32G379A,
		.data = &(struct s32cc_soc_cores_info) {
			.max_cores_per_cluster = SOC_MAX_CORES_PER_CLUSTER_S32G3,
			.cpu_mask = SOC_CPUMASK_S32G37X_DERIVATIVE,
		},
	},
	{
		.machine = SOC_MACHINE_S32G398A,
		.data = &(struct s32cc_soc_cores_info) {
			.max_cores_per_cluster = SOC_MAX_CORES_PER_CLUSTER_S32G3,
			.cpu_mask = SOC_CPUMASK_S32G3,
		},
	},
	{
		.machine = SOC_MACHINE_S32G399A,
		.data = &(struct s32cc_soc_cores_info) {
			.max_cores_per_cluster = SOC_MAX_CORES_PER_CLUSTER_S32G3,
			.cpu_mask = SOC_CPUMASK_S32G3,
		},
	},
	{
		.machine = SOC_MACHINE_S32R455A,
		.data = &(struct s32cc_soc_cores_info) {
			.max_cores_per_cluster = SOC_MAX_CORES_PER_CLUSTER_S32R,
			.cpu_mask = SOC_CPUMASK_S32R,
		},
	},
	{
		.machine = SOC_MACHINE_S32R458A,
		.data = &(struct s32cc_soc_cores_info) {
			.max_cores_per_cluster = SOC_MAX_CORES_PER_CLUSTER_S32R,
			.cpu_mask = SOC_CPUMASK_S32R,
		},
	},
	{ /* sentinel */ }
};

static int get_core_id(u32 core_mpidr, u32 max_cores_per_cluster)
{
	u32 cluster_id = (core_mpidr >> 8) & 0xFF;

	return (core_mpidr & 0xf) + cluster_id * max_cores_per_cluster;
}

static int get_cores_info(u32 *max_cores_per_cluster,
			  u32 *cpu_mask)
{
	const struct soc_attr *soc_match_data;
	const struct s32cc_soc_cores_info *s32cc_match_data;

	soc_match_data = soc_device_match(s32cc_soc_cores_info_data);
	if (!soc_match_data)
		return -EINVAL;

	s32cc_match_data = (struct s32cc_soc_cores_info *)soc_match_data->data;
	*max_cores_per_cluster = s32cc_match_data->max_cores_per_cluster;
	*cpu_mask = s32cc_match_data->cpu_mask;

	debug("%s: max_cores_per_cluster = %u, cpu_mask = 0x%x\n",
	      __func__, *max_cores_per_cluster, *cpu_mask);

	return 0;
}

static bool is_lockstep_enabled(void)
{
	struct udevice *soc;
	struct soc_s32cc_plat soc_data;
	int ret;

	ret = soc_get(&soc);
	if (ret) {
		printf(":%s: Failed to get SoC (err = %d)\n", __func__, ret);
		return ret;
	}

	ret = soc_get_platform_data(soc, &soc_data, sizeof(soc_data));
	if (ret) {
		printf(":%s: Failed to get SoC platform data (err = %d)\n",
		       __func__, ret);
		return ret;
	}

	return soc_data.lockstep_enabled;
}

static int ft_fixup_cpu(void *blob)
{
	int ret, addr_cells = 0;
	u32 max_cores_per_cluster = 0;
	u32 cpu_mask = 0;
	u64 core_mpidr, core_id;
	fdt32_t *reg;
	int off, off_prev, cluster1_off;
	bool lockstep_enabled;

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

	lockstep_enabled = is_lockstep_enabled();
	if (lockstep_enabled) {
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

	if (!lockstep_enabled)
		return 0;

	cluster1_off = fdt_path_offset(blob, FDT_CLUSTER1_PATH);
	if (cluster1_off < 0) {
		printf("couldn't find %s node\n", FDT_CLUSTER1_PATH);
		return -ENODEV;
	}

	return fdt_del_node(blob, cluster1_off);
}

static int disable_node_by_compatible(void *blob, const char *compatible,
				      uint32_t *phandle)
{
	const char *node_name;
	int nodeoff, ret;

	nodeoff = fdt_node_offset_by_compatible(blob, -1, compatible);
	if (nodeoff < 0) {
		pr_err("Failed to get a node based on compatible string '%s' (%s)\n",
		       compatible, fdt_strerror(nodeoff));
		return nodeoff;
	}

	node_name = fdt_get_name(blob, nodeoff, NULL);
	ret = fdt_set_node_status(blob, nodeoff, FDT_STATUS_DISABLED);
	if (ret) {
		pr_err("Failed to disable '%s' node\n", node_name);
		return ret;
	}

	*phandle = fdt_get_phandle(blob, nodeoff);
	if (!*phandle) {
		pr_warn("The node '%s' is not referenced by any other nodes.",
			node_name);
		return 0;
	}

	ret = fdt_delprop(blob, nodeoff, "phandle");
	if (ret) {
		pr_err("Failed to remove phandle property of '%s' node: %s\n",
		       node_name, fdt_strerror(ret));
		return ret;
	}

	return 0;
}

static int enable_scmi_protocol(void *blob, const char *path, uint32_t phandle)
{
	int nodeoff, ret;

	nodeoff = fdt_path_offset(blob, path);
	if (nodeoff < 0) {
		pr_err("Failed to get offset of '%s' node\n", path);
		return nodeoff;
	}

	if (phandle) {
		ret = fdt_set_phandle(blob, nodeoff, phandle);
		if (ret) {
			pr_err("Failed to set phandle property of '%s' node\n", path);
			return ret;
		}
	}

	ret = fdt_set_node_status(blob, nodeoff, FDT_STATUS_OKAY);
	if (ret) {
		pr_err("Failed to enable '%s' node\n", path);
		return ret;
	}

	return 0;
}

static int enable_scmi_gpio_node(void *blob, uint32_t phandle)
{
	return enable_scmi_protocol(blob, scmi_gpio_node_path, phandle);
}

static int enable_scmi_nvmem_node(void *blob, uint32_t phandle)
{
	return enable_scmi_protocol(blob, scmi_nvmem_node_path, phandle);
}

static int disable_siul2_gpio_node(void *blob, uint32_t *phandle)
{
	return disable_node_by_compatible(blob, s32cc_gpio_compatible,
					  phandle);
}

static int enable_scmi_gpio(void *blob)
{
	ofnode node;
	u32 phandle;
	int ret;

	node = ofnode_path(scmi_gpio_node_path);
	if (!ofnode_valid(node)) {
		printf("%s: Couldn't find \"%s\" node\n", __func__,
		       scmi_gpio_node_path);
		return -EINVAL;
	}

	/* SCMI GPIO isn't enabled for U-Boot */
	if (!ofnode_is_available(node))
		return 0;

	node = ofnode_by_compatible(ofnode_null(), s32cc_gpio_compatible);
	if (!ofnode_valid(node)) {
		printf("%s: Couldn't find \"%s\" node\n", __func__,
		       s32cc_gpio_compatible);
		return -EINVAL;
	}

	/* Skip if default GPIO node is used */
	if (ofnode_is_available(node))
		return 0;

	ret = disable_siul2_gpio_node(blob, &phandle);
	if (ret)
		return ret;

	ret = enable_scmi_gpio_node(blob, phandle);
	if (ret)
		return ret;

	return 0;
}

static int fdt_node_offset_by_prop_found(const void *fdt, int startoffset,
					 const char *propname)
{
	int offset;
	const void *val;
	int len;

	for (offset = fdt_next_node(fdt, startoffset, NULL);
	     offset >= 0;
	     offset = fdt_next_node(fdt, offset, NULL)) {
		val = fdt_getprop(fdt, offset, propname, &len);
		if (val)
			return offset;
	}

	return offset; /* error from fdt_next_node() */
}

static int find_nvmem_scmi_node(void *blob, int *nodeoff,
				const u32 **phandles)
{
	int scmi_nvmem_nodeoff;
	const u32 *scmi_nvmem_phandles;

	scmi_nvmem_nodeoff = fdt_path_offset(blob, scmi_nvmem_node_path);
	if (scmi_nvmem_nodeoff < 0) {
		pr_err("Failed to get NVMEM SCMI node with path '%s' (%s)\n",
		       scmi_nvmem_node_path, fdt_strerror(scmi_nvmem_nodeoff));
		return scmi_nvmem_nodeoff;
	}

	scmi_nvmem_phandles = fdt_getprop(blob, scmi_nvmem_nodeoff,
					  "nvmem-cells", NULL);
	if (!scmi_nvmem_phandles) {
		pr_err("Failed to get 'nvmem-cells' property of '%s' node\n",
		       scmi_nvmem_node_path);
		return -FDT_ERR_NOTFOUND;
	}

	*nodeoff = scmi_nvmem_nodeoff;
	*phandles = scmi_nvmem_phandles;

	return 0;
}

static int find_nvmem_consumer_node(void *blob, int nodeoff_scmi, int *nodeoff,
				    int *num_phandles)
{
	int count;
	int startoffset = *nodeoff;

	*nodeoff = fdt_node_offset_by_prop_found(blob, startoffset,
						 "nvmem-cells");
	/* Skip the NVMEM SCMI node */
	if (*nodeoff == nodeoff_scmi)
		*nodeoff = fdt_node_offset_by_prop_found(blob, *nodeoff,
							 "nvmem-cells");
	if (*nodeoff < 0) {
		if (startoffset == 0)
			pr_err("Failed to get at least 1 node with 'nvmem-cells' property (%s)\n",
			       fdt_strerror(*nodeoff));
		return -FDT_ERR_NOTFOUND;
	}

	/* Count string values in "nvmem-cell-names" property */
	count = fdt_stringlist_count(blob, *nodeoff, "nvmem-cell-names");
	if (count < 0) {
		pr_err("Failed to get 'nvmem-cell-names' property of node '%s' (%s)\n",
		       fdt_get_name(blob, *nodeoff, NULL), fdt_strerror(count));
		return count;
	}
	if (count == 0) {
		pr_err("Empty 'nvmem-cell-names' property of node '%s'\n",
		       fdt_get_name(blob, *nodeoff, NULL));
		return -FDT_ERR_NOTFOUND;
	}

	*num_phandles = count;

	return 0;
}

static int update_nvmem_consumer_phandles(void *blob, int nodeoff_consumer,
					  int num_phandles, int nodeoff_scmi,
					  const u32 *phandles_scmi)
{
	int ret, i, idx;
	const char *cell_name;
	static u32 new_phandles[S32CC_MAX_NVMEM_CELLS_PER_NODE];
	const u32 *old_phandles;

	old_phandles = (u32 *)fdt_getprop(blob, nodeoff_consumer, "nvmem-cells",
					  NULL);

	for (i = 0; i < num_phandles; i++) {
		/* Get nvmem cell name which corresponds to a phandle */
		cell_name = fdt_stringlist_get(blob, nodeoff_consumer,
					       "nvmem-cell-names", i, &ret);
		if (!cell_name) {
			pr_err("Failed to get cell name at index %d in node '%s' (%s)\n",
			       i, fdt_get_name(blob, nodeoff_consumer, NULL),
			       fdt_strerror(ret));
			return ret;
		}

		/* Find idx of 'cell_name' in nvmem node */
		idx = fdt_stringlist_search(blob, nodeoff_scmi,
					    "nvmem-cell-names", cell_name);
		if (idx < 0) {
			pr_warn("Cell '%s' not provided by '%s' node. Skipping.\n",
				cell_name, scmi_nvmem_node_path);
			/* No corresponding SCMI NVMEM cell, keep old cell */
			new_phandles[i] = old_phandles[i];
			continue;
		}

		/* Store new phandles to be written all at once */
		new_phandles[i] = phandles_scmi[idx];
	}

	ret = fdt_setprop_inplace(blob, nodeoff_consumer, "nvmem-cells",
				  new_phandles, num_phandles * sizeof(u32));
	if (ret) {
		pr_err("Failed to update 'nvmem-cells' property of '%s' (%s)\n",
		       fdt_get_name(blob, nodeoff_consumer, NULL),
		       fdt_strerror(ret));
		return ret;
	}

	return 0;
}

static int ft_fixup_scmi_nvmem(void *blob)
{
	int ret, nodeoff_scmi, nodeoff_consumer, num_phandles;
	const u32 *phandles_scmi;

	ret = find_nvmem_scmi_node(blob, &nodeoff_scmi, &phandles_scmi);
	if (ret)
		return ret;

	/*
	 * Find all nodes with "nvmem-cells" property, except the
	 * SCMI NVMEM node
	 */
	nodeoff_consumer = 0;
	while (!(ret = find_nvmem_consumer_node(blob, nodeoff_scmi,
						&nodeoff_consumer,
						&num_phandles))) {
		/* Update node NVMEM phandles to point to NVMEM SCMI cells */
		ret = update_nvmem_consumer_phandles(blob, nodeoff_consumer,
						     num_phandles, nodeoff_scmi,
						     phandles_scmi);
		if (ret)
			return ret;
	}

	if (ret && nodeoff_consumer >= 0) {
		pr_err("Malformed node '%s' (%s)\n",
		       fdt_get_name(blob, nodeoff_consumer, NULL),
		       fdt_strerror(ret));
		return -EINVAL;
	}

	return enable_scmi_nvmem_node(blob, 0);
}

static bool apply_pfe_sgmii_phy_fixup(void *fdt, int nodeoff, u32 phy_addr)
{
	const char *phy_mode, *sgmii_mode;
	int off, len = 0;
	const u32 *php;
	int err;

	phy_mode = fdt_getprop(fdt, nodeoff, "phy-mode", &len);
	if (!phy_mode || !len)
		return false;

	sgmii_mode = phy_string_for_interface(PHY_INTERFACE_MODE_SGMII);
	if (strncmp(phy_mode, sgmii_mode, strlen(sgmii_mode)))
		return false;

	php = fdt_getprop(fdt, nodeoff, "phy-handle", &len);
	if (!php || len != sizeof(*php))
		return false;

	off = fdt_node_offset_by_phandle(fdt, fdt32_to_cpu(*php));
	if (off < 0)
		return false;

	err = fdt_setprop_u32(fdt, off, "reg", phy_addr);
	if (err) {
		pr_err("failed to apply PFE SGMII PHY addr fixup (0x%x)\n",
		       phy_addr);
		return false;
	}

	return true;
}

static void ft_fixup_enet_pfe(void *fdt)
{
	char phy_envname[32], *phy_addr_str;
	const char *ifname;
	int i, nlen = 0;
	ulong phy_addr;
	char *ep = NULL;
	int ret;

	fdt_for_each_node_by_compatible(i, fdt, -1, s32g_pfe_compatible) {
		ifname = fdt_getprop(fdt, i, "nxp,pfeng-if-name", &nlen);
		if (!ifname || !nlen)
			continue;

		ret = snprintf(phy_envname, sizeof(phy_envname), "%s_phy_addr",
			       ifname);
		if (ret <= 0 || ret >= sizeof(phy_envname)) {
			pr_err("fixup: %s: env var name parsing failed (%s)",
			       ifname, phy_envname);
			continue;
		}

		phy_addr_str = env_get(phy_envname);
		if (!phy_addr_str)
			continue;

		phy_addr = simple_strtoul(phy_addr_str, &ep, 16);
		if (phy_addr >= PHY_MAX_ADDR || *ep != '\0') {
			pr_err("fixup: %s: invalid PHY fixup address 0x%lx\n",
			       ifname, phy_addr);
			continue;
		}

		if (apply_pfe_sgmii_phy_fixup(fdt, i, phy_addr))
			printf("   fixup: %s: update phy addr to 0x%lx\n",
			       ifname, phy_addr);
	}
}

static void ft_fixup_stdout_path(void *blob)
{
	const char *stdout_path_prop = "stdout-path";
	const char *chosen_node = "/chosen";
	int offset, ret;

	offset = fdt_path_offset(blob, chosen_node);
	if (offset == -FDT_ERR_NOTFOUND) {
		pr_err("Failed to remove '%s' prop: '%s' node not found\n",
		       stdout_path_prop, chosen_node);
		return;
	}

	ret = fdt_delprop(blob, offset, stdout_path_prop);
	if (ret) {
		pr_err("Failed to remove '%s' prop: %s\n", stdout_path_prop,
		       fdt_strerror(ret));
		return;
	}
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	int ret;

	/* Add some space for the following changes */
	ret = fdt_increase_size(blob, 512);
	if (ret < 0) {
		pr_err("Could not increase size of device tree: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

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

	ret = ft_fixup_atf(blob);
	if (ret)
		goto exit;

	if (CONFIG_IS_ENABLED(NXP_PFENG))
		ft_fixup_enet_pfe(blob);

	ret = apply_fdt_hwconfig_fixups(blob);
	if (ret)
		goto exit;

	if (CONFIG_IS_ENABLED(S32CC_SCMI_GPIO_FIXUP)) {
		ret = enable_scmi_gpio(blob);
		if (ret)
			return ret;
	}

	if (CONFIG_IS_ENABLED(S32CC_SCMI_NVMEM_FIXUP)) {
		ret = ft_fixup_scmi_nvmem(blob);
		if (ret)
			return ret;
	}

	if (CONFIG_IS_ENABLED(XEN_SUPPORT) &&
	    CONFIG_IS_ENABLED(OF_STDOUT_VIA_ALIAS))
		ft_fixup_stdout_path(blob);

exit:
	return ret;
}

int board_fix_fdt(void *rw_fdt_blob)
{
	return ft_fixup_cpu(rw_fdt_blob);
}
