// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019-2022 NXP
 */
#include <common.h>
#include <clk.h>
#include <dm.h>
#include <fdt_support.h>
#include <hwconfig.h>
#include <malloc.h>
#include <net.h>
#include <phy.h>
#include <s32gen1_clk_utils.h>
#include <s32gen1_gmac_utils.h>
#include <asm/io.h>
#include <asm/types.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#ifndef CONFIG_DM_ETH
#include <netdev.h>
#endif
#if CONFIG_IS_ENABLED(DWC_ETH_QOS_S32CC)
#include <dm/platform_data/dwc_eth_qos_dm.h>
#endif
#if CONFIG_IS_ENABLED(FSL_PFENG)
#include <dm/platform_data/pfeng_dm_eth.h>
#endif

#if CONFIG_IS_ENABLED(FSL_PFENG)
static void ft_update_eth_addr_by_name(const char *name, const u8 idx,
				       void *fdt, int nodeoff)
{
	u8 ea[ARP_HLEN];

	if (eth_env_get_enetaddr_by_index(name, idx, ea)) {
		fdt_setprop(fdt, nodeoff, "local-mac-address", ea, ARP_HLEN);
		printf("   fixup: %s%i set to %pM\n", name, idx, ea);
	}
}
#endif

__maybe_unused static bool intf_is_xmii(u32 intf)
{
	return intf == PHY_INTERFACE_MODE_MII ||
		intf == PHY_INTERFACE_MODE_RMII ||
		intf == PHY_INTERFACE_MODE_RGMII;
}

#if CONFIG_IS_ENABLED(FSL_PFENG)
#ifdef CONFIG_NXP_S32GRDB_BOARD
static int get_phy_handle(const void *fdt, int nodeoffset)
{
	const int *php;
	int len;

	php = fdt_getprop(fdt, nodeoffset, "phy-handle", &len);
	if (!php || len != sizeof(*php))
		return -1;

	return fdt32_to_cpu(*php);
}

static void ft_enet_pfe_fixup_phy(u32 idx, void *fdt, int nodeoff)
{
	int phy_handle;
	char env_name[32];
	char *phy_addr_str;
	u32 phy_addr;
	int phy_nodeoff;

	snprintf(env_name, sizeof(env_name), "pfe%d_phy_addr", idx);

	phy_addr_str = env_get(env_name);
	phy_handle = get_phy_handle(fdt, nodeoff);

	if (!phy_addr_str || phy_handle == -1)
		return;

	phy_addr = (unsigned int)simple_strtoul(phy_addr_str, NULL, 16);
	phy_nodeoff = fdt_node_offset_by_phandle(fdt, phy_handle);

	if (phy_nodeoff >= 0) {
		fdt_setprop_u32(fdt, phy_nodeoff, "reg", phy_addr);
		printf("   fixup: pfe%d: update phy addr to 0x%x\n", idx, phy_addr);
	}
}

static void ft_enet_pfe_fixup_fixed_link(u32 idx, void *fdt, int nodeoff)
{
	int fixedoff = -1;

	if (pfeng_cfg_emac_get_interface(idx) != PHY_INTERFACE_MODE_SGMII)
		return;

	fixedoff = fdt_subnode_offset(fdt, nodeoff, "fixed-link");
	if (fixedoff < 0)
		return;

#if defined(CONFIG_TARGET_S32G2XXAEVB) || defined(CONFIG_TARGET_S32G274ARDB2)
	if ((hwconfig_subarg_cmp("pcie1", "mode", "sgmii") &&
	    (hwconfig_subarg_cmp("pcie1", "xpcs_mode", "both") ||
	    hwconfig_subarg_cmp("pcie1", "xpcs_mode", "0"))) ||
	    ((hwconfig_subarg_cmp("pcie1", "mode", "ep&sgmii") ||
	    hwconfig_subarg_cmp("pcie1", "mode", "rc&sgmii")) &&
	    hwconfig_subarg_cmp("pcie1", "xpcs_mode", "0"))) {
		fdt_setprop_u32(fdt, fixedoff, "speed", 1000);
		printf("   fixup: pfe%d: Update fixed-link speed to 1000Mbps\n", idx);
	}
#endif
}
#endif

static void ft_enet_pfe_fixup_netif(u32 idx, void *fdt)
{
	int nlen = 0, nodeoff = -1;
	char *ifname;
	char reqname[8];

	sprintf(reqname, "pfe%i", idx);

	while (1) {

		nodeoff = fdt_node_offset_by_compatible(fdt, nodeoff, "nxp,s32g-pfe-netif");
		if (nodeoff < 0)
			return;

		ifname = (char *)fdt_getprop(fdt, nodeoff, "nxp,pfeng-if-name", &nlen);
		if (!ifname || !nlen)
			continue;

		if (strncmp(reqname, ifname, strlen(reqname)))
			continue;

		/* sync MAC HW addr to DT [local-mac-address] */
		ft_update_eth_addr_by_name("pfe", idx, fdt, nodeoff);

#ifdef CONFIG_NXP_S32GRDB_BOARD
		/* SGMII PHY address fixup needed by RDB2 rev.D */
		if (idx == 1 &&
		    pfeng_cfg_emac_get_interface(idx) == PHY_INTERFACE_MODE_SGMII)
			ft_enet_pfe_fixup_phy(idx, fdt, nodeoff);

		if (idx == 0)
			ft_enet_pfe_fixup_fixed_link(idx, fdt, nodeoff);
#endif

		/* We are done */
		return;
	}
}
#endif

/*
 * Ethernet DT fixup before OS load
 *
 */
void ft_enet_fixup(void *fdt)
{
	int __maybe_unused nodeoff;
	bool __maybe_unused ena;

	/* PFE */
#if CONFIG_IS_ENABLED(FSL_PFENG)
	nodeoff = fdt_node_offset_by_compatible(fdt, 0, "nxp,s32g-pfe");
	if (nodeoff >= 0) {
		/* Check for interfaces and manage accordingly */
		ft_enet_pfe_fixup_netif(0, fdt);
		ft_enet_pfe_fixup_netif(1, fdt);
		ft_enet_pfe_fixup_netif(2, fdt);
	}
#endif /* CONFIG_IS_ENABLED(FSL_PFENG) */
}

/*
 * GMAC driver for common chassis
 *
 */
#if CONFIG_IS_ENABLED(DWC_ETH_QOS_S32CC)

#if !CONFIG_IS_ENABLED(OF_CONTROL)
/* driver platform data (in case of no DT) */
static struct eqos_pdata dwmac_pdata = {
	.eth = {
		/* registers base address */
		.iobase = (phys_addr_t)ETHERNET_0_BASE_ADDR,
		/* default phy mode */
		.phy_interface = PHY_INTERFACE_MODE_RGMII,
		/* max 1 Gbps */
		.max_speed = SPEED_1000,
	},
	/* vendor specific driver config */
	.config = &eqos_s32cc_config,
};
#endif /* OF_CONTROL */

/* GMAC platform specific setup */

void setup_iomux_enet_gmac(struct udevice *dev, int intf)
{
	/* configure interface specific pins */
	switch (intf) {
	case PHY_INTERFACE_MODE_SGMII:
		pinctrl_select_state(dev, "gmac_sgmii");
		break;

	case PHY_INTERFACE_MODE_RGMII:
		pinctrl_select_state(dev, "gmac_rgmii");
		break;

	case PHY_INTERFACE_MODE_RMII:
		/* TODO: pinmuxing for RMII  */
		break;

	case PHY_INTERFACE_MODE_MII:
		/* TODO: pinmuxing for MII  */
		break;

	default:
		break;
	}
}

static ulong gmac_calc_link_speed(u32 speed)
{
	switch (speed) {
	case SPEED_10:   /* 2.5MHz */
		return 2500000UL;
	case SPEED_100:  /* 25MHz */
		return 25000000UL;
	default:
	case SPEED_1000: /* 125MHz (also 325MHz for 2.5G) */
		return 125000000UL;
	}
}

static int get_gmac_clocks(u32 mode, const char **rx,
			   const char **tx)
{
	switch (mode) {
	case PHY_INTERFACE_MODE_SGMII:
		if (rx)
			*rx = "rx_sgmii";
		if (tx)
			*tx = "tx_sgmii";
		break;
	case PHY_INTERFACE_MODE_RGMII:
		if (rx)
			*rx = "rx_rgmii";
		if (tx)
			*tx = "tx_rgmii";
		break;
	case PHY_INTERFACE_MODE_RMII:
	case PHY_INTERFACE_MODE_MII:
	default:
		return -EINVAL;
	}

	return 0;
}

int set_tx_clk_enet_gmac(struct udevice *gmac_dev, u32 speed)
{
	const char *tx;
	ulong freq = gmac_calc_link_speed(speed);
	u32 mode = eqos_get_interface_s32cc(gmac_dev);
	int ret = get_gmac_clocks(mode, NULL, &tx);

	if (ret) {
		dev_err(gmac_dev, "Invalid GMAC interface: %s\n",
			phy_string_for_interface(mode));
		return -EINVAL;
	}

	if (s32gen1_set_dev_clk_rate(tx, gmac_dev, freq) != freq)
		return -EINVAL;

	if (s32gen1_enable_dev_clk(tx, gmac_dev)) {
		dev_err(gmac_dev, "Failed to enable gmac_tx clock\n");
		return -EINVAL;
	}

	return 0;
}

void setup_clocks_enet_gmac(int intf, struct udevice *gmac_dev)
{
	const char *rx, *tx;
	int ret;

	ret = get_gmac_clocks(intf, &rx, &tx);
	/* Do nothing for the interfaces that are not supported */
	if (ret)
		return;

	if (set_tx_clk_enet_gmac(gmac_dev, SPEED_1000))
		dev_err(gmac_dev, "Failed to set GMAC TX frequency\n");

	ret = s32gen1_enable_dev_clk(rx, gmac_dev);
	if (ret)
		dev_err(gmac_dev, "Failed to enable %s clock\n", rx);

	ret = s32gen1_enable_dev_clk(tx, gmac_dev);
	if (ret)
		dev_err(gmac_dev, "Failed to enable %s clock\n", tx);
}

#endif /* CONFIG_DWC_ETH_QOS_S32CC */

/*
 * PFEng driver for S32G only
 *
 */
#if CONFIG_IS_ENABLED(FSL_PFENG) && !CONFIG_IS_ENABLED(OF_CONTROL)

/* driver platform data (in case of no DT) */
static struct pfeng_pdata pfeng_platdata = {
	.eth = {
		/* registers base address */
		.iobase = (phys_addr_t)ETHERNET_0_BASE_ADDR,
		/* default phy mode */
		.phy_interface = PHY_INTERFACE_MODE_RGMII,
	},
	/* vendor specific driver config */
	.config = &pfeng_s32g274a_config,
};

#endif /* CONFIG_FSL_PFENG */

#if !CONFIG_IS_ENABLED(OF_CONTROL)
/*
 * Platform network devices
 *
 */
U_BOOT_DEVICES(s32_enet) = {
#if CONFIG_IS_ENABLED(DWC_ETH_QOS_S32CC)
	{
		.name = "eth_eqos",
		.platdata = &dwmac_pdata,
	},
#endif
#if CONFIG_IS_ENABLED(FSL_PFENG)
	{
		.name = "eth_pfeng",
		.platdata = &pfeng_platdata,
	},
#endif
};
#endif
