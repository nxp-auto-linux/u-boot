// SPDX-License-Identifier:     GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2024-2025 NXP
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

void ft_fixup_ufs_nodes(void *new_blob)
{
	const char *ufs_compatible = "nxp,s32n79-ufshc";
	int new_blob_offset, ret;
	char *status;
	fdt_addr_t addr;
	ofnode node;

	ofnode_for_each_compatible_node(node, ufs_compatible) {
		status = ofnode_is_enabled(node) ? "okay" : "disabled";

		addr = ofnode_get_addr(node);
		if (addr == FDT_ADDR_T_NONE) {
			pr_err("%s: cannot read reg\n",
			       ofnode_get_name(node));
			continue;
		}

		new_blob_offset =
			fdt_node_offset_by_compat_reg(new_blob,
						      ufs_compatible, addr);
		if (new_blob_offset < 0) {
			pr_err("%s: cannot be found in OS FDT\n",
			       ofnode_get_name(node));
			continue;
		}

		ret = fdt_setprop_string(new_blob, new_blob_offset, "status",
					 status);
		if (ret < 0)
			pr_err("%s: status cannot be updated in OS FDT\n",
			       ofnode_get_name(node));
	}
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

	ft_fixup_ufs_nodes(blob);

	return ft_fixup_atf(blob);
}
