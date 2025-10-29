// SPDX-License-Identifier:     GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2024-2025 NXP
 */

#include <config.h>
#include <errno.h>
#include <linux/types.h>
#include <linux/string.h>
#include <stdio.h>
#include <vsprintf.h>
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

static int ft_fixup_bl2(void *new_blob)
{
	int ret = ft_add_reserved_memory_node(new_blob, "bl2-resume");

	if (ret)
		log_err("Copying 'bl2-resume' node from U-Boot DT to Linux DT failed!\n");

	return ret;
}

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

	ret = ft_fixup_atf(blob);
	if (ret)
		return ret;

	/* Reserve memory from BL2 space for resume */
	return ft_fixup_bl2(blob);
}
