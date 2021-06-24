// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2021 NXP
 */

#include "s32gen1_ocotp.h"
#include <common.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <fuse.h>
#include <misc.h>

int fuse_read(u32 bank, u32 word, u32 *val)
{
	int ret;
	struct udevice *dev;
	u32 offset;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_GET_DRIVER(s32gen1_ocotp),
					  &dev);
	if (ret) {
		log_err("Failed to get 's32gen1_ocotp' driver\n");
		return ret;
	}

	offset = S32GEN1_OCOTP_BANK_OFFSET;
	offset += bank * S32GEN1_OCOTP_BANK_SIZE;
	offset += word * S32GEN1_OCOTP_WORD_SIZE;

	ret = misc_read(dev, offset, val, S32GEN1_OCOTP_WORD_SIZE);
	/* Read less than 4 bytes */
	if (ret != S32GEN1_OCOTP_WORD_SIZE && ret > 0)
		ret = -EINVAL;

	return ret;
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	/* Not supported yet */
	return -EPERM;
}

int fuse_sense(u32 bank, u32 word, u32 *val)
{
	/* Not supported yet */
	return -EPERM;
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	/* Not supported yet */
	return -EPERM;
}
