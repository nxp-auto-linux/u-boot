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

	/* set PC08 - MSCR[40] - for UART1 TXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART1_TXD,
	       SIUL2_0_MSCRn(SIUL2_PC08_MSCR_S32_G1_UART1));

	/* set PC04 - MSCR[36] - for UART1 RXD */
	writel(SIUL2_MSCR_S32G_G1_PORT_CTRL_UART_RXD,
	       SIUL2_0_MSCRn(SIUL2_PC04_MSCR_S32_G1_UART1));

	/* set PC04 - MSCR[736]/IMCR[224] - for UART1 RXD */
	writel(SIUL2_IMCR_S32G_G1_UART1_RXD_to_pad,
	       SIUL2_1_IMCRn(SIUL2_PC04_IMCR_S32_G1_UART1));
#else
#error "Unsupported UART pinmuxing configuration"
#endif
}

void setup_iomux_i2c(void)
{
	/* Plaftorm board - PCI X16 Express (J99) */
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

	/* EEPROM - AT24C01B */
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

	/* Plaftorm board - Arduino connector (J56) */
	setup_iomux_i2c_pc05_pc06();

	/* PMIC - I2C4 */
	setup_iomux_i2c_pc01_pc02();
}

#ifdef CONFIG_SAF1508BET_USB_PHY
struct usb_data_iomux {
	u32 mscr;
	u32 imcr;
};

static const struct usb_data_iomux usb_data_pins[] = {
	{
		.mscr = SIUL2_MSCR_S32_G1_PD14_USB_ULPI_DATA_O0,
		.imcr = SIUL2_MSCR_S32_G1_PD14_USB_ULPI_DATA_I0_IN,
	},
	{
		.mscr = SIUL2_MSCR_S32_G1_PD15_USB_ULPI_DATA_O1,
		.imcr = SIUL2_MSCR_S32_G1_PD15_USB_ULPI_DATA_I1_IN,
	},
	{
		.mscr = SIUL2_MSCR_S32_G1_PE0_USB_ULPI_DATA_O2,
		.imcr = SIUL2_MSCR_S32_G1_PE0_USB_ULPI_DATA_I2_IN,
	},
	{
		.mscr = SIUL2_MSCR_S32_G1_PE1_USB_ULPI_DATA_O3,
		.imcr = SIUL2_MSCR_S32_G1_PE1_USB_ULPI_DATA_I3_IN,
	},
	{
		.mscr = SIUL2_MSCR_S32_G1_PL12_USB_ULPI_DATA_O4,
		.imcr = SIUL2_MSCR_S32_G1_PL12_USB_ULPI_DATA_I4_IN,
	},
	{
		.mscr = SIUL2_MSCR_S32_G1_PL13_USB_ULPI_DATA_O5,
		.imcr = SIUL2_MSCR_S32_G1_PL13_USB_ULPI_DATA_I5_IN,
	},
	{
		.mscr = SIUL2_MSCR_S32_G1_PL14_USB_ULPI_DATA_O6,
		.imcr = SIUL2_MSCR_S32_G1_PL14_USB_ULPI_DATA_I6_IN,
	},
	{
		.mscr = SIUL2_MSCR_S32_G1_PH0_USB_ULPI_DATA_O7,
		.imcr = SIUL2_MSCR_S32_G1_PH0_USB_ULPI_DATA_I7_IN,
	},
};

void setup_iomux_usb(void)
{
	size_t i;
	const struct usb_data_iomux *data_pin;

	for (i = 0; i < ARRAY_SIZE(usb_data_pins); i++) {
		data_pin = &usb_data_pins[i];
		if (i < 4)
			writel(SIUL2_MSCR_S32G_PAD_CTL_USB_DATA,
			       SIUL2_0_MSCRn(data_pin->mscr));
		else
			writel(SIUL2_MSCR_S32G_PAD_CTL_USB_DATA,
			       SIUL2_1_MSCRn(data_pin->mscr));
		writel(SIUL2_MSCR_MUX_MODE_ALT2, SIUL2_1_IMCRn(data_pin->imcr));
	}

	/* USB ULPI Clock In */
	writel(SIUL2_IMCR_S32G_PAD_CTL_USB_DATA,
	       SIUL2_1_MSCRn(SIUL2_MSCR_S32_G1_PL8_USB_ULPI_CLK_OUT));
	writel(SIUL2_MSCR_MUX_MODE_ALT2,
	       SIUL2_1_IMCRn(SIUL2_MSCR_S32_G1_PL8_USB_ULPI_CLK_IN));

	/* USB ULPI Bus Direction */
	writel(SIUL2_IMCR_S32G_PAD_CTL_USB_DATA,
	       SIUL2_1_MSCRn(SIUL2_MSCR_S32_G1_PL9_USB_ULPI_DIR_OUT));
	writel(SIUL2_MSCR_MUX_MODE_ALT2,
	       SIUL2_1_IMCRn(SIUL2_MSCR_S32_G1_PL9_USB_ULPI_DIR_IN));

	/* USB ULPI End Transfer */
	writel(SIUL2_MSCR_S32G_PAD_CTL_USB_STP,
	       SIUL2_1_MSCRn(SIUL2_MSCR_S32_G1_PL10_USB_ULPI_STP));

	/* USB ULPI Data Flow Control */
	writel(SIUL2_IMCR_S32G_PAD_CTL_USB_DATA,
	       SIUL2_1_MSCRn(SIUL2_MSCR_S32_G1_PL11_USB_ULPI_NXT_OUT));
	writel(SIUL2_MSCR_MUX_MODE_ALT2,
	       SIUL2_1_IMCRn(SIUL2_MSCR_S32_G1_PL11_USB_ULPI_NXT_IN));
}
#endif
