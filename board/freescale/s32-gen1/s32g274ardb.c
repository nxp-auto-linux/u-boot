// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2020 NXP
 */
#include <asm/arch/soc.h>
#include <board_common.h>
#include <common.h>
#include <s32g274a_common.h>

void setup_iomux_uart(void)
{
#if (CONFIG_FSL_LINFLEX_MODULE == 0)

	/* Muxing for linflex0 */
	setup_iomux_uart0_pc09_pc10();

#elif (CONFIG_FSL_LINFLEX_MODULE == 1)
	/* Muxing for linflex1 */

	/* set PA13 - MSCR[13] - for UART1 TXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART1_TXD,
	       SIUL2_0_MSCRn(SIUL2_PA13_MSCR_S32_G1_UART1));

	/* set PB00 - MSCR[16] - for UART1 RXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART_RXD,
	       SIUL2_0_MSCRn(SIUL2_PB00_MSCR_S32_G1_UART1));

	/* set PB00 - MSCR[736]/IMCR[224] - for UART1 RXD */
	writel(SIUL2_IMCR_S32G_G1_UART1_RXD_to_pad,
	       SIUL2_1_IMCRn(SIUL2_PB00_IMCR_S32_G1_UART1));
#else
#error "Unsupported UART pinmuxing configuration"
#endif
}

void setup_iomux_i2c(void)
{
	/* IO Expander */
	/* I2C0 Serial Data Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C0_SDA_ALT2,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PB_15));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C0_SDA_ALT3,
	       SIUL2_0_IMCRn(SIUL2_PB_15_IMCR_S32G_I2C1_SDA));

	/* I2C0 Serial Clock Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C0_SCLK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PC_00));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C0_SCLK_ALT3,
	       SIUL2_0_IMCRn(SIUL2_PC_00_IMCR_S32G_I2C1_SCLK));

	/* PCIe X1 (P1) */
	/* I2C1 Serial Data Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C1_SDA_ALT3,
	       SIUL2_1_MSCRn(SIUL2_MSCR_S32G_PK_05));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C1_SDA_ALT4,
	       SIUL2_1_IMCRn(SIUL2_PK_05_IMCR_S32G_I2C1_SDA));

	/* I2C1 Serial Clock Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C1_SCLK_ALT3,
	       SIUL2_1_MSCRn(SIUL2_MSCR_S32G_PK_03));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C1_SCLK_ALT5,
	       SIUL2_1_IMCRn(SIUL2_PK_03_IMCR_S32G_I2C1_SCLK));

	/* FLEXRAY connector (J5) */
	setup_iomux_i2c_pc05_pc06();

	/* PMIC - I2C4 */
	setup_iomux_i2c_pc01_pc02();
}
