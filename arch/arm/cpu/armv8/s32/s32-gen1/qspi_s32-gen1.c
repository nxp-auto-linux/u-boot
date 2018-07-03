/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/arch/qspi_common.h>
#include <asm/arch/siul.h>
#include <asm/io.h>

void qspi_iomux(void)
{
	/* QSPI0_A_CS0 */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_CS0_MUX |
	       SIUL2_MSCR_S32_G1_QSPI_CLK_BASE,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PG4__QSPI_A_CS0));
	/* QSPI0_A_SCK */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_SCK_MUX	|
	       SIUL2_MSCR_S32_G1_QSPI_CLK_BASE,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PG0__QSPI_A_SCK));

	/* A_DATA 0-7 */
	/* QSPI0_A_D0 */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF5__QSPI_A_DATA0_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF5__QSPI_A_DATA0_IN));
	/* QSPI0_A_D1 */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF6__QSPI_A_DATA1_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF6__QSPI_A_DATA1_IN));
	/* QSPI0_A_D2 */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF7__QSPI_A_DATA2_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF7__QSPI_A_DATA2_IN));
	/* QSPI0_A_D3 */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF8__QSPI_A_DATA3_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF8__QSPI_A_DATA3_IN));
	/* QSPI0_A_D4 */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF9__QSPI_A_DATA4_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF9__QSPI_A_DATA4_IN));
	/* QSPI0_A_D5 */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF10__QSPI_A_DATA5_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF10__QSPI_A_DATA5_IN));
	/* QSPI0_A_D6 */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF11__QSPI_A_DATA6_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF11__QSPI_A_DATA6_IN));
	/* QSPI0_A_D7 */
	writel(SIUL2_MSCR_S32_G1_QSPI_A_DATA0_7,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF12__QSPI_A_DATA7_OUT));
	writel(SIUL2_IMCR_S32_G1_QSPI_A_DATA_MUX,
	       SIUL2_MSCRn(SIUL2_MSCR_S32_G1_PF12__QSPI_A_DATA7_IN));
}

#ifndef CONFIG_S32_FLASH
int do_qspinor_setup(cmd_tbl_t *cmdtp, int flag, int argc,
		     char * const argv[])
{
	printf("SD/eMMC is disabled. SPI flash is active and can be used!\n");
	qspi_iomux();
	return 0;
}
#endif

/* qspinor setup */
U_BOOT_CMD(
	flsetup, 1, 1, do_qspinor_setup,
	"setup qspi pinmuxing and qspi registers for access to flash",
	"\n"
	"Set up the pinmuxing and qspi registers to access the flash\n"
	"    and disconnect from the SD/eMMC.\n"
);
