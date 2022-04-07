// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2021-2022 NXP
 */
#include <common.h>
#include <dm.h>
#include <fuse.h>
#include <misc.h>
#include <s32-cc/s32cc_ocotp.h>

static struct udevice *get_ocotp_driver(void)
{
	static struct udevice *dev;
	int ret;

	if (dev)
		return dev;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(s32cc_ocotp),
					  &dev);
	if (ret) {
		log_err("Failed to get %s driver\n", dev->driver->name);
		return NULL;
	}

	return dev;
}

static u32 get_offset(u32 bank, u32 word)
{
	u32 offset;

	offset = S32CC_OCOTP_BANK_OFFSET;
	offset += bank * S32CC_OCOTP_BANK_SIZE;
	offset += word * S32CC_OCOTP_WORD_SIZE;

	return offset;
}

int fuse_read(u32 bank, u32 word, u32 *val)
{
	int ret;
	struct udevice *dev;
	u32 offset;

	dev = get_ocotp_driver();
	if (!dev)
		return -EINVAL;

	offset = get_offset(bank, word);

	/* Read from shadow cache */
	ret = misc_read(dev, offset, val, S32CC_OCOTP_WORD_SIZE);

	/* Read less than 4 bytes */
	if (ret == S32CC_OCOTP_WORD_SIZE)
		ret = 0;

	return ret;
}

int fuse_override(u32 bank, u32 word, u32 val)
{
	int ret;
	struct udevice *dev;
	u32 offset;

	dev = get_ocotp_driver();
	if (!dev)
		return -EINVAL;

	offset = get_offset(bank, word);

	/* Write shadow value */
	ret = misc_write(dev, offset, &val, S32CC_OCOTP_WORD_SIZE);
	if (ret == S32CC_OCOTP_WORD_SIZE)
		ret = 0;

	return ret;
}

static int fuse_ioctl(u32 bank, u32 word, int command, u32 *val)
{
	struct udevice *dev;
	u32 offset;
	struct s32cc_ocotp_cmd cmd;

	dev = get_ocotp_driver();
	if (!dev)
		return -EINVAL;

	offset = get_offset(bank, word);

	cmd = (struct s32cc_ocotp_cmd) {
		.offset = offset,
		.buf = val,
		.size = S32CC_OCOTP_WORD_SIZE,
	};

	/* Read/write from/to fuses */
	return misc_ioctl(dev, command, &cmd);
}

int fuse_sense(u32 bank, u32 word, u32 *val)
{
	return fuse_ioctl(bank, word, S32CC_OCOTP_READ_FUSE_CMD, val);
}

int fuse_prog(u32 bank, u32 word, u32 val)
{
	return fuse_ioctl(bank, word, S32CC_OCOTP_WRITE_FUSE_CMD, &val);
}

