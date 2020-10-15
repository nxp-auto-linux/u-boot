/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020 NXP
 *
 */
#ifndef S32GEN1_GMAC_UTILS_H
#define S32GEN1_GMAC_UTILS_H

void setup_iomux_enet_gmac(int intf);
void setup_clocks_enet_gmac(int intf, struct udevice *gmac_dev);
int set_tx_clk_enet_gmac(struct udevice *gmac_dev, u32 speed);

#endif
