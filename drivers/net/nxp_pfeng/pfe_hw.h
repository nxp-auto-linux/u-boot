/* SPDX-License-Identifier: GPL 2.0 */
/*
 *  Copyright 2023-2024 NXP
 */

#ifndef PFE_HW_H_
#define PFE_HW_H_

#include <dm/device.h>

#define PFENG_EGPI_COUNT 3U
#define PFENG_ETGPI_COUNT 3U
#define PFENG_HGPI_COUNT 1U
#define PFENG_GPI_COUNT (PFENG_EGPI_COUNT + PFENG_ETGPI_COUNT + PFENG_HGPI_COUNT)
#define PFENG_BMU_COUNT 2U
#define PFENG_SHP_COUNT 4U
#define PFENG_SCH_COUNT 2U
#define PFENG_PE_COUNT 8U
#define PFENG_MASTER_UP 1U

struct pfe_hw_emac;
struct pfe_hw_hif;
struct pfe_hw_chnl;
struct pfe_hw_pe;
struct pfe_hw;

enum pfe_hw_ip_ver {
	PFE_IP_NONE = 0,
	PFE_IP_S32G2,
	PFE_IP_S32G3,
};

enum pfe_hw_gpi_blocks {
	PFENG_EGPI1 = 0,
	PFENG_EGPI2,
	PFENG_EGPI3,
	PFENG_ETGPI1,
	PFENG_ETGPI2,
	PFENG_ETGPI3,
	PFENG_HGPI,
	PFENG_GPIS_COUNT,
};

enum pfe_hw_bmu_blocks {
	PFENG_BMU1 = 0,
	PFENG_BMU2,
	PFENG_BMUS_COUNT,
};

enum pfe_hw_shp_blocks {
	PFENG_SHP1 = 0,
	PFENG_SHP2,
	PFENG_SHPS_COUNT,
};

enum pfe_hw_sch_blocks {
	PFENG_SCH1 = 0,
	PFENG_SCH2,
	PFENG_SCHS_COUNT,
};

enum pfe_hw_emac_block {
	PFENG_EMAC0 = 0,
	PFENG_EMAC1,
	PFENG_EMAC2,
	PFENG_EMACS_COUNT,
};

/* Device tree "nxp,s32g-pfe-netif" compatible node can contain either:
 * - nxp,pfeng-linked-phyif = < >;
 * - nxp,pfeng-netif-mode-aux;
 * This determines operation of the network interface.
 * The interface with specified linked physical interface injects (sends) and receives
 * traffic only to / from the given physical IF.
 * While the AUX mode interface sends packets to the PFE classifier with HIF cookie set
 * and receives all incoming traffic.
 */

enum pfe_hw_hif_block {
	PFENG_HIF_MULTI = 3U,	/* Container for all PFENG_HIF0..3, PFE AUX type */
	PFENG_HIF0 = 6U,	/* Synthetic HIF interfaces, id range 6..9 */
	PFENG_HIF1,
	PFENG_HIF2,
	PFENG_HIF3,
};

enum pfe_hif_channels {
	PFE_HIF_CHANNEL_0 = 0,
	PFE_HIF_CHANNEL_1,
	PFE_HIF_CHANNEL_2,
	PFE_HIF_CHANNEL_3,
};

enum pfe_hw_chnl_stage {
	PFE_HW_CHNL_UNINITIALIZED = 0,
	PFE_HW_CHNL_CREATED,
	PFE_HW_CHNL_READY,
	PFE_HW_CHNL_IN_ERROR,
};

struct pfe_hw_ext {
	struct pfe_hw *hw;
	struct pfe_hw_chnl *hw_chnl;
	/* variable number of emacs, from 0 to max */
	struct pfe_hw_emac *hw_emac[PFENG_EMACS_COUNT];
	enum pfe_hw_chnl_stage hw_chnl_state;
	int hw_chnl_error;
	bool in_grace_reset;
	bool hw_chnl_enabled;
};

struct pfe_hw_cfg {
	struct udevice *dev;
	phys_addr_t cbus_base; /* PFE control bus base address */
	u64 csr_clk_f; /* CSR clk frequency */
	char *fw_name; /* FW name */
	void *fw_class_data; /* The CLASS fw data buffer */
	u32 fw_class_size; /* The CLASS fw data size */
	phys_addr_t bmu_addr;
	phys_size_t bmu_addr_size;
	phys_addr_t bdrs_addr;
	phys_size_t bdrs_size;
	u8 hif_chnl_id;
	u8 ihc_hif_id;
	u8 master_hif_id;
	u8 emac_mask;
	u8 emac_mdio_div; /* Divider for mdio clk */
	bool on_g3; /* True if running on S32G3 */
};

/* PFE - general h/w init */
int pfe_hw_init(struct pfe_hw_ext *ext, const struct pfe_hw_cfg *cfg);
void pfe_hw_remove(struct pfe_hw_ext *ext);
void pfe_hw_print_stats(struct pfe_hw_ext *ext);
int pfe_hw_grace_reset(struct pfe_hw_ext *ext);
int pfe_hw_detect_version(phys_addr_t csr_base_addr, enum pfe_hw_ip_ver *pfe_ver);
int pfe_hw_hif_chnl_hw_init(struct pfe_hw_ext *ext, const struct pfe_hw_cfg *cfg);
void pfe_hw_hif_chnl_rings_attach(struct pfe_hw_ext *ext);
int pfe_hw_hif_enable(struct pfe_hw_ext *ext);

/* HIF - Rx/Tx */
int pfe_hw_hif_chnl_create(struct pfe_hw_ext *ext);
void pfe_hw_hif_chnl_destroy(struct pfe_hw_ext *ext);
void pfe_hw_hif_chnl_enable(struct pfe_hw_chnl *chnl);
void pfe_hw_hif_chnl_disable(struct pfe_hw_chnl *chnl);
void pfe_hw_chnl_rings_attach(struct pfe_hw_chnl *chnl);
int pfe_hw_chnl_xmit(struct pfe_hw_chnl *chnl, bool is_ihc, u8 phyif, void *packet, int length);
int pfe_hw_chnl_xmit_dummy(struct pfe_hw_chnl *chnl);
int pfe_hw_chnl_receive(struct pfe_hw_chnl *chnl, int flags, bool strip_hdr, u8 phyif,
			uchar **packetp);
int pfe_hw_chnl_free_pkt(struct pfe_hw_chnl *chnl, uchar *packet, int length);
bool pfe_hw_chnl_cfg_ltc_get(struct pfe_hw_chnl *chnl);
u32 pfe_hw_chnl_get_rx_bd_ring_addr(struct pfe_hw_chnl *chnl);
u32 pfe_hw_chnl_get_rx_wb_table_addr(struct pfe_hw_chnl *chnl);
u32 pfe_hw_chnl_get_tx_bd_ring_addr(struct pfe_hw_chnl *chnl);
u32 pfe_hw_chnl_get_tx_wb_table_addr(struct pfe_hw_chnl *chnl);
u32 pfe_hw_chnl_rx_bdp_fifo_len(struct pfe_hw_chnl *chnl);

/* EMAC - MDIO functionality */
void pfe_hw_emac_enable(struct pfe_hw_emac *emac);
void pfe_hw_emac_disable(struct pfe_hw_emac *emac);
void pfe_hw_emac_get_addr(struct pfe_hw_emac *emac, u8 *addr);
void pfe_hw_emac_set_addr(struct pfe_hw_emac *emac, const u8 *addr);
int pfe_hw_emac_set_duplex(struct pfe_hw_emac *emac, bool is_full);
int pfe_hw_emac_set_speed(struct pfe_hw_emac *emac, int speed);

int pfe_hw_emac_mdio_read(void __iomem *base_va, u8 pa, s32 dev, u16 ra, u16 *val);
int pfe_hw_emac_mdio_write(void __iomem *base_va, u8 pa, s32 dev, u16 ra, u16 val);

/* helpers */
phys_addr_t pfe_hw_emac_get_iobase(phys_addr_t pfe_iobase, enum pfe_hw_emac_block block_id);

static inline bool is_emac_active(const struct pfe_hw_cfg *pfe_hw, u8 emac_id)
{
	return !!(BIT_32(emac_id) & pfe_hw->emac_mask);
}

#endif /* PFE_HW_H_ */
