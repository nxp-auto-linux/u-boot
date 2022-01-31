// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2022 NXP
 */
#include <asm/arch/soc.h>
#include <board_common.h>
#include <common.h>
#include <miiphy.h>
#include <phy.h>
#include <i2c.h>
#include <asm/arch-s32/s32-gen1/serdes_regs.h>

#define BLUEBOX3_S32G_PHY_ADDR_5	0x05
#define BLUEBOX3_S32G_PHY_ADDR_6	0x06

int last_stage_init(void)
{
	struct udevice *eth;
	struct phy_device *phy;
	struct mii_dev *bus;

	eth = eth_get_dev_by_name("eth_pfeng");
	bus = miiphy_get_dev_by_name("pfeng_emac_0");
	if (eth && bus) {
		phy = phy_connect(bus, BLUEBOX3_S32G_PHY_ADDR_5, eth,
				  PHY_INTERFACE_MODE_RGMII);
		if (phy)
			phy_config(phy);

		phy = phy_connect(bus, BLUEBOX3_S32G_PHY_ADDR_6, eth,
				  PHY_INTERFACE_MODE_RGMII);
		if (phy)
			phy_config(phy);
	}

	return 0;
}

