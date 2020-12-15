// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2021 NXP
 */

#include <common.h>
#include <asm/arch/soc.h>

#include "board_common.h"

void setup_iomux_uart(void)
{
#if (CONFIG_FSL_LINFLEX_MODULE == 0)

	/* Muxing for linflex0 */
	setup_iomux_uart0_pc09_pc10();

#elif (CONFIG_FSL_LINFLEX_MODULE == 1)

	/* Muxing for linflex1 */
	setup_iomux_uart1_pb09_pb10();

#else
#error "Unsupported UART pinmuxing configuration"
#endif
}

void setup_iomux_i2c(void)
{
	/* I2C0 - Serial Data Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C0_SDA,
	       SIUL2_MSCRn(SIUL2_MSCR_S32G_PB_00));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C0_SDA,
	       SIUL2_IMCRn(SIUL2_PB_00_IMCR_S32G_I2C0_SDA));

	/* I2C0 - Serial Clock Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C0_SCLK,
	       SIUL2_MSCRn(SIUL2_MSCR_S32G_PB_01));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C0_SCLK,
	       SIUL2_IMCRn(SIUL2_PB_01_IMCR_S32G_I2C0_SCLK));

	/* I2C1 - Serial Data Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C1_SDA,
	       SIUL2_MSCRn(SIUL2_MSCR_S32G_PB_04));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C1_SDA,
	       SIUL2_IMCRn(SIUL2_PB_04_IMCR_S32G_I2C1_SDA));

	/* I2C1 - Serial Clock Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C1_SCLK,
	       SIUL2_MSCRn(SIUL2_MSCR_S32G_PB_03));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C1_SCLK,
	       SIUL2_IMCRn(SIUL2_PB_03_IMCR_S32G_I2C1_SCLK));

	/* I2C2 - Serial Data Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C2_SDA,
	       SIUL2_MSCRn(SIUL2_MSCR_S32G_PB_06));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C2_SDA,
	       SIUL2_IMCRn(SIUL2_PB_06_IMCR_S32G_I2C2_SDA));

	/* I2C2 - Serial Clock Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C2_SCLK,
	       SIUL2_MSCRn(SIUL2_MSCR_S32G_PB_05));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C2_SCLK,
	       SIUL2_IMCRn(SIUL2_PB_05_IMCR_S32G_I2C2_SCLK));

	/* I2C3 - Serial Data Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C3_SDA,
	       SIUL2_MSCRn(SIUL2_MSCR_S32G_PB_13));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C3_SDA,
	       SIUL2_IMCRn(SIUL2_PB_13_IMCR_S32G_I2C3_SDA));

	/* I2C3 - Serial Clock Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C3_SCLK,
	       SIUL2_MSCRn(SIUL2_MSCR_S32G_PB_07));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C3_SCLK,
	       SIUL2_IMCRn(SIUL2_PB_07_IMCR_S32G_I2C3_SCLK));

	/* I2C4 - Serial Data Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C4_SDA,
	       SIUL2_MSCRn(SIUL2_MSCR_S32G_PC_01));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C4_SDA,
	       SIUL2_IMCRn(SIUL2_PC_01_IMCR_S32G_I2C4_SDA));

	/* I2C4 - Serial Clock Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C4_SCLK,
	       SIUL2_MSCRn(SIUL2_MSCR_S32G_PC_02));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C4_SCLK,
	       SIUL2_IMCRn(SIUL2_PC_02_IMCR_S32G_I2C4_SCLK));
}

#ifdef CONFIG_FSL_DSPI
void setup_iomux_dspi(void)
{
	/* Muxing for DSPI0 */

	/* Configure Chip Select Pins */
	writel(SUIL2_MSCR_S32G_PAD_CTL_SPI0_CS0,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PB_00));

	/* MSCR */
	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI0_SOUT,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_15));

	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI0_SIN,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_14));

	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI0_SCK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_13));

	/* IMCR */
	writel(SIUL2_IMCR_S32G_PAD_CTL_SPI0_SIN,
	       SIUL2_1_IMCRn(SIUL2_PA_14_IMCR_S32G_SPI0_SIN));
}
#endif

int board_early_init_r(void)
{
	setup_iomux_i2c();
	setup_iomux_dspi();

	return 0;
}
