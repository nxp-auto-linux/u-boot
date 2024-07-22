// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022-2024 NXP
 */
#include <common.h>
#include <hwconfig.h>
#include <dm/device.h>
#include <dm/of_access.h>
#include <dm/ofnode.h>
#include <dm/read.h>
#include <linux/ethtool.h>
#include <s32-cc/serdes_hwconfig.h>

#define SERDES_NAME_SIZE 32

#define SERDES_SKIP_BOOT_STR	"boot"
#define SERDES_SKIP_KERNEL_STR	"kernel"
#define SERDES_SKIP_ALL_STR	"all"

bool s32_serdes_is_pcie_enabled_in_hwconfig(unsigned int id)
{
	enum serdes_mode ss_mode;

	ss_mode = s32_serdes_get_serdes_mode_from_hwconfig(id);
	return s32_serdes_is_pcie_mode(ss_mode);
}

bool s32_serdes_is_combo_mode_enabled_in_hwconfig(unsigned int id)
{
	enum serdes_mode ss_mode;

	ss_mode = s32_serdes_get_serdes_mode_from_hwconfig(id);
	return s32_serdes_is_pcie_mode(ss_mode) &&
		s32_serdes_is_xpcs_mode(ss_mode);
}

static inline
char *s32_serdes_get_pcie_hwconfig_subarg(unsigned int id,
					  const char *subarg,
					  size_t *subarg_len)
{
	char serdes_name[SERDES_NAME_SIZE + 1];
	char *subarg_str = NULL;

	snprintf(serdes_name, SERDES_NAME_SIZE, "pcie%d", id);
	debug("%s: testing hwconfig for '%s' option '%s'\n", __func__,
	      serdes_name, subarg);
	subarg_str = (char *)hwconfig_subarg(serdes_name, subarg,
		subarg_len);

	if (!subarg_str || !*subarg_len) {
		debug("'%s' option '%s' not found in hwconfig\n",
		      serdes_name, subarg);
		return NULL;
	}

	debug("found '%s' argument '%s=%s'\n", serdes_name, subarg, subarg_str);
	return subarg_str;
}

static inline
char *s32_serdes_get_serdes_hwconfig_subarg(int id,
					    const char *subarg,
					    size_t *subarg_len)
{
	char serdes_name[SERDES_NAME_SIZE + 1];
	char *subarg_str = NULL;
	int ret;

	if (!subarg || !subarg_len)
		return NULL;

	/*
	 * The SerDes mode is set by using option `serdesx`, where
	 * `x` is the ID.
	 */
	ret = snprintf(serdes_name, SERDES_NAME_SIZE, "serdes%u", id);
	if (ret < 0)
		return NULL;

	debug("%s: testing hwconfig for '%s' option '%s'\n", __func__,
	      serdes_name, subarg);

	subarg_str = (char *)hwconfig_subarg(serdes_name, subarg, subarg_len);

	if (!subarg_str || !*subarg_len) {
#ifdef CONFIG_S32CC_HWCONFIG_LEGACY
		/* Backwards compatibility:
		 * Initially the SerDes mode was set by using option `pciex`.
		 */
		char pcie_name[SERDES_NAME_SIZE + 1];

		ret = snprintf(pcie_name, SERDES_NAME_SIZE, "pcie%d", id);
		if (ret < 0)
			return NULL;
		subarg_str = s32_serdes_get_pcie_hwconfig_subarg(id, subarg,
								 subarg_len);
#endif
		if (!subarg_str || !*subarg_len) {
			debug("'%s' option '%s' not found in hwconfig\n",
			      serdes_name, subarg);
			return NULL;
		}
	}

	debug("found '%s' argument '%s=%s'\n", serdes_name, subarg, subarg_str);
	return subarg_str;
}

static inline
char *s32_serdes_get_xpcs_hwconfig_subarg(int serdes_id, int xpcs_id,
					  const char *subarg,
					  size_t *subarg_len)
{
	char xpcs_name[SERDES_NAME_SIZE + 1];
	char *subarg_str = NULL;

	snprintf(xpcs_name, SERDES_NAME_SIZE, "xpcs%d_%d", serdes_id, xpcs_id);
	debug("%s: testing hwconfig for '%s' option '%s'\n", __func__,
	      xpcs_name, subarg);
	subarg_str = (char *)hwconfig_subarg(xpcs_name, subarg,
					     subarg_len);

	if (!subarg_str || !*subarg_len) {
#ifdef CONFIG_S32CC_HWCONFIG_LEGACY
		/* Backwards compatibility:
		 * Initially the XPCS mode was set by using option `pciex`.
		 */
		subarg_str = s32_serdes_get_pcie_hwconfig_subarg(serdes_id,
								 subarg,
								 subarg_len);
#endif
		if (!subarg_str || !*subarg_len) {
			debug("'%s' option '%s' not found in hwconfig\n",
			      xpcs_name, subarg);
			return NULL;
		}
	}

	debug("found '%s' argument '%s=%s'\n", xpcs_name,
	      subarg, subarg_str);
	return subarg_str;
}

enum pcie_type s32_serdes_get_pcie_type_from_hwconfig(unsigned int id)
{
	enum pcie_type pcietype = PCIE_INVALID;
	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_pcie_hwconfig_subarg(id, "mode",
		&subarg_len);

	if (!option_str || !subarg_len)
		return PCIE_INVALID;

	/* 'mode' option */
	if (!strncmp(option_str, "rc", subarg_len))
		pcietype = PCIE_RC;
	else if (!strncmp(option_str, "ep", subarg_len))
		pcietype = PCIE_EP;
#ifdef CONFIG_S32CC_HWCONFIG_LEGACY
	else if (!strncmp(option_str, "rc&sgmii", subarg_len))
		pcietype = PCIE_RC;
	else if (!strncmp(option_str, "ep&sgmii", subarg_len))
		pcietype = PCIE_EP;
#endif

	debug("found pcie%d mode %d\n", id, pcietype);
	return pcietype;
}

enum serdes_skip_mode s32_serdes_get_skip_from_hwconfig(unsigned int id)
{
	enum serdes_skip_mode skip = SERDES_NO_SKIP;
	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_serdes_hwconfig_subarg(id, "skip",
								 &subarg_len);
	if (option_str &&
	    !strncmp(option_str, SERDES_SKIP_ALL_STR, subarg_len))
		skip = SERDES_SKIP_ALL;
	else if (option_str &&
		 !strncmp(option_str, SERDES_SKIP_BOOT_STR, subarg_len))
		skip = SERDES_SKIP_BOOT;
	else if (option_str &&
		 !strncmp(option_str, SERDES_SKIP_KERNEL_STR, subarg_len))
		skip = SERDES_SKIP_KERNEL;

	debug("found serdes%d skip %d\n", id, skip);
	return skip;
}

bool s32_serdes_get_xpcs_an_from_hwconfig(int serdes_id,
					  int xpcs_id)
{
	/* Set default autoneg to enabled */
	bool an = true;
	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_xpcs_hwconfig_subarg(serdes_id,
							       xpcs_id,
							       "an",
							       &subarg_len);
	if (option_str &&
	    !strncmp(option_str, "0", subarg_len))
		an = false;

	debug("found xpcs%d_%d an=%d\n", serdes_id, xpcs_id, (int)an);
	return an;
}

int s32_serdes_get_xpcs_speed_from_hwconfig(int serdes_id,
					    int xpcs_id)
{
	/* Set default mode to invalid to force configuration */
	int speed = SPEED_UNKNOWN;
	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_xpcs_hwconfig_subarg(serdes_id,
							       xpcs_id,
							       "speed",
							       &subarg_len);
	if (!option_str || !subarg_len) {
#ifdef CONFIG_S32CC_HWCONFIG_LEGACY
		option_str = s32_serdes_get_pcie_hwconfig_subarg(serdes_id,
								 "xpcs_mode",
								 &subarg_len);
		if (!option_str || !subarg_len)
			return speed;

		if (xpcs_id == 0 && !strncmp(option_str, "0", subarg_len))
			speed = SPEED_1000;
		else if (xpcs_id == 1 && !strncmp(option_str, "1", subarg_len))
			speed = SPEED_1000;
		else if (!strncmp(option_str, "both", subarg_len))
			speed = SPEED_1000;
		else if (xpcs_id == 0 && !strncmp(option_str, "2G5",
						  subarg_len))
			speed = SPEED_2500;
#endif
		debug("found xpcs%d_%d speed %d\n", serdes_id, xpcs_id, speed);
		return speed;
	}

	if (!strncmp(option_str, "10M", subarg_len))
		speed = SPEED_10;
	else if (!strncmp(option_str, "100M", subarg_len))
		speed = SPEED_100;
	else if (!strncmp(option_str, "1G", subarg_len))
		speed = SPEED_1000;
	else if (!strncmp(option_str, "2G5", subarg_len))
		speed = SPEED_2500;

	debug("found xpcs%d_%d speed %d\n", serdes_id, xpcs_id, speed);
	return speed;
}

enum serdes_xpcs_mode s32_serdes_get_xpcs_cfg_from_hwconfig(int serdes_id,
							    int xpcs_id)
{
	/* Set default mode to invalid to force configuration */
	enum serdes_xpcs_mode xpcs_mode = SGMII_INVALID;
	int speed = s32_serdes_get_xpcs_speed_from_hwconfig(serdes_id, xpcs_id);

	switch (speed) {
	case SPEED_10:
	case SPEED_100:
	case SPEED_1000:
		xpcs_mode = SGMII_XPCS_1G;
		break;
	case SPEED_2500:
		xpcs_mode = SGMII_XPCS_2G5;
		break;
	}

	debug("found xpcs%d_%d mode %d\n", serdes_id, xpcs_id, xpcs_mode);
	return xpcs_mode;
}

bool s32_serdes_is_external_clk_in_hwconfig(unsigned int id)
{
	size_t subarg_len = 0;
	bool ext = false;
	char *option_str = s32_serdes_get_serdes_hwconfig_subarg(id, "clock",
		&subarg_len);

	if (!option_str || !subarg_len)
		return false;

	if (!strncmp(option_str, "ext", subarg_len))
		ext = true;

	debug("found serdes%d ext clock %d\n", id, ext);
	return ext;
}

unsigned long s32_serdes_get_clock_fmhz_from_hwconfig(unsigned int id)
{
	size_t subarg_len = 0;
	unsigned long fmhz = MHZ_100;
	char *option_str = s32_serdes_get_serdes_hwconfig_subarg(id, "fmhz",
		&subarg_len);

	if (!option_str || !subarg_len)
		return fmhz;

	if (!strncmp(option_str, "125", subarg_len))
		fmhz = MHZ_125;

	debug("found serdes%d fmhz %lu\n", id, fmhz);
	return fmhz;
}

enum pcie_phy_mode s32_serdes_get_pcie_phy_mode_from_hwconfig(unsigned int id)
{
	enum pcie_phy_mode phy_mode = CRNS;
	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_pcie_hwconfig_subarg(id, "phy_mode",
		&subarg_len);

	if (!option_str || !subarg_len)
		return phy_mode;

	if (!strncmp(option_str, "crss", subarg_len))
		phy_mode = CRSS;
	else if (!strncmp(option_str, "srns", subarg_len))
		phy_mode = SRNS;
	else if (!strncmp(option_str, "sris", subarg_len))
		phy_mode = SRIS;

	debug("found pcie%d phy mode %d\n", id, phy_mode);
	return phy_mode;
}

enum serdes_mode s32_serdes_get_serdes_mode_from_hwconfig(unsigned int id)
{
	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_serdes_hwconfig_subarg(id, "mode",
		&subarg_len);

	/* Get the 'mode' substring for 'serdesx'. Supported values are:
	 * 'disabled', 'pcie', 'pcie&xpcsX' (X=0,1), 'xpcs0' or 'xpcs0&xpcs1' (no 'xpcs1').
	 * If 'disabled' is used, SerDes is disabled (PCIe and XPCS).
	 * If legacy support is enabled, it will get 'mode' from 'pciex'.
	 * If a SerDes module is not specified in 'hwconfig' or the configuration is invalid,
	 * return SERDES_MODE_INVAL and use default configuration from device tree.
	 */
	if (!option_str || !subarg_len)
		return SERDES_MODE_INVAL;

	if (!strncmp(option_str, "disabled", subarg_len))
		return SERDES_MODE_DISABLED;

	if (!strncmp(option_str, "pcie", subarg_len))
		return SERDES_MODE_PCIE_PCIE;

	/* SerDes mode based on XPCS: modes 1, 2 or 5 */
	if (!strncmp(option_str, "pcie&xpcs0", subarg_len))
		return SERDES_MODE_PCIE_XPCS0;
	if (!strncmp(option_str, "pcie&xpcs1", subarg_len))
		return SERDES_MODE_PCIE_XPCS1;

	/*
	 * Modes 3 or 4; Mode 4 has at least one xpcs @2G5
	 * and is selected during fixup.
	 */
	if (!strncmp(option_str, "xpcs0", subarg_len) ||
	    !strncmp(option_str, "xpcs0&xpcs1", subarg_len))
		return SERDES_MODE_XPCS0_XPCS1;

#ifdef CONFIG_S32CC_HWCONFIG_LEGACY
	/* Legacy options were 'rc', 'ep', 'sgmii' and combinations. */
	if (!strncmp(option_str, "rc", subarg_len))
		return SERDES_MODE_PCIE_PCIE;
	else if (!strncmp(option_str, "ep", subarg_len))
		return SERDES_MODE_PCIE_PCIE;
	else if (!strncmp(option_str, "rc&sgmii", subarg_len))
		return SERDES_MODE_PCIE_XPCS0;
	else if (!strncmp(option_str, "ep&sgmii", subarg_len))
		return SERDES_MODE_PCIE_XPCS0;
	else if (!strncmp(option_str, "sgmii", subarg_len))
		return SERDES_MODE_XPCS0_XPCS1;
#endif

	return SERDES_MODE_INVAL;
}

bool s32_serdes_is_mode5_enabled_in_hwconfig(unsigned int id)
{
	enum serdes_xpcs_mode xpcs1_mode;
	enum serdes_mode mode;

	xpcs1_mode = s32_serdes_get_xpcs_cfg_from_hwconfig(id, 1);
	mode = s32_serdes_get_serdes_mode_from_hwconfig(id);

	return (mode == SERDES_MODE_PCIE_XPCS1 && xpcs1_mode == SGMII_XPCS_2G5);
}

bool s32_serdes_is_cfg_valid(unsigned int id)
{
	enum serdes_xpcs_mode xpcs0_mode, xpcs1_mode, xpcs0_1_mode;
	enum serdes_mode mode;
	enum pcie_phy_mode phy_mode;
	unsigned long freq;
	bool ext_clk;
	char prefix[SERDES_NAME_SIZE + 1];

	ext_clk = s32_serdes_is_external_clk_in_hwconfig(id);
	freq = s32_serdes_get_clock_fmhz_from_hwconfig(id);
	xpcs0_mode = s32_serdes_get_xpcs_cfg_from_hwconfig(id, 0);
	xpcs1_mode = s32_serdes_get_xpcs_cfg_from_hwconfig(id, 1);
	mode = s32_serdes_get_serdes_mode_from_hwconfig(id);
	phy_mode = s32_serdes_get_pcie_phy_mode_from_hwconfig(id);
	snprintf(prefix, SERDES_NAME_SIZE, "SerDes%d: 'hwconfig':", id);

	if (mode == SERDES_MODE_INVAL) {
		printf("Invalid opmode config on SerDes%u\n", id);
		return false;
	}

	if (s32_serdes_is_pcie_mode(mode) && freq == MHZ_125) {
		printf("%s In PCIe/XPCS combo mode", prefix);
		printf(" reference clock has to be 100Mhz\n");
		return false;
	}

	if (phy_mode != CRNS && mode != SERDES_MODE_PCIE_PCIE) {
		printf("%s Only CRNS PHY mode can be used for PCIe/XPCS combo mode",
		       prefix);
		return false;
	}

	if (!ext_clk && (phy_mode == CRSS || phy_mode == SRIS)) {
		printf("%s CRSS or SRIS mode for PCIe PHY", prefix);
		printf(" cannot be used with internal clock\n");
		return false;
	}

	/* Mode 0 - Only PCIE, no XPCS config for this mode */
	if (mode == SERDES_MODE_PCIE_PCIE &&
	    xpcs0_mode != SGMII_INVALID && xpcs1_mode != SGMII_INVALID) {
		printf("%s No XPCS allowed when using PCIe mode\n", prefix);
		return false;
	}

	/* Mode 1 */
	if (mode == SERDES_MODE_PCIE_XPCS0 && xpcs0_mode != SGMII_XPCS_1G) {
		printf("%s Invalid xpcs%d_0 configuration for", prefix, id);
		printf("  PCIe/XPCS0 combo mode\n");
		return false;
	}
	if (mode == SERDES_MODE_PCIE_XPCS0 && xpcs1_mode != SGMII_INVALID) {
		printf("%s No xpcs%d_1 configuration allowed for", prefix, id);
		printf("  PCIe/XPCS0 combo mode\n");
		return false;
	}

	/* Mode 2 and 5 */
	if (mode == SERDES_MODE_PCIE_XPCS1 &&
	    !(xpcs1_mode == SGMII_XPCS_1G || xpcs1_mode == SGMII_XPCS_2G5)) {
		printf("%s Invalid xpcs%d_1 configuration for", prefix, id);
		printf("  PCIe/XPCS1 combo mode\n");
		return false;
	}
	if (mode == SERDES_MODE_PCIE_XPCS1 && xpcs0_mode != SGMII_INVALID) {
		printf("%s No xpcs%d_0 configuration allowed for", prefix, id);
		printf("  PCIe/XPCS1 combo mode\n");
		return false;
	}

	/* Modes 3 and 4 */
	/* There are multiple combinations between xpcs0 and xpcs1,
	 * including xpcs0 only or xpcs1 only, 1G or 2G5, depending on board,
	 * so do not overact and add unnecessary restrictions.
	 * It's perfectly sane to want to use only one interface or both.
	 */
	if (mode == SERDES_MODE_XPCS0_XPCS1 &&
	    xpcs0_mode == SGMII_INVALID && xpcs1_mode == SGMII_INVALID) {
		printf("%s No xpcs configured for", prefix);
		printf("  XPCS only mode\n");
		return false;
	}

	/* Combined SerDes validation. Performed for SerDes1 */
	if (id > 0) {
		/* XPCS0_1 (pfe2) and XPCS1_0 (pfe0) both configured as SGMII
		 * cannot be tested on reference S32CC EVB/RDB boards, since both
		 * are connected to the same PHY.
		 */
		xpcs0_1_mode = s32_serdes_get_xpcs_cfg_from_hwconfig(0, 1);
		if (xpcs0_mode != SGMII_INVALID && xpcs0_1_mode != SGMII_INVALID)
			printf("xpcs0_1 and xpcs 1_0 can't be both SGMII on S32CC EVB/RDB\n");
	}

	return true;
}

int s32_serdes_get_alias_id(struct udevice *serdes_dev, unsigned int *devnump)
{
	ofnode node = dev_ofnode(serdes_dev);
	const char *uc_name = "serdes";
	int ret, val = 0;

	if (ofnode_is_np(node)) {
		ret = of_alias_get_id(ofnode_to_np(node), uc_name);
		if (ret >= 0) {
			*devnump = ret;
			ret = 0;
		}
	} else {
		ret = fdtdec_get_alias_seq(gd->fdt_blob, uc_name,
					   ofnode_to_offset(node), &val);
		if (val >= 0)
			*devnump = val;
		else
			return -EINVAL;
	}

	return ret;
}

int s32_serdes_get_lane_speed(struct udevice *serdes_dev, u32 xpcs_id)
{
	int speed;
	unsigned int serdes_id = 0;
	int ret;

	if (!serdes_dev)
		return -EINVAL;

	if (xpcs_id > 1)
		return -EINVAL;

	ret = s32_serdes_get_alias_id(serdes_dev, &serdes_id);
	if (ret < 0) {
		printf("Failed to get SerDes device id for device %s:\n",
		       serdes_dev->name);
		return ret;
	}

	speed = s32_serdes_get_xpcs_speed_from_hwconfig(serdes_id, xpcs_id);
	debug("SerDes%d: hwconfig: xpcs%d_%d has speed %d\n",
	      serdes_id, serdes_id, xpcs_id, speed);

	return speed;
}
