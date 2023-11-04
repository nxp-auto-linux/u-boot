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
	ret = pfe_hw_init(&priv->pfe_hw, hw_cfg);
	if (ret) {
		dev_err(priv->cfg->dev, "Could not init PFE platform\n");
		return ret;
	}

	priv->pfe_hw.hw_chnl = NULL;

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

	return pfeng_init_hardware(priv);
}

static int pfeng_remove(struct udevice *dev)
{
	struct pfeng_cfg *cfg = dev_get_plat(dev);
	struct pfeng_priv *priv = dev_get_priv(dev);
	int i;

	/* Wait until all netdevs get deregistered */
	for (i = 0; i < ARRAY_SIZE(cfg->netdevs); i++)
		if (cfg->netdevs[i])
			return 0;

	/* GPR settings */
	pfeng_gpr_global_deinit(priv);

	unmap_physmem(priv->csr_base, MAP_NOCACHE);

	pfe_hw_hif_chnl_destroy(&priv->pfe_hw);
	pfe_hw_remove(&priv->pfe_hw);

	return 0;
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

static int do_pfeng_cmd(struct cmd_tbl *cmdtp, int flag, int argc,
			char *const argv[])
{
	struct pfeng_netif_cfg;
	struct pfeng_priv *priv;
	struct pfeng_cfg *cfg;
	struct udevice *dev = NULL;
	bool ip_is_ready;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_MISC, "pfeng-base", &dev);
	if (ret) {
		printf("ERR: no dev found!\n");
		return -EINVAL;
	}
	cfg = dev_get_plat(dev);
	priv = dev_get_priv(dev);

	ret = pfeng_is_ip_ready_get_nvmem_cell(dev, &ip_is_ready);
	if (ret) {
		dev_err(dev, "Failed to get PFE IP ready state\n");
		return ret;
	}

	if (!ip_is_ready) {
		printf("PFE IP is not ready\n");
		printf("Statistics are not available\n");
		return 0;
	}

	printf("PFE IP is ready\n");
	printf("HIF channel: %d\n", cfg->hif_id);

	pfe_hw_print_stats(&priv->pfe_hw);

	return 0;
}

U_BOOT_CMD(pfeng, 1, 0, do_pfeng_cmd,
	   "NXP S32G Slave PFE accelerator status",
	   "pfeng\n"
	   "       - Print various H/W statistics");
