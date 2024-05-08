// SPDX-License-Identifier: GPL-2.0+ or BSD-3-Clause
/*
 * Copyright 2024 NXP
 */

#include <common.h>
#include <dm/device.h>
#include <dm/of_access.h>
#include <s32-cc/dts_fixups_utils.h>

#define MMC_ALIAS	"mmc%u"

static int enable_hs400_emmc(void)
{
	int ret;
	ofnode node;

	node = ofnode_by_alias(MMC_ALIAS, 0);
	if (!ofnode_valid(node)) {
		pr_err("Failed to get 'mmc0' alias\n");
		return -ENOMEM;
	}

	ret = ofnode_delete_prop(node, "no-1-8-v");
	if (ret) {
		pr_err("Failed to remove mmc 'no-1-8-v' property: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	return ret;
}

int apply_dm_quick_boot_fixups(void)
{
	return enable_hs400_emmc();
}
