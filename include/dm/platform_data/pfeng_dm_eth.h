/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (c) 2020 Imagination Technologies Limited
 * Copyright 2019-2020 NXP
 */

#ifndef __PFENG_DM_ETH_H__
#define __PFENG_DM_ETH_H__
#include <net.h>

struct pfeng_config;

struct pfeng_pdata {
	struct eth_pdata eth;
	struct pfeng_config *config;
};

#if CONFIG_IS_ENABLED(FSL_PFENG)
extern struct pfeng_config pfeng_s32g274a_config;

u32 pfeng_cfg_get_mode(void);
u32 pfeng_cfg_emac_get_interface(u32 idx);
#endif

#if CONFIG_IS_ENABLED(OF_CONTROL)
static const struct udevice_id pfeng_eth_ids[] = {
#if CONFIG_IS_ENABLED(FSL_PFENG)
	{
		.compatible = "fsl,s32g274a-pfe",
		.data = (ulong)&pfeng_s32g274a_config
	},
#endif /* CONFIG_FSL_PFENG */

	{ }
};
#endif /* CONFIG_IS_ENABLED(OF_CONTROL) */

#endif /* __PFENG_DM_ETH_H__ */
