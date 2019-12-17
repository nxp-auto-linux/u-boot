// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019 NXP
 */

#include <config.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <asm/arch/siul-s32r45.h>

#if defined(CONFIG_TARGET_S32R45XEVB) || defined(CONFIG_TARGET_S32R45XSIM)
void setup_iomux_i2c(void)
{
	/* I2C0 - Serial Data Input */
	writel(SIUL2_MSCR_S32R45_PAD_CTRL_I2C0_SDA,
	       SIUL2_MSCRn(SIUL2_MSCR_S32R45_PB_00));
	writel(SIUL2_IMCR_S32R45_PAD_CTRL_I2C0_SDA,
	       SIUL2_IMCRn(SIUL2_PB_00_IMCR_S32R45_I2C0_SDA));

	/* I2C0 - Serial Clock Input */
	writel(SIUL2_MSCR_S32R45_PAD_CTRL_I2C0_SCLK,
	       SIUL2_MSCRn(SIUL2_MSCR_S32R45_PB_01));
	writel(SIUL2_IMCR_S32R45_PAD_CTRL_I2C0_SCLK,
	       SIUL2_IMCRn(SIUL2_PB_01_IMCR_S32R45_I2C0_SCLK));

	/* I2C1 - Serial Data Input */
	writel(SIUL2_MSCR_S32R45_PAD_CTRL_I2C1_SDA,
	       SIUL2_MSCRn(SIUL2_MSCR_S32R45_PA_15));
	writel(SIUL2_IMCR_S32R45_PAD_CTRL_I2C1_SDA,
	       SIUL2_IMCRn(SIUL2_PA_15_IMCR_S32R45_I2C1_SDA));

	/* I2C1 - Serial Clock Input */
	writel(SIUL2_MSCR_S32R45_PAD_CTRL_I2C1_SCLK,
	       SIUL2_MSCRn(SIUL2_MSCR_S32R45_PA_14));
	writel(SIUL2_IMCR_S32R45_PAD_CTRL_I2C1_SCLK,
	       SIUL2_IMCRn(SIUL2_PA_14_IMCR_S32R45_I2C1_SCLK));
}
#endif
