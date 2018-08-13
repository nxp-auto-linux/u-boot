/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __S32_GEN1_BOARD_COMMON_H__
#define __S32_GEN1_BOARD_COMMON_H__

void setup_iomux_i2c(void);

#ifdef CONFIG_FSL_DSPI
	void setup_iomux_dspi(void);
#endif

#endif /* __S32_GEN1_BOARD_COMMON_H__ */
