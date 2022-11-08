// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */
#include <common.h>
#include <malloc.h>
#include <phy_interface.h>
#include <dm/device.h>
#include <dm/of_access.h>
#include <dm/ofnode.h>
#include <dm/platform_data/pfeng_dm_eth.h>
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

#define GMAC_ALIAS_FMT			"gmac%d"
#define PFE_ALIAS_FMT			"pfe%d"

#define SERDES_COUNT			2
#define XPCS_COUNT			2

#define MAX_PATH_SIZE			100

#define EMAC_ID_INVALID			(u32)(-1)

#define MAX_PROP_ALLOC			(500)

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
	if (old_len < 0)
		return -EINVAL;

	if (old_len + len < len || old_len + len < old_len)
		return -EINVAL;

	/* New property */
	if (!old_val) {
		new_val = malloc(len);
		if (!new_val)
			return -ENOMEM;

		memcpy(new_val, val, len);
		return ofnode_write_prop(node, prop, len, new_val);
	}

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

#if (CONFIG_IS_ENABLED(FSL_PFENG))
static int node_get_prop_u32(struct dts_node *node, const char *prop, u32 *val)
{
	if (node->fdt) {
		const fdt32_t *cell;
		int len = 0;

		cell = fdt_getprop(node->blob, node->off, prop, &len);
		if (!cell || len != sizeof(*cell))
			return -EINVAL;

		*val = fdt32_to_cpu(*cell);
		return 0;
	}

	return ofnode_read_u32(node->node, prop, val);
}
#endif

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
	char *compatible;
	enum pcie_type pcie_mode;

	pcie_mode = s32_serdes_get_pcie_type_from_hwconfig(id);
	if (pcie_mode & PCIE_EP)
		compatible = "nxp,s32cc-pcie-ep";
	else
		compatible = "nxp,s32cc-pcie";

	ret = node_set_prop_str(node, "compatible", compatible);
	if (ret) {
		pr_err("Failed to set PCIE compatible: %s\n",
		       fdt_strerror(ret));
		return ret;
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
				 u32 lanes, u32 phandle)
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

	printf("Skipping configuration for SerDes%u,\n", id);

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

	if (length < 0 || length > MAX_PROP_ALLOC)
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

#if (CONFIG_IS_ENABLED(FSL_PFENG))
/* This static function is copied from lib/of_live.c */
static void *unflatten_dt_alloc(void **mem, unsigned long size,
				unsigned long align)
{
	void *res;

	*mem = PTR_ALIGN(*mem, align);
	res = *mem;
	*mem += size;

	return res;
}

/* See unflatten_dt_node() from lib/of_live.c */
static void *unflatten_dt_node(void *mem,
			       struct device_node *dad,
			       struct device_node **nodepp,
			       const char *node_name, bool dryrun)
{
	struct device_node *np;
	struct property *pp, **prev_pp = NULL;

	size_t namel = 0;
	size_t fpsize = 0;

	if (!node_name)
		return mem;

	/* Size of the node name */
	namel = strlen(node_name) + 1;
	/* Size of the node full path */
	fpsize = namel + 1;
	if (dad && dad->parent)
		fpsize += strlen(dad->full_name);

	/* Set the node full name */
	np = unflatten_dt_alloc(&mem, sizeof(struct device_node) + fpsize,
				__alignof__(struct device_node));
	if (!dryrun) {
		char *fn;

		fn = (char *)np + sizeof(*np);
		np->full_name = fn;
		/* Rebuild full path */
		if (dad && dad->parent) {
			strcpy(fn, dad->full_name);
			fn += strlen(fn);
		}
		*(fn++) = '/';
		memcpy(fn, node_name, namel);
		debug("%s: Generated full name: %s\n", __func__, (char *)np->full_name);

		prev_pp = &np->properties;
		if (dad) {
			np->parent = dad;
			np->sibling = dad->child;
			dad->child = np;
		}
	}

	/* Set the 'name' property */
	pp = unflatten_dt_alloc(&mem, sizeof(struct property) + namel,
				__alignof__(struct property));
	if (!dryrun) {
		pp->name = "name";
		pp->length = namel;
		pp->value = pp + 1;
		*prev_pp = pp;
		prev_pp = &pp->next;
		memcpy(pp->value, node_name, namel - 1);
			((char *)pp->value)[namel - 1] = 0;
		debug("%s: Generated node name: %s\n", __func__, (char *)pp->value);
	}

	/* Finish node initialization */
	if (!dryrun) {
		*prev_pp = NULL;
		np->name = pp->value; /* pp holds property 'name' */
		np->type = "<NULL>";
	}

	if (nodepp)
		*nodepp = np;

	return mem;
}

static int node_create_subnode(struct dts_node *node, struct dts_node *subnode,
			       const char *subnode_name)
{
	int ret;

	if (!node || !subnode)
		return -EINVAL;

	/* Check if node already exists */
	ret = node_by_path(node, subnode, subnode_name);
	if (!ret)
		return 0;

	*subnode = *node;
	if (node->fdt) {
		subnode->off = fdt_add_subnode(node->blob, node->off, subnode_name);
		if (subnode->off < 0)
			return subnode->off;
		return 0;
	}

	if (ofnode_is_np(node->node)) {
		struct device_node *np = (struct device_node *)ofnode_to_np(node->node);
		struct device_node *subnp;
		unsigned long size;
		void *mem;

		if (!np)
			return -EINVAL;

		size = (unsigned long)unflatten_dt_node(NULL, np, NULL,
						subnode_name, true);
		if (!size)
			return -EFAULT;
		size = ALIGN(size, 4);

		debug("%s: Size is %lx, allocating...\n", __func__, size);

		/* Allocate memory for the expanded device tree */
		mem = malloc(size + 4);
		memset(mem, '\0', size);

		/* End marker, for validation */
		*(__be32 *)(mem + size) = cpu_to_be32(0xdeadbeef);

		/* Second pass, do actual creation */
		unflatten_dt_node(mem, np, &subnp, subnode_name, false);
		if (be32_to_cpup(mem + size) != 0xdeadbeef) {
			debug("%s: End of tree marker overwritten: %08x\n",
			      __func__, be32_to_cpup(mem + size));
			return -ENOSPC;
		}

		subnode->node = np_to_ofnode(subnp);

		return 0;
	}

	return -EINVAL;
}

static int node_delete_property(struct dts_node *node, const char *prop_name)
{
	if (!node)
		return -EINVAL;

	if (node->fdt) {
		fdt_delprop(node->blob, node->off, prop_name);
		return 0;
	}

	if (ofnode_is_np(node->node)) {
		struct device_node *np = (struct device_node *)ofnode_to_np(node->node);
		struct property *pp;
		struct property *pp_last = NULL;
		struct property *del;

		if (!np)
			return -EINVAL;

		for (pp = np->properties; pp; pp = pp->next) {
			if (strcmp(pp->name, prop_name) == 0) {
				/* Property exists -> remove */
				del = pp;

				if (pp == np->properties) {
					/* First property */
					np->properties = del->next;
				} else if (!pp->next) {
					/* Last property */
					pp_last->next = NULL;
				} else {
					pp->next = del->next;
				}

				free(del);
				break;
			}
			pp_last = pp;
		}

		return 0;
	}

	return -EINVAL;
}

static int get_xpcs_node_from_alias(struct dts_node *root, struct dts_node *node,
				    u32 serdes_id, u32 xpcs_id)
{
	static const char * const xpcs_fmts[SERDES_COUNT][XPCS_COUNT] = {
		{GMAC_ALIAS_FMT, PFE_ALIAS_FMT},
		{PFE_ALIAS_FMT, PFE_ALIAS_FMT}
		};
	static const u32 xpcs_ids[SERDES_COUNT][XPCS_COUNT] = {
		{0, 2}, {0, 1}
		};

	/* Validate arguments. */
	if (serdes_id >= SERDES_COUNT || xpcs_id >= XPCS_COUNT) {
		debug("%s: Unsupported interface XPCS%u for SerDes%u\n",
		      __func__, xpcs_id, serdes_id);
		return -EINVAL;
	}

	return node_by_alias(root, node,
			     xpcs_fmts[serdes_id][xpcs_id],
			     xpcs_ids[serdes_id][xpcs_id]);
}

static int set_xpcs_config(struct dts_node *root, u32 serdes_id, u32 xpcs_id)
{
	int ret = 0;
	struct dts_node node;
	struct dts_node subnode;
	u32 emac_id = EMAC_ID_INVALID;

	/* We want this fixup only for Mode5.
	 * For other nodes, do nothing.
	 */
	if (!s32_serdes_is_mode5_enabled_in_hwconfig(serdes_id)) {
		debug("%s: SerDes%d not in Mode 5\n", __func__, serdes_id);
		return 0;
	}

	/* We need the device tree node for the ethernet interface.
	 * If we don't, exit silently as most probably we are not
	 * supposed to apply the fixups for this context.
	 */
	ret = get_xpcs_node_from_alias(root, &node,
				       serdes_id, xpcs_id);
	if (ret) {
		debug("%s: Failed to get the DT node for SerDes%u XPCS%u\n",
		      __func__, serdes_id, xpcs_id);
		return ret;
	}

	/* Check for PFE interfaces that the corresponding EMAC is configured as SGMII.
	 * If the property is missing, then we have another type of interface (e.g. GMAC)
	 * which we do not check if is SGMII (TBD).
	 */
	ret = node_get_prop_u32(&node, "nxp,pfeng-emac-id", &emac_id);
	if (ret) {
		pr_warn("Interface does not have ID\n");
		return -ENODATA;
	} else if (pfeng_cfg_emac_get_interface(emac_id) != PHY_INTERFACE_MODE_SGMII) {
		pr_warn("PFE EMAC %d is not SGMII\n", emac_id);
		return -ENODATA;
	}

	/* We may or may not have this property */
	node_delete_property(&node, "phy-handle");
	/* We need the fixed-link node */
	ret = node_create_subnode(&node, &subnode, "fixed-link");
	if (ret) {
		debug("%s: Cannot create node 'fixed-link'\n", __func__);
		goto exit;
	}
	ret = node_set_prop_u32(&subnode, "speed", 2500);
	if (ret) {
		debug("%s: Cannot set property 'speed'\n", __func__);
		goto exit;
	}
	ret = node_set_prop(&subnode, "full-duplex", NULL, 0);
	if (ret) {
		debug("%s: Cannot set property 'full-duplex'\n", __func__);
		goto exit;
	}

	ret = enable_node(&node);
	if (ret)
		pr_err("Failed to configure XPCS%d_%d\n", serdes_id, xpcs_id);

exit:
	return ret;
}
#endif

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

#if (CONFIG_IS_ENABLED(FSL_PFENG))
		/* Mode 5 only - uses XPCS1 for either SerDes */
		ret = set_xpcs_config(&root, id, 1);
		if (ret)
			pr_err("Failed to update pfe information for SerDes%d\n", id);
#endif
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
