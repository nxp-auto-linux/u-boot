// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019-2020 NXP
 */

#include <config.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <s32g274a_common.h>

#if defined(CONFIG_TARGET_S32G274ASIM) || defined(CONFIG_TARGET_S32G274AEMU)
void setup_iomux_uart1_pb09_pb10(void)
{
	/* Muxing for linflex1 */

	/* set PB09 - MSCR[25] - for UART1 TXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART1_TXD,
	       SIUL2_0_MSCRn(SIUL2_PB09_MSCR_S32_G1_UART1));

	/* set PB10 - MSCR[26] - for UART1 RXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART_RXD,
	       SIUL2_0_MSCRn(SIUL2_PB10_MSCR_S32_G1_UART1));

	/* set PB10 - MSCR[736]/IMCR[224] - for UART1 RXD */
	writel(SIUL2_IMCR_S32G_G1_UART1_RXD_to_pad,
	       SIUL2_1_IMCRn(SIUL2_PB10_IMCR_S32_G1_UART1));
}
#endif

#if defined(CONFIG_TARGET_S32G274AEVB) || \
	defined(CONFIG_TARGET_S32G274ARDB) || \
	defined(CONFIG_TARGET_S32G274ABLUEBOX3)
void setup_iomux_i2c_pb00_pb01(void)
{
	/* I2C0 Serial Data Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C0_SDA,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PB_00));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C0_SDA,
	       SIUL2_0_IMCRn(SIUL2_PB_00_IMCR_S32G_I2C0_SDA));

	/* I2C0 Serial Clock Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C0_SCLK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PB_01));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C0_SCLK,
	       SIUL2_0_IMCRn(SIUL2_PB_01_IMCR_S32G_I2C0_SCLK));
}

void setup_iomux_i2c_pb03_pb04(void)
{
	/* I2C1 Serial Data Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C1_SDA,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PB_04));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C1_SDA,
	       SIUL2_1_IMCRn(SIUL2_PB_04_IMCR_S32G_I2C1_SDA));

	/* I2C1 Serial Clock Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C1_SCLK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PB_03));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C1_SCLK,
	       SIUL2_1_IMCRn(SIUL2_PB_03_IMCR_S32G_I2C1_SCLK));
}

void setup_iomux_i2c_pb05_pb06(void)
{
	/* I2C2 - Serial Data Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C2_SDA,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PB_06));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C2_SDA,
	       SIUL2_1_IMCRn(SIUL2_PB_06_IMCR_S32G_I2C2_SDA));

	/* I2C2 - Serial Clock Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C2_SCLK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PB_05));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C2_SCLK,
	       SIUL2_1_IMCRn(SIUL2_PB_05_IMCR_S32G_I2C2_SCLK));
}

void setup_iomux_i2c_pc01_pc02(void)
{
	/* PMIC */
	/* I2C4 Serial Data Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C4_SDA,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PC_01));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C4_SDA,
	       SIUL2_1_IMCRn(SIUL2_PC_01_IMCR_S32G_I2C4_SDA));

	/* I2C4 Serial Clock Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C4_SCLK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PC_02));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C4_SCLK,
	       SIUL2_1_IMCRn(SIUL2_PC_02_IMCR_S32G_I2C4_SCLK));
}

void setup_iomux_i2c_pc05_pc06(void)
{
	/* I2C2 Serial Data Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C2_SDA,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PB_06));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C2_SDA,
	       SIUL2_1_IMCRn(SIUL2_PB_06_IMCR_S32G_I2C2_SDA));

	/* I2C2 Serial Clock Input */
	writel(SIUL2_MSCR_S32G_PAD_CTRL_I2C2_SCLK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PB_05));
	writel(SIUL2_IMCR_S32G_PAD_CTRL_I2C2_SCLK,
	       SIUL2_1_IMCRn(SIUL2_PB_05_IMCR_S32G_I2C2_SCLK));
}

#if defined(CONFIG_FSL_DSPI)
__weak void setup_iomux_dspi(void)
{
	/* Muxing for DSPI1 */

	/* Configure Chip Select Pins */
	writel(SUIL2_MSCR_S32G_PAD_CTL_SPI1_CS0,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_07));

	/* MSCR */
	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI1_SOUT,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_06));

	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI1_SIN,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PF_15));

	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI1_SCK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_08));

	/* IMCR */
	writel(SIUL2_IMCR_S32G_PAD_CTL_SPI1_SIN,
	       SIUL2_1_IMCRn(SIUL2_PF_15_IMCR_S32G_SPI1_SIN));

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

#if defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
void setup_iomux_sdhc(void)
{
	/* Set iomux PADS for USDHC */
	/* PC14 pad: uSDHC SD0_CLK_O  */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_CLK, SIUL2_0_MSCRn(46));

	/* PC15 pad: uSDHC SDO_CMD_0 */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_CMD, SIUL2_0_MSCRn(47));
	writel(0x2, SIUL2_0_MSCRn(515));

	/* PD00 pad: uSDHC SD0_D_O[0] */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_0_MSCRn(48));
	writel(0x2, SIUL2_0_MSCRn(516));

	/* PD01 pad: uSDHC SD0_D_O[1] */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_0_MSCRn(49));
	writel(0x2, SIUL2_0_MSCRn(517));

	/* PD02 pad: uSDHC SD0_D_O[2] */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_0_MSCRn(50));
	writel(0x2, SIUL2_0_MSCRn(520));

	/* PD03 pad: uSDHC SD0_D_O[3] */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_0_MSCRn(51));
	writel(0x2, SIUL2_0_MSCRn(521));

	/* PD04 pad: uSDHC SD0_D_O[4] */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_0_MSCRn(52));
	writel(0x2, SIUL2_0_MSCRn(522));

	/* PD05 pad: uSDHC SD0_D_O[5] */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_0_MSCRn(53));
	writel(0x2, SIUL2_0_MSCRn(523));

	/* PD06 pad: uSDHC SD0_D_O[6] */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_0_MSCRn(54));
	writel(0x2, SIUL2_0_MSCRn(519));

	/* PD07 pad: uSDHC SD0_D_O[7] */
	writel(SIUL2_USDHC_S32_G1_PAD_CTRL_DATA, SIUL2_0_MSCRn(55));
	writel(0x2, SIUL2_0_MSCRn(518));

	/* PD08 pad: uSDHC SDO_RST */
	writel(SIUL2_USDHC_S32_G1_PAD_RST, SIUL2_0_MSCRn(56));

	/* PD10 pad: uSDHC SD0_DQS_I */
	writel(0x2, SIUL2_0_MSCRn(524));
}
#else
/* The previous-stage bootloader (TF-A) is expected to have handled this. */
void setup_iomux_sdhc(void)
{
}
#endif /* CONFIG_S32_STANDALONE_BOOT_FLOW */
#endif
