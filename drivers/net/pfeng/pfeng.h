/* SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause */
/*
 * Copyright 2019-2020 NXP
 *
 */

#ifndef _PFENG_H_
#define _PFENG_H_

#include "oal.h"
#include "pfe_platform.h"
#include "pfe_hif_drv.h"

#define S32G_PFE_REGS_BASE		0x46000000
#define S32G_PFE_EMACn_MODE(n)		(S32G_PFE_REGS_BASE + 0xA0000 + \
					 (0x4000 * n) + 0x1000)
#define S32G_PFE_EMACS_INTF_SEL		0x4007CA04
#define S32G_PFE_PRW_CTRL		0x4007CA20
#define S32G_MAIN_GENCTRL1		0x4007CAE4

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
	PFENG_MODE_RUN,			/* PFE fully functional */
};

#define PFENG_MODE_DEFAULT PFENG_MODE_ENABLE

struct pfeng_config {
	u32 config_mac_mask;
	u32 config_phy_addr[PFENG_EMACS_COUNT];
};

struct pfeng_priv {
	/* pfe platform members */
	pfe_platform_config_t pfe_cfg;
	pfe_fw_t fw;
	pfe_platform_t *pfe;

	pfe_hif_drv_t *hif;
	pfe_hif_chnl_t *channel;

	pfe_hif_drv_client_t *client;
	pfe_log_if_t *logif_emac, *logif_hif;
	pfe_phy_if_t *phyif_emac, *phyif_hif;

	struct mii_dev *mii[PFENG_EMACS_COUNT];
	struct phy_device *phy[PFENG_EMACS_COUNT];

	struct udevice *dev;
	const struct pfeng_config *config;
	u32 dev_index;
	u32 if_index;
	int if_changed;

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


bool pfeng_cfg_set_mode(u32 mode);
int pfeng_set_emacs_from_env(char *env_mode);
void pfeng_cfg_emacs_enable_all(void);
void pfeng_cfg_emacs_disable_all(void);
u32 pfeng_cfg_emac_get_interface(u32 idx);
void pfeng_apply_clocks(void);

/* SGMII/XPCS */
int pfeng_serdes_wait_link(int emac);
int pfeng_serdes_emac_is_init(int emac);

/* cmd debug calls */
int pfeng_debug_emac(u32 idx);
int pfeng_debug_hif(void);
int pfeng_debug_class(void);

#endif
