/*
 * (C) Copyright 2016 NXP Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef S32V244_PINMUX_
#define S32V244_PINMUX_

#ifdef CONFIG_FSL_LINFLEXUART
void setup_iomux_uart(void);
#else
static inline void setup_iomux_uart(void)
{
}
#endif

#ifdef CONFIG_NET
void setup_iomux_enet(void);
#else
static inline void setup_iomux_enet(void)
{
}
#endif

#if defined(CONFIG_SYS_I2C) || defined(CONFIG_I2C_MULTI_BUS)
void setup_iomux_i2c(void);
#else
static inline void setup_iomux_i2c(void)
{
}
#endif

#ifdef CONFIG_SYS_USE_NAND
void setup_iomux_nfc(void);
#else
static inline void setup_iomux_nfc(void)
{
}
#endif

void setup_xrdc(void);

#endif /* S32V244_PINMUX_ */
