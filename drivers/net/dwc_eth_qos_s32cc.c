// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2023-2024 NXP
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <eth_phy.h>
#include <nvmem.h>
#include <reset.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/pinctrl.h>
#include <linux/delay.h>
#include <s32-cc/serdes_hwconfig.h>
#include <s32-cc/xpcs.h>

#include "dwc_eth_qos.h"

#define PHY_INTF_SEL_MII	0x00
#define PHY_INTF_SEL_SGMII	0x01
#define PHY_INTF_SEL_RGMII	0x02
#define PHY_INTF_SEL_RMII	0x08

#define CLK_NAME_SZ		16

static u32 s32cc_get_speed_advertised(int speed)
{
	switch (speed) {
	case SPEED_10:
		return ADVERTISED_10baseT_Full;
	case SPEED_100:
		return ADVERTISED_100baseT_Full;
	case SPEED_2500:
		return ADVERTISED_2500baseT_Full;
	case SPEED_1000:
	default:
		return ADVERTISED_1000baseT_Full;
	}
}

static int eqos_pcs_config_s32cc(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	const struct s32cc_xpcs_ops *xpcs_ops;
	struct phylink_link_state state;
	struct s32cc_xpcs *xpcs;
	int ret, phy_speed;
	int xpcs_id;

	if (!eqos->phy)
		return 0;

	if (eqos->phy->interface != PHY_INTERFACE_MODE_SGMII)
		return 0;

	xpcs_ops = s32cc_xpcs_get_ops();
	if (!xpcs_ops) {
		dev_err(dev, "Failed to get XPCS ops\n");
		return -EIO;
	}

	xpcs = s32cc_phy2xpcs(&eqos->pcs);
	if (!xpcs) {
		dev_err(dev, "Failed to get XPCS instance of 'gmac_xpcs'\n");
		return -EINVAL;
	}

	xpcs_id = xpcs_ops->get_id(xpcs);
	if (xpcs_id < 0) {
		dev_err(dev, "Failed to get XPCS id for 'gmac_xpcs'\n");
		return xpcs_id;
	}

	phy_speed = s32_serdes_get_lane_speed(eqos->pcs.dev, xpcs_id);
	if (phy_speed < 0) {
		dev_err(dev, "Failed to get speed of XPCS for 'gmac_xpcs'");
		return phy_speed;
	}

	state.speed = eqos->phy->speed;
	state.duplex = true;
	state.advertising = s32cc_get_speed_advertised(phy_speed);
	state.an_enabled = 0;
	state.an_complete = 0;
	ret = xpcs_ops->xpcs_config(xpcs, &state);
	if (ret) {
		dev_err(dev, "Failed to configure 'gmac_xpcs' PHY\n");
		return ret;
	}

	return 0;
}

static int eqos_pcs_init_s32cc(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	ret = generic_phy_get_by_name(dev, "gmac_xpcs", &eqos->pcs);
	if (ret) {
		dev_err(dev, "Failed to get 'gmac_xpcs' PHY\n");
		return ret;
	}

	ret = generic_phy_init(&eqos->pcs);
	if (ret) {
		dev_err(dev, "Failed to init 'gmac_xpcs' PHY\n");
		return ret;
	}

	ret = generic_phy_power_on(&eqos->pcs);
	if (ret) {
		dev_err(dev, "Failed to power on 'gmac_xpcs' PHY\n");
		return ret;
	}

	ret = generic_phy_configure(&eqos->pcs, NULL);
	if (ret) {
		dev_err(dev, "Failed to configure 'gmac_xpcs' PHY\n");
		return ret;
	}

	return 0;
}

static int eqos_set_interface_mode_s32cc(struct udevice *dev, u32 mode)
{
	struct nvmem_cell c;
	int ret;

	ret = nvmem_cell_get_by_name(dev, "gmac_phy_intf_sel", &c);
	if (ret) {
		pr_err("Failed to get 'gmac_phy_intf_sel' cell\n");
		return ret;
	}

	ret = nvmem_cell_write(&c, &mode, sizeof(mode));
	if (ret) {
		pr_err("%s: Failed to write cell 'gmac_phy_intf_sel' (err = %d)\n",
		       __func__, ret);
		return ret;
	}

	return 0;
}

static int eqos_start_resets_s32cc(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	phy_interface_t interface;
	u32 mode;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	interface = eqos->config->interface(dev);
	if (interface == PHY_INTERFACE_MODE_NONE) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	/* set the interface mode */
	switch (interface) {
	case PHY_INTERFACE_MODE_SGMII:
		mode = PHY_INTF_SEL_SGMII;
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		mode = PHY_INTF_SEL_RGMII;
		break;
	case PHY_INTERFACE_MODE_RMII:
		mode = PHY_INTF_SEL_RMII;
		break;
	case PHY_INTERFACE_MODE_MII:
		mode = PHY_INTF_SEL_MII;
		break;
	default:
		pr_err("Invalid interface: %s\n",
		       phy_string_for_interface(interface));
		return -EINVAL;
	}

	ret = eqos_set_interface_mode_s32cc(dev, mode);
	if (ret)
		return ret;

	/* reset DMA to reread phyif config */
	writel(EQOS_DMA_MODE_SWR, &eqos->dma_regs->mode);
	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_restart_rx_clk_s32cc(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	if (eqos->clk_rx.enable_count) {
		debug("%s(dev=%p): rx_clk already enabled\n", __func__, dev);
		return 0;
	}

	/*
	 * M7 SRM clock driver requires a clk_set_rate call for each
	 * clock that needs to be enabled by an agent.
	 * Thus, set the rate for GMAC's RX clock before enabling it.
	 */
	ret = clk_set_rate(&eqos->clk_rx, 125 * 1000 * 1000);
	if (ret < 0) {
		pr_err("clk_set_rate(rx_clk, 125MHz) failed: %d\n", ret);
		return ret;
	}

	ret = clk_enable(&eqos->clk_rx);
	if (ret < 0) {
		pr_err("clk_enable(rx_clk) failed: %d\n", ret);
		return ret;
	}

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_set_tx_clk_speed_s32cc(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	ulong rate;
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	switch (eqos->phy->speed) {
	case SPEED_1000:
		rate = 125 * 1000 * 1000;
		break;
	case SPEED_100:
		rate = 25 * 1000 * 1000;
		break;
	case SPEED_10:
		rate = 2.5 * 1000 * 1000;
		break;
	default:
		pr_err("invalid speed %d", eqos->phy->speed);
		return -EINVAL;
	}

	ret = clk_disable(&eqos->clk_tx);
	if (ret < 0) {
		pr_err("clk_disable(tx_clk) failed: %d\n", ret);
		return ret;
	}

	ret = clk_set_rate(&eqos->clk_tx, rate);
	if (ret < 0) {
		pr_err("clk_set_rate(tx_clk, %lu) failed: %d\n", rate, ret);
		return ret;
	}

	ret = clk_enable(&eqos->clk_tx);
	if (ret < 0) {
		pr_err("clk_enable(tx_clk) failed: %d\n", ret);
		return ret;
	}

	ret = eqos_pcs_config_s32cc(dev);
	if (ret < 0) {
		pr_err("eqos_pcs_config_s32cc(dev) failed: %d\n", ret);
		return ret;
	}

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_start_clks_s32cc(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = clk_enable(&eqos->clk_master_bus);
	if (ret < 0) {
		pr_err("clk_enable(clk_master_bus) failed: %d\n", ret);
		goto err;
	}
	debug("%s(dev=%p): %lu\n", __func__, dev, clk_get_rate(&eqos->clk_master_bus));

	/*
	 * M7 SRM clock driver requires a clk_set_rate call for each
	 * clock that needs to be enabled by an agent.
	 * Thus, set the rate for GMAC's RX clock before enabling it.
	 */
	ret = clk_set_rate(&eqos->clk_rx, 125 * 1000 * 1000);
	if (ret < 0) {
		pr_err("clk_set_rate(clk_rx, 125MHz) failed: %d\n", ret);
		goto err_disable_clk_master_bus;
	}

	ret = clk_enable(&eqos->clk_rx);
	if (ret < 0) {
		pr_err("clk_enable(clk_rx) failed: %d\n", ret);
		goto err_disable_clk_master_bus;
	}

	debug("%s(dev=%p): %lu\n", __func__, dev, clk_get_rate(&eqos->clk_rx));

	ret = clk_set_rate(&eqos->clk_tx, 125 * 1000 * 1000);
	if (ret < 0) {
		pr_err("clk_set_rate(clk_tx, 125MHz) failed: %d\n", ret);
		goto err_disable_clk_rx;
	}
	ret = clk_enable(&eqos->clk_tx);
	if (ret < 0) {
		pr_err("clk_enable(clk_tx) failed: %d\n", ret);
		goto err_disable_clk_rx;
	}
	debug("%s: OK (dev=%p): %lu\n", __func__, dev,
	      clk_get_rate(&eqos->clk_tx));

	return 0;

err_disable_clk_rx:
	clk_disable(&eqos->clk_rx);
err_disable_clk_master_bus:
	clk_disable(&eqos->clk_master_bus);
err:

	/* If the phy interface is SGMII and serdes is configured in a
	 * non-xpcs mode, clock enable operations will fail. To allow the
	 * user to fix the hwconfig variable, the error code will be
	 * ignored.
	 */
	if (eqos->config->interface(dev) == PHY_INTERFACE_MODE_SGMII) {
		dev_err(dev, "Failed to start clocks. Check XPCS configuration (err=%d)\n",
			ret);
		return 0;
	}

	dev_err(dev, "Failed to start clocks (err=%d)\n", ret);
	return ret;
}

static int eqos_stop_clks_s32cc(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clk_disable(&eqos->clk_tx);
	clk_disable(&eqos->clk_rx);
	clk_disable(&eqos->clk_master_bus);

	debug("%s: OK\n", __func__);
	return 0;
}

static ulong eqos_get_tick_clk_rate_s32cc(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	return clk_get_rate(&eqos->clk_master_bus);
}

static inline void eqos_get_clock_name(char *tx_clk, char *rx_clk,
				       const char *interface_mode)
{
	strncat(tx_clk, interface_mode, CLK_NAME_SZ - 1);
	strncat(rx_clk, interface_mode, CLK_NAME_SZ - 1);
}

static int eqos_probe_resources_s32cc(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	phy_interface_t interface;
	char rx_clk[CLK_NAME_SZ] = "rx_";
	char tx_clk[CLK_NAME_SZ] = "tx_";
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	interface = eqos->config->interface(dev);

	if (interface == PHY_INTERFACE_MODE_NONE) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	switch (interface) {
	case PHY_INTERFACE_MODE_MII:
		eqos_get_clock_name(tx_clk, rx_clk, "mii");
		pinctrl_select_state(dev, "gmac_mii");
		break;
	case PHY_INTERFACE_MODE_RMII:
		eqos_get_clock_name(tx_clk, rx_clk, "rmii");
		pinctrl_select_state(dev, "gmac_rmii");
		break;
	case PHY_INTERFACE_MODE_RGMII:
	case PHY_INTERFACE_MODE_RGMII_ID:
	case PHY_INTERFACE_MODE_RGMII_RXID:
	case PHY_INTERFACE_MODE_RGMII_TXID:
		eqos_get_clock_name(tx_clk, rx_clk, "rgmii");
		pinctrl_select_state(dev, "gmac_rgmii");
		break;
	case PHY_INTERFACE_MODE_SGMII:
		eqos_get_clock_name(tx_clk, rx_clk, "sgmii");
		pinctrl_select_state(dev, "gmac_sgmii");
		break;
	default:
		pr_err("%s: interface mode not supported %s\n", dev->name,
		       phy_string_for_interface(interface));
	}

	eqos->max_speed = dev_read_u32_default(dev, "max-speed", 0);

	ret = clk_get_by_name(dev, "stmmaceth", &eqos->clk_master_bus);
	if (ret) {
		pr_err("clk_get_by_name(stmmaceth) failed: %d\n", ret);
		goto err_probe;
	}

	ret = clk_get_by_name(dev, rx_clk, &eqos->clk_rx);
	if (ret) {
		pr_err("clk_get_by_name(%s) failed: %d\n", rx_clk, ret);
		goto err_free_clk_master_bus;
	}

	ret = clk_get_by_name(dev, tx_clk, &eqos->clk_tx);
	if (ret) {
		pr_err("clk_get_by_name(%s) failed: %d\n", tx_clk, ret);
		goto err_free_clk_rx;
	}

	if (interface == PHY_INTERFACE_MODE_SGMII) {
		ret = eqos_pcs_init_s32cc(dev);
		if (ret)
			dev_err(dev,
				"XPCS init failed. Check hwconfig. (err=%d)\n",
				ret);
	}

	debug("%s: OK\n", __func__);
	return 0;

err_free_clk_rx:
	clk_free(&eqos->clk_rx);
err_free_clk_master_bus:
	clk_free(&eqos->clk_master_bus);
err_probe:

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int eqos_remove_resources_s32cc(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clk_free(&eqos->clk_tx);
	clk_free(&eqos->clk_rx);
	clk_free(&eqos->clk_master_bus);
	if (clk_valid(&eqos->clk_ck))
		clk_free(&eqos->clk_ck);

	if (dm_gpio_is_valid(&eqos->phy_reset_gpio))
		dm_gpio_free(dev, &eqos->phy_reset_gpio);

	debug("%s: OK\n", __func__);
	return 0;
}

static phy_interface_t eqos_get_interface_s32cc(struct udevice *dev)
{
	const char *phy_mode;
	phy_interface_t interface = PHY_INTERFACE_MODE_NONE;

	debug("%s(dev=%p):\n", __func__, dev);

	phy_mode = dev_read_prop(dev, "phy-mode", NULL);
	if (phy_mode)
		interface = phy_get_interface_by_name(phy_mode);

	debug("%s: OK\n", __func__);
	return interface;
}

static struct eqos_ops eqos_s32cc_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_s32cc,
	.eqos_remove_resources = eqos_remove_resources_s32cc,
	.eqos_stop_resets = eqos_null_ops,
	.eqos_start_resets = eqos_start_resets_s32cc,
	.eqos_stop_clks = eqos_stop_clks_s32cc,
	.eqos_start_clks = eqos_start_clks_s32cc,
	.eqos_calibrate_pads = eqos_null_ops,
	.eqos_disable_calibration = eqos_null_ops,
	.eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_s32cc,
	.eqos_restart_rx_clk = eqos_restart_rx_clk_s32cc,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_s32cc
};

const struct eqos_config eqos_s32cc_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 50,
	.swr_wait = 50,
	.tx_fifo_size = 20480,
	.rx_fifo_size = 20480,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_500_800,
	.axi_bus_width = EQOS_AXI_WIDTH_64,
	.interface = eqos_get_interface_s32cc,
	.ops = &eqos_s32cc_ops
};
