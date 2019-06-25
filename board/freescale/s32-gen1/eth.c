// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 NXP
 */

#include <common.h>
#include <dm.h>
#include <asm/io.h>
#include <net.h>
#include <netdev.h>
#include <malloc.h>
#include <asm/types.h>
#include <asm/arch/soc.h>
#if CONFIG_IS_ENABLED(DWC_ETH_QOS_S32CC)
#include <dm/platform_data/dwc_eth_qos_dm.h>
#endif

/* GMAC driver for common chassis */
#if CONFIG_IS_ENABLED(DWC_ETH_QOS_S32CC)

/* driver platform data (TODO: remove when switching to DT) */
static struct eqos_pdata dwmac_pdata = {
	.eth = {
		/* registers base address */
		.iobase = (phys_addr_t)ETHERNET_0_BASE_ADDR,
		/* there is only one phy */
		.phy_interface = 0,
		/* generic fake HW addr */
		.enetaddr = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05 },
		/* max 1 Gbps */
		.max_speed = 1000,
	},
	/* vendor specific driver config */
	.config = &eqos_s32cc_config,
};

U_BOOT_DEVICE(dwmac_s32cc) = {
	.name = "eth_eqos",
	.platdata = &dwmac_pdata,
};

#endif
