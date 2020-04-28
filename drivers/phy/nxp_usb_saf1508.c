// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2020 NXP */
#include <common.h>
#include <dm.h>
#include <generic-phy.h>
#include <linux/printk.h>
#include <usb/ehci-ci.h>
#include <usb/ulpi.h>

struct nxp_phy_priv {
	struct ulpi_viewport viewport;
};

static int nxp_phy_power_on(struct phy *phy)
{
	int ret;
	struct nxp_phy_priv *priv = dev_get_priv(phy->dev);

	ret = ulpi_init(&priv->viewport);
	if (ret) {
		pr_err("Failed to initialize SAF1508 PHY\n");
		return ret;
	}

	ret = ulpi_select_transceiver(&priv->viewport, ULPI_FC_FULL_SPEED);
	if (ret) {
		pr_err("Failed to set speed of SAF1508\n");
		return ret;
	}

	ret = ulpi_set_pd(&priv->viewport, 1);
	if (ret) {
		pr_err("Failed to set pull-up resitors\n");
		return ret;
	}

	ret = ulpi_set_vbus_indicator(&priv->viewport, 1, 1, 1);
	if (ret) {
		pr_err("Failed to set VBUS indicator\n");
		return ret;
	}

	ret = ulpi_set_vbus(&priv->viewport, 1, 1);
	if (ret) {
		pr_err("Failed to set VBUS\n");
		return ret;
	}

	/* Clear ULPI_OTG_DRVVBUS */
	ret = ulpi_set_vbus(&priv->viewport, 0, 0);
	if (ret) {
		pr_err("Failed to set VBUS\n");
		return ret;
	}

	return 0;
}

static int nxp_phy_probe(struct udevice *dev)
{
	struct nxp_phy_priv *priv = dev_get_priv(dev);
	/* USB IP */
	struct usb_ehci *ehci;
	void __iomem *regs;

	log_info("USB PHY: SAF1508BET\n");
	regs = dev_remap_addr(dev);
	if (!regs)
		return -EINVAL;

	ehci = (struct usb_ehci *)regs;

	priv->viewport.port_num = 0;
	priv->viewport.viewport_addr = (phys_addr_t)&ehci->ulpi_viewpoint;

	return 0;
}

static const struct udevice_id nxp_phy_ids[] = {
	{ .compatible = "nxp,saf1508bet" },
	{ }
};

static struct phy_ops nxp_phy_ops = {
	.power_on = nxp_phy_power_on,
};

U_BOOT_DRIVER(nxp_usb_phy) = {
	.name = "saf1508bet",
	.id = UCLASS_PHY,
	.of_match = nxp_phy_ids,
	.probe = nxp_phy_probe,
	.ops = &nxp_phy_ops,
	.priv_auto = sizeof(struct nxp_phy_priv),
};
