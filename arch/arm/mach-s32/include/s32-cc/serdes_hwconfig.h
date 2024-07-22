/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2021-2024 NXP */

#ifndef SERDES_HWCONFIG_H
#define SERDES_HWCONFIG_H

#include <generic-phy.h>
#include <nxp-s32cc-pcie-phy-submode.h>
#include <linux/types.h>

#define SERDES_CLK_SUBARG	"clock"
#define   SERDES_EXT_CLK	"ext"
#define   SERDES_INT_CLK	"int"

#define KHZ			(1000)
#define MHZ			(1000 * KHZ)
#define MHZ_100			(100 * MHZ)
#define MHZ_125			(125 * MHZ)

#define SPEED_UNKNOWN		(-1)

/* Same as in ethtool.h */
/* The forced speed, 10Mb, 100Mb, gigabit, 2.5Gb, 10GbE. */
#define SPEED_10		10
#define SPEED_100		100
#define SPEED_1000		1000
#define SPEED_2500		2500
#define SPEED_10000		10000

#define PCIE_COMPATIBLE_RC	"nxp,s32cc-pcie"
#define PCIE_COMPATIBLE_EP	"nxp,s32cc-pcie-ep"

/**
 * @brief	SerDes Subsystem Modes
 *
 * SS_RW_REG_0[SUBSYS_MODE] Based on S32G2RSERDESRM Rev5 / S32G3
 *
 * 000b - PCIe Gen3 X2 mode
 * 001b - PCIe Gen3 X1 and SGMII XPCS0 1G bifurcation mode
 * 010b - PCIe Gen3 X1 and SGMII XPCS1 1G bifurcation mode
 * 011b - Two SGMII 1G/2G5 bifurcation mode, PHY control XPCS0
 * 100b - Two SGMII 1G/2G5 bifurcation mode, PHY control XPCS1
 *
 * Note: PCIe GenX means all speeds from Gen1 to GenX
 * Note: Values for SUBSYS_MODE above are not 1:1 match with SerDes Modes below.
 *
 * Based on S32G2RM Rev5 / S32G3RM Rev2:
 *
 * SerDes_0 working modes
 *
 * Mode Num - Name   PCIe     XPCS0     XPCS1     PHY        PHY        PHY Clk
 *                                                lane 0     lane 1       (MHz)
 * ----------------------------------------------------------------------------
 * 0 - PCIe only     X2 Gen3  N/A       N/A       PCIe Gen3  PCIe Gen3      100
 *
 * 1 - PCIe/SGMII    X1 Gen3  SGMII 1G  N/A       PCIe Gen3  XPCS0 1G       100
 *     bifurcation                                           (GMAC0)
 *
 * 2 - PCIe/SGMII    X1 Gen3    N/A     SGMII 1G  PCIe Gen3  XPCS1 1G       100
 *     bifurcation   X1 Gen2(^) N/A     SGMII 2G5 PCIe Gen2  XPCS1 2G5      100
 *                                                           (PFE_MAC2)
 *
 * 3 - SGMII only    N/A      SGMII 1G  SGMII 1G  XPCS0 1G   XPCS1 1G   100/125
 *                                                (GMAC0)    (PFE_MAC2)
 *
 * SerDes_1 working modes
 *
 * Mode Num - Name   PCIe     XPCS0     XPCS1     PHY        PHY        PHY Clk
 *                                                lane 0     lane 1       (MHz)
 * ----------------------------------------------------------------------------
 * 0 - PCIe only     X2 Gen3  N/A       N/A       PCIe Gen3  PCIe Gen3      100
 *
 * 1 - PCIe/SGMII    X1 Gen3  SGMII 1G  N/A       PCIe Gen3  XPCS0 1G       100
 *     bifurcation                                           (PFE_MAC0)
 *
 * 2 - PCIe/SGMII    X1 Gen3    N/A     SGMII 1G  PCIe Gen3  XPCS1 1G       100
 *     bifurcation   X1 Gen2(^) N/A     SGMII 2G5 PCIe Gen2  XPCS1 2G5      100
 *                                                           (PFE_MAC1)
 *
 * 3 - SGMII only    N/A      SGMII 1G  SGMII 1G  XPCS0 1G   XPCS1 1G   100/125
 *                                                (PFE_MAC0) (PFE_MAC1)
 *
 * 4 - SGMII only(^) N/A      SGMII 2G5 SGMII 1G  XPCS0 2G5  XPCS1 1G   100/125
 *                   N/A      SGMII 1G  SGMII 2G5 XPCS0 1G   XPCS1 2G5  100/125
 *                   N/A      SGMII 2G5 SGMII 2G5 XPCS0 2G5  XPCS1 2G5      125
 *                                                (PFE_MAC0) (PFE_MAC1)
 * Notes (^):
 * Mode 2 - PCIe Gen2 on lane 0 and SGMII XPCS1 2G5 on lane 1 is supported only
 *          for S32G3. To disambiguate, we'll call this second configuration of
 *          Mode 2 as Mode 5.
 * Mode 4 - Fully supported only on S32G3 (SerDes1). To disambiguate, we'll
 *          call the three Mode 4 configuations as 4.1, 4.2, 4.3.
 *          On S32G2, there is one limited flavor of configuration 4.3, running
 *          at 125 MHz, with only XPCS0 2G5 on lane 0 and nothing on lane 1.
 *
 *
 */
enum serdes_mode {
	SERDES_MODE_INVAL = -2,
	/*	Disabled. Disable PCIe and XPCS interfaces connected to the SerDes */
	SERDES_MODE_DISABLED = -1,
	/*	Lane0=PCIe, Lane1=PCIe */
	SERDES_MODE_PCIE_PCIE = 0,
	/*	Lane0=PCIe, Lane1=SGMII(XPCS0) (1G) */
	SERDES_MODE_PCIE_XPCS0 = 1,
	/*	Lane0=PCIe, Lane1=SGMII(XPCS1) (1G) */
	SERDES_MODE_PCIE_XPCS1 = 2,
	/*
	 * True Mode 3: Lane0=SGMII(XPCS0) (1G), Lane1=SGMII(XPCS1) (1G)
	 * True Mode 4: Lane0=SGMII(XPCS0) (1G/2G5), Lane1=SGMII(XPCS1) (1G/2G5)
	 */
	SERDES_MODE_XPCS0_XPCS1 = 3,
	SERDES_MODE_MAX = SERDES_MODE_XPCS0_XPCS1
};

enum serdes_xpcs_mode {
	SGMII_INVALID = -1,
	SGMII_XPCS_1G = 0,
	SGMII_XPCS_2G5 = 1,
	SGMII_XPCS_LAST = SGMII_XPCS_2G5
};

enum pcie_type {
	PCIE_INVALID = 0,
	PCIE_EP = 0x1, /* EP mode is 0x0, use 0x1 to allow us to use masks */
	PCIE_RC = 0x4,
};

/* Supported link speeds for PCIe on S32CC
 * Maximum supported value is in sync with field
 * PCIE_CAP_SUPPORT_LINK_SPEED_VECTOR of register LINK_CAPABILITIES2_REG
 */
enum pcie_link_speed {
	GEN1 = 0x1,
	GEN2 = 0x2,
	GEN3 = 0x3,
	GEN_MAX = GEN3
};

/* Supported link widths for PCIe on S32CC */
enum pcie_link_width {
	X1 = 0x1,
	X2 = 0x2,
	X_MAX = X2
};

enum serdes_skip_mode {
	SERDES_NO_SKIP	= 0,		/* don't skip, apply all fixups */
	SERDES_SKIP_BOOT = BIT(0),	/* skip 'hwconfig' fixups in U-Boot */
	SERDES_SKIP_KERNEL = BIT(1),	/* skip 'hwconfig' fixups in kernel (Linux) */
	SERDES_SKIP_ALL = SERDES_SKIP_BOOT | SERDES_SKIP_KERNEL
};

enum serdes_mode s32_serdes_get_serdes_mode_from_hwconfig(unsigned int id);
bool s32_serdes_is_external_clk_in_hwconfig(unsigned int id);
unsigned long s32_serdes_get_clock_fmhz_from_hwconfig(unsigned int id);
enum serdes_skip_mode s32_serdes_get_skip_from_hwconfig(unsigned int id);
enum pcie_type s32_serdes_get_pcie_type_from_hwconfig(unsigned int id);
bool s32_serdes_get_xpcs_an_from_hwconfig(int serdes_id,
					  int xpcs_id);
int s32_serdes_get_xpcs_speed_from_hwconfig(int serdes_id,
					    int xpcs_id);
enum serdes_xpcs_mode s32_serdes_get_xpcs_cfg_from_hwconfig(int serdes_id,
							    int xpcs_id);
enum pcie_phy_mode s32_serdes_get_pcie_phy_mode_from_hwconfig(unsigned int id);
bool s32_serdes_is_cfg_valid(unsigned int id);
bool s32_serdes_is_pcie_enabled_in_hwconfig(unsigned int id);
bool s32_serdes_is_combo_mode_enabled_in_hwconfig(unsigned int id);
bool s32_serdes_is_mode5_enabled_in_hwconfig(unsigned int id);

static inline bool s32_serdes_is_pcie_mode(enum serdes_mode mode)
{
	return (mode >= SERDES_MODE_PCIE_PCIE) &&
		(mode < SERDES_MODE_XPCS0_XPCS1);
}

static inline bool s32_serdes_is_xpcs_mode(enum serdes_mode mode)
{
	return (mode > SERDES_MODE_PCIE_PCIE) &&
		(mode <= SERDES_MODE_XPCS0_XPCS1);
}

int s32_serdes_get_alias_id(struct udevice *serdes_dev, unsigned int *devnump);
int s32_serdes_get_lane_speed(struct udevice *serdes_dev, u32 lane);

#endif
