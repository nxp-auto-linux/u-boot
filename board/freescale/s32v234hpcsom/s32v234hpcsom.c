// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/xrdc.h>
#include <asm/arch/soc.h>
#include <fdt_support.h>
#include <linux/libfdt.h>

DECLARE_GLOBAL_DATA_PTR;

static void setup_iomux_uart(void)
{
	/* Muxing for linflex0 */

	/* set PA12 - MSCR[12] - for UART0 TXD */
	writel(SIUL2_MSCR_PORT_CTRL_UART_TXD, SIUL2_MSCRn(SIUL2_MSCR_PA12));

	/* set PA11 - MSCR[11] - for UART0 RXD */
	writel(SIUL2_MSCR_PORT_CTRL_UART_RXD, SIUL2_MSCRn(SIUL2_MSCR_PA11));
	/* set UART0 RXD - IMCR[200] - to link to PA11 */
	writel(SIUL2_IMCR_UART_RXD_to_pad, SIUL2_IMCRn(SIUL2_IMCR_UART0_RXD));

	/* set PA14 - MSCR[14] - for UART1 TXD*/
	writel(SIUL2_MSCR_PORT_CTRL_UART_TXD, SIUL2_MSCRn(SIUL2_MSCR_PA14));

	/* set PA13 - MSCR[13] - for UART1 RXD */
	writel(SIUL2_MSCR_PORT_CTRL_UART_RXD, SIUL2_MSCRn(SIUL2_MSCR_PA13));
	/* set UART1 RXD - IMCR[202] - to link to PA13 */
	writel(SIUL2_IMCR_UART_RXD_to_pad, SIUL2_IMCRn(SIUL2_IMCR_UART1_RXD));
}

static void setup_iomux_i2c(void)
{
	/* I2C0 - Serial Data Input */
	writel(SIUL2_PAD_CTRL_I2C0_MSCR_SDA_AC15, SIUL2_MSCRn(SIUL2_MSCR_PG3));
	writel(SIUL2_PAD_CTRL_I2C0_IMCR_SDA_AC15,
	       SIUL2_IMCRn(SIUL2_IMCR_I2C0_DATA));

	/* I2C0 - Serial Clock Input */
	writel(SIUL2_PAD_CTRL_I2C0_MSCR_SCLK_AE15, SIUL2_MSCRn(SIUL2_MSCR_PG4));
	writel(SIUL2_PAD_CTRL_I2C0_IMCR_SCLK_AE15,
	       SIUL2_IMCRn(SIUL2_IMCR_I2C0_CLK));

	/* I2C1 - Serial Data Input */
	writel(SIUL2_PAD_CTRL_I2C1_MSCR_SDA, SIUL2_MSCRn(SIUL2_MSCR_PG5));
	writel(SIUL2_PAD_CTRL_I2C1_IMCR_SDA,
	       SIUL2_IMCRn(SIUL2_IMCR_I2C1_DATA));

	/* I2C1 - Serial Clock Input */
	writel(SIUL2_PAD_CTRL_I2C1_MSCR_SCLK, SIUL2_MSCRn(SIUL2_MSCR_PG6));
	writel(SIUL2_PAD_CTRL_I2C1_IMCR_SCLK,
	       SIUL2_IMCRn(SIUL2_IMCR_I2C1_CLK));

	/* I2C2 - Serial Data Input */
	writel(SIUL2_PAD_CTRL_I2C2_MSCR_SDA, SIUL2_MSCRn(SIUL2_MSCR_PB3));
	writel(SIUL2_PAD_CTRL_I2C2_IMCR_SDA,
	       SIUL2_IMCRn(SIUL2_IMCR_I2C2_DATA));

	/* I2C2 - Serial Clock Input */
	writel(SIUL2_PAD_CTRL_I2C2_MSCR_SCLK, SIUL2_MSCRn(SIUL2_MSCR_PB4));
	writel(SIUL2_PAD_CTRL_I2C2_IMCR_SCLK,
	       SIUL2_IMCRn(SIUL2_IMCR_I2C2_CLK));
}

void setup_iomux_sdhc(void)
{
	/* Set iomux PADS for USDHC */

	/* PK6 pad: uSDHC clk */
	writel(SIUL2_USDHC_PAD_CTRL_CLK, SIUL2_MSCRn(150));
	writel(0x3, SIUL2_MSCRn(902));

	/* PK7 pad: uSDHC CMD */
	writel(SIUL2_USDHC_PAD_CTRL_CMD, SIUL2_MSCRn(151));
	writel(0x3, SIUL2_MSCRn(901));

	/* PK8 pad: uSDHC DAT0 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT0_3, SIUL2_MSCRn(152));
	writel(0x3, SIUL2_MSCRn(903));

	/* PK9 pad: uSDHC DAT1 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT0_3, SIUL2_MSCRn(153));
	writel(0x3, SIUL2_MSCRn(904));

	/* PK10 pad: uSDHC DAT2 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT0_3, SIUL2_MSCRn(154));
	writel(0x3, SIUL2_MSCRn(905));

	/* PK11 pad: uSDHC DAT3 */
	writel(SIUL2_USDHC_PAD_CTRL_DAT0_3, SIUL2_MSCRn(155));
	writel(0x3, SIUL2_MSCRn(906));
}

static void mscm_init(void)
{
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_BASE_ADDR;
	int i;

	for (i = 0; i < MSCM_IRSPRC_NUM; i++)
		writew(MSCM_IRSPRC_CPn_EN, &mscmir->irsprc[i]);
}

static void setup_xrdc(void)
{
	/* See S32V234 User Manual, chapter Extended Resource Domain Controller
	 * (XRDC), section S32V234 specific MRC instance for SRAM controller
	 * memory protection.
	 * Let ISP, Camera, Decoder Pixel Interface and Encoder Bit Stream to
	 * access the SRAM memory.
	 */

	/* Write start of the memory region. */
	writel(XRDC_ADDR_MIN, XRDC_MRGD_W0_16);
	/* Write end of the memory region. */
	writel(XRDC_ADDR_MAX, XRDC_MRGD_W1_16);
	/* Write valid bit for the memory region */
	writel(XRDC_VALID, XRDC_MRGD_W3_16);

	writel(XRDC_ADDR_MIN, XRDC_MRGD_W0_17);
	writel(XRDC_ADDR_MAX, XRDC_MRGD_W1_17);
	writel(XRDC_VALID, XRDC_MRGD_W3_17);

	writel(XRDC_ADDR_MIN, XRDC_MRGD_W0_18);
	writel(XRDC_ADDR_MAX, XRDC_MRGD_W1_18);
	writel(XRDC_VALID, XRDC_MRGD_W3_18);

	writel(XRDC_ADDR_MIN, XRDC_MRGD_W0_19);
	writel(XRDC_ADDR_MAX, XRDC_MRGD_W1_19);
	writel(XRDC_VALID, XRDC_MRGD_W3_19);
}

int board_early_init_f(void)
{
	clock_init();
	mscm_init();

	setup_iomux_uart();
	setup_iomux_enet();
	setup_iomux_i2c();
#ifdef CONFIG_FSL_DCU_FB
	setup_iomux_dcu();
#endif
#ifdef CONFIG_DCU_QOS_FIX
	board_dcu_qos();
#endif
	setup_xrdc();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_FSL_DRAM_SIZE1 + 0x100;

	return 0;
}

int checkboard(void)
{
	printf("Board: %s\n", CONFIG_SYS_CONFIG_NAME);

	return 0;
}

#if defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	return 0;
}
#endif

