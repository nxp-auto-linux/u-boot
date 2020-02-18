/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2017-2020 NXP
 */

/*
 * Configuration settings for all the Freescale S32G274A boards.
 */

#ifndef __S32G274A_H
#define __S32G274A_H

#define CONFIG_S32G274A

#include <configs/s32-gen1.h>

#ifndef CONFIG_PRAM
#define CONFIG_PRAM	2048	/* 2MB */
#endif

#if defined(CONFIG_TARGET_S32G274AEVB)
/* EEPROM */
#define CONFIG_SYS_I2C_MXC_I2C1
/* Platform board PCI X16 EXPRESS - I2C_SCL_S0, I2C_SDA_S0  */
#define CONFIG_SYS_I2C_MXC_I2C2
/* Platform board GPIO_J3-17 (SDA), GPIO_J3-19 (SCL0)  */
#define CONFIG_SYS_I2C_MXC_I2C3
/* PMIC */
#define CONFIG_SYS_I2C_MXC_I2C5
#endif

#if defined(CONFIG_TARGET_S32G274ARDB)
/* IO Expander  */
#define CONFIG_SYS_I2C_MXC_I2C1
/* PCIe X1 Connector  */
#define CONFIG_SYS_I2C_MXC_I2C2
/* FLEXRAY_LIN  */
#define CONFIG_SYS_I2C_MXC_I2C3
/* PMIC */
#define CONFIG_SYS_I2C_MXC_I2C5
#endif


#endif
