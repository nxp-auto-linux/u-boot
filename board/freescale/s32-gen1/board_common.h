/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018-2021 NXP
 */

#ifndef __S32_GEN1_BOARD_COMMON_H__
#define __S32_GEN1_BOARD_COMMON_H__

#include <config.h>

void setup_iomux_i2c(void);
void setup_iomux_sdhc(void);
void setup_iomux_uart(void);

#ifdef CONFIG_FSL_DSPI
	void setup_iomux_dspi(void);
#endif

#if defined(CONFIG_S32_GEN1)
void setup_iomux_uart0_pc09_pc10(void);
#endif

#if defined(CONFIG_TARGET_S32G274ASIM) || \
	defined(CONFIG_TARGET_S32G274AEMU) || \
	defined(CONFIG_TARGET_S32G399AEMU)
void setup_iomux_uart1_pb09_pb10(void);
#endif

#if CONFIG_IS_ENABLED(NETDEVICES)
void ft_enet_fixup(void *fdt);
u32 s32ccgmac_cfg_get_mode(void);
#endif

#if defined(CONFIG_SAF1508BET_USB_PHY)
void setup_iomux_usb(void);
#endif

#endif /* __S32_GEN1_BOARD_COMMON_H__ */
