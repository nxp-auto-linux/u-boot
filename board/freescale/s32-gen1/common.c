// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017-2022 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/soc.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <miiphy.h>
#include <asm/arch/s32-gen1/ncore.h>
#include <asm/arch/siul-s32r45.h>

#if defined(CONFIG_TARGET_S32G2XXAEVB) || defined(CONFIG_TARGET_S32G3XXAEVB) ||\
	defined(CONFIG_TARGET_S32G274ARDB)
#include <dm/uclass.h>
#include <misc.h>
#include <s32gen1_siul2_nvram.h>
#endif

#include "board_common.h"

DECLARE_GLOBAL_DATA_PTR;

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_early_init_f(void)
{
	setup_iomux_uart();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = CONFIG_SYS_FSL_DRAM_SIZE1 + 0x100;

	return 0;
}

#if defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

#if CONFIG_IS_ENABLED(NETDEVICES)
	ft_enet_fixup(blob);
#endif

	return 0;
}
#endif /* defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP) */

#if defined(CONFIG_TARGET_S32G274AEMU) || \
	defined(CONFIG_TARGET_S32G399AEMU)
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

void setup_iomux_uart0_pc09_pc10(void)
{
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
}

