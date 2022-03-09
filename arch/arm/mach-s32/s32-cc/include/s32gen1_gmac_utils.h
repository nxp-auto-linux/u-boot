/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020-2021 NXP
 *
 */
#ifndef S32GEN1_GMAC_UTILS_H
#define S32GEN1_GMAC_UTILS_H

#include <dm/device.h>
#include <phy_interface.h>

void setup_iomux_enet_gmac(struct udevice *dev, int intf);
void setup_clocks_enet_gmac(int intf, struct udevice *gmac_dev);
int set_tx_clk_enet_gmac(struct udevice *gmac_dev, u32 speed);
phy_interface_t eqos_get_interface_s32cc(struct udevice *dev);

#endif
