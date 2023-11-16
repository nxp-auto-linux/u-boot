// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022-2023 NXP
 */
#include <common.h>
#include <fdt_support.h>
#include <malloc.h>
#include <dm/device.h>
#include <dm/of_access.h>
#include <dm/ofnode.h>
#include <linux/ctype.h>
#include <s32-cc/serdes_hwconfig.h>
#include <dt-bindings/phy/phy.h>

#define PCIE_ALIAS_FMT			"pci%u"
#define PCIE_ALIAS_SIZE			sizeof(PCIE_ALIAS_FMT)

#define SERDES_ALIAS_FMT		"serdes%u"
#define SERDES_ALIAS_SIZE		sizeof(SERDES_ALIAS_FMT)

#define SERDES_EXT_PATH_FMT		"/clocks/serdes_%u_ext"
#define SERDES_EXT_PATH_FMT_SIZE	sizeof(SERDES_EXT_PATH_FMT)
/* Add some space for SerDes ID */
#define SERDES_EXT_PATH_SIZE		(SERDES_EXT_PATH_FMT_SIZE + 2)

#define SERDES_EXT_CLK			"ext"
#define SERDES_EXT_SIZE			sizeof(SERDES_EXT_CLK)

#define SERDES_LINE_NAME_FMT		"serdes_lane%u"
#define SERDES_LINE_NAME_LEN		sizeof(SERDES_LINE_NAME_FMT)

#define MAX_PATH_SIZE			100

struct dts_node {
	union {
		struct {
			void *blob;
			int off;
		};
		ofnode node;
	};
	/* Selects between external device tree and internal ofnode */
	bool fdt;
};

static int fdt_alias2node(void *blob, const char *alias_fmt,
			  unsigned int alias_id)
{
	const char *alias_path;
	char alias_name[MAX_PATH_SIZE];
	int nodeoff, ret;

	ret = sprintf(alias_name, alias_fmt, alias_id);
	if (ret < 0)
		return ret;

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

static ofnode ofnode_by_alias(const char *alias_fmt, u32 alias_id)
{
	char alias_name[MAX_PATH_SIZE];
	int ret;

	ret = sprintf(alias_name, alias_fmt, alias_id);
	if (ret < 0)
		return ofnode_null();

	return ofnode_path(alias_name);
}

static unsigned int get_max_phandle(void)
{
	struct device_node *np;
	unsigned int phandle = 0;

	for_each_of_allnodes(np) {
		phandle = max(np->phandle, phandle);
	}

	return phandle;
}

static int ofnode_write_prop_u32(ofnode node, const char *prop, u32 val)
{
	__be32 *prop_val = malloc(sizeof(*prop_val));

	if (!prop_val)
		return -ENOMEM;

	*prop_val = cpu_to_be32p(&val);
	return ofnode_write_prop(node, prop, sizeof(*prop_val), prop_val);
}

static int ofnode_append_prop(ofnode node, const char *prop,
			      size_t len, const void *val)
{
	const void *old_val;
	void *new_val;
	int old_len = 0;

	if (len > INT_MAX)
		return -EINVAL;

	old_val = ofnode_get_property(node, prop, &old_len);

	/* Check if new property */
	if (!old_val || old_len < 0) {
		new_val = malloc(len);
		if (!new_val)
			return -ENOMEM;

		memcpy(new_val, val, len);
		return ofnode_write_prop(node, prop, len, new_val);
	}

	if (old_len + len < len || old_len + len < old_len)
		return -EINVAL;

	new_val = malloc(old_len + len);
	if (!new_val)
		return -ENOMEM;

	memcpy(new_val, old_val, old_len);
	memcpy(new_val + old_len, val, len);

	return ofnode_write_prop(node, prop, old_len + len, new_val);
}

static int ofnode_append_prop_u32(ofnode node, const char *prop,
				  u32 val)
{
	__be32 be32val = cpu_to_be32p(&val);

	return ofnode_append_prop(node, prop, sizeof(be32val), &be32val);
}

static int ofnode_append_prop_str(ofnode node, const char *prop,
				  const char *val)
{
	size_t size = strlen(val);

	if (check_size_overflow(size, 1))
		return -1;

	return ofnode_append_prop(node, prop, size + 1, val);
}

static unsigned int ofnode_create_phandle(ofnode node)
{
	struct device_node *dn = (struct device_node *)ofnode_to_np(node);
	u32 phandle;
	int ret;

	if (!dn)
		return 0;

	phandle = dn->phandle;
	if (!phandle) {
		phandle = get_max_phandle();
		if (check_u32_overflow(phandle, 1)) {
			return 0;
		}
		phandle++;

		ret = ofnode_write_prop_u32(node, "phandle", phandle);
		if (ret)
			return 0;

		dn->phandle = phandle;
	}

	return phandle;
}

static int enable_ofnode_device(ofnode node)
{
	int ret;
	char buf[MAX_PATH_SIZE];

	ret = ofnode_get_path(node, &buf[0], ARRAY_SIZE(buf));
	if (ret) {
		pr_err("Failed to get the path of the node '%s'\n",
		       ofnode_get_name(node));
		return ret;
	}

	/*
	 * Unbind to make sure the new compatible (if the case) has effect.
	 * Otherwise the driver will be probed using the original settings
	 * instead of using the updated version.
	 *
	 * Unbinding might fail if the node is already disabled
	 */
	dev_disable_by_path(buf);

	ret = dev_enable_by_path(buf);
	if (ret) {
		pr_err("Failed to (re)enable '%s'\n", buf);
		return ret;
	}

	return 0;
}

static int disable_ofnode_device(ofnode node)
{
	int ret;
	char buf[MAX_PATH_SIZE];

	ret = ofnode_get_path(node, &buf[0], ARRAY_SIZE(buf));
	if (ret) {
		pr_err("Failed to get the path of the node '%s'\n",
		       ofnode_get_name(node));
		return ret;
	}

	/* Unbinding might fail if the node is already disabled */
	dev_disable_by_path(buf);

	return 0;
}

static int node_by_alias(struct dts_node *root, struct dts_node *node,
			 const char *alias_fmt, unsigned int alias_id)
{
	*node = *root;

	if (root->fdt) {
		node->blob = root->blob;
		node->off = fdt_alias2node(node->blob, alias_fmt,
					   alias_id);
		if (node->off < 0)
			return node->off;

		return 0;
	}

	node->node = ofnode_by_alias(alias_fmt, alias_id);
	if (!ofnode_valid(node->node))
		return -ENOMEM;

	return 0;
}

static int enable_node(struct dts_node *node)
{
	if (node->fdt)
		return fdt_status_okay(node->blob, node->off);

	return enable_ofnode_device(node->node);
}

static int disable_node(struct dts_node *node)
{
	if (node->fdt)
		return fdt_status_disabled(node->blob, node->off);

	return disable_ofnode_device(node->node);
}

static int node_set_prop(struct dts_node *node, const char *prop, void *val,
			 int len)
{
	if (node->fdt)
		return fdt_setprop(node->blob, node->off, prop, val, len);

	return ofnode_write_prop(node->node, prop, len, val);
}

static int node_set_prop_str(struct dts_node *node, const char *prop,
			     const char *val)
{
	if (node->fdt)
		return fdt_setprop_string(node->blob, node->off, prop, val);

	return ofnode_write_string(node->node, prop, val);
}

static int node_set_prop_u32(struct dts_node *node, const char *prop,
			     u32 val)
{
	if (node->fdt)
		return fdt_setprop_u32(node->blob, node->off, prop, val);

	return ofnode_write_prop_u32(node->node, prop, val);
}

static int node_append_prop_str(struct dts_node *node, const char *prop,
				const char *val)
{
	if (node->fdt)
		return fdt_appendprop_string(node->blob, node->off, prop, val);

	return ofnode_append_prop_str(node->node, prop, val);
}

static int node_append_prop_u32(struct dts_node *node, const char *prop,
				u32 val)
{
	if (node->fdt)
		return fdt_appendprop_u32(node->blob, node->off, prop, val);

	return ofnode_append_prop_u32(node->node, prop, val);
}

static unsigned int node_create_phandle(struct dts_node *node)
{
	if (node->fdt)
		return fdt_create_phandle(node->blob, node->off);

	return ofnode_create_phandle(node->node);
}

static int node_stringlist_search(struct dts_node *node,
				  const char *property, const char *string)
{
	if (node->fdt)
		return fdt_stringlist_search(node->blob, node->off, property,
					     string);

	return ofnode_stringlist_search(node->node, property, string);
}

static const void *node_get_prop(struct dts_node *node, const char *prop,
				 int *len)
{
	if (node->fdt)
		return fdt_getprop(node->blob, node->off, prop, len);

	return ofnode_get_property(node->node, prop, len);
}

static int node_get_prop_u32(struct dts_node *node, const char *prop, u32 *val)
{
	const fdt32_t *cell;
	int len = 0;

	if (!node->fdt)
		return ofnode_read_u32(node->node, prop, val);

	cell = fdt_getprop(node->blob, node->off, prop, &len);
	if (!cell || len != sizeof(*cell))
		return -EINVAL;

	*val = fdt32_to_cpu(*cell);
	return 0;
}

static int node_by_path(struct dts_node *root, struct dts_node *node,
			const char *path)
{
	*node = *root;

	if (node->fdt) {
		node->off = fdt_path_offset(root->blob, path);
		if (node->off < 0)
			return node->off;
		return 0;
	}

	node->node = ofnode_path(path);
	if (!ofnode_valid(node->node))
		return -EINVAL;

	return 0;
}

static int set_pcie_mode(struct dts_node *node, unsigned int id)
{
	int ret;
	char *compatible = NULL;
	enum pcie_type pcie_mode;

	pcie_mode = s32_serdes_get_pcie_type_from_hwconfig(id);
	if (pcie_mode & PCIE_RC)
		compatible = PCIE_COMPATIBLE_RC;
	else if (pcie_mode & PCIE_EP)
		compatible = PCIE_COMPATIBLE_EP;

	if (compatible) {
		debug("PCIe%d: Set compatible to %s\n", id, compatible);
		ret = node_set_prop_str(node, "compatible", compatible);
		if (ret) {
			pr_err("Failed to set PCIE compatible: %s\n",
			       fdt_strerror(ret));
			return ret;
		}
	} else {
		pr_err("No RC/EP PCIe mode selected\n");
		return -EINVAL;
	}

	return 0;
}

static int set_pcie_phy_mode(struct dts_node *node, unsigned int id)
{
	int ret = 0;
	const char *mode;
	enum pcie_phy_mode phy_mode;

	phy_mode = s32_serdes_get_pcie_phy_mode_from_hwconfig(id);
	if (phy_mode == PCIE_PHY_MODE_INVALID) {
		pr_err("Invalid PCIe%u PHY mode", id);
		return -EINVAL;
	}

	switch (phy_mode) {
	case CRNS:
		mode = "crns";
		break;
	case CRSS:
		mode = "crss";
		break;
	case SRIS:
		mode = "sris";
		break;
	default:
		pr_err("PCIe PHY mode not supported\n");
		return -EINVAL;
	}

	ret = node_set_prop_str(node, "nxp,phy-mode", mode);
	if (ret)
		pr_err("Failed to set 'nxp,phy-mode'\n");

	return ret;
}

static int add_pcie_serdes_lines(struct dts_node *root, unsigned int id,
				 int lanes, u32 phandle)
{
	char serdes_lane[SERDES_LINE_NAME_LEN];
	struct dts_node node;
	u32 i;
	int ret;

	ret = node_by_alias(root, &node, PCIE_ALIAS_FMT, id);
	if (ret)
		return ret;

	ret = node_set_prop_u32(&node, "num-lanes", lanes);
	if (ret) {
		pr_err("Failed to set 'num-lanes'\n");
		return ret;
	}

	for (i = 0; i < lanes; i++) {
		ret = sprintf(serdes_lane, SERDES_LINE_NAME_FMT, i);
		if (ret < 0)
			return ret;

		ret = node_append_prop_str(&node, "phy-names", serdes_lane);
		if (ret) {
			pr_err("Failed to append serdes lane to 'phy-names': %s\n",
			       fdt_strerror(ret));
			return ret;
		}

		ret = node_append_prop_u32(&node, "phys", phandle);
		if (ret) {
			pr_err("Failed to append serdes phandle to 'phys': %s\n",
			       fdt_strerror(ret));
			return ret;
		}

		ret = node_append_prop_u32(&node, "phys", PHY_TYPE_PCIE);
		if (ret) {
			pr_err("Failed to append PHY type to 'phys': %s\n",
			       fdt_strerror(ret));
			return ret;
		}

		ret = node_append_prop_u32(&node, "phys", id);
		if (ret) {
			pr_err("Failed to append PCIE instance to 'phys': %s\n",
			       fdt_strerror(ret));
			return ret;
		}

		ret = node_append_prop_u32(&node, "phys", i);
		if (ret) {
			pr_err("Failed to append SerDes line to 'phys': %s\n",
			       fdt_strerror(ret));
			return ret;
		}
	}

	return 0;
}

static int set_pcie_serdes_lines(struct dts_node *node, unsigned int id)
{
	enum serdes_mode mode;
	u32 phandle, lanes = 0u;
	int ret;
	struct dts_node serdes, root = *node;

	mode = s32_serdes_get_serdes_mode_from_hwconfig(id);
	if (mode == SERDES_MODE_PCIE_PCIE)
		lanes = 2u;

	if (mode == SERDES_MODE_PCIE_XPCS0 || mode == SERDES_MODE_PCIE_XPCS1)
		lanes = 1u;

	if (!lanes) {
		pr_err("Invalid PCIe%u lanes config\n", id);
		return -EINVAL;
	}

	ret = node_by_alias(&root, &serdes, SERDES_ALIAS_FMT, id);
	if (ret) {
		pr_err("Failed to get 'serdes%u' alias\n", id);
		return ret;
	}

	phandle = node_create_phandle(&serdes);
	if (!phandle) {
		pr_err("Failed to create phandle for %s%u\n",
		       SERDES_ALIAS_FMT, id);
		return ret;
	}

	ret = add_pcie_serdes_lines(&root, id, lanes, phandle);
	if (ret)
		return ret;

	return 0;
}

static int skip_dts_node(struct dts_node *root, const char *alias_fmt,
			 unsigned int id)
{
	int ret;
	struct dts_node node;

	ret = node_by_alias(root, &node, alias_fmt, id);
	if (ret) {
		pr_err("Failed to get 'pcie%u' alias\n", id);
		return ret;
	}

	/**
	 * Nothing to be performed on Linux device tree as skip option has
	 * effect on U-Boot drivers only
	 */
	if (root->fdt)
		return 0;

	ret = disable_node(&node);
	if (ret) {
		pr_err("Failed to disable %s%u\n", alias_fmt, id);
		return ret;
	}

	return 0;
}

static int skip_dts_nodes_config(struct dts_node *root, unsigned int id,
				 bool *skip)
{
	int ret = -EINVAL;
	struct dts_node serdes;

	if (!skip || !root)
		return ret;

	ret = node_by_alias(root, &serdes, SERDES_ALIAS_FMT, id);
	if (ret) {
		pr_err("Failed to get 'serdes%u' alias\n", id);
		return ret;
	}

	/* Do not skip SerDes & PCIe fixup for Linux device tree */
	if (root->fdt)
		return 0;

	*skip = s32_serdes_get_skip_from_hwconfig(id);

	if (!*skip)
		return 0;

	printf("Skipping configuration for SerDes%u.\n", id);

	ret = skip_dts_node(root, PCIE_ALIAS_FMT, id);
	if (ret)
		return ret;

	ret = skip_dts_node(root, SERDES_ALIAS_FMT, id);
	if (ret)
		return ret;

	return 0;
}

static int set_pcie_width_and_speed(struct dts_node *root, unsigned int id)
{
	int ret;
	struct dts_node node;
	u32 linkwidth = X_MAX, linkspeed = GEN_MAX;

	ret = node_by_alias(root, &node, PCIE_ALIAS_FMT, id);
	if (ret) {
		pr_err("Failed to get 'pcie%u' alias\n", id);
		return ret;
	}

	if (s32_serdes_is_combo_mode_enabled_in_hwconfig(id)) {
		linkwidth = X1;

		if (s32_serdes_is_mode5_enabled_in_hwconfig(id))
			linkspeed = GEN2;
	}

	if (linkwidth < X_MAX) {
		ret = node_set_prop_u32(&node, "num-lanes", linkwidth);
		if (ret)
			pr_err("Failed to set 'num-lanes'\n");
	}

	if (linkspeed < GEN_MAX) {
		ret = node_set_prop_u32(&node, "max-link-speed", linkspeed);
		if (ret)
			pr_err("Failed to set 'max-link-speed'\n");
	}

	return ret;
}

static int prepare_pcie_node(struct dts_node *root, unsigned int id)
{
	int ret;
	struct dts_node node;

	ret = node_by_alias(root, &node, PCIE_ALIAS_FMT, id);
	if (ret) {
		pr_err("Failed to get 'pcie%u' alias\n", id);
		return ret;
	}

	if (!s32_serdes_is_pcie_enabled_in_hwconfig(id)) {
		ret = disable_node(&node);
		if (ret) {
			pr_err("Failed to disable PCIe%u\n", id);
			return ret;
		}

		/* Skip rest of the configuration if not enabled */
		return 0;
	}

	ret = set_pcie_mode(&node, id);
	if (ret)
		return ret;

	ret = set_pcie_phy_mode(&node, id);
	if (ret)
		return ret;

	ret = set_pcie_serdes_lines(&node, id);
	if (ret)
		return ret;

	ret = set_pcie_width_and_speed(&node, id);
	if (ret)
		return ret;

	ret = node_by_alias(root, &node, PCIE_ALIAS_FMT, id);
	if (ret) {
		pr_err("Failed to get 'pcie%u' alias\n", id);
		return ret;
	}

	ret = enable_node(&node);
	if (ret) {
		pr_err("Failed to enable PCIe%u\n", id);
		return ret;
	}

	return ret;
}

static int rename_ext_clk(struct dts_node *node, int prop_pos)
{
	size_t i, str_pos, prop_name_len;
	int ret, length = 0;
	const char *list;
	char *propval;

	if (prop_pos < 0)
		return -EINVAL;

	list = node_get_prop(node, "clock-names", &length);
	if (!list)
		return -EINVAL;

	if (length < 0)
		return -EINVAL;

	propval = malloc(length);
	if (!propval)
		return -ENOMEM;

	memcpy(propval, list, length);

	/* Jump over elements before 'ext' clock */
	for (str_pos = 0, i = 0; i < prop_pos; i++) {
		prop_name_len = strlen(&propval[str_pos]);

		if (check_size_overflow(prop_name_len, 1))
			return -EINVAL;

		prop_name_len += 1;

		if (check_size_overflow(str_pos, prop_name_len))
			return -EINVAL;

		str_pos += prop_name_len;
	}

	if (str_pos >= length)
		return -EINVAL;

	propval[str_pos] = toupper(propval[str_pos]);

	ret = node_set_prop(node, "clock-names", propval, length);
	if (ret) {
		pr_err("Failed to rename 'ext' SerDes clock: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	return 0;
}

static int get_ext_clk_phandle(struct dts_node *root, unsigned int id,
			       uint32_t *phandle)
{
	unsigned long mhz;
	char ext_clk_path[SERDES_EXT_PATH_SIZE];
	unsigned int clk_mhz;
	struct dts_node node;
	int ret;

	mhz = s32_serdes_get_clock_fmhz_from_hwconfig(id);
	if (mhz == MHZ_100)
		clk_mhz = 100u;
	else
		clk_mhz = 125u;

	ret = sprintf(ext_clk_path, SERDES_EXT_PATH_FMT, clk_mhz);
	if (ret < 0)
		return ret;

	ret = node_by_path(root, &node, ext_clk_path);
	if (ret) {
		pr_err("Failed to get offset of '%s' node\n", ext_clk_path);
		return ret;
	}

	*phandle = node_create_phandle(&node);
	if (!*phandle) {
		pr_err("Failed to create new phandle for %s\n",
		       ext_clk_path);
		return ret;
	}

	return 0;
}

static int add_ext_clk(struct dts_node *node, unsigned int id)
{
	struct dts_node root = *node;
	u32 phandle;
	int ret;

	ret = get_ext_clk_phandle(&root, id, &phandle);
	if (ret)
		return ret;

	ret = node_by_alias(&root, node, SERDES_ALIAS_FMT, id);
	if (ret) {
		pr_err("Failed to get 'serdes%u' alias\n", id);
		return ret;
	}

	ret = node_append_prop_str(node, "clock-names", SERDES_EXT_CLK);
	if (ret) {
		pr_err("Failed to append ext clock to 'clock-names'\n");
		return ret;
	}

	ret = node_append_prop_u32(node, "clocks", phandle);
	if (ret) {
		pr_err("Failed to append ext clock to 'clock-names'\n");
		return ret;
	}

	return ret;
}

static int set_serdes_clk(struct dts_node *root, unsigned int id)
{
	bool ext_clk = s32_serdes_is_external_clk_in_hwconfig(id);
	int prop_pos, ret;
	struct dts_node node;

	ret = node_by_alias(root, &node, SERDES_ALIAS_FMT, id);
	if (ret) {
		pr_err("Failed to get 'serdes%u' alias\n", id);
		return ret;
	}

	prop_pos = node_stringlist_search(&node, "clock-names", SERDES_EXT_CLK);

	if (!ext_clk && prop_pos >= 0)
		return rename_ext_clk(&node, prop_pos);

	if (ext_clk && prop_pos <= 0)
		return add_ext_clk(&node, id);

	return 0;
}

static int set_serdes_mode(struct dts_node *root, unsigned int id)
{
	int ret;
	enum serdes_mode mode;
	u32 mode_num;
	struct dts_node node;

	mode = s32_serdes_get_serdes_mode_from_hwconfig(id);
	if (mode == SERDES_MODE_INVAL) {
		pr_err("Invalid SerDes%u mode\n", id);
		return -EINVAL;
	}

	ret = node_by_alias(root, &node, SERDES_ALIAS_FMT, id);
	if (ret) {
		pr_err("Failed to get 'serdes%u' alias\n", id);
		return ret;
	}

	mode_num = (u32)mode;
	if (s32_serdes_is_mode5_enabled_in_hwconfig(id))
		mode_num = 5;

	ret = node_set_prop_u32(&node, "nxp,sys-mode", mode_num);
	if (ret)
		pr_err("Failed to set 'nxp,sys-mode'\n");

	return ret;
}

static void disable_serdes_pcie_nodes(struct dts_node *root, unsigned int id)
{
	size_t i;
	int ret;
	struct dts_node node;
	static const char * const fmts[] = {SERDES_ALIAS_FMT, PCIE_ALIAS_FMT};

	for (i = 0; i < ARRAY_SIZE(fmts); i++) {
		ret = node_by_alias(root, &node, fmts[i], id);
		if (ret) {
			pr_err("Failed to get '%s%u' alias\n", fmts[i], id);
			continue;
		}

		ret = disable_node(&node);
		if (ret) {
			pr_err("Failed to disable %s%u\n", fmts[i], id);
		}
	}
}

static int node_get_path(struct dts_node *node, char *buf, int len)
{
	if (node->fdt)
		return fdt_get_path(node->blob, node->off, buf, len);

	if (ofnode_is_np(node->node))
		return ofnode_get_path(node->node, buf, len);

	return -EINVAL;
}

static int node_find_subnode(struct dts_node *node, struct dts_node *subnode,
			     const char *subnode_name)
{
	int ret, len;
	char buf[MAX_PATH_SIZE] = "";

	if (!node || !subnode)
		return -EINVAL;

	ret = node_get_path(node, buf, MAX_PATH_SIZE);
	if (ret) {
		debug("%s: Cannot get node path\n", __func__);
		return -EINVAL;
	}

	len = strlen(buf);
	ret = snprintf(&buf[len], MAX_PATH_SIZE - len, "/%s", subnode_name);
	if (ret < 0 || ret >= MAX_PATH_SIZE - len) {
		debug("%s: Subnode full path too long\n", __func__);
		return -EINVAL;
	}

	debug("%s: searching for subnode: %s\n", __func__, buf);
	ret = node_by_path(node, subnode, buf);
	if (!ret) {
		/* Already exists */
		debug("Subnode %s already exists\n", buf);
		return 0;
	}

	/* All good but no subnode */
	debug("Subnode %s not found\n", buf);
	return -ENOENT;
}

static int apply_hwconfig_fixups(bool fdt, void *blob)
{
	bool skip = false;
	int ret;
	unsigned int id;
	struct dts_node root = {
		.fdt = fdt,
		.blob = blob,
	};

	for (id = 0; id <= 1; id++) {
		if (!s32_serdes_is_hwconfig_instance_enabled(id)) {
			disable_serdes_pcie_nodes(&root, id);
			continue;
		}

		if (!s32_serdes_is_cfg_valid(id)) {
			disable_serdes_pcie_nodes(&root, id);
			pr_err("SerDes%u configuration will be ignored as it's invalid\n",
			       id);
			continue;
		}

		ret = skip_dts_nodes_config(&root, id, &skip);
		/* Either if we manage to disable the node or not, go to next node
		 * if 'skip' is true
		 */
		if (skip || ret)
			continue;

		ret = prepare_pcie_node(&root, id);
		if (ret)
			pr_warn("Failed to prepare PCIe node%u\n", id);

		ret = set_serdes_clk(&root, id);
		if (ret)
			pr_err("Failed to set the clock for SerDes%u\n", id);

		ret = set_serdes_mode(&root, id);
		if (ret)
			pr_err("Failed to set mode for SerDes%u\n", id);
	}

	return 0;
}

int apply_dm_hwconfig_fixups(void)
{
	return apply_hwconfig_fixups(false, NULL);
}

int apply_fdt_hwconfig_fixups(void *blob)
{
	return apply_hwconfig_fixups(true, blob);
}
