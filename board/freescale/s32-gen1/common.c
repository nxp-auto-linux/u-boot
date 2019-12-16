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
#include <asm/arch/s32-gen1/ncore.h>
#include <asm/arch/siul-s32r45.h>

#include "board_common.h"

DECLARE_GLOBAL_DATA_PTR;

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

#ifdef CONFIG_S32_RUN_AT_EL3
	/* Do these only if TF-A hasn't already. */
	setup_iomux_i2c();
	setup_iomux_uart();
#endif
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
#if defined(CONFIG_TARGET_S32G274AEVB)
	puts("Board:\tNXP S32G274A-EVB\n");
#elif defined(CONFIG_TARGET_S32G274ARDB)
	puts("Board:\tNXP S32G274A-RDB\n");
#elif defined(CONFIG_TARGET_S32G274ASIM)
	puts("Board:\tVDK for NXP S32G274A VP\n");
#elif defined(CONFIG_TARGET_S32G274AEMU)
	puts("Board:\tZeBu model for NXP S32G274A\n");
#elif defined(CONFIG_TARGET_S32R45XEVB)
	puts("Board:\tNXP S32R45X-EVB\n");
#elif defined(CONFIG_TARGET_S32R45XSIM)
	puts("Board:\tVDK for NXP S32R45 VP\n");
#elif defined(CONFIG_TARGET_S32R45XEMU)
	puts("Board:\tZeBu model for NXP S32R45\n");
#elif defined(CONFIG_TARGET_S32V344EVB)
	puts("Board:\tNXP S32V344-EVB\n");
#elif defined(CONFIG_TARGET_S32V344SIM)
	puts("Board:\tVDK for NXP S32V344 VP\n");
#else
	puts("Board:\tNXP S32-gen1-xxxxxxx\n");
#endif

	return 0;
}


#if defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	return 0;
}
#endif /* defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP) */

/* Pinmuxing code which is common between at least two boards */

#ifdef CONFIG_S32G274A
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
#elif defined(CONFIG_S32R45X)
void setup_iomux_uart0_pc09_pc10(void)
{
	/* Muxing for linflex0 */

	/* Set PC_10 - MSCR[42] - for LIN0_RX */
	writel(SIUL2_MSCR_S32R45_PORT_CTRL_UART_RXD,
	       SIUL2_0_MSCRn(SIUL2_PC10_MSCR_S32R45_UART0));

	/* Set LIN0_RX - IMCR[512] - to link to PC_10 */
	writel(SIUL2_IMCR_S32R45_UART_RXD_to_pad,
	       SIUL2_0_IMCRn(SIUL2_PC10_IMCR_S32R45_UART0));

	/* Set PC_09 - MSCR[41] - for LIN0_TX */
	writel(SIUL2_MSCR_S32R45_PORT_CTRL_UART0_TXD,
	       SIUL2_0_MSCRn(SIUL2_PC09_MSCR_S32R45_UART0));
}
#endif
