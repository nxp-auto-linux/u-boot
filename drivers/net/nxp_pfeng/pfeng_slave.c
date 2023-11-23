// SPDX-License-Identifier: GPL-2.0+
/*
 * NXP S32G PFE Ethernet accelerator (base) driver
 *
 * Copyright 2023 NXP
 */

#define LOG_CATEGORY UCLASS_ETH

#include <common.h>
#include <command.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/ioport.h>

#include "pfeng.h"

#define HIF_ID_NOT_SET 255

static int pfeng_eval_mem_limit(phys_addr_t base)
{
	if (base > U32_MAX)
		return -EINVAL;

	return 0;
}

static int reserved_bdr_pool_region_init(struct udevice *dev, struct resource *res)
{
	struct ofnode_phandle_args phandle;
	int index;
	int ret;

	index = ofnode_stringlist_search(dev_ofnode(dev), "memory-region-names", "pfe-bdr-pool");
	if (index < 0) {
		dev_warn(dev, "Memory region 'pfe-bdr-pool' not found'\n");
		goto out;
	}

	ret = ofnode_parse_phandle_with_args(dev_ofnode(dev), "memory-region",
					     NULL, 0, index, &phandle);
	if (ret) {
		dev_warn(dev, "Memory region node with index %d not found'\n", index);
		goto out;
	}

	if (!ofnode_device_is_compatible(phandle.node,
					 "nxp,s32g-pfe-bdr-pool")) {
		dev_warn(dev, "nxp,s32g-pfe-bdr-pool node missing\n");
		goto out;
	}

	if (!ofnode_is_enabled(phandle.node)) {
		dev_warn(dev, "Node %s is not enabled\n",
			 ofnode_get_name(phandle.node));
		goto out;
	}

	ret = ofnode_read_resource(phandle.node, 0, res);
	if (ret) {
		dev_warn(dev, "PFE: Failed to read region from node: %s\n",
			 ofnode_get_name(phandle.node));
		goto out;
	}

	dev_dbg(dev, "pfe-bdr-pool addr 0x%llx size 0x%llx\n", res->start, resource_size(res));

	return ret;

out:
	dev_warn(dev, "Allocate BDRs in non-cacheable memory\n");

	return -EINVAL;
}

static int get_pfe_hif_channel(struct udevice *dev, const char *name, u8 *hif_chan)
{
	ofnode node = dev_ofnode(dev);
	u32 val;
	int ret;

	ret =  ofnode_read_u32(node, name, &val);
	if (ret) {
		dev_err(dev, "Missing '%s'\n", name);
		return ret;
	}

	if (val > PFE_HIF_CHANNEL_3) {
		dev_err(dev, "Invalid value '%s': %u\n", name, val);
		return -EINVAL;
	}

	*hif_chan = val;

	return 0;
}

static int pfeng_of_to_plat(struct udevice *dev)
{
	struct pfeng_cfg *cfg = dev_get_plat(dev);
	struct resource res = {0};
	ofnode node;
	int ret;

	if (cfg->parsed)
		return 0;

	cfg->dev = dev;
	cfg->hif_id = HIF_ID_NOT_SET;

	ret = dev_read_resource_byname(dev, "pfe-cbus", &res);
	if (ret < 0) {
		dev_err(dev, "Missing reg 'pfe-cbus'\n");
		return ret;
	}

	cfg->csr_phys_addr = res.start;
	cfg->csr_size = resource_size(&res);
	ret = pfeng_eval_mem_limit(cfg->csr_phys_addr);
	if (ret) {
		dev_err(dev, "Invalid reg 'pfe-cbus'\n");
		return ret;
	}

	log_debug("DEB: CSR base %llx\n", cfg->csr_phys_addr);

	ret = reserved_bdr_pool_region_init(dev, &res);
	if (!ret) {
		cfg->bdrs_addr = res.start;
		cfg->bdrs_size = resource_size(&res);
	} else {
		cfg->bdrs_addr = 0ULL;
	}

	ret = pfeng_eval_mem_limit(cfg->bdrs_addr);
	if (ret) {
		dev_err(dev, "Invalid value 'pfe-bdr-pool'\n");
		return ret;
	}

	ret = get_pfe_hif_channel(dev, "nxp,pfeng-ihc-channel", &cfg->ihc_hif_chnl);
	if (ret)
		return ret;

	ret = get_pfe_hif_channel(dev, "nxp,pfeng-master-channel", &cfg->master_hif_chnl);
	if (ret)
		return ret;

	cfg->parsed = true;
	/* Collect all used netifs */
	ofnode_for_each_subnode(node, dev_ofnode(dev)) {
		if (ofnode_device_is_compatible(node, "nxp,s32g-pfe-netif") &&
		    ofnode_is_enabled(node)) {
			struct udevice *devp = NULL;

			ret = device_find_global_by_ofnode(node, &devp);
			if (ret) {
				dev_err(dev, "Failed to get the %s child device\n",
					ofnode_get_name(node));
				return ret;
			}

			/* trigger parsing of pfe netifs before probing the base pfe device */
			ret = device_of_to_plat(devp);
			if (ret) {
				dev_err(dev, "Failed to parse the %s child device\n",
					ofnode_get_name(node));
				return ret;
			}
		}
	}

	return 0;
}

static int pfeng_init_hardware(struct pfeng_priv *priv)
{
	struct pfe_hw_cfg *hw_cfg = &priv->pfe_hw_cfg;
	int ret;

	hw_cfg->dev = priv->cfg->dev;

	hw_cfg->cbus_base = priv->cfg->csr_phys_addr;
	hw_cfg->bdrs_addr = priv->cfg->bdrs_addr;
	hw_cfg->bdrs_size = priv->cfg->bdrs_size;
	ret = pfe_hw_init(&priv->pfe_hw, hw_cfg);
	if (ret) {
		dev_err(priv->cfg->dev, "Could not init PFE platform\n");
		return ret;
	}

	priv->pfe_hw.hw_chnl = NULL;
	priv->pfe_hw.in_grace_reset = false;

	return 0;
}

static void pfeng_gpr_global_deinit(struct pfeng_priv *priv)
{
	/* Clear PFE HIF coherency */
	if (IS_ENABLED(CONFIG_NXP_PFENG_SLAVE_MANAGE_COHERENCY))
		(void)pfeng_clear_port_coherency_nvmem(priv->cfg->dev);
}

static int pfeng_probe(struct udevice *dev)
{
	const struct pfeng_cfg *cfg = dev_get_plat(dev);
	struct pfeng_priv *priv = dev_get_priv(dev);

	priv->cfg = cfg;

	/* Prepare HW config for platform code */

	priv->csr_base = map_physmem(cfg->csr_phys_addr, cfg->csr_size, MAP_NOCACHE);

	/* Init PFE HW */
	priv->pfe_hw_cfg.hif_chnl_id = cfg->hif_id;
	priv->pfe_hw_cfg.ihc_hif_id = cfg->ihc_hif_chnl + PFENG_HIF0;
	priv->pfe_hw_cfg.master_hif_id = cfg->master_hif_chnl + PFENG_HIF0;

	return pfeng_init_hardware(priv);
}

static int pfeng_remove(struct udevice *dev)
{
	struct pfeng_cfg *cfg = dev_get_plat(dev);
	struct pfeng_priv *priv = dev_get_priv(dev);
	int i;
	int ret = 0;

	/* Wait until all netdevs get deregistered */
	for (i = 0; i < ARRAY_SIZE(cfg->netdevs); i++)
		if (cfg->netdevs[i])
			return 0;

	if (priv->pfe_hw.hw_chnl &&
	    (priv->pfe_hw.hw_chnl_state == PFE_HW_CHNL_CREATED ||
	     priv->pfe_hw.hw_chnl_state == PFE_HW_CHNL_READY))
		ret = pfe_hw_grace_reset(&priv->pfe_hw);

	/* GPR settings */
	pfeng_gpr_global_deinit(priv);

	unmap_physmem(priv->csr_base, MAP_NOCACHE);

	pfe_hw_hif_chnl_destroy(&priv->pfe_hw);
	pfe_hw_remove(&priv->pfe_hw);

	return ret;
}

static int pfeng_bind(struct udevice *dev)
{
	return device_set_name(dev, "pfeng-base");
}

static const struct udevice_id pfeng_ids[] = {
	{ .compatible = "nxp,s32g-pfe-slave" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(pfeng) = {
	.name		= "pfeng",
	.id		= UCLASS_MISC,
	.of_match	= pfeng_ids,
	.bind		= pfeng_bind,
	.probe		= pfeng_probe,
	.remove		= pfeng_remove,
	.of_to_plat	= pfeng_of_to_plat,
	.plat_auto	= sizeof(struct pfeng_cfg),
	.priv_auto	= sizeof(struct pfeng_priv),
	.flags		= DM_FLAG_OS_PREPARE,
};

static bool is_pfe_ip_ready(struct udevice *dev, int ip_check_err, bool ip_is_ready, u32 hif_id)
{
	if (ip_check_err) {
		dev_err(dev, "Failed to get PFE IP ready state\n");
		return false;
	}

	if (ip_is_ready) {
		printf("PFE IP is ready\n");
		printf("HIF channel: %d\n", hif_id);
		return true;
	}

	printf("PFE IP is not ready\n");

	return false;
}

static int do_pfeng_cmd(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct pfeng_netif_cfg;
	struct pfeng_priv *priv;
	struct pfeng_cfg *cfg;
	struct udevice *dev = NULL;
	bool ip_is_ready = false;
	char cmd = 's';
	int ip_check_err;
	int ret;

	if (argc > 1)
		cmd = argv[1][0];

	ret = uclass_get_device_by_name(UCLASS_MISC, "pfeng-base", &dev);
	if (ret) {
		printf("ERR: no dev found!\n");
		return -EINVAL;
	}
	cfg = dev_get_plat(dev);
	priv = dev_get_priv(dev);

	ip_check_err = pfeng_is_ip_ready_get_nvmem_cell(dev, &ip_is_ready);

	switch (cmd) {
	case 's':
		if (is_pfe_ip_ready(dev, ip_check_err, ip_is_ready, cfg->hif_id))
			pfe_hw_print_stats(&priv->pfe_hw);
		break;
	case 'r':
		if (is_pfe_ip_ready(dev, ip_check_err, ip_is_ready, cfg->hif_id))
			ret = pfe_hw_grace_reset(&priv->pfe_hw);
		break;
	default:
		ret = CMD_RET_USAGE;
		break;
	}

	return ret;
}

U_BOOT_CMD(pfeng, 2, 1, do_pfeng_cmd,
	   "NXP S32G Slave PFE accelerator",
	   "\n"
	   "	- Print various H/W statistics\n"
	   "pfeng stat\n"
	   "	- Print various H/W statistics\n"
	   "pfeng reset\n"
	   "	- Force PFE HIF channel reset\n"
	   "	  HIF channel will be relinquished in the current u-boot session");
