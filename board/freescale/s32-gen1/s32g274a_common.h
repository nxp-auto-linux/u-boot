/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020 NXP
 *
 */

#ifndef S32G274A_COMMON_H
#define S32G274A_COMMON_H

#include <config.h>

#if defined(CONFIG_TARGET_S32G274AEVB) || \
	defined(CONFIG_TARGET_S32G274ARDB) || \
	defined(CONFIG_TARGET_S32G274ABLUEBOX3)
void setup_iomux_i2c_pb00_pb01(void);
void setup_iomux_i2c_pb03_pb04(void);
void setup_iomux_i2c_pb05_pb06(void);
void setup_iomux_i2c_pc01_pc02(void);
void setup_iomux_i2c_pc05_pc06(void);
#endif

#endif /* S32G274A_COMMON_H */
