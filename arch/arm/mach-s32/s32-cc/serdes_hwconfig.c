// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */
#include <common.h>
#include <hwconfig.h>
#include <asm/arch/s32-cc/serdes_regs.h>
#include <s32-cc/serdes_hwconfig.h>

#define PCIE_DEFAULT_INTERNAL_CLK_FMHZ	CLK_100MHZ
#define PCIE_DEFAULT_PHY_MODE		CRNS

#define SERDES_RC_MODE_STR "RootComplex"
#define SERDES_EP_MODE_STR "EndPoint"
#define SERDES_SGMII_MODE_STR "SGMII"
#define SERDES_SGMII_MODE_NONE_STR "None"
#define MODE5_TOKEN "mode5"

#define SERDES_NAME_SIZE 32

bool is_pcie_enabled_in_hwconfig(int id)
{
	enum serdes_dev_type pcie_mode;

	pcie_mode = s32_serdes_get_mode_from_hwconfig(id);
	if ((pcie_mode & PCIE_EP) || (pcie_mode & PCIE_RC))
		return true;

	return false;
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

enum serdes_clock_fmhz s32_serdes_get_clock_fmhz_from_hwconfig(int id)
{
	enum serdes_clock_fmhz clk = PCIE_DEFAULT_INTERNAL_CLK_FMHZ;
	size_t subarg_len = 0;
	char *option_str = s32_serdes_get_hwconfig_subarg(id, "fmhz",
		&subarg_len);

	if (!option_str || !subarg_len)
		return clk;

	if (!strncmp(option_str, "100", subarg_len))
		clk = CLK_100MHZ;
	else if (!strncmp(option_str, "125", subarg_len))
		clk = CLK_125MHZ;

	return clk;
}

enum serdes_phy_mode s32_serdes_get_phy_mode_from_hwconfig(int id)
{
	enum serdes_phy_mode phy_mode = PCIE_DEFAULT_PHY_MODE;
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
	enum serdes_phy_mode ss_mode;

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

bool s32_serdes_has_mode5_enabled(int id)
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
	enum serdes_clock_fmhz freq;
	enum serdes_mode mode;
	enum serdes_phy_mode phy_mode;
	bool mode5, ext_clk;

	ext_clk = s32_serdes_is_external_clk_in_hwconfig(id);
	freq = s32_serdes_get_clock_fmhz_from_hwconfig(id);
	devtype = s32_serdes_get_mode_from_hwconfig(id);
	xpcs_mode = s32_serdes_get_xpcs_cfg_from_hwconfig(id);
	mode = s32_serdes_get_op_mode_from_hwconfig(id);
	phy_mode = s32_serdes_get_phy_mode_from_hwconfig(id);

	if (devtype == SERDES_INVALID) {
		printf("Invalid SerDes%d configuration\n", id);
		return false;
	}

	if (IS_SERDES_PCIE(devtype) && freq == CLK_125MHZ) {
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

	mode5 = s32_serdes_has_mode5_enabled(id);
	if (mode5) {
		if (!(ext_clk && freq == CLK_100MHZ &&
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
