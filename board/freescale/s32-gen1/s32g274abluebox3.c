// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2020 NXP
 */
#include <asm/arch/soc.h>
#include <board_common.h>
#include <common.h>
#include <s32g274a_common.h>
#include <miiphy.h>
#include <phy.h>
#include <i2c.h>
#include <asm/arch-s32/s32-gen1/serdes_regs.h>

#define BLUEBOX3_S32G_PHY_ADDR_5	0x05
#define BLUEBOX3_S32G_PHY_ADDR_6	0x06

struct pin {
	int nr;
	const char *name;
};

static const struct pin input_pins[] = {
	{SIUL2_MSCR_S32G_PA_09, "S32G_SD_CD_B"},
	{SIUL2_MSCR_S32G_PA_11, "FS85_G_PGOOD"},
	{SIUL2_MSCR_S32G_PB_07, "IRQ_HDR_B"},
	{SIUL2_MSCR_S32G_PB_08, "IRQ_EPHY34_B"},
	{SIUL2_MSCR_S32G_PB_11, "ACC_INT1"},
	{SIUL2_MSCR_S32G_PB_12, "ACC_INT2"},
	{SIUL2_MSCR_S32G_PB_13, "FS85_G_FCCU0"},
	{SIUL2_MSCR_S32G_PB_14, "FS85_G_FCCU1"},
	{SIUL2_MSCR_S32G_PB_15, "FS85_G_INTB"},
	{SIUL2_MSCR_S32G_PC_06, "FS85_G_FS0B"},
	{SIUL2_MSCR_S32G_PD_14, "S32G_PGOOD_3V3_PCIE"},
	{SIUL2_MSCR_S32G_PD_15, "S32G_PGOOD_12V0_G"},
	{SIUL2_MSCR_S32G_PE_14, "S32G_PGOOD_1V2_DDR2"},
	{SIUL2_MSCR_S32G_PH_00, "S32G_FS85_G_PGOOD"},
	{SIUL2_MSCR_S32G_PH_01, "S32G_PGOOD_0V7"},
	{SIUL2_MSCR_S32G_PH_02, "S32G_PS_VDD_PG"},
	{SIUL2_MSCR_S32G_PH_03, "S32G_PGOOD_1V2_DDR1"},
	{SIUL2_MSCR_S32G_PH_06, "S32G_SD_FAULT_B"},
	{SIUL2_MSCR_S32G_PH_10, "S32G_PS_VTT1_PG_B"},
	{SIUL2_MSCR_S32G_PJ_00, "S32G_PGOOD_PF71"},
	{SIUL2_MSCR_S32G_PL_10, "S32G_PS_VTT2_PG_B"},
	{SIUL2_MSCR_S32G_PL_11, "S32G_PGOOD_12V0_PCIE_B"},
	{SIUL2_MSCR_S32G_PL_12, "S32G_12V0_LX_PGOOD"},
	{SIUL2_MSCR_S32G_PL_13, "S32G_5V0_LX_PGOOD"},
	{SIUL2_MSCR_S32G_PL_14, "S32G_FS85_LX_PGOOD"},
};

static const size_t input_pins_size = ARRAY_SIZE(input_pins);

void setup_iomux_uart(void)
{
#if (CONFIG_FSL_LINFLEX_MODULE == 0)

	/* Muxing for linflex0 */
	setup_iomux_uart0_pc09_pc10();
#else
#error "Unsupported UART pinmuxing configuration"
#endif
}

void setup_iomux_i2c(void)
{
	setup_iomux_i2c_pb00_pb01();
	setup_iomux_i2c_pb03_pb04();
	setup_iomux_i2c_pb05_pb06();
	setup_iomux_i2c_pc01_pc02();
}

#ifdef CONFIG_FSL_DSPI
void setup_iomux_dspi(void)
{
	/* DSPI0 */
	writel(SUIL2_MSCR_S32G_PAD_CTL_SPI0_CS1,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PB_09));

	writel(SUIL2_MSCR_S32G_PAD_CTL_SPI0_CS2,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PB_10));

	/* MSCR */
	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI0_SOUT,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_15));

	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI0_SIN,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_14));

	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI0_SCK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_13));

	/* IMCR */
	writel(SIUL2_IMCR_S32G_PAD_CTL_SPI0_SIN,
	       SIUL2_1_IMCRn(SIUL2_PA_14_IMCR_S32G_SPI0_SIN));

	/* DSPI1 */
	writel(SUIL2_MSCR_S32G_PAD_CTL_SPI1_CS0,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_07));

	writel(SUIL2_MSCR_S32G_PAD_CTL_SPI1_CS3,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PE_05));

	/* MSCR */
	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI1_SOUT,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_06));

	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI1_SIN,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PF_15));

	writel(SIUL2_MSCR_S32G_PAD_CTL_SPI1_SCK,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32G_PA_08));

	/* IMCR */
	writel(SIUL2_IMCR_S32G_PAD_CTL_SPI1_SIN,
	       SIUL2_1_IMCRn(SIUL2_PF_15_IMCR_S32G_SPI1_SIN));
}
#endif

static void setup_iomux_mdio(void)
{
	/* set PF2 - MSCR[82] - for PFE MAC0 MDC*/
	writel(SIUL2_MSCR_S32_G1_PFE_OUT | SIUL2_MSCR_MUX_MODE_ALT1,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PF2));

	/* set PE15 - MSCR[79] - for PFE MAC0 MDO*/
	writel(SIUL2_MSCR_S32_G1_PFE_OUT | SIUL2_MSCR_S32_G1_PFE_IN |
	       SIUL2_MSCR_MUX_MODE_ALT1,
	       SIUL2_0_MSCRn(SIUL2_MSCR_S32_G1_PE15));
	writel(SIUL2_MSCR_MUX_MODE_ALT2,
	       SIUL2_1_MSCRn(SIUL2_MSCR_S32_G1_PFE_MAC0_MDO_IN));
}

static int do_board_status(cmd_tbl_t *cmdtp, int flag, int argc,
			   char * const argv[])
{
	int i;

	for (i = 0; i < input_pins_size ; i++)
		printf("PIN: %3d\t%25s\tlogical level: %d\n",
		       input_pins[i].nr, input_pins[i].name,
		       readb(SIUL2_PDI_N(input_pins[i].nr)));

	return 0;
}

static void setup_iomux_input_gpios(void)
{
	int i;

	for (i = 0; i < input_pins_size ; i++) {
		if (input_pins[i].nr < SIUL2_0_MAX_GPIO)
			writel(SIUL2_MSCR_GPI_G1,
			       SIUL2_0_MSCRn(input_pins[i].nr));
		else
			writel(SIUL2_MSCR_GPI_G1,
			       SIUL2_1_MSCRn(input_pins[i].nr));
	}
}

static void release_resets(void)
{
	int i;
	static const u8 reset_pins[] = {
		SIUL2_MSCR_S32G_PA_12,
		SIUL2_MSCR_S32G_PB_02,
		SIUL2_MSCR_S32G_PC_00,
		SIUL2_MSCR_S32G_PC_05
	};

	for (i = 0; i < sizeof(reset_pins); i++) {
		writeb(SIUL2_GPIO_VALUE1, SIUL2_PDO_N(reset_pins[i]));
		writel(SIUL2_MSCR_GPO_G1, SIUL2_0_MSCRn(reset_pins[i]));
	}
}

int board_early_init_r(void)
{
	release_resets();
	setup_iomux_input_gpios();
	setup_iomux_mdio();

	return 0;
}

int last_stage_init(void)
{
	struct udevice *eth;
	struct phy_device *phy;
	struct mii_dev *bus;

	eth = eth_get_dev_by_name("eth_pfeng");
	bus = miiphy_get_dev_by_name("pfeng_emac_0");
	if (eth && bus) {
		phy = phy_connect(bus, BLUEBOX3_S32G_PHY_ADDR_5, eth,
				  PHY_INTERFACE_MODE_RGMII);
		if (phy)
			phy_config(phy);

		phy = phy_connect(bus, BLUEBOX3_S32G_PHY_ADDR_6, eth,
				  PHY_INTERFACE_MODE_RGMII);
		if (phy)
			phy_config(phy);
	}

	return 0;
}

U_BOOT_CMD(board_status, CONFIG_SYS_MAXARGS, 1, do_board_status,
	   "Print status of all of the GPIO input pins",
	   NULL
);

