// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2022 NXP
 */
#include <asm/arch/soc.h>
#include <board_common.h>
#include <common.h>
#include <dm/uclass.h>

#define SJA1105_NAME	"ethernet-switch@0"

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

int misc_init_r(void)
{
#if CONFIG_IS_ENABLED(NET) && CONFIG_IS_ENABLED(FSL_PFENG) && \
	CONFIG_IS_ENABLED(SJA1105)
	struct udevice *dev;

	/* Probe sja1105 in order to provide a clock for the PFE2 interface,
	 * otherwise clock init for this interface will fail.
	 * The return value is not checked on purpose. If the S32G PROCEVB
	 * board is used without PLATEVB board the uclass_get_device_by_name
	 * call will fail and returning and error code will break the init_call
	 * sequence of the u-boot.
	 */
	uclass_get_device_by_name(UCLASS_MISC, SJA1105_NAME, &dev);
#endif
	return 0;
}
