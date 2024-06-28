// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2024 NXP
 */

#include <common.h>
#include <dm.h>
#include <ufs.h>
#include <dm/device_compat.h>

#include "ufs.h"

static int s32n_ufs_link_startup_notify(struct ufs_hba *hba,
					enum ufs_notify_change_status status)
{
	switch (status) {
	case PRE_CHANGE:
	case POST_CHANGE:
		;
	}

	return 0;
}

static int s32n_ufs_init(struct ufs_hba *hba)
{
	return 0;
}

static struct ufs_hba_ops s32n_hba_ops = {
	.init = s32n_ufs_init,
	.link_startup_notify = s32n_ufs_link_startup_notify,
};

static int s32n_ufs_pltfm_probe(struct udevice *dev)
{
	int err = ufshcd_probe(dev, &s32n_hba_ops);

	if (err)
		dev_err(dev, "ufshcd_probe() failed %d\n", err);

	return 0;
}

static int s32n_ufs_pltfm_bind(struct udevice *dev)
{
	struct udevice *scsi_dev;

	return ufs_scsi_bind(dev, &scsi_dev);
}

static const struct udevice_id s32n_ufs_pltfm_ids[] = {
	{
		.compatible = "nxp,s32n-ufs",
	},
	{},
};

U_BOOT_DRIVER(s32n_ufs_pltfm) = {
	.name		= "ufs-s32n",
	.id		= UCLASS_UFS,
	.of_match	= s32n_ufs_pltfm_ids,
	.bind		= s32n_ufs_pltfm_bind,
	.probe		= s32n_ufs_pltfm_probe,
};
