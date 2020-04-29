// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 * Copyright 2019-2020 NXP
 *
 */

/*
 * stm32:
 *    Synopsys GMAC 4.20 is used. And Phy mode for eval and disco is RMII
 *    with PHY Realtek RTL8211 (RGMII)
 *    We also support some other PHY config on stm32mp157c
 *    PHY_MODE (MII,GMII, RMII, RGMII) and in normal,
 *    PHY wo crystal (25Mhz and 50Mhz), No 125Mhz from PHY config
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <memalign.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <phy.h>
#include <reset.h>
#include <wait_bit.h>
#include <asm/gpio.h>
#include <asm/io.h>

#include "dwc_eth_eqos.h"

static int eqos_start_clks_stm32(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = clk_enable(&eqos->clk_master_bus);
	if (ret < 0) {
		pr_err("clk_enable(clk_master_bus) failed: %d", ret);
		goto err;
	}

	ret = clk_enable(&eqos->clk_rx);
	if (ret < 0) {
		pr_err("clk_enable(clk_rx) failed: %d", ret);
		goto err_disable_clk_master_bus;
	}

	ret = clk_enable(&eqos->clk_tx);
	if (ret < 0) {
		pr_err("clk_enable(clk_tx) failed: %d", ret);
		goto err_disable_clk_rx;
	}

	if (clk_valid(&eqos->clk_ck)) {
		ret = clk_enable(&eqos->clk_ck);
		if (ret < 0) {
			pr_err("clk_enable(clk_ck) failed: %d", ret);
			goto err_disable_clk_tx;
		}
	}

	debug("%s: OK\n", __func__);
	return 0;

err_disable_clk_tx:
	clk_disable(&eqos->clk_tx);
err_disable_clk_rx:
	clk_disable(&eqos->clk_rx);
err_disable_clk_master_bus:
	clk_disable(&eqos->clk_master_bus);
err:
	debug("%s: FAILED: %d\n", __func__, ret);
	return ret;
}

static void eqos_stop_clks_stm32(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clk_disable(&eqos->clk_tx);
	clk_disable(&eqos->clk_rx);
	clk_disable(&eqos->clk_master_bus);
	if (clk_valid(&eqos->clk_ck))
		clk_disable(&eqos->clk_ck);

	debug("%s: OK\n", __func__);
}

static int eqos_start_resets_stm32(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);
	if (dm_gpio_is_valid(&eqos->phy_reset_gpio)) {
		ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 1);
		if (ret < 0) {
			pr_err("dm_gpio_set_value(phy_reset, assert) failed: %d",
			       ret);
			return ret;
		}

		udelay(2);

		ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 0);
		if (ret < 0) {
			pr_err("dm_gpio_set_value(phy_reset, deassert) failed: %d",
			       ret);
			return ret;
		}
	}
	debug("%s: OK\n", __func__);

	return 0;
}

static int eqos_stop_resets_stm32(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	if (dm_gpio_is_valid(&eqos->phy_reset_gpio)) {
		ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 1);
		if (ret < 0) {
			pr_err("dm_gpio_set_value(phy_reset, assert) failed: %d",
			       ret);
			return ret;
		}
	}

	return 0;
}

static ulong eqos_get_tick_clk_rate_stm32(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	return clk_get_rate(&eqos->clk_master_bus);
}

static int eqos_calibrate_pads_stm32(struct udevice *dev)
{
	return 0;
}

static int eqos_disable_calibration_stm32(struct udevice *dev)
{
	return 0;
}

static int eqos_set_tx_clk_speed_stm32(struct udevice *dev)
{
	return 0;
}

static int eqos_probe_resources_stm32(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;
	phy_interface_t interface;
	struct ofnode_phandle_args phandle_args;

	debug("%s(dev=%p):\n", __func__, dev);

	interface = eqos->config->interface(dev);

	if (interface == PHY_INTERFACE_MODE_NONE) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	ret = board_interface_eth_init(dev, interface);
	if (ret)
		return -EINVAL;

	eqos->max_speed = dev_read_u32_default(dev, "max-speed", 0);

	ret = clk_get_by_name(dev, "stmmaceth", &eqos->clk_master_bus);
	if (ret) {
		pr_err("clk_get_by_name(master_bus) failed: %d", ret);
		goto err_probe;
	}

	ret = clk_get_by_name(dev, "mac-clk-rx", &eqos->clk_rx);
	if (ret) {
		pr_err("clk_get_by_name(rx) failed: %d", ret);
		goto err_free_clk_master_bus;
	}

	ret = clk_get_by_name(dev, "mac-clk-tx", &eqos->clk_tx);
	if (ret) {
		pr_err("clk_get_by_name(tx) failed: %d", ret);
		goto err_free_clk_rx;
	}

	/*  Get ETH_CLK clocks (optional) */
	ret = clk_get_by_name(dev, "eth-ck", &eqos->clk_ck);
	if (ret)
		pr_warn("No phy clock provided %d", ret);

	eqos->phyaddr = -1;
	ret = dev_read_phandle_with_args(dev, "phy-handle", NULL, 0, 0,
					 &phandle_args);

	if (!ret) {
		/* search "reset-gpios" in phy node */
		ret = gpio_request_by_name_nodev(phandle_args.node,
						 "reset-gpios", 0,
						 &eqos->phy_reset_gpio,
						 GPIOD_IS_OUT |
						 GPIOD_IS_OUT_ACTIVE);
		if (ret)
			pr_warn("gpio_request_by_name(phy reset) not provided %d",
				ret);
		eqos->phyaddr = ofnode_read_u32_default(phandle_args.node,
							"reg", -1);
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

static phy_interface_t eqos_get_interface_stm32(struct udevice *dev)
{
	const char *phy_mode;
	phy_interface_t interface = PHY_INTERFACE_MODE_NONE;

	debug("%s(dev=%p):\n", __func__, dev);

	phy_mode = fdt_getprop(gd->fdt_blob, dev_of_offset(dev), "phy-mode",
			       NULL);
	if (phy_mode)
		interface = phy_get_interface_by_name(phy_mode);

	return interface;
}

static int eqos_remove_resources_stm32(struct udevice *dev)
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

static int eqos_pre_init_stm32(struct udevice *dev)
{
	return 0;
}

static struct eqos_ops eqos_stm32_ops = {
	.eqos_pre_init = eqos_pre_init_stm32,
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_stm32,
	.eqos_remove_resources = eqos_remove_resources_stm32,
	.eqos_stop_resets = eqos_stop_resets_stm32,
	.eqos_start_resets = eqos_start_resets_stm32,
	.eqos_stop_clks = eqos_stop_clks_stm32,
	.eqos_start_clks = eqos_start_clks_stm32,
	.eqos_calibrate_pads = eqos_calibrate_pads_stm32,
	.eqos_disable_calibration = eqos_disable_calibration_stm32,
	.eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_stm32,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_stm32
};

/* vendor specific driver config */

struct eqos_config eqos_stm32_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10000,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_AV,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.interface = eqos_get_interface_stm32,
	.ops = &eqos_stm32_ops
};
