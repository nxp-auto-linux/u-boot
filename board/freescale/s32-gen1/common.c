// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017-2021 NXP
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
	setup_iomux_uart();

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
#if defined(CONFIG_TARGET_S32G2XXAEVB)
	printf("Board:\tNXP %s-EVB\n", get_s32g2_deriv_name());
#elif defined(CONFIG_TARGET_S32G274ARDB)
	printf("Board:\tNXP %s-RDB\n", get_s32g2_deriv_name());
#elif defined(CONFIG_TARGET_S32G274ABLUEBOX3)
	puts("Board:\tNXP S32G274A BlueBox3\n");
#elif defined(CONFIG_TARGET_S32G3XXAEVB)
	puts("Board:\tNXP S32G399A-EVB\n");
#elif defined(CONFIG_TARGET_S32G274ASIM)
	puts("Board:\tVDK for NXP S32G274A VP\n");
#elif defined(CONFIG_TARGET_S32G274AEMU)
	puts("Board:\tZeBu model for NXP S32G274A\n");
#elif defined(CONFIG_TARGET_S32G398AEMU)
	puts("Board:\tZeBu model for NXP S32G398A\n");
#elif defined(CONFIG_TARGET_S32R45EVB)
	puts("Board:\tNXP S32R45-EVB\n");
#elif defined(CONFIG_TARGET_S32R45SIM)
	puts("Board:\tVDK for NXP S32R45 VP\n");
#elif defined(CONFIG_TARGET_S32R45EMU)
	puts("Board:\tZeBu model for NXP S32R45\n");
#else
	puts("Board:\tNXP S32-gen1-xxxxxxx\n");
#endif

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

#if defined(CONFIG_TARGET_S32G274ASIM) || \
	defined(CONFIG_TARGET_S32G274AEMU) || \
	defined(CONFIG_TARGET_S32G398AEMU)
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

