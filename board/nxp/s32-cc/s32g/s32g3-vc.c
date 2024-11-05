// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2024 NXP
 */

#include <dm.h>
#include <sysinfo.h>

#define BOARD_REV_STR_SIZE	32

int board_late_init(void)
{
	char board_rev[BOARD_REV_STR_SIZE];
	struct udevice *sysinfo_gpio;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_SYSINFO,
					"sysinfo-gpio", &sysinfo_gpio);
	if (ret) {
		printf("Could not get UCLASS_SYSINFO. Check DTB. err=%d\n", ret);
		return 0;
	}

	ret = sysinfo_detect(sysinfo_gpio);
	if (ret) {
		printf("Could not detect sysinfo. Check DTB. err=%d\n", ret);
		return 0;
	}

	ret = sysinfo_get_str(sysinfo_gpio, SYSINFO_ID_BOARD_MODEL,
			      BOARD_REV_STR_SIZE, board_rev);
	if (ret) {
		printf("Could not get revision. Check DTB. err=%d\n", ret);
		return 0;
	}

	printf("Board revision:\t%s\n", board_rev);
	return 0;
}

