// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2017-2022 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <mmc.h>
#include <div64.h>
#include <errno.h>
#include <hang.h>
#include <board_common.h>
#include <fdtdec.h>
#ifdef CONFIG_SAF1508BET_USB_PHY
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <generic-phy.h>
#endif

DECLARE_GLOBAL_DATA_PTR;

__weak int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

#ifdef CONFIG_SAF1508BET_USB_PHY
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
#endif

int arch_misc_init(void)
{
#ifdef CONFIG_SAF1508BET_USB_PHY
	/* The usb phy must be probed in u-boot in order to have a working USB
	 * interface in linux.
	 */
	enable_saf1508bet();
#endif
	return 0;
}
