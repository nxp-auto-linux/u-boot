// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2020, 2022 NXP
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <i2c.h>
#include <power/pmic.h>
#include <u-boot/crc.h>

#define VR5510_ADDR_SIZE	2
#define VR5510_REG_SIZE		2
#define VR5510_CRC_SIZE		1

#define VR5510_CRC_POLY		0x1Du
#define VR5510_CRC_SEED		0xFFu

#define DEV_ADDR_MASK		0xFE00U
#define DEV_ADDR_SHIFT		9
#define DEV_RW_MASK		0x100U
#define DEV_RW_SHIFT		8

/* Includes zeros from 8-6 */
#define REG_ADDR_MASK		0xFF
#define REG_ADDR_SHIFT		0

#define VR5510_M_LVB1_STBY_DVS_ID	17
#define VR5510_M_MEMORY0_ID		41

#define VF5510_MU_N_REGS		43
#define VF5510_FSU_N_REGS		24

struct read_msg {
	u16 address;
	u16 data;
	u8 crc;
};

static bool is_mu(struct udevice *dev)
{
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);

	if (chip->chip_addr & 1)
		return false;

	return true;
}

static bool valid_register(struct udevice *dev, uint reg)
{
	/* There are no gaps in FSU */
	if (!is_mu(dev))
		return true;

	if (reg > VR5510_M_LVB1_STBY_DVS_ID && reg < VR5510_M_MEMORY0_ID)
		return false;

	return true;
}

static int vr5510_reg_count(struct udevice *dev)
{
	if (is_mu(dev))
		return VF5510_MU_N_REGS;
	return VF5510_FSU_N_REGS;
}

static void set_dev_addr(struct read_msg *m, u8 addr)
{
	m->address &= ~DEV_ADDR_MASK;
	m->address |= (addr << DEV_ADDR_SHIFT);
}

static void set_rw(struct read_msg *m, bool read)
{
	m->address &= ~DEV_RW_MASK;
	if (read)
		m->address |= DEV_RW_MASK;
}

static void set_reg_addr(struct read_msg *m, u8 addr)
{
	m->address &= ~REG_ADDR_MASK;
	m->address |= (addr << REG_ADDR_SHIFT);
}

static int vr5510_read(struct udevice *dev, uint reg, u8 *buff, int len)
{
	unsigned int crc;
	struct read_msg msg = {0};
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);

	if (!valid_register(dev, reg)) {
		pr_err("Invalid vr5510 register %d\n", reg);
		return -EIO;
	}

	set_dev_addr(&msg, chip->chip_addr);
	set_rw(&msg, true);
	set_reg_addr(&msg, reg);
	msg.address = swab16(msg.address);

	if (dm_i2c_read(dev, reg, (u8 *)&msg.data,
			VR5510_REG_SIZE + VR5510_CRC_SIZE)) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	crc = crc8poly(VR5510_CRC_SEED, VR5510_CRC_POLY,
		       (const unsigned char *)&msg,
		       VR5510_ADDR_SIZE + VR5510_REG_SIZE);

	if (crc != msg.crc) {
		pr_err("read error from device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	*(u16 *)buff = swab16(msg.data);

	return 0;
}

static int vr5510_write(struct udevice *dev, uint reg, const u8 *buff, int len)
{
	struct read_msg msg = {0};
	struct dm_i2c_chip *chip = dev_get_parent_plat(dev);

	if (!valid_register(dev, reg)) {
		pr_err("Invalid vr5510 register %d\n", reg);
		return -EIO;
	}

	set_dev_addr(&msg, chip->chip_addr);
	set_rw(&msg, false);
	set_reg_addr(&msg, reg);
	msg.data = swab16(*(u16 *)buff);
	msg.address = swab16(msg.address);

	msg.crc = crc8poly(VR5510_CRC_SEED, VR5510_CRC_POLY,
			   (const unsigned char *)&msg,
			   VR5510_ADDR_SIZE + VR5510_REG_SIZE);

	if (dm_i2c_write(dev, reg, (const unsigned char *)&msg.data,
			 VR5510_REG_SIZE + VR5510_CRC_SIZE)) {
		pr_err("write error to device: %p register: %#x!\n", dev, reg);
		return -EIO;
	}

	return 0;
}

static int vr5510_probe(struct udevice *dev)
{
	struct uc_pmic_priv *priv = dev_get_uclass_priv(dev);

	/* 16 bit registers */
	priv->trans_len = VR5510_REG_SIZE;
	return 0;
}

static struct dm_pmic_ops vr5510_ops = {
	.reg_count = vr5510_reg_count,
	.read = vr5510_read,
	.write = vr5510_write,
};

static const struct udevice_id vr5510_ids[] = {
	{.compatible = "nxp,vr5510"},
	{}
};

U_BOOT_DRIVER(pmic_vr5510) = {
	.name = "vr5510_pmic",
	.id = UCLASS_PMIC,
	.of_match = vr5510_ids,
	.ops = &vr5510_ops,
	.probe = vr5510_probe,
};
