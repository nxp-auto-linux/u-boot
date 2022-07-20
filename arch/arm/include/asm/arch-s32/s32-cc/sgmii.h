/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2022 NXP
 *
 * The SerDes XPCS config function definitions
 */

#ifndef SERDES_SGMII_H
#define SERDES_SGMII_H

enum serdes_mode;
enum serdes_xpcs_mode;
enum serdes_clock_fmhz;
enum serdes_xpcs_mode_gen2;

void serdes_pma_mode5(void __iomem *base);
void serdes_pcs_mode5(void __iomem *base);

int s32_eth_xpcs_init(void __iomem *xpcs0, void __iomem *xpcs1,
		      int platform_serdes_id, enum serdes_mode ss_mode,
		      enum serdes_xpcs_mode xpcs_mode,
		      bool ext_clk,
		      enum serdes_clock_fmhz fmhz,
		      enum serdes_xpcs_mode_gen2 xpcs[2]);

#endif  /* SERDES_SGMII_H */
