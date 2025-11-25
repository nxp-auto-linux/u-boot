// SPDX-License-Identifier:     GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2024 NXP
 */

#include <common.h>
#include <env.h>
#include <fdt_support.h>
#include <misc.h>
#include <phy.h>
#include <soc.h>
#include <asm/global_data.h>
#include <dm/uclass.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/libfdt.h>
#include <s32/fdt.h>

int ft_system_setup(void *blob, struct bd_info *bd)
{
	int ret;

	/* Add some space for the following changes */
	ret = fdt_increase_size(blob, 512);
	if (ret < 0) {
		pr_err("Could not increase size of device tree: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	/*
	 * Skip these fixups when reusing U-Boot dtb for Linux
	 * as they don't make sense.
	 *
	 * This block should be removed once the bindings and the dtbs
	 * used by Linux and U-Boot are fully compatible.
	 */
	if (IS_ENABLED(CONFIG_DISTRO_DEFAULTS)) {
		printf("Skipping %s ...\n", __func__);
		return 0;
	}

	ret = ft_fixup_memory(blob, bd);
	if (ret)
		return ret;

	return ft_fixup_atf(blob);
}
