// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2022 NXP
 */

#include <common.h>
#include <linux/err.h>
#include <miiphy.h>

#define S32R_PHY_ADDR_1 0x01

/**
 * Due to the hardware design of the reset circuit on S32R45,
 * the PHY does not properly reset after a software reboot.
 * PHY's control register is not correctly initialized afterwards
 * and GMAC clock settings fail to apply.
 */
int last_stage_init(void)
{
#if CONFIG_IS_ENABLED(DWC_ETH_QOS_S32CC)
	struct udevice *gmac0;
	struct mii_dev *bus;
	struct phy_device *phy;
	phy_interface_t mode;
	const char *phy_mode;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_ETH, 0, &gmac0);
	if (ret)
		return ret;

	bus = miiphy_get_dev_by_name(gmac0->name);
	if (!bus)
		return -ENODEV;

	phy_mode = dev_read_prop(gmac0, "phy-mode", NULL);
	if (!phy_mode) {
		pr_err("%s: phy-mode not found\n", gmac0->name);
		return -EINVAL;
	}

	mode = phy_get_interface_by_name(phy_mode);
	/* phy_connect will reset the PHY */
	phy = phy_connect(bus, S32R_PHY_ADDR_1, gmac0, mode);
	if (IS_ERR(phy))
		pr_err("%s: %s: phy reset done\n", __func__, gmac0->name);
	/* Is not critical if the phy has not been found.
	 * The error code is ignored on purpose.
	 */
#endif
	return 0;
}
