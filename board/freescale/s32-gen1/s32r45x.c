/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/soc.h>

#include "board_common.h"

void setup_iomux_i2c(void)
{
	/* I2C0 - Serial Data Input */
	writel(SIUL2_MSCR_S32R_PAD_CTRL_I2C0_SDA,
	       SIUL2_MSCRn(SIUL2_MSCR_S32R_PB_00));
	writel(SIUL2_IMCR_S32R_PAD_CTRL_I2C0_SDA,
	       SIUL2_IMCRn(SIUL2_PB_00_IMCR_S32R_I2C0_SDA));

	/* I2C0 - Serial Clock Input */
	writel(SIUL2_MSCR_S32R_PAD_CTRL_I2C0_SCLK,
	       SIUL2_MSCRn(SIUL2_MSCR_S32R_PB_01));
	writel(SIUL2_IMCR_S32R_PAD_CTRL_I2C0_SCLK,
	       SIUL2_IMCRn(SIUL2_PB_01_IMCR_S32R_I2C0_SCLK));

	/* I2C1 - Serial Data Input */
	writel(SIUL2_MSCR_S32R_PAD_CTRL_I2C1_SDA,
	       SIUL2_MSCRn(SIUL2_MSCR_S32R_PA_15));
	writel(SIUL2_IMCR_S32R_PAD_CTRL_I2C1_SDA,
	       SIUL2_IMCRn(SIUL2_PA_15_IMCR_S32R_I2C1_SDA));

	/* I2C1 - Serial Clock Input */
	writel(SIUL2_MSCR_S32R_PAD_CTRL_I2C1_SCLK,
	       SIUL2_MSCRn(SIUL2_MSCR_S32R_PA_14));
	writel(SIUL2_IMCR_S32R_PAD_CTRL_I2C1_SCLK,
	       SIUL2_IMCRn(SIUL2_PA_14_IMCR_S32R_I2C1_SCLK));
}

#ifdef CONFIG_FSL_DSPI
static void setup_iomux_dspi(void)
{
	/* Configure Chip Select Pins */
	writel(SUIL2_MSCR_S32R_PAD_CTL_SPI5_CS0,
	       SIUL2_MSCRn(SIUL2_MSCR_S32R_PA_10));

	/* MSCR */
	writel(SIUL2_MSCR_S32R_PAD_CTL_SPI5_SOUT,
	       SIUL2_MSCRn(SIUL2_MSCR_S32R_PJ_5));

	writel(SIUL2_MSCR_S32R_PAD_CTL_SPI5_SIN,
	       SIUL2_MSCRn(SIUL2_MSCR_S32R_PJ_4));

	writel(SIUL2_MSCR_S32R_PAD_CTL_SPI5_SCK,
	       SIUL2_MSCRn(SIUL2_MSCR_S32R_PA_9));

	/* IMCR */
	writel(SIUL2_IMCR_S32R_PAD_CTL_SPI5_SIN,
	       SIUL2_IMCRn(SIUL2_PJ_4_IMCR_S32R_SPI5_SIN));
}
#endif
