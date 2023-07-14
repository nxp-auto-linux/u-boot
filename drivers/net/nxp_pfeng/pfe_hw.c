// SPDX-License-Identifier: GPL 2.0
/*
 *  Copyright (c) 2019 Imagination Technologies Limited
 *  Copyright (c) 2020-2021 Imagination Technologies Limited
 *  Copyright 2018-2023 NXP
 */

#include "pfe_hw.h"

phys_addr_t pfe_hw_get_iobase(phys_addr_t pfe_iobase, enum pfe_hw_blocks block_id)
{
	return 0;
}

/* external API */
int pfe_hw_init(struct pfe_hw_ext *ext, const struct pfe_hw_cfg *hw_cfg)
{
	return 0;
}

void pfe_hw_remove(struct pfe_hw_ext *ext)
{
}

int pfe_hw_detect_version(phys_addr_t csr_base_addr, enum pfe_hw_ip_ver *pfe_ver)
{
	return 0;
}

int pfe_hw_print_stats(struct pfe_hw_ext *ext)
{
	return 0;
}

/* HIF - Rx/Tx */
int pfe_hw_hif_chnl_create(struct pfe_hw_ext *ext)
{
	return 0;
}

void pfe_hw_hif_chnl_destroy(struct pfe_hw_ext *ext)
{
}

void pfe_hw_hif_chnl_enable(struct pfe_hw_chnl *chnl)
{
}

void pfe_hw_hif_chnl_disable(struct pfe_hw_chnl *chnl)
{
}

int pfe_hw_chnl_xmit(struct pfe_hw_chnl *chnl, u8 phyif, void *packet, int length)
{
	return 0;
}

int pfe_hw_chnl_receive(struct pfe_hw_chnl *chnl, int flags, uchar **packetp)
{
	return 0;
}

int pfe_hw_chnl_free_pkt(struct pfe_hw_chnl *chnl, uchar *packet, int length)
{
	return 0;
}

/* EMAC - MDIO functionality */
void pfe_hw_emac_enable(struct pfe_hw_emac *emac)
{
}

void pfe_hw_emac_disable(struct pfe_hw_emac *emac)
{
}

void pfe_hw_emac_get_addr(struct pfe_hw_emac *emac, u8 *addr)
{
}

void pfe_hw_emac_set_addr(struct pfe_hw_emac *emac, const u8 *addr)
{
}

int pfe_hw_emac_set_duplex(struct pfe_hw_emac *emac, bool is_full)
{
	return 0;
}

int pfe_hw_emac_set_speed(struct pfe_hw_emac *emac, u32 speed)
{
	return 0;
}

int pfe_hw_emac_mdio_read(void __iomem *base_va, u8 pa, s32 dev, u16 ra, u16 *val)
{
	return 0;
}

int pfe_hw_emac_mdio_write(void __iomem *base_va, u8 pa, s32 dev, u16 ra, u16 val)
{
	return 0;
}
