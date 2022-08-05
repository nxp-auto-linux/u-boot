/* SPDX-License-Identifier: GPL 2.0 */
/*
 * Copyright (c) 2020 Imagination Technologies Limited
 * Copyright 2019-2022 NXP
 *
 */

#ifndef _PFENG_H_
#define _PFENG_H_

#include <generic-phy.h>
#include "pfe_hif_ring.h"
#include "pfeng_hw.h"

#define S32G_PFE_REGS_BASE		0x46000000
#define S32G_PFE_EMACn_MODE(n)		(S32G_PFE_REGS_BASE + 0xA0000 + \
					 (0x4000 * n) + 0x1000)
#define S32G_PFE_COH_EN			0x4007CA00
#define S32G_PFE_EMACS_INTF_SEL		0x4007CA04
#define S32G_PFE_PRW_CTRL		0x4007CA20
#define S32G_MAIN_GENCTRL1		0x4007CAE4

#define PFE_COH_PORT_MASK_DDR		BIT(0)
#define PFE_COH_PORT_MASK_HIF0		BIT(1)
#define PFE_COH_PORT_MASK_HIF1		BIT(2)
#define PFE_COH_PORT_MASK_HIF2		BIT(3)
#define PFE_COH_PORT_MASK_HIF3		BIT(4)
#define PFE_COH_PORT_MASK_UTIL		BIT(5)
#define PFE_COH_PORTS_MASK_FULL		(PFE_COH_PORT_MASK_DDR  | \
					 PFE_COH_PORT_MASK_HIF0 | \
					 PFE_COH_PORT_MASK_HIF1 | \
					 PFE_COH_PORT_MASK_HIF2 | \
					 PFE_COH_PORT_MASK_HIF3 | \
					 PFE_COH_PORT_MASK_UTIL)
#define PFE_COH_PORTS_MASK_HIF_0_3	(PFE_COH_PORT_MASK_HIF0 | \
					 PFE_COH_PORT_MASK_HIF1 | \
					 PFE_COH_PORT_MASK_HIF2 | \
					 PFE_COH_PORT_MASK_HIF3)

#define SGMII_CSEL			(1U << 0)

#define GPR_PFE_EMACn_PWR_ACK(n)	(1 << (9 + n)) /* RD Only */
#define GPR_PFE_EMACn_PWR_ISO(n)	(1 << (6 + n))
#define GPR_PFE_EMACn_PWR_DWN(n)	(1 << (3 + n))
#define GPR_PFE_EMACn_PWR_CLAMP(n)	(1 << (0 + n))
#define GPR_PFE_EMAC_IF_MII		(1)
#define GPR_PFE_EMAC_IF_RMII		(9)
#define GPR_PFE_EMAC_IF_RGMII		(2)
#define GPR_PFE_EMAC_IF_SGMII		(0)
#define EMAC_MODE_SWR_MASK		(1)

#define PFENG_EMACS_COUNT 3

enum {
	PFENG_MODE_DISABLE = 0,		/* PFE not accessible */
	PFENG_MODE_ENABLE,		/* PFE/EMACs out of reset,
					 * pin/clocks muxing enabled
					 */
};

#define PFENG_MODE_DEFAULT PFENG_MODE_ENABLE

/* MC_ME partition definitions */
#define MC_ME_BASE_ADDR				0x40088000
#define MC_RGM_BASE_ADDR			0x40078000

#define UPTR(PTR)				((uintptr_t)(PTR))
#define MC_ME_PRTN_N(MC_ME, n)			(UPTR(MC_ME) + 0x100 + \
						 (n) * 0x200)
#define MC_ME_PRTN_N_PCONF(MC_ME, n)		(MC_ME_PRTN_N(MC_ME, n))
#define MC_ME_PRTN_N_PUPD(MC_ME, n)		(MC_ME_PRTN_N(MC_ME, n) + 0x4)
#define MC_ME_PRTN_N_STAT(MC_ME, n)		(MC_ME_PRTN_N(MC_ME, n) + 0x8)
#define MC_ME_PRTN_N_COFB0_STAT(MC_ME, n)	(MC_ME_PRTN_N(MC_ME, n) + 0x10)
#define MC_ME_PRTN_N_COFB0_CLKEN(MC_ME, n)	(MC_ME_PRTN_N(MC_ME, n) + 0x30)

/* MC_ME_PRTN_N_* register fields */
#define MC_ME_PRTN_N_PCE		BIT(0)
#define MC_ME_PRTN_N_PCUD		BIT(0)
#define MC_ME_PRTN_N_PCS		BIT(0)
#define MC_ME_PRTN_N_OSSE		BIT(2)
#define MC_ME_PRTN_N_OSSUD		BIT(2)
#define MC_ME_PRTN_N_OSSS		BIT(2)
#define MC_ME_PRTN_N_BLOCK(n)		BIT(n)
#define MC_ME_PRTN_N_REQ(n)		BIT(n)

#define MC_ME_CTL_KEY(MC_ME)		(UPTR(MC_ME) + 0x0)
#define MC_ME_CTL_KEY_KEY		(0x00005AF0)
#define MC_ME_CTL_KEY_INVERTEDKEY	(0x0000A50F)

/* RGM peripheral reset registers */
#define RGM_PRST(MC_RGM, per)		(UPTR(MC_RGM) + 0x40 + \
					 ((per) * 0x8))
#define RGM_PSTAT(rgm, per)		(UPTR(rgm) + 0x140 + \
					 ((per) * 0x8))
#define PRST_PFE			128

/* Reset domain definitions */
#define RDC_BASE_ADDR				0x40080000
#define RDC_RD_N_CTRL(RDC, N)			(UPTR(RDC) + (0x4 * (N)))
#define RDC_RD_N_STATUS(RDC, N)			(UPTR(RDC) + 0x80 + (0x4 * (N)))
#define RD_CTRL_UNLOCK_MASK			(0x80000000)
#define RD_XBAR_DISABLE_MASK			(0x00000008)
#define RDC_RD_CTRL_UNLOCK			BIT(31)
#define RDC_RD_INTERCONNECT_DISABLE		BIT(3)
#define RDC_RD_INTERCONNECT_DISABLE_REQ_STAT	BIT(3)
#define RDC_RD_INTERCONNECT_DISABLE_STAT	BIT(4)
#define RDC_RD_STAT_XBAR_DISABLE_MASK		BIT(4)

struct pfe_hif_ring_my;

struct pfeng_config {
	u32 config_mac_mask;
	u32 config_phy_addr[PFENG_EMACS_COUNT];
};

struct pfeng_priv {
	/* pfe platform members */
	struct pfe_platform_config pfe_cfg;
	struct pfe_fw fw;
	struct pfe_platform *pfe;

	struct mii_dev *mii[PFENG_EMACS_COUNT];
	struct phy_device *phy[PFENG_EMACS_COUNT];
	struct phy xpcs[PFENG_EMACS_COUNT];

	struct udevice *dev;
	const struct pfeng_config *config;
	u32 dev_index;
	u32 if_index;
	bool if_changed;
	bool clocks_done;
	struct pfe_hw_chnl *chnl;
	void *last_rx;
	void *last_tx;
};

/*
 * pfeng_mode environment variable syntax:
 * <state>[,<emac0_interface_type>[,<emac1_intf_type>[,<emac2_intf_type>]]]
 *
 * where:
 *	<state>			: enable/disable
 *	<emac_interface_type>	: none/sgmii/rgmii/rmii/mii(+)
 *				  (+) some might be unsupported on particular
 *				  boards
 */
#define PFENG_ENV_VAR_MODE_NAME "pfeng_mode"

/* pfengemac:
 * the emac used for communication: 0, 1 or 2
 */
#define PFENG_ENV_VAR_EMAC	"pfengemac"

/* pfengfw environment variable syntax:
 * <interface>@<dev>:<part>:<fw_name>
 *
 * where
 *	<interface>		: mmc
 *	<dev>			: device id (in case of sdcard it is 0)
 *	<part>			: partition id
 */
#define PFENG_ENV_VAR_FW_SOURCE	"pfengfw"


bool pfeng_cfg_set_mode(u32 mode, struct udevice *pfe_dev);
int pfeng_set_emacs_from_env(char *env_mode);
void pfeng_cfg_emacs_enable_all(void);
void pfeng_cfg_emacs_disable_all(void);
u32 pfeng_cfg_emac_get_interface(u32 idx);
void pfeng_apply_clocks(struct udevice *pfe_dev);
unsigned long long get_pfe_axi_clk_f(struct udevice *pfe_dev);

/* cmd debug calls */
void pfeng_debug(void);

#endif
