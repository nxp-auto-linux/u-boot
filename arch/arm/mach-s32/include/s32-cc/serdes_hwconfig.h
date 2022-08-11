/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2021,2022 NXP */

#ifndef SERDES_HWCONFIG_H
#define SERDES_HWCONFIG_H

#include <fsl-s32gen1-pcie-phy-submode.h>
#include <generic-phy.h>
#include <linux/types.h>

#define SERDES_CLK_SUBARG	"clock"
#define   SERDES_EXT_CLK	"ext"
#define   SERDES_INT_CLK	"int"

#define KHZ			(1000)
#define MHZ			(1000 * KHZ)
#define MHZ_100			(100 * MHZ)
#define MHZ_125			(125 * MHZ)

enum serdes_dev_type {
	SERDES_INVALID = 0,
	PCIE_EP = 0x1, /* EP mode is 0x0, use 0x1 to allow us to use masks */
	PCIE_RC = 0x4,
	SGMII = 0x10, /* outside range of PE0_GEN_CTRL_1:DEVICE_TYPE */
	/* TODO: If both PCIe/SGMII bifurcation modes are used, we may
	 * need to use 2 SGMII modes - SGMII0 and SGMII1
	 */
	SERDES_SKIP = 0x20
};

/* New enum */
enum serdes_xpcs_mode_gen2 {
	SGMII_XPCS_PCIE = 0,
	SGMII_XPCS_DISABLED,
	SGMII_XPCS_1G_OP,
	SGMII_XPCS_2G5_OP,
};

/* Old enum TODO remove*/
enum serdes_xpcs_mode {
	SGMII_INAVALID = 0,
	SGMII_XPCS0,		/* Combo mode PCIex1/SGMII(XPCS0) */
	SGMII_XPCS1,		/* Combo mode PCIex1/SGMII(XPCS1) */
	SGMII_XPCS0_XPCS1,	/* SGMII 2 x 1G mode */
	SGMII_XPCS0_2G5,	/* SGMII 2.5G mode */
	SGMII_XPCS_LAST = SGMII_XPCS0_2G5,
};

/**
 * @brief	SerDes Subsystem Modes
 *
 * Based on doc from Synopsys:
 *
 * 000b - PCIe Gen3x2 mode
 * 001b - PCIe Gen3x1 and SGMII 1G bifurcation mode
 * 010b - PCIe Gen3x1 and SGMII 1G bifurcation mode
 * 011b - Two SGMII 1G/2.5G bifurcation mode
 *
 * SerDes_0 working modes
 * Mode Num-Name  PCIe        XPCS0     XPCS1     PHY        PHY        PHY Clk
 *                                                lane 0     lane 1     (MHz)
 * 0-PCIe only    X2 Gen3     N/A       N/A       PCIe Gen3  PCIe Gen3  100
 * 1-PCIe/SGMII   PCIe X1     SGMII     N/A       PCIe       SGMII      100
 * bifurcation    Gen2/3      1.25Gbps            Gen2/3     1.25Gbps
 *                                                           (GMAC0)
 * 2-PCIe/SGMII   PCIe X1     N/A       SGMII     PCIe       SGMII      100
 * bifurcation    Gen2/3                1.25Gbps  Gen2/3     1.25Gbps   100
 *                                                           (PFE_EMAC2)
 * 3-SGMII only   N/A         SGMII     SGMII     SGMII      SGMII      100/125
 *                            1.25Gbps  1.25Gbps  1.25Gbps   1.25Gbps
 *                            3.125Gbps 3.125Gbps 3.125Gbps  3.125Gbps
 *                                                (GMAC0)    (PFE_EMAC2)
 *
 * SerDes_1 working modes
 * Mode Num-Name  PCIe        XPCS0     XPCS1     PHY        PHY        PHY Clk
 *                                                lane 0     lane 1     (MHz)
 * 0-PCIe only    X2 Gen3     N/A       N/A       PCIe Gen3  PCIe Gen3  100
 * 1-PCIe/SGMII   PCIe X1     SGMII     N/A       PCIe       SGMII      100
 * bifurcation    Gen2/3      1.25Gbps            Gen2/3     1.25Gbps
 *                                                          (PFE_MAC0/PFE_MAC1)
 * 2-PCIe/SGMII   PCIe X1     N/A       SGMII     PCIe       SGMII      100
 * bifurcation    Gen2/3                1.25Gbps  Gen2/3     1.25Gbps   100
 *                                                          (PFE_MAC0/PFE_MAC1)
 * 3-SGMII only   N/A         SGMII     SGMII     SGMII      SGMII      100/125
 *                            1.25Gbps  1.25Gbps  1.25Gbps   1.25Gbps
 *                            3.125Gbps 3.125Gbps 3.125Gbps  3.125Gbps
 *                                                (PFE_MAC0) (PFE_MAC0)
 * TODO: Investigate which of the two PCIe/SGMII bifurcation modes are
 * actually supported by S32G.
 */
enum serdes_mode {
	SERDES_MODE_INVAL = -1,
	/*	Lane0=PCIe, Lane1=PCIe */
	SERDES_MODE_PCIE_PCIE = 0,
	/*	Lane0=PCIe, Lane1=SGMII(XPCS0) (1G) */
	SERDES_MODE_PCIE_SGMII0 = 1,
	/*	Lane0=PCIe, Lane1=SGMII(XPCS1) (1G) */
	SERDES_MODE_PCIE_SGMII1 = 2,
	/*	Lane0=SGMII(1G/2.5G), Lane1=SGMII(1G/2.5G) */
	SERDES_MODE_SGMII_SGMII = 3,
	/*	Lane0=SGMII(1G/2.5G), Lane1=SGMII(1G/2.5G) */
	SERDES_MODE_SGMII_SGMII_ALT = 4,
	SERDES_MODE_MAX = SERDES_MODE_SGMII_SGMII_ALT
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

enum serdes_dev_type s32_serdes_get_mode_from_hwconfig(int id);
bool s32_serdes_is_external_clk_in_hwconfig(int i);
unsigned long s32_serdes_get_clock_fmhz_from_hwconfig(int id);
enum serdes_xpcs_mode s32_serdes_get_xpcs_cfg_from_hwconfig(int id);
enum serdes_mode s32_serdes_get_op_mode_from_hwconfig(int id);
enum pcie_phy_mode s32_serdes_get_phy_mode_from_hwconfig(int id);
bool s32_serdes_is_cfg_valid(int id);
bool s32_serdes_is_pcie_enabled_in_hwconfig(int id);
bool s32_serdes_is_combo_mode_enabled_in_hwconfig(int id);
bool s32_serdes_is_mode5_enabled_in_hwconfig(int id);
int apply_dm_hwconfig_fixups(void);
int apply_fdt_hwconfig_fixups(void *blob);
int s32_serdes_get_alias_id(struct udevice *serdes_dev, int *devnump);
int s32_serdes_get_lane_speed(struct udevice *serdes_dev, u32 lane);

#endif
