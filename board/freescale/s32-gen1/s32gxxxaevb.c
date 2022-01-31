// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2022 NXP
 */
#include <asm/arch/soc.h>
#include <board_common.h>
#include <common.h>
#include <dm/uclass.h>

#define SJA1105_NAME	"ethernet-switch@0"

int misc_init_r(void)
{
#if CONFIG_IS_ENABLED(NET) && CONFIG_IS_ENABLED(FSL_PFENG) && \
	CONFIG_IS_ENABLED(SJA1105)
	struct udevice *dev;

	/* Probe sja1105 in order to provide a clock for the PFE2 interface,
	 * otherwise clock init for this interface will fail.
	 * The return value is not checked on purpose. If the S32G PROCEVB
	 * board is used without PLATEVB board the uclass_get_device_by_name
	 * call will fail and returning and error code will break the init_call
	 * sequence of the u-boot.
	 */
	uclass_get_device_by_name(UCLASS_MISC, SJA1105_NAME, &dev);
#endif
	return 0;
}
