/*
 * Copyright 2018-2019 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/soc.h>

#include "board_common.h"
#include "s32-gen1/mem_map/mem_map_siul.h"

void setup_iomux_uart(void)
{
#if (CONFIG_FSL_LINFLEX_MODULE == 0)
	/* Muxing for linflex0 */

	/* set PC09 - MSCR[41] - for UART0 TXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART0_TXD,
	       SIUL2_0_MSCRn(SIUL2_PC09_MSCR_S32_G1_UART0));

	/* set PC10 - MSCR[42] - for UART0 RXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART_RXD,
	       SIUL2_0_MSCRn(SIUL2_PC10_MSCR_S32_G1_UART0));

	/* set PC10 - MSCR[512]/IMCR[0] - for UART0 RXD */
	writel(SIUL2_IMCR_S32G_G1_UART0_RXD_to_pad,
	       SIUL2_0_IMCRn(SIUL2_PC10_IMCR_S32_G1_UART0));
#elif (CONFIG_FSL_LINFLEX_MODULE == 1)
	/* Muxing for linflex1 */

#if defined(CONFIG_TARGET_TYPE_S32GEN1_SIMULATOR) || \
	defined(CONFIG_TARGET_TYPE_S32GEN1_EMULATOR)
	/* set PB09 - MSCR[25] - for UART1 TXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART1_TXD,
	       SIUL2_0_MSCRn(SIUL2_PB09_MSCR_S32_G1_UART1));

	/* set PB10 - MSCR[26] - for UART1 RXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART_RXD,
	       SIUL2_0_MSCRn(SIUL2_PB10_MSCR_S32_G1_UART1));

	/* set PB10 - MSCR[736]/IMCR[224] - for UART1 RXD */
	writel(SIUL2_IMCR_S32G_G1_UART1_RXD_to_pad,
	       SIUL2_1_IMCRn(SIUL2_PB10_IMCR_S32_G1_UART1));
#else
	/* set PC08 - MSCR[40] - for UART1 TXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART1_TXD,
	       SIUL2_0_MSCRn(SIUL2_PC08_MSCR_S32_G1_UART1));

	/* set PC04 - MSCR[36] - for UART1 RXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART_RXD,
	       SIUL2_0_MSCRn(SIUL2_PC04_MSCR_S32_G1_UART1));

	/* set PC04 - MSCR[736]/IMCR[224] - for UART1 RXD */
	writel(SIUL2_IMCR_S32G_G1_UART1_RXD_to_pad,
	       SIUL2_1_IMCRn(SIUL2_PC04_IMCR_S32_G1_UART1));
#endif

#else
#error "Unsupported UART pinmuxing configuration"
#endif
}

void setup_iomux_i2c(void)
{
#ifdef CONFIG_TARGET_TYPE_S32GEN1_SIMULATOR
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
#endif
}

#ifdef CONFIG_FSL_DSPI
void setup_iomux_dspi(void)
{
	/* Muxing for DSPI5 */

	/* Configure Chip Select Pins */
	writel(SUIL2_MSCR_S32G_PAD_CTL_SPI5_CS0,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_12));

	/* MSCR */
	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI5_SOUT,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_11));

	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI5_SIN,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_10));

	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI5_SCK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_09));

	/* IMCR */
	writel(SIUL2_IMCR_S32G_PAD_CTL_SPI5_SIN,
	       SIUL2_1_IMCRn(SIUL2_PA_10_IMCR_S32G_SPI5_SIN));
}
#endif