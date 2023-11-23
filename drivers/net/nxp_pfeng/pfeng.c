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
#include <nvmem.h>
#include <dm/device-internal.h>
#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/ioport.h>

#include "pfeng.h"

#define HIF_ID_NOT_SET 255
#define SW_RESET_DELAY 100

static const char * const pfeng_emac_mode_strings[] = {
	[PFENG_EMAC_NONE]	= "none",
	[PFENG_EMAC_MII]	= "mii",
	[PFENG_EMAC_RMII]	= "rmii",
	[PFENG_EMAC_RGMII]	= "rgmii",
	[PFENG_EMAC_SGMII]	= "sgmii",
};

void pfeng_emac_mode_to_str(enum pfeng_emac_mode mode, const char **str)
{
	if (mode < ARRAY_SIZE(pfeng_emac_mode_strings))
		*str = pfeng_emac_mode_strings[mode];
	else
		*str = pfeng_emac_mode_strings[PFENG_EMAC_NONE];
}

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
	u32 phandle, val;
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

	/* Memory regions */
	phandle = 0;
	ret = dev_read_u32(dev, "memory-region", &phandle);
	if (ret) {
		dev_err(dev, "PFE: Couldn't read 'memory-region' phandle\n");
		return ret;
	}
	node = ofnode_get_by_phandle(phandle);
	if (!ofnode_valid(node)) {
		dev_err(dev, "PFE: Couldn't find the node linked to 'memory-region'\n");
		return -ENODEV;
	}
	ret = ofnode_read_resource(node, 0, &res);
	if (ret) {
		dev_err(dev, "PFE: Failed to read region from node: %s\n",
			ofnode_get_name(node));
		return ret;
	}
	cfg->bmu_addr = res.start;
	cfg->bmu_size = resource_size(&res);
	log_debug("DEB: bmu addr 0x%llx size 0x%llx\n", cfg->bmu_addr, cfg->bmu_size);

	/* PFE IP reset */
	cfg->rst = devm_reset_control_get(dev, "pfe_part");
	if (IS_ERR(cfg->rst)) {
		dev_err(dev, "Failed to get 'pfe_part' reset control: %ld\n",
			PTR_ERR(cfg->rst));
		return PTR_ERR(cfg->rst);
	}

	cfg->parsed = true;
	/* Collect all used EMACs */
	ofnode_for_each_subnode(node, dev_ofnode(dev)) {
		if (ofnode_device_is_compatible(node, "nxp,s32g-pfe-netif") &&
		    ofnode_is_enabled(node)) {
			struct udevice *devp = NULL;

			val = PFENG_EMACS_COUNT;
			ret = ofnode_read_u32(node, "nxp,pfeng-linked-phyif", &val);
			if (!ret && val < (u32)PFENG_EMACS_COUNT)
				cfg->emacs_mask |= (1U << val) & 0xFF;

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
	log_debug("DEB: emac-mask: %x\n", cfg->emacs_mask);

	return 0;
}

static u32 emac_mode_to_gpr_intf_sel(const struct pfeng_cfg *cfg, u8 emac_id)
{
	switch (cfg->emac_int_mode[emac_id]) {
	case PFENG_EMAC_MII:
		return GPR_PFE_EMAC_IF_MII(emac_id);
	case PFENG_EMAC_RMII:
		return GPR_PFE_EMAC_IF_RMII(emac_id);
	case PFENG_EMAC_RGMII:
		return GPR_PFE_EMAC_IF_RGMII(emac_id);
	case PFENG_EMAC_SGMII:
		return GPR_PFE_EMAC_IF_SGMII(emac_id);
	default:
		return 0;
	}
}

/* Set EMAC modes */
static int pfeng_set_emacs_intf_mode(struct pfeng_priv *priv)
{
	u32 intf_sel, pfe_pwr;
	int ret;

	/* set up interfaces */
	intf_sel = emac_mode_to_gpr_intf_sel(priv->cfg, 2) |
		   emac_mode_to_gpr_intf_sel(priv->cfg, 1) |
		   emac_mode_to_gpr_intf_sel(priv->cfg, 0);

	ret = pfeng_write_nvmem_cell(priv->cfg->dev, "pfe_emacs_intf_sel",
				     intf_sel);
	if (ret) {
		dev_err(priv->cfg->dev, "Failed to set EMACs Interface Modes\n");
		return ret;
	}

	/* power down and up the EMACs */
	pfe_pwr = PFE_EMAC_PWRDWN(0) | PFE_EMAC_PWRDWN(1) | PFE_EMAC_PWRDWN(2);
	ret = pfeng_write_nvmem_cell(priv->cfg->dev, "pfe_pwr_ctrl", pfe_pwr);
	if (ret) {
		dev_err(priv->cfg->dev, "Failed to power down the EMACs\n");
		return ret;
	}

	udelay(SW_RESET_DELAY);

	pfe_pwr = 0;
	ret = pfeng_write_nvmem_cell(priv->cfg->dev, "pfe_pwr_ctrl", pfe_pwr);
	if (ret) {
		dev_err(priv->cfg->dev, "Failed to power up the EMACs\n");
		return ret;
	}

	return 0;
}

static int pfeng_init_hardware(struct pfeng_priv *priv)
{
	struct pfe_hw_cfg *hw_cfg = &priv->pfe_hw_cfg;
	int ret;

	hw_cfg->dev = priv->cfg->dev;

	hw_cfg->emac_mask = priv->cfg->emacs_mask;
	hw_cfg->cbus_base = priv->cfg->csr_phys_addr;
	hw_cfg->bmu_addr = priv->cfg->bmu_addr;
	hw_cfg->bmu_addr_size = priv->cfg->bmu_size;
	hw_cfg->csr_clk_f = priv->clk_sys_rate;
	hw_cfg->on_g3 = (priv->pfe_ver == PFE_IP_S32G3);
	hw_cfg->fw_class_data = priv->fw_class_data;
	hw_cfg->fw_class_size = priv->fw_class_size;

	ret = pfe_hw_init(&priv->pfe_hw, hw_cfg);
	if (ret) {
		dev_err(priv->cfg->dev, "Could not init PFE platform\n");
		return ret;
	}

	/* init HIF channel */
	ret = pfe_hw_hif_chnl_create(&priv->pfe_hw);
	if (ret) {
		dev_err(priv->cfg->dev, "HIF channel init failed (%d)\n", ret);
		pfe_hw_remove(&priv->pfe_hw);
		return ret;
	}

	pfe_hw_hif_chnl_rings_attach(&priv->pfe_hw);

	return 0;
}

static int pfeng_gpr_global_init(struct pfeng_priv *priv)
{
	int ret;

	/* Setup PFE HIF coherency */
	ret = pfeng_set_port_coherency_nvmem(priv->cfg->dev);
	if (ret)
		return ret;

	/* Configure working mode of EMAC interfaces */
	return pfeng_set_emacs_intf_mode(priv);
}

static int pfeng_clocks_init(struct pfeng_priv *priv)
{
	struct udevice *dev = priv->cfg->dev;
	struct clk *clk_sys, *clk_pe;
	ulong rate;
	int ret;

	clk_sys = devm_clk_get(dev, "pfe_sys");
	if (IS_ERR(clk_sys)) {
		dev_err(dev, "Failed to read clock 'pfe_sys': %ld\n", PTR_ERR(clk_sys));
		return -ENODEV;
	}

	clk_pe = devm_clk_get(dev, "pfe_pe");
	if (IS_ERR(clk_pe)) {
		dev_err(dev, "Failed to read clock 'pfe_pe': %ld\n", PTR_ERR(clk_pe));
		return -ENODEV;
	}

	ret = clk_enable(clk_sys);
	if (ret < 0) {
		dev_err(dev, "Clock 'pfe_sys' enabling failed: %d", ret);
		return ret;
	}

	rate = clk_get_rate(clk_sys);
	if (IS_ERR_VALUE(rate)) {
		dev_err(dev, "Failed to get clock 'pfe_sys' rate: %lu", rate);
		return rate;
	}

	priv->clk_sys_rate = rate;

	rate = clk_set_rate(clk_pe, rate * 2);
	if (IS_ERR_VALUE(rate)) {
		dev_err(dev, "Failed to set clock 'pfe_pe': %lu", rate);
		return rate;
	}

	ret = clk_enable(clk_pe);
	if (ret < 0) {
		dev_err(dev, "Clock 'pfe_pe' enabling failed: %d", ret);
		return ret;
	}

	return 0;
}

static int pfeng_pfe_partition_reset(const struct pfeng_cfg *cfg)
{
	int ret;

	ret = reset_assert(cfg->rst);
	if (ret) {
		dev_err(cfg->dev, "Failed to assert PFE reset: %d\n", ret);
		return ret;
	}

	udelay(SW_RESET_DELAY);
	ret = reset_deassert(cfg->rst);
	if (ret) {
		dev_err(cfg->dev, "Failed to deassert PFE reset: %d\n", ret);
		return ret;
	}
	udelay(SW_RESET_DELAY);

	return 0;
}

static int pfeng_probe(struct udevice *dev)
{
	const struct pfeng_cfg *cfg = dev_get_plat(dev);
	struct pfeng_priv *priv = dev_get_priv(dev);
	int ret;

	priv->cfg = cfg;

	/* Prepare HW config for platform code */
	priv->csr_base = map_physmem(cfg->csr_phys_addr, cfg->csr_size, MAP_NOCACHE);

	/* GPR settings */
	ret = pfeng_gpr_global_init(priv);
	if (ret)
		return ret;

	/* PFE partition reset */
	if (cfg->rst) {
		ret = pfeng_pfe_partition_reset(cfg);
		if (ret)
			return ret;
	}

	/* Clocks */
	ret = pfeng_clocks_init(priv);
	if (ret)
		return ret;

	/* Try to detect HW. Will fail if not present or unsupported */
	ret = pfe_hw_detect_version(cfg->csr_phys_addr, &priv->pfe_ver);
	if (ret)
		return ret;

	/* Load FW */
	ret = pfeng_fw_set_from_env_and_load(priv);
	if (ret < 0)
		return ret;

	/* Init PFE HW */
	ret = pfeng_init_hardware(priv);

	return ret;
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
	{ .compatible = "nxp,s32g-pfe" },
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
	struct pfeng_netif_cfg *netif_cfg;
	struct pfeng_priv *priv;
	struct udevice *netdev;
	struct pfeng_cfg *cfg;
	struct udevice *dev = NULL;
	int ret, i;

	ret = uclass_get_device_by_name(UCLASS_MISC, "pfeng-base", &dev);
	if (ret) {
		printf("ERR: no dev found!\n");
		return -EINVAL;
	}
	cfg = dev_get_plat(dev);
	priv = dev_get_priv(dev);

	printf("\n");
	printf("Silicon: %s (%04x)\n", priv->pfe_ver == PFE_IP_S32G3 ?
	       "S32G3" : priv->pfe_ver == PFE_IP_S32G2 ? "S32G2" : "<invalid>",
	       priv->pfe_ver);

	printf("EMAC modes:");
	for (i = 0; i < PFENG_EMACS_COUNT; i++)
		printf(" %d (%s);", cfg->emac_int_mode[i],
		       pfeng_emac_mode_strings[cfg->emac_int_mode[i]]);
	printf("\n");

	for (i = 0; i < ARRAY_SIZE(cfg->netdevs); i++) {
		if (!cfg->netdevs[i])
			continue;
		netdev = cfg->netdevs[i];
		netif_cfg = pfeng_get_plat(netdev);
		printf("Netif#%d interface %d (%s)\n", i,
		       netif_cfg->phy_interface, netif_cfg->phy_mode);
	}

	printf("HIF channel: %d\n", cfg->hif_id);

	pfe_hw_print_stats(&priv->pfe_hw);

	return 0;
}

U_BOOT_CMD(pfeng, 1, 0, do_pfeng_cmd,
	   "NXP S32G PFE accelerator status",
	   "pfeng\n"
	   "       - Print various H/W statistics");
