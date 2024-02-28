// SPDX-License-Identifier: GPL-2.0+
/*
 * NXP S32G PFE Ethernet MDIO driver
 *
 * Copyright 2023-2024 NXP
 */

#define LOG_CATEGORY UCLASS_ETH

#include <common.h>
#include <miiphy.h>
#include <dm/device_compat.h>

#include "pfeng.h"

#define PFENG_MDIO_PHY_ADDR_MAX 31

static int pfeng_mdio_validate_addr(int addr, int devad, int reg)
{
	if (addr < 0 || devad < MDIO_DEVAD_NONE || addr > PFENG_MDIO_PHY_ADDR_MAX ||
	    devad > PFENG_MDIO_PHY_ADDR_MAX || reg < 0 ||
	    reg > 0xffff)
		return -EINVAL;

	return 0;
}

static int pfeng_mdio_read(struct udevice *dev, int addr, int devad, int reg)
{
	struct pfeng_mdio_cfg *cfg = dev_get_plat(dev);
	int ret;
	u16 val;

	ret = pfeng_mdio_validate_addr(addr, devad, reg);
	if (ret)
		return ret;

	ret = pfe_hw_emac_mdio_read((void __iomem *)cfg->iobase, addr, devad, reg, &val);
	if (ret) {
		dev_err(dev, "MDIO read on MAC#%d failed: %d\n", cfg->id, ret);
		return ret;
	}

	return val;
}

static int pfeng_mdio_write(struct udevice *dev, int addr, int devad, int reg, u16 val)
{
	struct pfeng_mdio_cfg *cfg = dev_get_plat(dev);
	int ret;

	ret = pfeng_mdio_validate_addr(addr, devad, reg);
	if (ret)
		return ret;

	ret = pfe_hw_emac_mdio_write((void __iomem *)cfg->iobase, addr, devad, reg, val);
	if (ret) {
		dev_err(dev, "MDIO write on MAC#%d failed: %d\n", cfg->id, ret);
		return ret;
	}

	return ret;
}

static int pfeng_mdio_of_to_plat(struct udevice *dev)
{
	struct pfeng_mdio_cfg *mdio_cfg = dev_get_plat(dev);
	struct pfeng_cfg *cfg = dev_get_plat(dev->parent);
	char name[16];
	fdt_addr_t dev_addr;
	enum pfe_hw_emac_block emac_id;
	int ret;

	if (!dev_read_enabled(dev))
		return 0;

	/* EMAC port id */
	dev_addr = dev_read_addr(dev);
	if (dev_addr == FDT_ADDR_T_NONE) {
		dev_err(dev, "Failed to read EMAC id\n");
		return -EINVAL;
	}

	if (dev_addr >= PFENG_EMACS_COUNT) {
		dev_err(dev, "Out of range EMAC id\n");
		return -EINVAL;
	}

	switch (dev_addr) {
	case 0ULL:
		emac_id = PFENG_EMAC0;
		break;
	case 1ULL:
		emac_id = PFENG_EMAC1;
		break;
	case 2ULL:
		emac_id = PFENG_EMAC2;
		break;
	default:
		emac_id = PFENG_EMACS_COUNT;
		break;
	}

	if (emac_id == PFENG_EMACS_COUNT)
		return -EINVAL;

	mdio_cfg->id = emac_id;
	mdio_cfg->iobase = pfe_hw_emac_get_iobase(cfg->csr_phys_addr, mdio_cfg->id);

	if (!mdio_cfg->iobase)
		return -EINVAL;

	ret = snprintf(name, sizeof(name), "pfeng-mdio-%u", mdio_cfg->id);
	if (ret >= sizeof(name) || ret < 0)
		return -ENOSPC;

	device_set_name(dev, name);

	return 0;
}

static int pfeng_mdio_bind(struct udevice *dev)
{
	static int id;
	char name[16];
	int ret;

	if (id >= PFENG_EMAC2) {
		dev_err(dev, "EMAC MDIO id out of range\n");
		return -ENODEV;
	}

	ret = snprintf(name, sizeof(name), "pfeng-mdio#%u", id++);
	if (ret >= sizeof(name) || ret < 0)
		return -ENOSPC;

	return device_set_name(dev, name);
}

static const struct udevice_id pfeng_mdio_ids[] = {
	{ .compatible = "nxp,s32g-pfe-mdio" },
	{ /* sentinel */ }
};

static const struct mdio_ops pfeng_mdio_ops = {
	.read = pfeng_mdio_read,
	.write = pfeng_mdio_write,
};

U_BOOT_DRIVER(pfeng_mdio) = {
	.name		= "pfeng_mdio",
	.id		= UCLASS_MDIO,
	.of_match	= pfeng_mdio_ids,
	.bind		= pfeng_mdio_bind,
	.ops		= &pfeng_mdio_ops,
	.of_to_plat	= pfeng_mdio_of_to_plat,
	.plat_auto	= sizeof(struct pfeng_mdio_cfg),
};
