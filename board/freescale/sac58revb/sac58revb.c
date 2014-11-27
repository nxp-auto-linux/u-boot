/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * U-Boot Implementation for the Freescale SAC58R EVB
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/arch/iomux-sac58r.h>
#include <asm/arch/crm_regs.h>
#include <asm/arch/clock.h>
#include <mmc.h>
#include <fsl_esdhc.h>
#include <miiphy.h>
#include <netdev.h>
#include <i2c.h>
#include <asm/imx-common/mxc_i2c.h>
#include <asm/arch/dmachmux-sac58r.h>
#include <asm/gpio.h>

#include "sac58revb_int_routing.h"

DECLARE_GLOBAL_DATA_PTR;


#ifdef CONFIG_SYS_I2C_MXC
/* I2C0, backlight, MIPI-CSI, CS4288 */
struct i2c_pads_info i2c_pad_info0 = {
	.scl = {
		.i2c_mode = SAC58R_PAD_PA1_I2C0_SCL,
		.gpio_mode = SAC58R_PAD_PA1_GPIO_1,
		.gp = 1
	},
	.sda = {
		.i2c_mode = SAC58R_PAD_PA0_I2C0_SDA,
		.gpio_mode = SAC58R_PAD_PA0_GPIO_0,
		.gp = 0
	}
};

/* I2C1 Tuner I2S */
struct i2c_pads_info i2c_pad_info1 = {
	.scl = {
		.i2c_mode = SAC58R_PAD_PE19_I2C1_SCL,
		.gpio_mode = SAC58R_PAD_PE19_GPIO_148,
		.gp = 148
	},
	.sda = {
		.i2c_mode = SAC58R_PAD_PE10_I2C1_SDA,
		.gpio_mode = SAC58R_PAD_PE10_GPIO_147,
		.gp = 147
	}
};

/* I2C3: iPod header */
struct i2c_pads_info i2c_pad_info3 = {
	.scl = {
		.i2c_mode = SAC58R_PAD_PH2_I2C3_SCL,
		.gpio_mode = SAC58R_PAD_PH2_GPIO_226,
		.gp = 226
	},
	.sda = {
		.i2c_mode = SAC58R_PAD_PH4_I2C3_SDA,
		.gpio_mode = SAC58R_PAD_PH4_GPIO_228,
		.gp = 228
	}
};
#endif


/* Micron MT41J64M16 @ 416 MHz*/
/* 13 row addr, 10 col addr, burst length 8, data size 32 */
#define MDCTL_CONFIG 	0x02190000

void setup_iomux_ddr(void)
{
	int i;
	int ddr_iomuxc_reg = 0x400B1000;

	/* 0x001701E0 For all control signals (not DQS) and address pins.
	This means:
		- DDR_ODT = 0b010 (60 Ohm ODT, change)
		- DDR_SEL = 0b11 (DDR 3 Mode, default)
		- DDR_INPUT = 0b1 (Differential, default)
		- DDR_TRIM = 0b (No delay, default)
		- HYS = 0b1 (CMOS input, default)
		- DSE = 0b111 (34 Ohm DDR, change)
		- PUS = 0b10 (100K Pullup, default)
		- PKE = 0b0 (Pull/Keeper disabled, default).
		- PUE = 0b0 (Keeper select, default).

		No change has been made to the DDR_ZQ pin,
		which keeps its default value of 0x00060000 (DDR3 Mode).
	*/
	for (i=0;i<32;i++) {
		writel( 0x1701E0, ddr_iomuxc_reg);
		ddr_iomuxc_reg += 4;
	}


	/* 0x000701E0 For D0-D31 (Data) and DQS signals.
		This means:
		- DDR_SEL = 0b11 (DDR 3 Mode, default)
		- DDR_INPUT = 0b1 (Differential, default)
		- DDR_TRIM = 0b (No delay, default)
		- HYS = 0b1 (CMOS input, default)
		- DSE = 0b111 (34 Ohm DDR, change)
		- PUS = 0b10 (100K Pullup, default)
		- PKE = 0b0 (Pull/Keeper disabled, default).
		- PUE = 0b0 (Keeper select, default).
	*/
	ddr_iomuxc_reg = 0x400B1000 + 0x80;
	for (i=0;i<36;i++) {
		/* DQS[0..3] and DATA[0..31] */
		writel( 0x0701E0, ddr_iomuxc_reg);
		ddr_iomuxc_reg += 4;
	}

	writel( 0x1701E0, 0x400B1110); /* ODT-1 */
	writel( 0x1701E0, 0x400B1114); /* ODT-2 */
}

void ddr_ctrl_init(void)
{
	int con_ack = 0;

	/* Soft reset */
	writel(2, 0x40169018); //MDMISC

	/* Set MDSCR[CON_REQ] (configuration request) */
	writel(0x00008000, 0x4016901C); //MDSCR

	/* Wait for configuration acknowledgement */
	while (con_ack == 0) {
		con_ack = readl(0x4016901C) & (1<<14);
	}

	/* MDCFG0: tRFC=48,tXS=52,tXP=3,tXPDLL=10,tFAW=30,tCL=6 */
	writel(0x303475D3, 0x4016900C); //MDCFG0

	/* MDCFG1:  tRCD=6,tRP=6,tRC=20,tRAS=15,tRPA=1,tWR=6,tMRD=4,tCWL=5 */
	writel(0xB66E8A83, 0x40169010); //MDCFG1

	/* MDCFG2: tDLLK=512,tRTP=4,tWTR=4,tRRD=4 */
	writel(0x01FF00DB, 0x40169014); //MDCFG2

	/*MDOTC:  tAOFPD=1,tAONPD=1,tANPD=4,tAXPD=4,tODTLon=5,tODT_idle_off=0 */
	writel(0x11335030, 0x40169008); //MDOTC (timing param)

	/* MDMISC: WALAT=1, BI bank interleave off, MIF3=3, RALAT=1, 8 banks, DDR3 */
	writel(0x00010640, 0x40169018); //MDMISC 

	/* MDOR: WALAT=1, BI bank interleave on, MIF3=3, RALAT=1, 8 banks, DDR3 */
	writel(0x00341023, 0x40169030); //MDOR
	writel(MDCTL_CONFIG, 0x40169000); //MDCTL

	/* Perform ZQ calibration */
	writel(0xA1390003, 0x40169800); //MPZQHWCTRL: Force h/w calibration
	writel(0x33333333, 0x4016981C);
	writel(0x33333333, 0x40169820);
	writel(0x33333333, 0x40169824);
	writel(0x33333333, 0x40169828);
	writel(0x33333333, 0x4016982C);
	writel(0x33333333, 0x40169830);
	writel(0x33333333, 0x40169834);
	writel(0x33333333, 0x40169838);

	/* Enable MMDC with CS0 */
	writel(MDCTL_CONFIG + 0x80000000, 0x40169000); //MDCTL

	/*** Complete the initialization sequence as defined by JEDEC */
	/* Configure MR2: CWL=5, self-refresh=off, self-refresh temp=normal */
	writel(0x00008032, 0x4016901C);
	/* Configure MR3: normal operation */
	writel(0x00008033, 0x4016901C);
	/* Configure MR1: enable DLL, drive strength=40R, AL off, ODT=40R, write levelling off, TDQS=0, Qoff=on */
	writel(0x00068031, 0x4016901C);
	/* Configure MR0: BL=8, CL=6, DLL reset, write recovery=6, precharge PD off */
	writel(0x05208030, 0x4016901C);
	/* DDR ZQ calibration */
	writel(0x04008040, 0x4016901C);

	writel(0x0000004F, 0x40169040); //MDASP: 256 MB memory

	/* Configure the power down and self-refresh entry and exit parameters */
	/* Read delay line offsets */
	writel(0x4B494F4E, 0x40169848); //MPRDDLCTL,
	/* Write delay line offsets */
	writel(0x38353635, 0x40169850); //MPWRDLCTL0
	/* Read DQS gating control 0 */
	writel(0x41520142, 0x4016983C); //MPDGCTRL0
	/* Read DQS gating control 1 */
	writel(0x01560152, 0x40169840); //MPDGCTRL1
	/* Read/write command delay - default */
	writel(0x000026D2, 0x4016902C); //MDRWD
	/* ODT timing control */
	writel(0x22334010, 0x40169008); //MDOTC (timing param)

	/* Power down control */
	writel(0x00020024, 0x40169004); //MDPDC
	/* Refresh control */
	writel(0x30B01800, 0x40169020); //MDREF
	/* 60R nominal */
	writel(0x0002222F, 0x40169818); //MPODTCTRL
	/* Deassert the configuration request */
	writel(0x00000000, 0x4016901C); //MDSCR 1
}


int dram_init(void)
{
	gd->ram_size = ((ulong)CONFIG_DDR_MB * SZ_1M);

	return 0;
}

#ifdef	CONFIG_LVDS
static void setup_iomux_lvds(void)
{
	static const iomux_v3_cfg_t lvds_pads[] = {
			SAC58R_PAD_PD12__LVDS0_CLKP,
			SAC58R_PAD_PD13__LVDS0_CLKN,
			SAC58R_PAD_PD11__LVDS0_TX0P,
			SAC58R_PAD_PD14__LVDS0_TX0N,
			SAC58R_PAD_PD15__LVDS0_TX1P,
			SAC58R_PAD_PD16__LVDS0_TX1N,
			SAC58R_PAD_PD17__LVDS0_TX2P,
			SAC58R_PAD_PD18__LVDS0_TX2N,
			SAC58R_PAD_PD19__LVDS0_TX3P,
			SAC58R_PAD_PD20__LVDS0_TX3N,
			SAC58R_PAD_PK7__GPIO327,
			SAC58R_PAD_PK9__GPIO329
	};
	
	imx_iomux_v3_setup_multiple_pads(lvds_pads, ARRAY_SIZE(lvds_pads));
}
#endif

static void setup_iomux_uart(void)
{
	static const iomux_v3_cfg_t uart_pads[] = {
		SAC58R_PAD_PH14__UART0_TX,
		SAC58R_PAD_PH15__UART0_RX,
		SAC58R_PAD_PF11__UART1_TX,
		SAC58R_PAD_PF12__UART1_RX,
		SAC58R_PAD_PL5__UART2_TX,
		SAC58R_PAD_PL4__UART2_RX,
		SAC58R_PAD_PE28__UART3_TX,
		SAC58R_PAD_PE27__UART3_RX,
		SAC58R_PAD_PH0__UART4_TX,
		SAC58R_PAD_PH1__UART4_RX,
		SAC58R_PAD_PH9__UART5_TX,
		SAC58R_PAD_PH10__UART5_RX,
	};

	imx_iomux_v3_setup_multiple_pads(uart_pads, ARRAY_SIZE(uart_pads));
}

static void setup_iomux_dspi(void)
{
	static const iomux_v3_cfg_t dspi0_pads[] = {
		SAC58R_PAD_PH8_SPIO_PCS2,
		SAC58R_PAD_PH3_SPIO_PCS3,
		SAC58R_PAD_PK14_SPIO_SOUT,
		SAC58R_PAD_PK15_SPIO_PCS0,
		SAC58R_PAD_PK16_SPIO_SCK,
		SAC58R_PAD_PK17_SPIO_SIN,
	};

	imx_iomux_v3_setup_multiple_pads(dspi0_pads, ARRAY_SIZE(dspi0_pads));
}

#ifdef	CONFIG_FEC_MXC
static void setup_iomux_enet(void)
{
	static const iomux_v3_cfg_t enet_pads[] = {
		SAC58R_PAD_PA10_ENET_RMII_CLKOUT,
		SAC58R_PAD_PB0_ENET_RMII_RXD1,
		SAC58R_PAD_PB2_ENET_RMII_CRS_DV,
		SAC58R_PAD_PB3_ENET_RMII_TXDEN,
		SAC58R_PAD_PB4_ENET_RMII_RXD0,
		SAC58R_PAD_PB7_ENET_RMII_MDIO,
		SAC58R_PAD_PB8_ENET_RMII_TXD0,
		SAC58R_PAD_PB9_ENET_RMII_TXD1,
		SAC58R_PAD_PB10_ENET_RMII_MDC,
		SAC58R_PAD_PB12_ENET_REF_MII_CLK,
		SAC58R_PAD_PL13_GPIO365, /* PHY_RESET */
	};

	imx_iomux_v3_setup_multiple_pads(enet_pads, ARRAY_SIZE(enet_pads));

	/* Reset LAN8720 PHY */
	gpio_direction_output(PHY_RESET_GPIO , 0);
	udelay(1000);
	gpio_set_value(PHY_RESET_GPIO, 1);
}
#endif

#ifdef CONFIG_SYS_I2C_MXC
static void setup_board_i2c(void)
{
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info0);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	setup_i2c(3, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info3);
}
#endif

#ifdef CONFIG_SYS_USE_NAND
void setup_iomux_nfc(void)
{
	static const iomux_v3_cfg_t nfc_pads[] = {
		SAC58R_PAD_PF29__NFC_IO15,
		SAC58R_PAD_PF28__NFC_IO14,
		SAC58R_PAD_PF27__NFC_IO13,
		SAC58R_PAD_PF26__NFC_IO12,
		SAC58R_PAD_PF23__NFC_IO11,
		SAC58R_PAD_PF22__NFC_IO10,
		SAC58R_PAD_PF20__NFC_IO9,
		SAC58R_PAD_PF19__NFC_IO8,
		SAC58R_PAD_PC13__NFC_IO7,
		SAC58R_PAD_PC12__NFC_IO6,
		SAC58R_PAD_PC11__NFC_IO5,
		SAC58R_PAD_PC10__NFC_IO4,
		SAC58R_PAD_PC9__NFC_IO3,
		SAC58R_PAD_PC6__NFC_IO2,
		SAC58R_PAD_PC8__NFC_IO1,
		SAC58R_PAD_PC7__NFC_IO0,
		SAC58R_PAD_PC5__NFC_WEB,
		SAC58R_PAD_PC2__NFC_CE0B,
		SAC58R_PAD_PA16__NFC_CE1B,
		SAC58R_PAD_PC1__NFC_REB,
		SAC58R_PAD_PC0__NFC_RB0B,
		SAC58R_PAD_PF25__NFC_RB1B,
		SAC58R_PAD_PC4__NFC_ALE,
		SAC58R_PAD_PC3__NFC_CLE,
	};
	imx_iomux_v3_setup_multiple_pads(nfc_pads, ARRAY_SIZE(nfc_pads));

}
#endif

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[CONFIG_SYS_FSL_ESDHC_NUM] = {
	{USDHC0_BASE_ADDR},
#ifndef CONFIG_SYS_USE_NAND
	{USDHC1_BASE_ADDR},
	{USDHC2_BASE_ADDR},
#endif
};

int board_mmc_getcd(struct mmc *mmc)
{
	/* eSDHC0 is always present */
	return 1;
}

int board_mmc_init(bd_t *bis)
{
	int i;
	int ret;

	static const iomux_v3_cfg_t sdhc_pads[] = {
		SAC58R_PAD_PF0__SDHC0_CLK,
		SAC58R_PAD_PF1__SDHC0_CMD,
		SAC58R_PAD_PF2__SDHC0_DAT0,
		SAC58R_PAD_PF3__SDHC0_DAT1,
		SAC58R_PAD_PF4__SDHC0_DAT2,
		SAC58R_PAD_PF5__SDHC0_DAT3,
		SAC58R_PAD_PF0__SDHC0_CLK,
		SAC58R_PAD_PF1__SDHC0_CMD,
		SAC58R_PAD_PF2__SDHC0_DAT0,
		SAC58R_PAD_PF3__SDHC0_DAT1,
		SAC58R_PAD_PF4__SDHC0_DAT2,
		SAC58R_PAD_PF5__SDHC0_DAT3,

#ifndef CONFIG_SYS_USE_NAND
		SAC58R_PAD_PF26__SDHC1_CLK,
		SAC58R_PAD_PF27__SDHC1_CMD,
		SAC58R_PAD_PF22__SDHC1_DAT0,
		SAC58R_PAD_PF23__SDHC1_DAT1,
		SAC58R_PAD_PF19__SDHC1_DAT2,
		SAC58R_PAD_PF20__SDHC1_DAT3,
		SAC58R_PAD_PC12__SDHC2_CLK,
		SAC58R_PAD_PC13__SDHC2_CMD,
		SAC58R_PAD_PC9__SDHC2_DAT0,
		SAC58R_PAD_PC6__SDHC2_DAT1,
		SAC58R_PAD_PC5__SDHC2_DAT2,
		SAC58R_PAD_PC2__SDHC2_DAT3,
		SAC58R_PAD_PC10__SDHC2_DAT4,
		SAC58R_PAD_PC11__SDHC2_DAT5,
		SAC58R_PAD_PC7__SDHC2_DAT6,
		SAC58R_PAD_PC8__SDHC2_DAT7,
#endif

	};

	imx_iomux_v3_setup_multiple_pads(
		sdhc_pads, ARRAY_SIZE(sdhc_pads));

	/* Enable SDHCs modules in GPC */
	enable_periph_clk(AIPS2,AIPS2_OFF_SDHC0);
	enable_periph_clk(AIPS2,AIPS2_OFF_SDHC1);
	enable_periph_clk(AIPS2,AIPS2_OFF_SDHC2);

	for (i = 0; i < CONFIG_SYS_FSL_ESDHC_NUM; i++) {
		esdhc_cfg[i].sdhc_clk = mxc_get_clock(MXC_USDHC0_CLK + i);
		ret = fsl_esdhc_initialize(bis, &esdhc_cfg[i]);
		if (ret < 0) {
			printf("Warning: failed to initialize mmc dev %d\n", i);
		}
	}

	return 0;
}
#endif


#ifdef CONFIG_MVF_GPIO
static void setup_iomux_gpio(void)
{
	static const iomux_v3_cfg_t gpio_pads[] = {
		SAC58R_PAD_PA4__GPIO_4,   /* CS42888 reset line */
		SAC58R_PAD_PF14__GPIO_174, /* USR-SW3 button */
		SAC58R_PAD_PL8__GPIO_360, /* USR-SW1 button */
		SAC58R_PAD_PL9__GPIO_361, /* USR-SW2 button */
		SAC58R_PAD_PK12__GPIO_332, /* USB HOST VBUS ENABLE */
		SAC58R_PAD_PL0_GPIO352,  /* GPIO352 TUNER_PIN7 */
		SAC58R_PAD_PF18_GPIO178, /* GPIO178 BT_RESET */
	};

	imx_iomux_v3_setup_multiple_pads(
		gpio_pads, ARRAY_SIZE(gpio_pads));

	gpio_direction_input(174);
	gpio_direction_input(360);
	gpio_direction_input(361);

	/* On some boards, CS42888 holds I2C0 bus lines low. Resetting audio codec
		puts things back to normal */
	gpio_direction_output(4, 1);
}
#endif

void setup_iomux_audio(void)
{
	static const iomux_v3_cfg_t audio_pads[] = {
		/* SAI0 */
		SAC58R_PAD_PG5_SAI0_TX_BCLK,
		SAC58R_PAD_PG6_SAI0_TX_SYNC,
		SAC58R_PAD_PF10_SAI0_RX_DATA,
		SAC58R_PAD_PF9_SAI0_TX_DATA,

		/* SAI1 */
		SAC58R_PAD_PH11_SAI1_TX_BCLK,
		SAC58R_PAD_PH12_SAI1_TX_SYNC,
		SAC58R_PAD_PH16_SAI1_RX_BCLK,
		SAC58R_PAD_PH17_SAI1_RX_SYNC,
		SAC58R_PAD_PH18_SAI1_RX_DATA,
		SAC58R_PAD_PH19_SAI1_TX_DATA,

		/* ESAI0 */
		SAC58R_PAD_PA5_ESAI0_FSR,
		SAC58R_PAD_PA6_ESAI0_SCKR,
		SAC58R_PAD_PA12_ESAI0_SDO5_SDI0,
		SAC58R_PAD_PA14_ESAI0_SDO2_SDI3,
		SAC58R_PAD_PA15_ESAI0_SDO4_SDI3,
		SAC58R_PAD_PA16_ESAI0_SDO3_SDI2,
		SAC58R_PAD_PA17_ESAI0_SDO1,
		SAC58R_PAD_PA18_ESAI0_SD00,
		SAC58R_PAD_PA19_ESAI0_SCKT,
		SAC58R_PAD_PA20_ESAI0_FST,
	};

	imx_iomux_v3_setup_multiple_pads(
		audio_pads, ARRAY_SIZE(audio_pads));
}

#define VPU_HD_SUPPORT	0x8
static void vpu_init(void)
{
	/* Enable HD-support on VPU */

	u32 gpcr03 = readl(REG_BANK_PD2_BASE_ADDR + 0x0C);
	gpcr03 |= VPU_HD_SUPPORT;
	writel(gpcr03, REG_BANK_PD2_BASE_ADDR + 0x0C);

	printf("Enabled HD-support in VPU\n");
}


static void audiocodec_clock_init(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;

	/* Audio codec subsystem will use AUDIO1_PLL_DIV input
		as defined below:
		PLL8_FREQ = 1179 MHz
		CCM_AUDIO1_CLK_CTL = PLL8/2
		TDM_SRC0 input = AUDIO1_PLL_DIV/2/24 = 24.576 MHz
		TDM_SRC1 input = AUDIO1_PLL_DIV/2/24 = 24.576 MHz
		CODEC MCLK = FSOXC
	*/

	/* CCM_AUDIO1_CLK_CTL = AUDIO1_PLL/2 = 1179/2 MHz */
	writel(CCM_MODULE_ENABLE_CTL_EN |(0x1 << CCM_MUX_CTL_OFFSET),
			&ccm->AUDIO1_pll_div_clk);

	/* CODEC_MCLK INPUT SEL = FSOXC */
	writel( (0x1 << CCM_MUX_CTL_OFFSET), &ccm->codec_mclk);

	/* SAI0 MCLK selection = AUDIO1_PLL_DIV */
	writel(0x4 << CCM_MUX_CTL_OFFSET, &ccm->sai0_mclk);

	/* SAI1 MCLK selection = AUDIO1_PLL_DIV */
	writel(0x4 << CCM_MUX_CTL_OFFSET, &ccm->sai1_mclk);

	/* CODEC SAI_BCLK0 input clock sel = AUDIO1_PLL_DIV/2
		SAI BCLK0 clock frequency = 589/(23+1) = 24,576MHz */
	writel(CCM_MODULE_ENABLE_CTL_EN | (23 << CCM_PREDIV_CTRL_OFFSET)
		| (0x5 << CCM_MUX_CTL_OFFSET), &ccm->codec_sai_bclk0);

	/* CODEC SAI_BCLK1 input clock sel = AUDIO1_PLL_DIV/2
		SAI BCLK1 clock frequency = 589/(23+1) = 24,576MHz */
	writel(CCM_MODULE_ENABLE_CTL_EN | (23 << CCM_PREDIV_CTRL_OFFSET)
		| (0x5 << CCM_MUX_CTL_OFFSET), &ccm->codec_sai_bclk1);

	/* TDM_SRC0 input clock sel = AUDIO1_PLL_DIV/2
		TDM SRC0 clock frequency = 589/(23+1) = 24,576MHz */
	writel(CCM_MODULE_ENABLE_CTL_EN | (23 << CCM_PREDIV_CTRL_OFFSET)
		| (0x5 << CCM_MUX_CTL_OFFSET), &ccm->codec_dac_tdm_src0);

	/* TDM_SRC1 input clock sel = AUDIO1_PLL_DIV/2
		TDM SRC1 clock frequency = 589/(23+1) = 24,576MHz */
	writel(CCM_MODULE_ENABLE_CTL_EN | (23 << CCM_PREDIV_CTRL_OFFSET)
			| (0x5 << CCM_MUX_CTL_OFFSET), &ccm->codec_dac_tdm_src1);

	/* Enable AUDIO1 PLL */
	enable_pll(PLL_AUDIO1);

	/* Enable Audio Codec IPG clock divider */
	writel( CCM_MODULE_ENABLE_CTL_EN | (0x3 << CCM_PREDIV_CTRL_OFFSET),
			&ccm->audio_codec_ipg_clk);
}

static void esai_clock_init(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;

	/* ESAI will use AUDIO0_PLL_DIV input
		as defined below:
		PLL8_FREQ = 1179 MHz
		CCM_AUDIO0_CLK_CTL = PLL4/48
	*/

	/* CCM_AUDIO0_CLK_CTL = AUDIO0_PLL/48 = 1179/48 MHz */
	writel(CCM_MODULE_ENABLE_CTL_EN | (23 << CCM_PREDIV_CTRL_OFFSET) |
		   (0x1 << CCM_MUX_CTL_OFFSET),
			&ccm->AUDIO0_pll_div_clk);

	/* ESAI0 MCLK selection = AUDIO0_PLL_DIV */
	writel(0x3 << CCM_MUX_CTL_OFFSET, &ccm->esai0_mclk);

	/* Enable AUDIO0 PLL */
	enable_pll(PLL_AUDIO0);
}

static void clock_init(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	struct anadig_reg *anadig = (struct anadig_reg *)ANADIG_BASE_ADDR;

	/* Enable some modules in GPC */
	enable_periph_clk(AIPS0, AIPS0_OFF_GPC);
	enable_periph_clk(AIPS0, AIPS0_OFF_SRC);
	enable_periph_clk(AIPS0, AIPS0_OFF_CCM);
	enable_periph_clk(AIPS0, AIPS0_OFF_SCSC);
	enable_periph_clk(AIPS0, AIPS0_OFF_CMU);
	enable_periph_clk(AIPS0, AIPS0_OFF_ANADIG);
	enable_periph_clk(AIPS0, AIPS0_OFF_IOMUXC);
	enable_periph_clk(AIPS1, AIPS1_OFF_MISC_PIN_CONTROL);
	enable_periph_clk(AIPS0, AIPS0_OFF_WKUP);
	enable_periph_clk(AIPS0, AIPS0_OFF_PIT);
	enable_periph_clk(AIPS2, AIPS2_OFF_SDHC0);
	enable_periph_clk(AIPS1, AIPS1_OFF_UART0);
	enable_periph_clk(AIPS2, AIPS2_OFF_UART4);
	enable_periph_clk(AIPS0, AIPS0_OFF_PORTA);
	enable_periph_clk(AIPS0, AIPS0_OFF_PORTB);
	enable_periph_clk(AIPS0, AIPS0_OFF_PORTC);
	enable_periph_clk(AIPS0, AIPS0_OFF_PORTD);
	enable_periph_clk(AIPS0, AIPS0_OFF_PORTE);
	enable_periph_clk(AIPS0, AIPS0_OFF_PORTF);
	enable_periph_clk(AIPS0, AIPS0_OFF_PORTG);
	enable_periph_clk(AIPS0, AIPS0_OFF_PORTH);
	enable_periph_clk(AIPS0, AIPS0_OFF_PORTJ);
	enable_periph_clk(AIPS0, AIPS0_OFF_PORTK);
	enable_periph_clk(AIPS0, AIPS0_OFF_PORTL);
	enable_periph_clk(AIPS0, AIPS0_OFF_GPIOC);
	enable_periph_clk(AIPS2, AIPS2_OFF_ENET);
	enable_periph_clk(AIPS2, AIPS2_OFF_MMDC);

	enable_periph_clk(AIPS1, AIPS1_OFF_AUD_ADC_DAC0);
	enable_periph_clk(AIPS1, AIPS1_OFF_AUD_ADC_DAC1);
	enable_periph_clk(AIPS1, AIPS1_OFF_AUD_ADC_DAC2);
	enable_periph_clk(AIPS1, AIPS1_OFF_AUD_ADC_DAC3);

	enable_periph_clk(AIPS1, AIPS2_OFF_SAI0);
	enable_periph_clk(AIPS1, AIPS2_OFF_SAI1);
	enable_periph_clk(AIPS1, AIPS1_OFF_SAI4);
	enable_periph_clk(AIPS1, AIPS1_OFF_SAI5);
	enable_periph_clk(AIPS1, AIPS1_OFF_SAI6);
	enable_periph_clk(AIPS1, AIPS1_OFF_SAI7);

	enable_periph_clk(AIPS2, AIPS2_OFF_NFC0);
	enable_periph_clk(AIPS2, AIPS2_OFF_NFC1);
	enable_periph_clk(AIPS2, AIPS2_OFF_NFC2);
	enable_periph_clk(AIPS2, AIPS2_OFF_NFC3);

	/* enable PLL1 = PLL_CORE/ARM */
	clrsetbits_le32(&anadig->pll1_ctrl,
					ANADIG_PLL_CTRL_POWERDOWN | ANADIG_PLL_CTRL_BYPASS,
					ANADIG_PLL_CTRL_ENABLE);
	/* wait for PLL1 to be locked */
	while(!(readl(&anadig->pll1_ctrl) & ANADIG_PLL_CTRL_LOCK));

	/* configure ARM A7 clock => From PLL1 (PLL_CORE) = 1200/2 = 600MHz */
	writel( (0x1 << CCM_PREDIV_CTRL_OFFSET) | (0x3 << CCM_MUX_CTL_OFFSET), &ccm->a7_clk);

	/* SDHC0,1,2 clocks => from PLL_SYS/5 = 480/5 = 96 MHz */
	writel(CCM_MODULE_ENABLE_CTL_EN | (0x4<< CCM_PREDIV_CTRL_OFFSET)
		| (0x3 << CCM_MUX_CTL_OFFSET), &ccm->uSDHC0_perclk);
#ifndef CONFIG_SYS_USE_NAND
	writel(CCM_MODULE_ENABLE_CTL_EN | (0x4<< CCM_PREDIV_CTRL_OFFSET)
		| (0x3 << CCM_MUX_CTL_OFFSET), &ccm->uSDHC1_perclk);
	writel(CCM_MODULE_ENABLE_CTL_EN | (0x4<< CCM_PREDIV_CTRL_OFFSET)
		| (0x3 << CCM_MUX_CTL_OFFSET), &ccm->uSDHC2_perclk);
#endif
	/*NFC clock => from USBO_PLL_PFD3 = 375.6/20 (0x13 +1)= 19 MHz validation value*/
	writel(0x1b171a1c,0x400260f0);
	writel(CCM_MODULE_ENABLE_CTL_EN | (0x13<< CCM_PREDIV_CTRL_OFFSET)
		| (0x5 << CCM_MUX_CTL_OFFSET), &ccm->nfc_flash_clk_div);

	/* Refine AUDIO0 and AUDIO1 PLLs MFN and MFD parameters:
		By default, they are way too big, kernel is unable to compute
		the right values of the PLLs and end up with a erroneous
		1176MHz instead of the correct 1179MHz value.
		(see arch/arm/mach-imx/clk-pllv3.c, clk_pllv3_av_recalc_rate()).
		These values make it easy for the kernel to compute the right
		value of these PLLs (1179648000 Hz)
		This value divided by 48 allows providing a 24,576MHz
		input clock to the different audio IPs */

	/* Setting PLL4_MAIN and PLL8_MAIN to 1179MHz:
			- (24 MHz * 49) + ((24 MHz/1000)*152)
	*/
	writel(152, &anadig->pll4_num);
	writel(1000, &anadig->pll4_denom);

	writel(152, &anadig->pll8_num);
	writel(1000, &anadig->pll8_denom);
}

static void gpu_clk_init(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;
	struct anadig_reg *anadig = (struct anadig_reg *)ANADIG_BASE_ADDR;
	/* Setting PLL6_MAIN to 640MHz = 24MHz * (26 + 2/3)
	   => DIV_SELECT = 26 = 0x1A
	   => PLL6_NUM   =  2 = 0x02
	   => PLL6_DENOM =  3 = 0x03
	*/
	writel(0x00000002, &anadig->pll6_num);
	writel(0x00000003, &anadig->pll6_denom);
	writel(0x0000201A, &anadig->pll6_ctrl);
	/* wait for PLL6 to be locked */
	while(!(readl(&anadig->pll6_ctrl) & ANADIG_PLL_CTRL_LOCK));

	/* Setting GPU input clock to VIDEO_PLL (PLL6_MAIN) */
	writel(0x00000005, &ccm->GCC_clk2x);
}

static void mscm_init(void)
{
	struct mscm_ir *mscmir = (struct mscm_ir *)MSCM_IR_BASE_ADDR;
	int i;

	/* Interrupt Routing Configuration */
	for (i = 0; i < MSCM_IRSPRC_NUM; i++)
		writew(int_routing_conf[i], &mscmir->irsprc[i]);
}

static void dmamux_init(void)
{
	/* Example DMA CHANNEL MUXING FOR SAC58R. This function is not called */
	static const dmamux_cfg_t dma_channels [] = {
		DMAMUX_CHANNEL(DMAMUX_0, 0, SAC58R_DMAREQSRC_UART0_RX, 0, 1), /* DMAMUX 0, Channel 0 => UART0_RX, no trigger, channel enable */
		DMAMUX_CHANNEL(DMAMUX_1, 8, SAC58R_DMAREQSRC_PORTA, 0, 1), /* DMAMUX 1, Channel 8 => PORT_A, no trigger, channel enable */
		DMAMUX_CHANNEL(DMAMUX_2, 4, SAC58R_DMAREQSRC_SAI6_TX, 0, 1), /* DMAMUX 2, Channel 4  => SAI6_TX, no trigger, channel enable */
		DMAMUX_CHANNEL(DMAMUX_3, 12, SAC58R_DMAREQSRC_QSPIO_RX, 0, 1), /* DMAMUX 3, Channel 12 => QSPI0_TX, no trigger, channel enable */
	};
	imx_dmamux_setup_multiple_channels(dma_channels, ARRAY_SIZE(dma_channels));
}

#ifdef	CONFIG_FEC_MXC
int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config) {
		phydev->drv->config(phydev);
	}

	return 0;
}


int board_eth_init(bd_t *bis)
{
	int ret;

	setup_iomux_enet();

	ret = cpu_eth_init(bis);
	if (ret)
		printf("FEC MXC: %s:failed\n", __func__);

	return 0;
}
#endif

int board_early_init_f(void)
{
	clock_init();
	mscm_init();

	setup_iomux_ddr();
	ddr_ctrl_init();

	setup_iomux_uart();
	setup_iomux_dspi();

#ifdef CONFIG_SYS_USE_NAND
	setup_iomux_nfc();
#endif

#ifdef CONFIG_MVF_GPIO
	setup_iomux_gpio();
#endif

	setup_iomux_audio();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_SYS_I2C_MXC
	setup_board_i2c();
#endif

#ifdef	CONFIG_FEC_MXC
	enable_fec_clock();
#endif

#ifdef CONFIG_LVDS
	setup_iomux_lvds();
#endif

	audiocodec_clock_init();

	esai_clock_init();

	vpu_init();

	gpu_clk_init();

	return 0;
}

int checkboard(void)
{
	puts("Board: SAC58R EVB\n");

	return 0;
}
