// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <fdt_support.h>
#include <linux/libfdt.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c.h>

#ifdef CONFIG_PHY_MICREL
#include <micrel.h>
#endif

#ifdef CONFIG_SJA1105
#include "sja1105.h"
#endif

DECLARE_GLOBAL_DATA_PTR;

static void setup_iomux_gpio(void)
{
	/* Only Device ID pin configuration.
	 * To release the slave V2s out of reset, the output buffers will not be
	 * configured now, but in release_slaves(), after setting the data
	 * registers.
	 */
	static const u8 id_pins[] = {
		SIUL2_MSCR_PF11,
		SIUL2_MSCR_PF12,
		SIUL2_MSCR_PF13,
	};
	u8 i, size = ARRAY_SIZE(id_pins);

	for (i = 0; i < size; i++)
		writel(SIUL2_MSCR_GPI, SIUL2_MSCRn(id_pins[i]));
}

static int get_device_id(void)
{
	/* SIUL2_PGPDI5 lsbyte provides the values of all the three ID pins. */
	u8 values = readb(SIUL2_PGPDIn(5));

	return (values & SIUL2_PPDIO_BIT(SIUL2_MSCR_PF11)) >> 4 |
	       (values & SIUL2_PPDIO_BIT(SIUL2_MSCR_PF12)) >> 2 |
	       (values & SIUL2_PPDIO_BIT(SIUL2_MSCR_PF13));
}

static void release_slaves(void)
{
	static const u8 reset_pins[] = {
		SIUL2_PK12_MSCR,
		SIUL2_PK15_MSCR,
		SIUL2_PL0_MSCR,
		SIUL2_PL1_MSCR,
		SIUL2_PL2_MSCR
	};
	u8 i, data = 0, size = ARRAY_SIZE(reset_pins);
	unsigned long reg, old_reg = 0;

	for (i = 0; i < size; i++, old_reg = reg) {
		reg = SIUL2_PPDO_BYTE(reset_pins[i]);
		if (i && reg != old_reg) {
			writeb(data | readb(old_reg), old_reg);
			data = 0;
		}
		data |= SIUL2_PPDIO_BIT(reset_pins[i]);
	}

	writeb(data | readb(old_reg), old_reg);

	for (i = 0; i < size; i++)
		writel(SIUL2_MSCR_GPO, SIUL2_MSCRn(reset_pins[i]));
}

#ifdef CONFIG_FSL_DSPI
static void setup_iomux_dspi(void)
{
	/* Muxing for DSPI0 */

	/* Configure Chip Select Pins */
	writel(SIUL2_PAD_CTRL_DSPI0_MSCR_CS0_OUT, SIUL2_MSCRn(SIUL2_MSCR_PB8));
	writel(SIUL2_PAD_CTRL_DSPI0_MSCR_CS1_OUT, SIUL2_MSCRn(SIUL2_MSCR_PB13));
	writel(SIUL2_PAD_CTRL_DSPI0_MSCR_CS2_OUT, SIUL2_MSCRn(SIUL2_MSCR_PB14));
	writel(SIUL2_PAD_CTRL_DSPI0_MSCR_CS3_OUT, SIUL2_MSCRn(SIUL2_MSCR_PB15));

	/* MSCR */
	writel(SIUL2_PAD_CTRL_DSPI0_MSCR_SOUT_OUT, SIUL2_MSCRn(SIUL2_MSCR_PB6));

	writel(SIUL2_PAD_CTRL_DSPI0_MSCR_SCK_OUT, SIUL2_MSCRn(SIUL2_MSCR_PB5));

	writel(SIUL2_PAD_CTRL_DSPI0_MSCR_SIN_OUT, SIUL2_MSCRn(SIUL2_MSCR_PB7));

	/* IMCR */
	writel(SIUL2_PAD_CTRL_DSPI0_IMCR_SIN_IN,
	       SIUL2_IMCRn(SIUL2_PB7_IMCR_SPI0_SIN));
}
#endif

static void setup_iomux_uart(void)
{
	/* Muxing for linflex0 */

	/* set PA12 - MSCR[12] - for UART0 TXD */
	writel(SIUL2_MSCR_PORT_CTRL_UART_TXD, SIUL2_MSCRn(SIUL2_MSCR_PA12));

	/* set PA11 - MSCR[11] - for UART0 RXD */
	writel(SIUL2_MSCR_PORT_CTRL_UART_RXD, SIUL2_MSCRn(SIUL2_MSCR_PA11));
	/* set UART0 RXD - IMCR[200] - to link to PA11 */
	writel(SIUL2_IMCR_UART_RXD_to_pad, SIUL2_IMCRn(SIUL2_IMCR_UART0_RXD));
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

static void mscm_init(void)
{
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_BASE_ADDR;
	int i;

	for (i = 0; i < MSCM_IRSPRC_NUM; i++)
		writew(MSCM_IRSPRC_CPn_EN, &mscmir->irsprc[i]);
}

int board_phy_config(struct phy_device *phydev)
{
#ifdef CONFIG_PHY_MICREL
	/* Enable all AutoNeg capabilities */
	ksz9031_phy_extended_write(phydev, 0x02,
				   MII_KSZ9031_EXT_OP_MODE_STRAP_OVRD,
				   MII_KSZ9031_MOD_DATA_NO_POST_INC,
				   MII_KSZ9031_EXT_OMSO_RGMII_ALL_CAP_OVRD);

	/* Reset the PHY so that the previous changes take effect */
	phy_write(phydev, CONFIG_FEC_MXC_PHYADDR, MII_BMCR, BMCR_RESET);
#endif

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_early_init_f(void)
{
	u8 id;

	clock_init();
	mscm_init();

	setup_iomux_gpio();
	id = get_device_id();
	if (id == 1)
		release_slaves();

	setup_iomux_uart();
	setup_iomux_enet();
	setup_iomux_i2c();
#ifdef CONFIG_FSL_DSPI
	if (id == 1)
		setup_iomux_dspi();
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
	printf("Board: %s (V2-%d)\n", CONFIG_SYS_CONFIG_NAME, get_device_id());

	return 0;
}

void board_net_init(void)
{
#ifdef CONFIG_SJA1105
	/* Only probe the switch if we are going to use networking.
	 * The probe has a self check so it will quietly exit if we call it
	 * twice.
	 */
	sja1105_probe(SJA_1_CS, SJA_1_BUS);
	/* The SJA switch can have its ports RX lines go out of sync. They need
	 * to be reseted in order to allow network traffic.
	 */
	sja1105_reset_ports(SJA_1_CS, SJA_1_BUS);
#endif
}

#if defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);
	return 0;
}
#endif /* defined(CONFIG_OF_FDT) && defined(CONFIG_OF_BOARD_SETUP) */
