// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017,2019-2022 NXP
 */

#include <common.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

static int ft_fixup_ddr_polling(const void *old_blob, void *new_blob)
{
	int off, ret;
	const char *status;
	const char *exp_compatible = "nxp,s32cc-ddr";

	/* Get node offset in U-Boot DT */
	off = fdt_node_offset_by_compatible(old_blob, -1, exp_compatible);
	if (off < 0) {
		printf("%s: Couldn't find \"%s\" node: %s\n", __func__,
		       exp_compatible, fdt_strerror(off));
		return -ENODEV;
	}

	/* Check "status" property */
	status = fdt_getprop(old_blob, off, "status", NULL);
	if (!status) {
		printf("%s: Node \"%s\" does not have \"status\" set",
		       __func__, exp_compatible);
		return -EINVAL;
	}

	if (!strncmp("disabled", status, 8))
		return 0;

	/* Get node offset in Linux DT */
	off = fdt_node_offset_by_compatible(new_blob, -1, exp_compatible);
	if (off < 0) {
		printf("%s: Couldn't find \"%s\" node: %s\n", __func__,
		       exp_compatible, fdt_strerror(off));
		return -ENODEV;
	}

	/* Copy the status from the U-Boot DT */
	ret = fdt_setprop_string(new_blob, off, "status", status);
	if (ret) {
		printf("WARNING: Could not fix up the Linux DT, err=%s\n",
		       fdt_strerror(ret));
		return ret;
	}

	return 0;
}

int ft_system_setup(void *blob, struct bd_info *bd)
{
	return ft_fixup_ddr_polling(gd->fdt_blob, blob);
}
