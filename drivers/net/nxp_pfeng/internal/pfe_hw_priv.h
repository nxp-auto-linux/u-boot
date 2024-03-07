/* SPDX-License-Identifier: GPL 2.0 */
/*
 *  Copyright 2023-2024 NXP
 */

#ifndef PFE_HW_PRIV_H_
#define PFE_HW_PRIV_H_

#include <asm/io.h>

#include "pfe_ct.h"
#include "pfe_hif_ring.h"
#include "pfe_hw_abi.h"
#include "pfe_platform_cfg.h"

struct pfe_hw_emac {
	void __iomem *base;
};

struct pfe_hw_hif {
	void __iomem *base;
	u8 hif_chnl;
};

struct pfe_hw_chnl_stat {
	u32 foreign_rx;
};

struct pfe_hw_chnl {
	void __iomem *base;
	struct pfe_hif_ring *rx_ring;
	struct pfe_hif_ring *tx_ring;
	struct pfe_hw_hif *hif;
	struct pfe_hw_chnl_stat stat;
	u8 id;
};

struct pfe_hw_pe {
	void __iomem *base;
	u8 *fw;
	u8 class_pe_count;    /* Number of CLASS PEs */
};

struct pfe_hw {
	phys_addr_t cbus_baseaddr;
	void __iomem *base;
	void *bmu_buffers_va;
	u64 bmu_buffers_size;
	void *bdr_buffers_va;
	u64 bdr_buffers_size;
	u32 hif_chnl_count;	/* Number of HIF channels */
	bool on_g3;		/* True if running on S32G3 */
	u8 emac_mdio_div;	/* Divider for mdio clk */
	u8 hif_chnl;		/* HIF channel ID to be used by u-boot */
	u8 ihc_hif_id;
	u8 master_hif_id;
	struct pfe_hw_hif hif;
	struct pfe_hw_pe class;
	void *hif_base;
	const struct pfe_hw_cfg *cfg;
};

static inline void __iomem *pfe_hw_reg_addr(void __iomem *base, u32 off)
{
	return base + off;
}

#define pfe_hw_addr(priv, off)		pfe_hw_reg_addr((priv)->base, off)

static inline u32 pfe_hw_read_reg(void __iomem *base, u32 off)
{
	return readl(base + off);
}

static inline void pfe_hw_write_reg(void __iomem *base, u32 off, u32 v)
{
	writel(v, base + off);
}

#define pfe_hw_read(priv, off)		pfe_hw_read_reg((priv)->base, off)
#define pfe_hw_write(priv, off, v)	pfe_hw_write_reg((priv)->base, off, v)

/* internal API */

static inline dma_addr_t pfe_hw_dma_addr(void *addr)
{
	return (dma_addr_t)virt_to_phys(addr);
}

static inline void *pfe_hw_phys_addr(phys_addr_t paddr)
{
	return phys_to_virt(paddr);
}

/* EMAC internal */
int pfe_hw_emac_init(struct pfe_hw_emac *emac, enum pfe_hw_emac_block emac_id,
		     const struct pfe_hw_cfg *cfg);
void pfe_hw_emac_deinit(struct pfe_hw_emac *emac);

/* PFE PE/CLASS */
int pfe_hw_pe_init_class(struct pfe_hw_pe *class, const struct pfe_hw_cfg *cfg);
void pfe_hw_pe_enable_class(struct pfe_hw_pe *class);
void pfe_hw_pe_deinit_class(struct pfe_hw_pe *class);

/* HIF internal */
int pfe_hw_hif_init(struct pfe_hw_hif *hif, void __iomem *base, u8 hif_chnl,
		    const struct pfe_hw_cfg *config);

/* helpers */
int pfe_hw_soft_reset(struct pfe_hw *hw);
u8 pfe_hw_get_platform_emac_mdio_div(void);
#define PFE_HW_PLATFORM(config) pfe_hw_get_platform_##config()

struct pfe_hw_stats {
	char *reg_name;
	u32 reg_off;
};

void pfe_hw_chnl_print_stats(struct pfe_hw_chnl *chnl);
void pfe_hw_emac_print_stats(struct pfe_hw_emac *emac);

#endif /* PFE_HW_PRIV_H_ */
