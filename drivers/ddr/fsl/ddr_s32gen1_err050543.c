// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 *
 * This driver serves in context of NXP SYS_ERRATUM_ERR050543.
 * It is used only in ATF boot flow.
 * During binding, it sets and exports the value of 'polling_needed'
 * variable in order to apply Linux dtb fixup logic signaling
 * that ERR050543 workaround has been applied in ATF and needs
 * to be reverted, when possible, by the associated Linux driver.
 */
#include <common.h>
#include <dm.h>
#include "ddr_s32gen1_err050543.h"

static int s32gen1_ddr_bind(struct udevice *dev)
{
	/* Hook in order to set the 'polling_needed' variable value
	 * in case of ATF boot flow.
	 */
	polling_needed = 1;

	return 0;
}

static const struct udevice_id s32gen1_ddr_ids[] = {
	{ .compatible = "fsl,s32gen1-ddr-err050543" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(ddr_s32gen1) = {
	.name = "s32gen1_ddr",
	.id = UCLASS_MISC,
	.of_match = s32gen1_ddr_ids,
	.bind = s32gen1_ddr_bind,
};
