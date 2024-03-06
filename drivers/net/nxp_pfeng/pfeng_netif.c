// SPDX-License-Identifier: GPL-2.0+
/*
 * NXP S32G PFE Ethernet port (netif) driver
 *
 * Copyright 2023-2024 NXP
 */

#define LOG_CATEGORY UCLASS_ETH

#include <common.h>
#include <dm.h>
#include <miiphy.h>
#include <net.h>
#include <phy.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <linux/time.h>
#include <s32-cc/serdes_hwconfig.h>
#include <s32-cc/xpcs.h>

#include "pfeng.h"

#define PHYIF_ID_NOT_SET 255

enum pfeng_dir {TX, RX};

static bool is_phyif_emac(u8 phyif)
{
	return phyif < PFENG_EMACS_COUNT;
}

static bool is_phyif_active_emac(struct pfeng_netif *netif, u8 phyif)
{
	const struct pfe_hw_cfg *hw_cfg = pfeng_priv_get_hw_cfg(netif->priv);

	return is_emac_active(hw_cfg, phyif);
}

static int pfeng_xpcs_start_link(struct udevice *dev, struct phy *xpcs, u8 phyif)
{
	const struct s32cc_xpcs_ops *xpcs_ops;
	struct s32cc_xpcs *xpcs_dev;
	struct phylink_link_state state;
	unsigned long timeout;
	int ret;

	xpcs_ops = s32cc_xpcs_get_ops();
	if (!xpcs_ops) {
		dev_err(dev, "Failed to get XPCS ops\n");
		return -EIO;
	}

	xpcs_dev = s32cc_phy2xpcs(xpcs);
	if (!xpcs_dev) {
		dev_err(dev, "Failed to get XPCS attached to PFE%d\n", phyif);
		return -EINVAL;
	}

	timeout = timer_get_us() + USEC_PER_SEC;
	state.link = 0;
	while (!state.link && (timer_get_us() <= timeout)) {
		ret = xpcs_ops->xpcs_get_state(xpcs_dev, &state);
		if (ret) {
			dev_err(dev, "Failed to get link state of emac%d\n", phyif);
			return ret;
		}
	}

	if (!state.link) {
		dev_err(dev, "Failed to establish XPCS link on PFE%d\n", phyif);
		return -EIO;
	}

	return 0;
}

static ulong get_mac_mode_freq(const struct pfeng_cfg *cfg, u8 id)
{
	ulong freq;

	switch (cfg->emac_int_mode[id]) {
	case PFENG_EMAC_SGMII:
		freq = 125000000UL;
		break;
	case PFENG_EMAC_RGMII:
		freq = 125000000UL;
		break;
	case PFENG_EMAC_RMII:
		freq = 25000000UL;
		break;
	default:
		freq = 0;
		break;
	}

	return freq;
}

static int emac_clocks_start(struct udevice *dev)
{
	const struct pfeng_cfg *cfg = dev_get_plat(dev->parent);
	struct pfeng_netif *netif = dev_get_priv(dev);
	const char * const dir_str[] = {"tx", "rx"};
	u8 id = netif->cfg->phyif;
	char emac_clk_str[16];
	const char *mode_str;
	struct clk *emac_clk;
	ulong rate;
	int i, ret;

	/* skip unused EMACs */
	if (!dev)
		return 0;

	pfeng_emac_mode_to_str(cfg->emac_int_mode[id], &mode_str);

	for (i = 0; i < ARRAY_SIZE(dir_str); i++) {
		ret = snprintf(emac_clk_str, sizeof(emac_clk_str), "%s_%s",
			       dir_str[i], mode_str);
		if (ret <= 0 || ret >= sizeof(emac_clk_str)) {
			dev_err(dev, "Failed to parse %s clock name, ret: %d",
				dir_str[i], ret);
			return -EINVAL;
		}

		emac_clk = devm_clk_get(dev, emac_clk_str);
		if (IS_ERR(emac_clk)) {
			dev_err(dev, "Failed to read clock '%s': %ld\n",
				emac_clk_str, PTR_ERR(emac_clk));
			return PTR_ERR(emac_clk);
		}

		rate = clk_set_rate(emac_clk, get_mac_mode_freq(cfg, id));
		if (IS_ERR_VALUE(rate)) {
			ret = rate;
			dev_err(dev, "Failed to set clock '%s' rate to: %ld\n",
				emac_clk_str, get_mac_mode_freq(cfg, id));
			return ret;
		}

		/* Start EMAC clock */
		ret = clk_enable(emac_clk);
		if (ret < 0) {
			dev_err(dev, "Clock '%s' enabling failed: %d\n",
				emac_clk_str, ret);
			return ret;
		}

		if (i == TX)
			netif->tx_clk = emac_clk;

		if (i == RX)
			netif->rx_clk = emac_clk;

		log_debug("DEB: phyif#%u '%s' clock rate = %lu\n", id,
			  emac_clk_str, rate);
	}

	return 0;
}

static void emac_clocks_stop(struct udevice *dev)
{
	struct pfeng_netif *netif = dev_get_priv(dev);

	clk_disable(netif->tx_clk);
	clk_disable(netif->rx_clk);
	devm_clk_put(dev, netif->tx_clk);
	devm_clk_put(dev, netif->rx_clk);
}

static int pfeng_netif_start(struct udevice *dev)
{
	struct pfeng_netif *netif = dev_get_priv(dev);
	struct pfe_hw_ext *hw = pfeng_priv_get_hw(netif->priv);
	u8 phyif = netif->cfg->phyif;
	int ret;

	netif->hw_chnl = hw->hw_chnl;
	pfe_hw_hif_chnl_enable(netif->hw_chnl);

	if (!is_phyif_active_emac(netif, phyif))
		return 0;

	ret = emac_clocks_start(dev);
	if (ret)
		goto err;

	if (netif->xpcs) {
		ret = pfeng_xpcs_start_link(dev, netif->xpcs, phyif);
		if (ret)
			goto err;
	}

	ret = phy_startup(netif->phy);
	if (ret) {
		dev_err(dev, "PHY startup failed\n");
		goto err;
	}

	pfe_hw_emac_set_speed(hw->hw_emac[phyif], netif->phy->speed);
	pfe_hw_emac_set_duplex(hw->hw_emac[phyif], netif->phy->duplex);

	pfe_hw_emac_enable(hw->hw_emac[phyif]);

	return 0;

err:
	netif->hw_chnl = NULL;

	return ret;
}

static void pfeng_netif_stop(struct udevice *dev)
{
	struct pfeng_netif *netif = dev_get_priv(dev);
	struct pfe_hw_ext *hw = pfeng_priv_get_hw(netif->priv);
	u8 phyif = netif->cfg->phyif;

	if (!netif->hw_chnl)
		return;

	pfe_hw_hif_chnl_disable(netif->hw_chnl);

	if (!is_phyif_active_emac(netif, phyif))
		return;

	pfe_hw_emac_disable(hw->hw_emac[phyif]);

	emac_clocks_stop(dev);
}

static int pfeng_netif_send(struct udevice *dev, void *packet, int length)
{
	struct pfeng_netif *netif = dev_get_priv(dev);

	return pfe_hw_chnl_xmit(netif->hw_chnl, false, netif->cfg->phyif, packet, length);
}

static int pfeng_netif_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct pfeng_netif *netif = dev_get_priv(dev);

	return pfe_hw_chnl_receive(netif->hw_chnl, flags, true, netif->cfg->phyif, packetp);
}

static int pfeng_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct pfeng_netif *netif = dev_get_priv(dev);

	if (!packet)
		return 0;

	return pfe_hw_chnl_free_pkt(netif->hw_chnl, packet, length);
}

static bool pfeng_netif_update_mac_address(struct pfeng_netif *netif, u8 phyif,
					   struct eth_pdata *eth_pdata)
{
	struct pfe_hw_ext *hw = pfeng_priv_get_hw(netif->priv);

	if (!is_phyif_active_emac(netif, phyif))
		return false;

	pfe_hw_emac_get_addr(hw->hw_emac[phyif], eth_pdata->enetaddr);
	return true;
}

static int pfeng_netif_read_rom_hwaddr(struct udevice *dev)
{
	struct pfeng_netif *netif = dev_get_priv(dev);
	struct eth_pdata *eth_pdata = dev_get_plat(dev);
	u8 phyif = netif->cfg->phyif;

	if (!pfeng_netif_update_mac_address(netif, phyif, eth_pdata))
		return -ENODEV;

	return 0;
}

static int pfeng_netif_write_hwaddr(struct udevice *dev)
{
	struct pfeng_netif *netif = dev_get_priv(dev);
	struct eth_pdata *eth_pdata = dev_get_plat(dev);
	struct pfe_hw_ext *hw = pfeng_priv_get_hw(netif->priv);
	u8 phyif = netif->cfg->phyif;

	if (!is_phyif_active_emac(netif, phyif))
		return -ENODEV;

	pfe_hw_emac_set_addr(hw->hw_emac[phyif], eth_pdata->enetaddr);

	return 0;
}

static void pfeng_netif_unregister(struct udevice *dev)
{
	struct pfeng_netif_cfg *netif_cfg = pfeng_get_plat(dev);
	struct pfeng_cfg *pcfg = dev_get_plat(dev->parent);

	pcfg->netdevs[netif_cfg->phyif] = NULL;
}

static void pfeng_netif_register(struct udevice *dev)
{
	struct pfeng_netif_cfg *netif_cfg = pfeng_get_plat(dev);
	struct pfeng_cfg *pcfg = dev_get_plat(dev->parent);

	pcfg->netdevs[netif_cfg->phyif] = dev;
}

static int pfeng_netif_phy_config(struct udevice *dev)
{
	struct pfeng_netif *netif = dev_get_priv(dev);

	netif->phy = dm_eth_phy_connect(dev);
	if (!netif->phy) {
		dev_err(dev, "PHY device not found\n");
		return -ENODEV;
	}

	netif->phy->supported &= PHY_GBIT_FEATURES;
	netif->phy->advertising &= PHY_GBIT_FEATURES;

	return phy_config(netif->phy);
}

static int pfeng_netif_remove(struct udevice *dev)
{
	struct eth_pdata *eth_pdata = dev_get_plat(dev);
	struct pfeng_netif *netif = dev_get_priv(dev);

	kfree(netif->xpcs);
	netif->xpcs = NULL;

	/* clear netif device reference in parent */
	pfeng_netif_unregister(dev);

	eth_pdata->priv_pdata = NULL;

	return 0;
}

#define XPCS_PHY_FORMAT	"emac%d_xpcs"

static u32 get_speed_advertised(int speed)
{
	switch (speed) {
	case SPEED_10:
		return ADVERTISED_10baseT_Full;
	case SPEED_100:
		return ADVERTISED_100baseT_Full;
	case SPEED_2500:
		return ADVERTISED_2500baseT_Full;
	case SPEED_1000:
		fallthrough;
	default:
		return ADVERTISED_1000baseT_Full;
	}
}

static struct phy *pfeng_xpcs_configure_phy(struct udevice *dev, int emac_id)
{
	char xpcs_name[sizeof(XPCS_PHY_FORMAT)];
	const struct s32cc_xpcs_ops *xpcs_ops;
	struct s32cc_xpcs *xpcs_dev;
	struct phylink_link_state state;
	struct phy *xpcs;
	int phy_speed;
	int xpcs_id;
	int ret;

	xpcs = kzalloc(sizeof(*xpcs), GFP_KERNEL);
	if (!xpcs)
		return NULL;

	xpcs_ops = s32cc_xpcs_get_ops();
	if (!xpcs_ops) {
		dev_err(dev, "Failed to get XPCS ops\n");
		goto err;
	}

	ret = snprintf(xpcs_name, sizeof(xpcs_name), XPCS_PHY_FORMAT, emac_id);
	if (ret >= sizeof(xpcs_name) || ret < 0) {
		dev_err(dev, "Failed to get XPCS name\n");
		goto err;
	}

	ret = generic_phy_get_by_name(dev->parent, xpcs_name, xpcs);
	if (ret < 0 || !xpcs->dev) {
		dev_err(dev, "Failed to get '%s' PHY\n", xpcs_name);
		goto err;
	}

	xpcs_dev = s32cc_phy2xpcs(xpcs);
	if (!xpcs_dev) {
		dev_err(dev, "Failed to get XPCS instance of '%s'\n",
			xpcs_name);
		goto err;
	}

	xpcs_id = xpcs_ops->get_id(xpcs_dev);
	if (xpcs_id < 0) {
		dev_err(dev, "Failed to get XPCS id for %s\n", xpcs_name);
		goto err;
	}

	phy_speed = s32_serdes_get_lane_speed(xpcs->dev, xpcs_id);
	if (phy_speed < 0) {
		dev_err(dev, "Failed to get speed of XPCS for '%s'\n", xpcs_name);
		goto err;
	}

	if (!generic_phy_valid(xpcs)) {
		dev_err(dev, "Invalid '%s' PHY\n", xpcs_name);
		goto err;
	}

	ret = generic_phy_init(xpcs);
	if (ret) {
		dev_err(dev, "Failed to init '%s' PHY\n", xpcs_name);
		goto err;
	}

	ret = generic_phy_power_on(xpcs);
	if (ret) {
		dev_err(dev, "Failed to power on '%s' PHY\n", xpcs_name);
		goto err;
	}

	ret = generic_phy_configure(xpcs, NULL);
	if (ret) {
		dev_err(dev, "Failed to configure '%s' PHY\n", xpcs_name);
		goto err;
	}

	state.speed = phy_speed;
	state.duplex = true;
	state.advertising = get_speed_advertised(phy_speed);
	state.an_enabled = 0;
	state.an_complete = 0;
	ret = xpcs_ops->xpcs_config(xpcs_dev, &state);
	if (ret) {
		dev_err(dev, "Failed to configure '%s' PHY\n", xpcs_name);
		goto err;
	}

	return xpcs;

err:
	kfree(xpcs);
	return NULL;
}

static int pfeng_netif_probe(struct udevice *dev)
{
	const struct pfeng_netif_cfg *cfg = pfeng_get_plat(dev);
	struct pfeng_cfg *pcfg = dev_get_plat(dev->parent);
	struct pfeng_netif *netif = dev_get_priv(dev);
	int ret;

	netif->cfg = cfg;

	/* keep reference to pfeng base driver priv */
	netif->priv = dev_get_priv(dev->parent);

	if (!is_phyif_active_emac(netif, cfg->phyif))
		return 0;

	if (pcfg->emac_int_mode[cfg->phyif] == PFENG_EMAC_SGMII)
		netif->xpcs = pfeng_xpcs_configure_phy(dev, cfg->phyif);

	ret = pfeng_netif_phy_config(dev);
	if (ret) {
		dev_err(dev, "PHY config failed (%d)\n", ret);
		return ret;
	}

	return 0;
}

static enum pfeng_emac_mode phy_intf_to_pfe_emac(phy_interface_t intf)
{
	switch (intf) {
	case PHY_INTERFACE_MODE_MII:
		return PFENG_EMAC_MII;
	case PHY_INTERFACE_MODE_RMII:
		return PFENG_EMAC_RMII;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		return PFENG_EMAC_RGMII;
	case PHY_INTERFACE_MODE_SGMII:
		return PFENG_EMAC_SGMII;
	default:
		return PFENG_EMAC_NONE;
	}
}

static int netif_dt_parse_phy(struct udevice *dev)
{
	struct pfeng_cfg *pfeng_cfg = dev_get_plat(dev->parent);
	struct pfeng_netif_cfg *cfg = pfeng_get_plat(dev);
	int ret;

	cfg->phy_mode = dev_read_string(dev, "phy-mode");
	if (!cfg->phy_mode) {
		dev_err(dev, "Invalid PHY mode\n");
		return -EINVAL;
	}

	ret = phy_get_interface_by_name(cfg->phy_mode);
	if ((ret == -1) || ret >= PHY_INTERFACE_MODE_COUNT) {
		dev_err(dev, "Invalid PHY interface '%s'\n", cfg->phy_mode);
		return -EINVAL;
	}

	cfg->phy_interface = (phy_interface_t)ret;
	pfeng_cfg->emac_int_mode[cfg->phyif] = phy_intf_to_pfe_emac(cfg->phy_interface);

	return 0;
}

static int pfeng_netif_of_to_plat(struct udevice *dev)
{
	struct pfeng_netif_cfg *cfg = pfeng_get_plat(dev);
	u32 val = 0;
	int ret;

	if (!dev_read_enabled(dev))
		return 0;

	/* Linked PhyIf port */
	ret = dev_read_u32_default(dev, "nxp,pfeng-linked-phyif", PHYIF_ID_NOT_SET);
	if (ret == PHYIF_ID_NOT_SET) {
		dev_err(dev, "Failed to read PHYIF id\n");
		return -EINVAL;
	}

	if (ret >= PFENG_EMACS_COUNT) {
		dev_err(dev, "PHYIF id ot of range: %d\n", ret);
		return -EINVAL;
	}

	cfg->phyif = ret;

	/* Netdev name */
	cfg->name = dev_read_string(dev, "nxp,pfeng-if-name");
	if (!cfg->name || !strlen(cfg->name)) {
		dev_warn(dev, "Failed to read if-name\n");
		cfg->name = dev->name;
	}
	device_set_name(dev, cfg->name);

	/* NOTE: Only first HIF channel from the first netif node is used
	 *	 for all netdevs.
	 */
	ret = dev_read_u32_array(dev, "nxp,pfeng-hif-channels", &val, 1);
	if (ret) {
		dev_err(dev, "Failed to read hif-channels\n");
		return ret;
	}

	pfeng_register_first_hif_channel(dev->parent, val);

	/* Parse PHY */
	if (is_phyif_emac(cfg->phyif)) {
		ret = netif_dt_parse_phy(dev);
		if (ret)
			return ret;
	}

	/* register netif device reference with parent */
	pfeng_netif_register(dev);

	return 0;
}

static int pfeng_netif_bind(struct udevice *dev)
{
	struct eth_pdata *eth_pdata = dev_get_plat(dev);
	struct pfeng_netif_cfg *cfg;
	static int id;
	int ret;
	char name[16];

	if (id >= PFENG_EMACS_COUNT)
		return -ENODEV;

	if (!dev_read_enabled(dev)) {
		id++;
		return 0;
	}

	cfg = devm_kzalloc(dev, sizeof(*cfg), GFP_KERNEL);
	if (!cfg)
		return -ENOMEM;

	eth_pdata->priv_pdata = cfg;
	cfg->dev = dev;
	cfg->netif_id = (u8)id;

	ret = snprintf(name, sizeof(name), "pfe%u", id++);
	if (ret >= sizeof(name) || ret < 0)
		return -ENOSPC;

	return device_set_name(dev, name);
}

static const struct eth_ops pfeng_netif_eth_ops = {
	.start		= pfeng_netif_start,
	.send		= pfeng_netif_send,
	.recv		= pfeng_netif_recv,
	.free_pkt	= pfeng_free_pkt,
	.stop		= pfeng_netif_stop,
	.write_hwaddr	= pfeng_netif_write_hwaddr,
	.read_rom_hwaddr = pfeng_netif_read_rom_hwaddr,
};

static const struct udevice_id pfeng_netif_ids[] = {
	{ .compatible = "nxp,s32g-pfe-netif" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(pfeng_netif) = {
	.name		= "pfeng_netif",
	.id		= UCLASS_ETH,
	.of_match	= pfeng_netif_ids,
	.bind		= pfeng_netif_bind,
	.probe		= pfeng_netif_probe,
	.remove		= pfeng_netif_remove,
	.ops		= &pfeng_netif_eth_ops,
	.of_to_plat	= pfeng_netif_of_to_plat,
	.plat_auto	= sizeof(struct eth_pdata),
	.priv_auto	= sizeof(struct pfeng_netif),
	.flags		= DM_FLAG_ALLOC_PRIV_DMA | DM_FLAG_OS_PREPARE | DM_FLAG_ACTIVE_DMA,
};
