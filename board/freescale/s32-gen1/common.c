/*
 * Copyright 2017-2020 NXP
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

#ifdef CONFIG_FSL_QSPI
static void setup_iomux_qspi(void)
{
	/* QSPI_DATA_A[0] */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF5__QSPI_A_DATA0_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF5__QSPI_A_DATA0_IN));

	/* QSPI_DATA_A[1] */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF6__QSPI_A_DATA1_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF6__QSPI_A_DATA1_IN));

	/* QSPI_DATA_A[2] */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF7__QSPI_A_DATA2_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF7__QSPI_A_DATA2_IN));

	/* QSPI_DATA_A[3] */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF8__QSPI_A_DATA3_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF8__QSPI_A_DATA3_IN));

	/* QSPI_DATA_A[4] */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF9__QSPI_A_DATA4_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF9__QSPI_A_DATA4_IN));

	/* QSPI_DATA_A[5] */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF10__QSPI_A_DATA5_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF10__QSPI_A_DATA5_IN));

	/* QSPI_DATA_A[6] */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF11__QSPI_A_DATA6_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF11__QSPI_A_DATA6_IN));

	/* QSPI_DATA_A[7] */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF12__QSPI_A_DATA7_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF12__QSPI_A_DATA7_IN));

	/* QSPI_DQS_A */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DQS,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF13__QSPI_A_DQS_OUT));
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DQS_MUX,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF13__QSPI_A_DQS_IN));

	/* QSPI_CK_A */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_CLK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PG0__QSPI_A_SCK));

	/* QSPI_CK_A_b */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_CLK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PG1__QSPI_A_B_SCK));

	/* QSPI_CK_2A */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_CLK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PG2__QSPI_2A_SCK));

	/* QSPI_CK_2A_b */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_CLK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PG2__QSPI_2A_B_SCK));

	/* QSPI_CS_A0 */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_CS,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PG4__QSPI_A_CS0));

	/* QSPI_CS_A1 */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_CS,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PG5__QSPI_A_CS1));
}
#endif

int board_early_init_f(void)
{
	mscm_init();

#ifdef CONFIG_S32_STANDALONE_BOOT_FLOW
	/* Do these only if TF-A hasn't already. */
	setup_iomux_i2c();
	setup_iomux_uart();
#endif
#ifdef CONFIG_DM_MMC
	setup_iomux_sdhc();
#endif
#ifdef CONFIG_FSL_DSPI
	setup_iomux_dspi();
#endif
#ifdef CONFIG_FSL_QSPI
	setup_iomux_qspi();
#endif
#ifdef CONFIG_SAF1508BET_USB_PHY
	setup_iomux_usb();
#endif
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
#if defined(CONFIG_TARGET_S32G274AEVB)
	puts("Board:\tNXP S32G274A-EVB\n");
#elif defined(CONFIG_TARGET_S32G274ARDB)
	puts("Board:\tNXP S32G274A-RDB\n");
#elif defined(CONFIG_TARGET_S32G274ABLUEBOX3)
	puts("Board:\tNXP S32G274A BlueBox3\n");
#elif defined(CONFIG_TARGET_S32G274ASIM)
	puts("Board:\tVDK for NXP S32G274A VP\n");
#elif defined(CONFIG_TARGET_S32G274AEMU)
	puts("Board:\tZeBu model for NXP S32G274A\n");
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
#elif defined(CONFIG_S32R45)
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
