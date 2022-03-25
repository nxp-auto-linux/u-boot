// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2020, 2022 NXP
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>

#define PF5020_FUNC_REG_END	0x77
#define PF5020_OTP_REG_START	0xA0
#define PF5020_N_REGS		0xD7

static int pf5020_reg_count(struct udevice *dev)
{
	return PF5020_N_REGS;
}

static bool valid_register(struct udevice *dev, uint reg)
{
	return reg <= PF5020_FUNC_REG_END ||
		(reg >= PF5020_OTP_REG_START && reg < pf5020_reg_count(dev));
}

static int pf5020_read(struct udevice *dev, uint reg, u8 *buff, int len)
{
	if (!valid_register(dev, reg))
		return -ENODATA;

	if (dm_i2c_read(dev, reg, buff, len)) {
		pr_err("read error from device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static int pf5020_write(struct udevice *dev, uint reg, const u8 *buff, int len)
{
	if (!valid_register(dev, reg))
		return -ENODATA;

	if (dm_i2c_write(dev, reg, buff, len)) {
		pr_err("write error to device: %p register: %#x!", dev, reg);
		return -EIO;
	}

	return 0;
}

static struct dm_pmic_ops pf5020_ops = {
	.reg_count = pf5020_reg_count,
	.read = pf5020_read,
	.write = pf5020_write,
};

static const struct udevice_id pf5020_ids[] = {
	{.compatible = "nxp,pf5020"},
	{}
};

U_BOOT_DRIVER(pmic_pf5020) = {
	.name = "pf5020_pmic",
	.id = UCLASS_PMIC,
	.of_match = pf5020_ids,
	.ops = &pf5020_ops,
};
