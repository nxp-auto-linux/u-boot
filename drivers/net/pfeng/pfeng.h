// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019 NXP
 *
 */

#ifndef _PFENG_H_
#define _PFENG_H_

#include "oal.h"
#include "pfe_platform.h"
#include "pfe_hif_drv.h"

#define S32G_PFE_REGS_BASE 0x46000000
#define S32G_PFE_EMACS_INTF_SEL 0x4007CA04
#define S32G_PFE_PRW_CTRL 0x4007CA20

#define PFENG_EMACS_COUNT 3

struct pfeng_config {
	u32 config_mac_mask;
	u32 config_phy_addr[PFENG_EMACS_COUNT];
};

struct pfeng_priv {
	/* pfe platform members */
        pfe_platform_config_t pfe_cfg;
        pfe_fw_t fw;
        pfe_platform_t *pfe;
        pfe_log_if_t *iface;
        pfe_hif_drv_t *hif;
        pfe_hif_chnl_t *channel;
        pfe_hif_drv_client_t *client;

	struct mii_dev *mii[PFENG_EMACS_COUNT];
	struct phy_device *phy[PFENG_EMACS_COUNT];

	struct udevice *dev;
	const struct pfeng_config *config;
	u32 dev_index;
	u32 emac_index;
	int emac_changed;

	void *last_rx;
	void *last_tx;
};


/*
 * pfeng_mode environment variable syntax:
 * <state>[,<emac0_interface_type>[,<emac1_intf_type>[,<emac2_intf_type>]]]
 *
 * where:
 * 	state			: enable/disable
 * 	emac_interface_type	: none/sgmii/rgmii/rmii/mii(+)
 * 				  (+) some might be unsupported on your board
 */
#define PFENG_ENV_VAR_MODE_NAME "pfeng_mode"

#define PFENG_ENV_VAR_EMAC	"pfengemac"
#define PFENG_ENV_VAR_FW_SOURCE	"pfengfw"

/* cmd debug calls */
int pfeng_debug_emac(u32 idx);
int pfeng_debug_hif(void);
int pfeng_debug_class(void);

#endif
