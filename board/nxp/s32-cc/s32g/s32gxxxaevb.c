// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2023 NXP
 */
#include <common.h>
#include <generic-phy.h>
#include <dm/device-internal.h>
#include <dm/device.h>
#include <dm/uclass.h>

#define SJA1105_NAME   "ethernet-switch@0"

static int enable_saf1508bet(void)
{
	int ret = 0;
	struct udevice *dev;
	struct phy phy;
	struct uclass *uc;
	struct udevice *bus;

	ret = uclass_get_device(UCLASS_USB, 0, &dev);
	if (ret) {
		pr_err("%s: Cannot find USB device\n", __func__);
		return ret;
	}
	ret = uclass_get(UCLASS_USB, &uc);
	if (ret)
		return ret;

	/* Probe USB controller */
	uclass_foreach_dev(bus, uc) {
		ret = device_probe(bus);
		if (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			continue;
		}
	}

	/* SAF1508BET PHY */
	ret = generic_phy_get_by_index(dev, 0, &phy);
	if (ret) {
		pr_err("failed to get %s USB PHY\n", dev->name);
		return ret;
	}

	ret = generic_phy_power_on(&phy);
	if (ret) {
		pr_err("failed to get %s USB PHY\n", dev->name);
		return ret;
	}

	return ret;
}

int misc_init_r(void)
{
	if (IS_ENABLED(CONFIG_NET) && IS_ENABLED(CONFIG_NXP_PFENG) &&
	    IS_ENABLED(CONFIG_SJA1105X)) {
		struct udevice *dev;
		/* Probe sja1105 in order to provide a clock for the PFE2 interface,
		 * otherwise clock init for this interface will fail.
		 * The return value is not checked on purpose. If the S32G PROCEVB
		 * board is used without PLATEVB board the uclass_get_device_by_name
		 * call will fail and returning and error code will break the init_call
		 * sequence of the u-boot.
		 */
		uclass_get_device_by_name(UCLASS_MISC, SJA1105_NAME, &dev);
	}
	/* The usb phy must be probed in u-boot in order to have a working USB
	 * interface in linux.
	 */
	if (IS_ENABLED(CONFIG_SAF1508BET_USB_PHY))
		enable_saf1508bet();

	return 0;
}
