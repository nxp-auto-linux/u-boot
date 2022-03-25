// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2020, 2022 NXP
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>

#define FS5600_REG_SIZE		2
#define FS5600_N_REGS		0x2F

static const u64 valid_regs_mask = 0x407B56F7B6DBULL;

static int fs5600_reg_count(struct udevice *dev)
{
	return FS5600_N_REGS;
}

static bool valid_register(struct udevice *dev, uint reg)
{
	return valid_regs_mask & BIT(reg);
}

static int fs5600_read(struct udevice *dev, uint reg, u8 *buff, int len)
{
	u16 data;

	if (len != FS5600_REG_SIZE)
		return -EINVAL;

	if (!valid_register(dev, reg))
		return -ENODATA;

	if (dm_i2c_read(dev, reg, (u8 *)&data, len)) {
		pr_err("read error from device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	*(u16 *)buff = swab16(data);

	return 0;
}

static int fs5600_write(struct udevice *dev, uint reg, const u8 *buff, int len)
{
	u16 data;

	if (len != FS5600_REG_SIZE)
		return -EINVAL;

	if (!valid_register(dev, reg))
		return -ENODATA;

	data = swab16(*(u16 *)buff);

	if (dm_i2c_write(dev, reg, (const unsigned char *)&data, len)) {
		pr_err("write error to device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int fs5600_probe(struct udevice *dev)
{
	struct uc_pmic_priv *priv = dev_get_uclass_priv(dev);

	/* 16 bit registers */
	priv->trans_len = FS5600_REG_SIZE;
	return 0;
}

static struct dm_pmic_ops fs5600_ops = {
	.reg_count = fs5600_reg_count,
	.read = fs5600_read,
	.write = fs5600_write,
};

static const struct udevice_id fs5600_ids[] = {
	{.compatible = "nxp,fs5600"},
	{}
};

U_BOOT_DRIVER(pmic_fs5600) = {
	.name = "fs5600_pmic",
	.id = UCLASS_PMIC,
	.of_match = fs5600_ids,
	.ops = &fs5600_ops,
	.probe = fs5600_probe,
};
