// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 NXP
 *
 */

/*
 * s32cc:
 *    NXP S32G/S32R/S32V chips.
 *    Based on Synopsys DW EQOS MAC 5.10a
 *
 *    Note: Currently running on virtual platform only (VDK)
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
#include <asm/io.h>

#include "dwc_eth_qos.h"

static int eqos_start_clks_s32cc(struct udevice *dev)
{
	return 0;
}

static void eqos_stop_clks_s32cc(struct udevice *dev)
{
	/* empty */
}

static int eqos_start_resets_s32cc(struct udevice *dev)
{
	return 0;
}

static int eqos_stop_resets_s32cc(struct udevice *dev)
{
	return 0;
}

static ulong eqos_get_tick_clk_rate_s32cc(struct udevice *dev)
{
	/* TODO: retrieve from CSR clock */
	return 100 * 1000000;
}

static int eqos_calibrate_pads_s32cc(struct udevice *dev)
{
	return 0;
}

static int eqos_disable_calibration_s32cc(struct udevice *dev)
{
	return 0;
}

static int eqos_set_tx_clk_speed_s32cc(struct udevice *dev)
{
	return 0;
}

static int eqos_probe_resources_s32cc(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	phy_interface_t interface;

	debug("%s(dev=%p):\n", __func__, dev);

	interface = eqos->config->interface(dev);

	if (interface == PHY_INTERFACE_MODE_NONE) {
		pr_err("Invalid PHY interface\n");
		return -EINVAL;
	}

	debug("%s: OK\n", __func__);
	return 0;
}

static phy_interface_t eqos_get_interface_s32cc(struct udevice *dev)
{
	return PHY_INTERFACE_MODE_MII;
}

static int eqos_remove_resources_s32cc(struct udevice *dev)
{
	return 0;
}

static struct eqos_ops eqos_s32cc_ops = {
	.eqos_inval_desc = eqos_inval_desc_generic,
	.eqos_flush_desc = eqos_flush_desc_generic,
	.eqos_inval_buffer = eqos_inval_buffer_generic,
	.eqos_flush_buffer = eqos_flush_buffer_generic,
	.eqos_probe_resources = eqos_probe_resources_s32cc,
	.eqos_remove_resources = eqos_remove_resources_s32cc,
	.eqos_stop_resets = eqos_stop_resets_s32cc,
	.eqos_start_resets = eqos_start_resets_s32cc,
	.eqos_stop_clks = eqos_stop_clks_s32cc,
	.eqos_start_clks = eqos_start_clks_s32cc,
	.eqos_calibrate_pads = eqos_calibrate_pads_s32cc,
	.eqos_disable_calibration = eqos_disable_calibration_s32cc,
	.eqos_set_tx_clk_speed = eqos_set_tx_clk_speed_s32cc,
	.eqos_get_tick_clk_rate = eqos_get_tick_clk_rate_s32cc
};

struct eqos_config eqos_s32cc_config = {
	.reg_access_always_ok = false,
	.mdio_wait = 10000,
	.swr_wait = 50,
	.config_mac = EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB,
	.config_mac_mdio = EQOS_MAC_MDIO_ADDRESS_CR_250_300,
	.interface = eqos_get_interface_s32cc,
	.ops = &eqos_s32cc_ops
};

