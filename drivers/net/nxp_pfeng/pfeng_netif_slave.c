// SPDX-License-Identifier: GPL-2.0+
/*
 * NXP S32G PFE Ethernet port (netif) driver
 *
 * Copyright 2023 NXP
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

static int pfeng_netif_start(struct udevice *dev)
{
	return 0;
}

static void pfeng_netif_stop(struct udevice *dev)
{
}

static int pfeng_netif_send(struct udevice *dev, void *packet, int length)
{
	struct pfeng_netif *netif = dev_get_priv(dev);

	log_debug("pfeng:%s: TX phyif#%d len %d\n", netif->cfg->name, netif->cfg->phyif, length);

	return 0;
}

static int pfeng_netif_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct pfeng_netif *netif = dev_get_priv(dev);

	log_debug("pfeng:%s: RX phyif#%d\n", netif->cfg->name, netif->cfg->phyif);

	return 0;
}

static int pfeng_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct pfeng_netif *netif = dev_get_priv(dev);

	if (!packet)
		return 0;

	log_debug("pfeng:%s: FREE phyif#%d len %d\n", netif->cfg->name, netif->cfg->phyif, length);

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

static int pfeng_netif_remove(struct udevice *dev)
{
	struct eth_pdata *eth_pdata = dev_get_plat(dev);

	/* clear netif device reference in parent */
	pfeng_netif_unregister(dev);

	eth_pdata->priv_pdata = NULL;

	return 0;
}

static int pfeng_netif_probe(struct udevice *dev)
{
	const struct pfeng_netif_cfg *cfg = pfeng_get_plat(dev);
	struct pfeng_netif *netif = dev_get_priv(dev);

	netif->cfg = cfg;

	/* keep reference to pfeng base driver priv */
	netif->priv = dev_get_priv(dev->parent);

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

	if (ret > PFENG_HIF3 || (ret > PFENG_EMAC2 && ret < PFENG_HIF0)) {
		dev_err(dev, "PHYIF id ot of range: %d\n", ret);
		return -EINVAL;
	}

	cfg->phyif = ret;

	/* Netdev name */
	cfg->name = dev_read_string(dev, "nxp,pfeng-if-name");
	if (!cfg->name || !strlen(cfg->name)) {
		dev_warn(dev, "Failed to read if-name, '%s' name is used\n", dev->name);
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

	/* register netif device reference with parent */
	pfeng_netif_register(dev);

	return 0;
}

static int pfeng_netif_bind(struct udevice *dev)
{
	struct eth_pdata *eth_pdata = dev_get_plat(dev);
	ofnode node = dev_ofnode(dev);
	struct pfeng_netif_cfg *cfg;
	int ret, id;
	char name[16];

	if (!ofnode_is_np(node))
		return -ENODEV;

	id = of_alias_get_id(ofnode_to_np(node), "ethernet");
	if (id < 0)
		return id;

	if (!dev_read_enabled(dev))
		return 0;

	cfg = devm_kzalloc(dev, sizeof(*cfg), GFP_KERNEL);
	if (!cfg)
		return -ENOMEM;

	eth_pdata->priv_pdata = cfg;
	cfg->dev = dev;
	cfg->netif_id = (u8)id;

	ret = snprintf(name, sizeof(name), "pfesl%u", id);
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
