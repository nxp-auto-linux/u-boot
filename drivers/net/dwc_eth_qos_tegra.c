// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 * Copyright 2019 NXP
 */

/*
 * The following configurations are currently supported:
 * tegra186:
 *    NVIDIA's Tegra186 chip. This configuration uses an AXI master/DMA bus, an
 *    AHB slave/register bus, contains the DMA, MTL, and MAC sub-blocks, and
 *    supports a single RGMII PHY. This configuration also has SW control over
 *    all clock and reset signals to the HW block.
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

#include "dwc_eth_qos.h"

static int eqos_start_clks_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = clk_enable(&eqos->clk_slave_bus);
	if (ret < 0) {
		pr_err("clk_enable(clk_slave_bus) failed: %d", ret);
		goto err;
	}

	ret = clk_enable(&eqos->clk_master_bus);
	if (ret < 0) {
		pr_err("clk_enable(clk_master_bus) failed: %d", ret);
		goto err_disable_clk_slave_bus;
	}

	ret = clk_enable(&eqos->clk_rx);
	if (ret < 0) {
		pr_err("clk_enable(clk_rx) failed: %d", ret);
		goto err_disable_clk_master_bus;
	}

	ret = clk_enable(&eqos->clk_ptp_ref);
	if (ret < 0) {
		pr_err("clk_enable(clk_ptp_ref) failed: %d", ret);
		goto err_disable_clk_rx;
	}

	ret = clk_set_rate(&eqos->clk_ptp_ref, 125 * 1000 * 1000);
	if (ret < 0) {
		pr_err("clk_set_rate(clk_ptp_ref) failed: %d", ret);
		goto err_disable_clk_ptp_ref;
	}

	ret = clk_enable(&eqos->clk_tx);
	if (ret < 0) {
		pr_err("clk_enable(clk_tx) failed: %d", ret);
		goto err_disable_clk_ptp_ref;
	}

	debug("%s: OK\n", __func__);
	return 0;

err_disable_clk_ptp_ref:
	clk_disable(&eqos->clk_ptp_ref);
err_disable_clk_rx:
	clk_disable(&eqos->clk_rx);
err_disable_clk_master_bus:
	clk_disable(&eqos->clk_master_bus);
err_disable_clk_slave_bus:
	clk_disable(&eqos->clk_slave_bus);
err:
	debug("%s: FAILED: %d\n", __func__, ret);
	return ret;
}

static void eqos_stop_clks_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clk_disable(&eqos->clk_tx);
	clk_disable(&eqos->clk_ptp_ref);
	clk_disable(&eqos->clk_rx);
	clk_disable(&eqos->clk_master_bus);
	clk_disable(&eqos->clk_slave_bus);

	debug("%s: OK\n", __func__);
}

static int eqos_start_resets_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 1);
	if (ret < 0) {
		pr_err("dm_gpio_set_value(phy_reset, assert) failed: %d", ret);
		return ret;
	}

	udelay(2);

	ret = dm_gpio_set_value(&eqos->phy_reset_gpio, 0);
	if (ret < 0) {
		pr_err("dm_gpio_set_value(phy_reset, deassert) failed: %d",
		       ret);
		return ret;
	}

	ret = reset_assert(&eqos->reset_ctl);
	if (ret < 0) {
		pr_err("reset_assert() failed: %d", ret);
		return ret;
	}

	udelay(2);

	ret = reset_deassert(&eqos->reset_ctl);
	if (ret < 0) {
		pr_err("reset_deassert() failed: %d", ret);
		return ret;
	}

	debug("%s: OK\n", __func__);
	return 0;
}

static int eqos_stop_resets_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	reset_assert(&eqos->reset_ctl);
	dm_gpio_set_value(&eqos->phy_reset_gpio, 1);

	return 0;
}

static int eqos_calibrate_pads_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&eqos->tegra186_regs->sdmemcomppadctrl,
		     EQOS_SDMEMCOMPPADCTRL_PAD_E_INPUT_OR_E_PWRD);

	udelay(1);

	setbits_le32(&eqos->tegra186_regs->auto_cal_config,
		     EQOS_AUTO_CAL_CONFIG_START | EQOS_AUTO_CAL_CONFIG_ENABLE);

	ret = wait_for_bit_le32(&eqos->tegra186_regs->auto_cal_status,
				EQOS_AUTO_CAL_STATUS_ACTIVE, true, 10, false);
	if (ret) {
		pr_err("calibrate didn't start");
		goto failed;
	}

	ret = wait_for_bit_le32(&eqos->tegra186_regs->auto_cal_status,
				EQOS_AUTO_CAL_STATUS_ACTIVE, false, 10, false);
	if (ret) {
		pr_err("calibrate didn't finish");
		goto failed;
	}

	ret = 0;

failed:
	clrbits_le32(&eqos->tegra186_regs->sdmemcomppadctrl,
		     EQOS_SDMEMCOMPPADCTRL_PAD_E_INPUT_OR_E_PWRD);

	debug("%s: returns %d\n", __func__, ret);

	return ret;
}

static int eqos_disable_calibration_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&eqos->tegra186_regs->auto_cal_config,
		     EQOS_AUTO_CAL_CONFIG_ENABLE);

	return 0;
}

static ulong eqos_get_tick_clk_rate_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	return clk_get_rate(&eqos->clk_slave_bus);
}

static int eqos_set_tx_clk_speed_tegra186(struct udevice *dev)
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

	ret = clk_set_rate(&eqos->clk_tx, rate);
	if (ret < 0) {
		pr_err("clk_set_rate(tx_clk, %lu) failed: %d", rate, ret);
		return ret;
	}

	return 0;
}

static int eqos_probe_resources_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = reset_get_by_name(dev, "eqos", &eqos->reset_ctl);
	if (ret) {
		pr_err("reset_get_by_name(rst) failed: %d", ret);
		return ret;
	}

	ret = gpio_request_by_name(dev, "phy-reset-gpios", 0,
				   &eqos->phy_reset_gpio,
				   GPIOD_IS_OUT | GPIOD_IS_OUT_ACTIVE);
	if (ret) {
		pr_err("gpio_request_by_name(phy reset) failed: %d", ret);
		goto err_free_reset_eqos;
	}

	ret = clk_get_by_name(dev, "slave_bus", &eqos->clk_slave_bus);
	if (ret) {
		pr_err("clk_get_by_name(slave_bus) failed: %d", ret);
		goto err_free_gpio_phy_reset;
	}

	ret = clk_get_by_name(dev, "master_bus", &eqos->clk_master_bus);
	if (ret) {
		pr_err("clk_get_by_name(master_bus) failed: %d", ret);
		goto err_free_clk_slave_bus;
	}

	ret = clk_get_by_name(dev, "rx", &eqos->clk_rx);
	if (ret) {
		pr_err("clk_get_by_name(rx) failed: %d", ret);
		goto err_free_clk_master_bus;
	}

	ret = clk_get_by_name(dev, "ptp_ref", &eqos->clk_ptp_ref);
	if (ret) {
		pr_err("clk_get_by_name(ptp_ref) failed: %d", ret);
		goto err_free_clk_rx;
		return ret;
	}

	ret = clk_get_by_name(dev, "tx", &eqos->clk_tx);
	if (ret) {
		pr_err("clk_get_by_name(tx) failed: %d", ret);
		goto err_free_clk_ptp_ref;
	}

	debug("%s: OK\n", __func__);
	return 0;

err_free_clk_ptp_ref:
	clk_free(&eqos->clk_ptp_ref);
err_free_clk_rx:
	clk_free(&eqos->clk_rx);
err_free_clk_master_bus:
	clk_free(&eqos->clk_master_bus);
err_free_clk_slave_bus:
	clk_free(&eqos->clk_slave_bus);
err_free_gpio_phy_reset:
	dm_gpio_free(dev, &eqos->phy_reset_gpio);
err_free_reset_eqos:
	reset_free(&eqos->reset_ctl);

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static phy_interface_t eqos_get_interface_tegra186(struct udevice *dev)
{
	return PHY_INTERFACE_MODE_MII;
}

static int eqos_remove_resources_tegra186(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clk_free(&eqos->clk_tx);
	clk_free(&eqos->clk_ptp_ref);
	clk_free(&eqos->clk_rx);
	clk_free(&eqos->clk_slave_bus);
	clk_free(&eqos->clk_master_bus);
	dm_gpio_free(dev, &eqos->phy_reset_gpio);
	reset_free(&eqos->reset_ctl);

	debug("%s: OK\n", __func__);
	return 0;
}

static struct eqos_ops eqos_tegra186_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_tegra186,
	.eqos_remove_resources = eqos_remove_resources_tegra186,
	.eqos_stop_resets = eqos_stop_resets_tegra186,
	.eqos_start_resets = eqos_start_resets_tegra186,
	.eqos_stop_clks = eqos_stop_clks_tegra186,
	.eqos_start_clks = eqos_start_clks_tegra186,
	.eqos_calibrate_pads = eqos_calibrate_pads_tegra186,
	.eqos_disable_calibration = eqos_disable_calibration_tegra186,
	.eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_tegra186,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_tegra186
};

/* vendor specific driver config */

struct eqos_config eqos_tegra186_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10,
	.swr_wait = 10,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_20_35,
	.interface = eqos_get_interface_tegra186,
	.ops = &eqos_tegra186_ops
};
