/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * NXP S32G Ethernet accelerator driver
 *
 * Copyright 2023 NXP
 */

#ifndef PFENG_H_
#define PFENG_H_

#include <clk.h>
#include <net.h>
#include <phy.h>
#include <reset.h>
#include <dm/ofnode.h>
#include <linux/io.h>

#include "pfe_hw.h"

enum pfeng_emac_mode {
	PFENG_EMAC_NONE,
	PFENG_EMAC_MII,
	PFENG_EMAC_RMII,
	PFENG_EMAC_RGMII,
	PFENG_EMAC_SGMII,
};

#define PFENG_NETDEVS_COUNT	9 /* 3xEMACs, 1xAUX, 4xHIFs, 1xNOCPY */

struct pfeng_cfg {
	phys_addr_t		csr_phys_addr;
	resource_size_t		csr_size;
	phys_addr_t		gpr_phys_addr;
	resource_size_t		gpr_size;
	struct reset_ctl	*rst;
	u32			hif_id;
	phys_addr_t		bmu_addr;
	phys_size_t		bmu_size;
	struct udevice		*dev;
	bool			parsed;
	u8			emacs_mask;
	enum pfeng_emac_mode	emac_int_mode[PFENG_EMACS_COUNT];
	struct udevice		*netdevs[PFENG_NETDEVS_COUNT];
};

struct pfeng_priv {
	void			*csr_base;
	void			*gpr_base;
	enum pfe_hw_ip_ver	pfe_ver;
	ulong			clk_sys_rate;
	void			*fw_class_data;	/* The CLASS fw data buffer */
	u32			fw_class_size;	/* The CLASS fw data size */

	struct pfe_hw_ext	pfe_hw;
	struct pfe_hw_cfg	pfe_hw_cfg;

	const struct pfeng_cfg	*cfg;
};

static inline void pfeng_register_first_hif_channel(struct udevice *dev, u32 hif_id)
{
	struct pfeng_cfg *cfg = dev_get_plat(dev);

	if (hif_id < cfg->hif_id)
		cfg->hif_id = hif_id;
}

static inline struct pfe_hw_ext *pfeng_priv_get_hw(struct pfeng_priv *priv)
{
	return &priv->pfe_hw;
}

static inline struct pfe_hw_cfg *pfeng_priv_get_hw_cfg(struct pfeng_priv *priv)
{
	return &priv->pfe_hw_cfg;
}

struct pfeng_netif_cfg {
	const char	*name;
	struct udevice	*dev;
	const char	*phy_mode;
	phy_interface_t	phy_interface;
	u8		phyif;
	u8		netif_id;
};

static inline struct pfeng_netif_cfg *pfeng_get_plat(struct udevice *dev)
{
	struct eth_pdata *eth_pdata = dev_get_plat(dev);

	return eth_pdata->priv_pdata;
}

struct pfeng_netif {
	struct pfe_hw_chnl		*hw_chnl;
	struct phy_device		*phy;
	struct pfeng_priv		*priv;
	struct phy			*xpcs;
	const struct pfeng_netif_cfg	*cfg;
};

struct pfeng_mdio_cfg {
	phys_addr_t		iobase;
	enum pfe_hw_blocks	id;
};

static inline u32 pfe_read_gpr(struct pfeng_priv *priv, u32 off)
{
	return readl(priv->gpr_base + off);
}

static inline u32 pfe_write_gpr(struct pfeng_priv *priv, u32 off, u32 v)
{
	return writel(v, priv->gpr_base + off);
}

int pfeng_hw_detect_version(struct pfeng_priv *priv);

int pfeng_fw_set_from_env_and_load(struct pfeng_priv *priv);

/* S32G global (GPR) regs for PFE */
#define GPR_PFE_COH_EN_OFF		0x0

#define PFE_COH_PORTS_MASK_HIF_0_3	GENMASK(4, 1)

#define GPR_PFE_EMACX_INTF_SEL_OFF	0x04
#define GPR_PFE_EMAC_IF_MII(n)		(BIT_32(4 * (n)))
#define GPR_PFE_EMAC_IF_RMII(n)		(9U << (4 * (n)))
#define GPR_PFE_EMAC_IF_RGMII(n)	(2U << (4 * (n)))
#define GPR_PFE_EMAC_IF_SGMII(n)	0U

#define GPR_PFE_PRW_CTRL_OFF		0x20
#define PFE_EMAC_PWRDWN(n)		(BIT_32(3 + (n)))

#define GPR_PFE_GENCTRL1_OFF		0xE4

#endif /* PFENG_H_ */