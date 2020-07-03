// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2019-2020 NXP
 */

#include <config.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <asm/arch/siul-s32r45.h>

#if defined(CONFIG_TARGET_S32R45EVB) || defined(CONFIG_TARGET_S32R45SIM)
void setup_iomux_i2c(void)
{
	/* I2C0 - Serial Data Input */
	writel(SIUL2_MSCR_S32R45_PAD_CTRL_I2C0_SDA,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32R45_PB_00));
	writel(SIUL2_IMCR_S32R45_PAD_CTRL_I2C0_SDA,
	       SIUL2_0_IMCRn(SIUL2_PB_00_IMCR_S32R45_I2C0_SDA));

	/* I2C0 - Serial Clock Input */
	writel(SIUL2_MSCR_S32R45_PAD_CTRL_I2C0_SCLK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32R45_PB_01));
	writel(SIUL2_IMCR_S32R45_PAD_CTRL_I2C0_SCLK,
	       SIUL2_0_IMCRn(SIUL2_PB_01_IMCR_S32R45_I2C0_SCLK));

	/* I2C1 - Serial Data Input */
	writel(SIUL2_MSCR_S32R45_PAD_CTRL_I2C1_SDA,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32R45_PA_15));
	writel(SIUL2_IMCR_S32R45_PAD_CTRL_I2C1_SDA,
	       SIUL2_1_IMCRn(SIUL2_PA_15_IMCR_S32R45_I2C1_SDA));

	/* I2C1 - Serial Clock Input */
	writel(SIUL2_MSCR_S32R45_PAD_CTRL_I2C1_SCLK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32R45_PA_14));
	writel(SIUL2_IMCR_S32R45_PAD_CTRL_I2C1_SCLK,
	       SIUL2_1_IMCRn(SIUL2_PA_14_IMCR_S32R45_I2C1_SCLK));
}
#endif

#if defined(CONFIG_TARGET_S32R45EVB)
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

	/* PD08 pad: uSDHC SD0_RST */
	writel(SIUL2_USDHC_S32_G1_PAD_RST, SIUL2_0_MSCRn(56));
}
#endif

