/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018-2022 NXP
 */

#ifndef __S32_GEN1_BOARD_COMMON_H__
#define __S32_GEN1_BOARD_COMMON_H__

#include <config.h>
#include <asm/types.h>

#if CONFIG_IS_ENABLED(NETDEVICES)
void ft_enet_fixup(void *fdt);
u32 s32ccgmac_cfg_get_mode(int cardnum);
#endif

#endif /* __S32_GEN1_BOARD_COMMON_H__ */
