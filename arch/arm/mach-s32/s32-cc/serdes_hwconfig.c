// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */
#include <common.h>
#include <hwconfig.h>
#include <dm/device.h>
#include <dm/of_access.h>
#include <dm/ofnode.h>
#include <dm/read.h>
#include <linux/ethtool.h>
#include <s32-cc/serdes_hwconfig.h>

/* use a mask to fix DEVICE_TYPE for EP */
#define SERDES_MODE(mode) ((mode) & 0xe)
#define IS_SERDES_PCIE(mode) ((mode) & (PCIE_EP | PCIE_RC))
#define IS_SERDES_SGMII(mode) ((mode) & (SGMII))

#define SERDES_RC_MODE_STR "RootComplex"
#define SERDES_EP_MODE_STR "EndPoint"
#define SERDES_SGMII_MODE_STR "SGMII"
#define SERDES_SGMII_MODE_NONE_STR "None"
#define MODE5_TOKEN "mode5"

#define SERDES_NAME_SIZE 32

bool s32_serdes_is_pcie_enabled_in_hwconfig(int id)
{
	enum serdes_dev_type ss_mode;

	ss_mode = s32_serdes_get_mode_from_hwconfig(id);
	return IS_SERDES_PCIE(ss_mode);
}

bool s32_serdes_is_combo_mode_enabled_in_hwconfig(int id)
{
	enum serdes_dev_type ss_mode;

	ss_mode = s32_serdes_get_mode_from_hwconfig(id);
	return IS_SERDES_PCIE(ss_mode) &&
			IS_SERDES_SGMII(ss_mode);
}

bool s32_serdes_is_hwconfig_instance_enabled(int id)
{
	char serdes_name[SERDES_NAME_SIZE];
	const char *arg;
	size_t len;

	/*
	 * The SerDes mode is set by using option `serdesx`, where
	 * `x` is the ID.
	 */
	snprintf(serdes_name, SERDES_NAME_SIZE, "serdes%d", id);
	arg = hwconfig_arg(serdes_name, &len);
	if (!arg || !len) {
		/* Backwards compatibility:
		 * Initially the SerDes mode was set by using option `pciex`.
		 */
		sprintf(serdes_name, "pcie%d", id);
		arg = hwconfig_arg(serdes_name, &len);
		if (!arg || !len)
			return false;
	}

	return true;
}

static inline
char *s32_serdes_get_hwconfig_subarg(int id,
				     const char *subarg,
				     size_t *subarg_len)
{
	char serdes_name[SERDES_NAME_SIZE];
	char *subarg_str = NULL;

	if (!subarg || !subarg_len)
		return NULL;

	/*
	 * The SerDes mode is set by using option `serdesx`, where
	 * `x` is the ID.
	 */
	snprintf(serdes_name, SERDES_NAME_SIZE, "serdes%d", id);
	debug("%s: testing hwconfig for '%s'\n", __func__,
	      serdes_name);

	subarg_str = (char *)hwconfig_subarg(serdes_name, subarg, subarg_len);

	if (!subarg_str || !*subarg_len) {
		/* Backwards compatibility:
		 * Initially the SerDes mode was set by using option `pciex`.
		 */
		sprintf(serdes_name, "pcie%d", id);
		debug("%s: testing hwconfig for '%s'\n", __func__,
		      serdes_name);
		subarg_str = (char *)hwconfig_subarg(serdes_name, subarg,
			subarg_len);

		if (!subarg_str || !*subarg_len) {
			debug("'serdes%d' option '%s' not found in hwconfig\n",
			      id, subarg);
			return NULL;
		}
	}

	debug("found 'serdes%d' argument '%s=%s\n'", id, subarg, subarg_str);
	return subarg_str;
}

enum serdes_dev_type s32_serdes_get_mode_from_hwconfig(int id)
{
	enum serdes_dev_type devtype = SERDES_INVALID;
	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_hwconfig_subarg(id, "mode",
		&subarg_len);

	if (!option_str || !subarg_len)
		return SERDES_INVALID;

	/* 'mode' option */
	if (!strncmp(option_str, "rc", subarg_len))
		devtype = PCIE_RC;
	else if (!strncmp(option_str, "ep", subarg_len))
		devtype = PCIE_EP;
	else if (!strncmp(option_str, "sgmii", subarg_len))
		devtype = SGMII;
	else if (!strncmp(option_str, "rc&sgmii", subarg_len))
		devtype = (enum serdes_dev_type)((u32)PCIE_RC | (u32)SGMII);
	else if (!strncmp(option_str, "ep&sgmii", subarg_len))
		devtype = (enum serdes_dev_type)((u32)PCIE_EP | (u32)SGMII);

	/* 'skip' option */
	option_str = s32_serdes_get_hwconfig_subarg(id, "skip", &subarg_len);
	if (option_str && devtype != SERDES_INVALID &&
	    (!strncmp(option_str, "true", subarg_len) ||
	    !strncmp(option_str, "1", subarg_len)))
		devtype |= SERDES_SKIP;

	return devtype;
}

enum serdes_xpcs_mode s32_serdes_get_xpcs_cfg_from_hwconfig(int id)
{
	/* Set default mode to invalid to force configuration */
	enum serdes_xpcs_mode xpcs_mode = SGMII_INAVALID;
	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_hwconfig_subarg(id, "xpcs_mode",
		&subarg_len);

	if (!option_str || !subarg_len)
		return xpcs_mode;

	if (!strncmp(option_str, "0", subarg_len))
		xpcs_mode = SGMII_XPCS0;
	else if (!strncmp(option_str, "1", subarg_len))
		xpcs_mode = SGMII_XPCS1;
	else if (!strncmp(option_str, "both", subarg_len))
		xpcs_mode = SGMII_XPCS0_XPCS1;
	else if (!strncmp(option_str, "2G5", subarg_len))
		xpcs_mode = SGMII_XPCS0_2G5;

	return xpcs_mode;
}

bool s32_serdes_is_external_clk_in_hwconfig(int id)
{
	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_hwconfig_subarg(id, "clock",
		&subarg_len);

	if (!option_str || !subarg_len)
		return false;

	if (!strncmp(option_str, "ext", subarg_len))
		return true;
	else if (!strncmp(option_str, "int", subarg_len))
		return false;

	return false;
}

unsigned long s32_serdes_get_clock_fmhz_from_hwconfig(int id)
{
	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_hwconfig_subarg(id, "fmhz",
		&subarg_len);

	if (!option_str || !subarg_len)
		return MHZ_100;

	if (!strncmp(option_str, "100", subarg_len))
		return MHZ_100;
	else if (!strncmp(option_str, "125", subarg_len))
		return MHZ_125;

	return MHZ_100;
}

enum pcie_phy_mode s32_serdes_get_phy_mode_from_hwconfig(int id)
{
	enum pcie_phy_mode phy_mode = CRNS;
	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_hwconfig_subarg(id, "phy_mode",
		&subarg_len);

	if (!option_str || !subarg_len)
		return phy_mode;

	if (!strncmp(option_str, "crss", subarg_len))
		phy_mode = CRSS;
	else if (!strncmp(option_str, "sris", subarg_len))
		phy_mode = SRIS;

	return phy_mode;
}

enum serdes_mode s32_serdes_get_op_mode_from_hwconfig(int id)
{
	enum serdes_dev_type mode;
	enum serdes_xpcs_mode xpcs_mode;
	enum pcie_phy_mode ss_mode;

	mode = s32_serdes_get_mode_from_hwconfig(id);
	ss_mode = s32_serdes_get_phy_mode_from_hwconfig(id);
	xpcs_mode = s32_serdes_get_xpcs_cfg_from_hwconfig(id);

	/* Do not configure SRIS or CRSS PHY MODE in conjunction
	 * with any SGMII mode on the same SerDes subsystem
	 */
	if (ss_mode == CRSS || ss_mode == SRIS) {
		if (IS_SERDES_PCIE(mode) && !IS_SERDES_SGMII(mode))
			return SERDES_MODE_PCIE_PCIE;

		return SERDES_MODE_INVAL;
	}

	if (IS_SERDES_PCIE(mode) && !IS_SERDES_SGMII(mode))
		return SERDES_MODE_PCIE_PCIE;

	if (IS_SERDES_PCIE(mode) && IS_SERDES_SGMII(mode)) {
		/* Configure SS mode based on XPCS: modes 1 & 2 */
		if (xpcs_mode == SGMII_XPCS0)
			return SERDES_MODE_PCIE_SGMII0;

		if (xpcs_mode == SGMII_XPCS1)
			return SERDES_MODE_PCIE_SGMII1;

		return SERDES_MODE_INVAL;
	}

	/* Mode 3 */
	if (!IS_SERDES_PCIE(mode) && IS_SERDES_SGMII(mode))
		return SERDES_MODE_SGMII_SGMII;

	return SERDES_MODE_INVAL;
}

bool s32_serdes_is_mode5_enabled_in_hwconfig(int id)
{
	size_t demo_len = 0;
	char *demo;

	demo = s32_serdes_get_hwconfig_subarg(id, "demo", &demo_len);

	if (!demo || !demo_len)
		return false;

	if (strncmp(demo, MODE5_TOKEN, sizeof(MODE5_TOKEN) - 1))
		return false;

	return true;
}

bool s32_serdes_is_cfg_valid(int id)
{
	enum serdes_dev_type devtype;
	enum serdes_xpcs_mode xpcs_mode;
	enum serdes_mode mode;
	enum pcie_phy_mode phy_mode;
	unsigned long freq;
	bool mode5, ext_clk;

	ext_clk = s32_serdes_is_external_clk_in_hwconfig(id);
	freq = s32_serdes_get_clock_fmhz_from_hwconfig(id);
	devtype = s32_serdes_get_mode_from_hwconfig(id);
	xpcs_mode = s32_serdes_get_xpcs_cfg_from_hwconfig(id);
	mode = s32_serdes_get_op_mode_from_hwconfig(id);
	phy_mode = s32_serdes_get_phy_mode_from_hwconfig(id);

	if (mode == SERDES_MODE_INVAL) {
		printf("Invalid opmode config on SerDes%d\n", id);
		return false;
	}

	if (devtype == SERDES_INVALID) {
		printf("Invalid SerDes%d configuration\n", id);
		return false;
	}

	if (IS_SERDES_PCIE(devtype) && freq == MHZ_125) {
		printf("Invalid \"hwconfig\": In PCIe/SGMII combo");
		printf(" reference clock has to be 100Mhz\n");
		/* SGMII configuration fail */
		return false;
	}

	if (IS_SERDES_SGMII(devtype) && xpcs_mode == SGMII_INAVALID) {
		printf("Invalid \"hwconfig\": \"xpcs_mode\" is missing\n");
		/* SGMII configuration fail */
		return false;
	}

	/* validate that required 'mode' does not interfere
	 * with 'hwconfig'
	 */
	switch (mode & ~(uint32_t)(SERDES_SKIP)) {
	case SERDES_MODE_PCIE_PCIE:
		/* only PCIE, no SGMII for this mode */
		if (!IS_SERDES_PCIE(devtype) || IS_SERDES_SGMII(devtype)) {
			printf("SGMII isn't allowed when using PCIe mode\n");
			return false;
		}
		break;
	/* Will have to figure out how to handle SERDES_MODE_SGMII_PCIE
	 * and SERDES_MODE_PCIE_SGMII, since lane assignment may differ.
	 */
	case SERDES_MODE_PCIE_SGMII0:
	case SERDES_MODE_PCIE_SGMII1:
		if (!IS_SERDES_PCIE(devtype) || !IS_SERDES_SGMII(devtype)) {
			printf("The SerDes mode is incompletely described\n");
			return false;
		}
		break;
	case SERDES_MODE_SGMII_SGMII:
		if (IS_SERDES_PCIE(devtype) || !IS_SERDES_SGMII(devtype)) {
			printf("The SerDes mode is incompletely described\n");
			return false;
		}
		break;
	default:
		return false;
	}

	mode5 = s32_serdes_is_mode5_enabled_in_hwconfig(id);
	if (mode5) {
		if (!(ext_clk && freq == MHZ_100 &&
		      xpcs_mode == SGMII_XPCS1)) {
			pr_err("SerDes%d: Invalid mode5 demo configuration\n",
			       id);
			return false;
		}
	}

	if (!ext_clk) {
		if (phy_mode == CRSS || phy_mode == SRIS) {
			printf("SerDes%d: CRSS or SRIS for PCIe%d PHY mode cannot be used with internal clock\n",
			       id, id);
			return false;
		}
	}

	return true;
}

int s32_serdes_get_alias_id(struct udevice *serdes_dev, int *devnump)
{
	ofnode node = dev_ofnode(serdes_dev);
	const char *uc_name = "serdes";
	int ret;

	if (ofnode_is_np(node)) {
		ret = of_alias_get_id(ofnode_to_np(node), uc_name);
		if (ret >= 0) {
			*devnump = ret;
			ret = 0;
		}
	} else {
		ret = fdtdec_get_alias_seq(gd->fdt_blob, uc_name,
					   ofnode_to_offset(node), devnump);
	}

	return ret;
}

int s32_serdes_get_lane_speed(struct udevice *serdes_dev, u32 lane)
{
	enum serdes_xpcs_mode xpcs_mode;
	int serdes_id = 0;
	int ret;

	if (!serdes_dev)
		return -EINVAL;

	ret = s32_serdes_get_alias_id(serdes_dev, &serdes_id);
	if (ret < 0) {
		printf("Failed to get SerDes device id for device %s:\n",
		       serdes_dev->name);
		return ret;
	}

	xpcs_mode = s32_serdes_get_xpcs_cfg_from_hwconfig(serdes_id);
	switch (xpcs_mode) {
	/* XPCS is on lane1 when using ss mode = 1 or 2 */
	case SGMII_XPCS0:
		if (lane)
			return SPEED_1000;
		break;
	case SGMII_XPCS1:
		if (lane) {
			if (s32_serdes_is_mode5_enabled_in_hwconfig(serdes_id))
				return SPEED_2500;
			return SPEED_1000;
		}
		break;
	case SGMII_XPCS0_XPCS1:
		if (!lane || lane == 1)
			return SPEED_1000;
		break;
	case SGMII_XPCS0_2G5:
		if (!lane)
			return SPEED_2500;
		break;
	case SGMII_INAVALID:
		return -EINVAL;
	}

	return -EINVAL;
}
