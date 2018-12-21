/*
 * Copyright 2017-2019 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/clock.h>
#include <asm/arch/soc.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <miiphy.h>
#include <netdev.h>

#include "board_common.h"

DECLARE_GLOBAL_DATA_PTR;

__weak void setup_iomux_uart(void)
{
	/* Muxing for linflex0 */

	/* set PC09 - MSCR[41] - for UART0 TXD */
	writel(SIUL2_MSCR_S32_G1_PORT_CTRL_UART_TXD,
	       SIUL2_MSCRn(SIUL2_PC09_MSCR_S32_G1_UART0));

	/* set PC10 - MSCR[42] - for UART0 RXD */
	writel(SIUL2_MSCR_S32_G1_PORT_CTRL_UART_RXD,
	       SIUL2_MSCRn(SIUL2_PC10_MSCR_S32_G1_UART0));
	/* set UART0 RXD - IMCR[512] - to link to PC10 */
	writel(SIUL2_IMCR_S32_G1_UART_RXD_to_pad,
	       SIUL2_IMCRn(SIUL2_PC10_ISCR_S32_G1_UART0));
}

static void mscm_init(void)
{
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_BASE_ADDR;
	int i;

	for (i = 0; i < MSCM_IRSPRC_NUM; i++)
		writew(MSCM_IRSPRC_CPn_EN, &mscmir->irsprc[i]);
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_early_init_f(void)
{
	mscm_init();
	clock_init();

	setup_iomux_enet();
	setup_iomux_i2c();
	//setup_iomux_nfc();
	setup_iomux_uart();
#ifdef CONFIG_FSL_DSPI
	setup_iomux_dspi();
#endif
	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

	return 0;
}

int checkboard(void)
{
	puts("Board: s32x simulator\n");

	return 0;
}


#if defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	return 0;
}
#endif /* defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP) */
