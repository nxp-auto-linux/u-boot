/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * U-Boot Implementation for the Freescale SAC58R Functional Validation Board
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

#include "sac58rfvb_int_routing.h"

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_I2C_MXC
/* I2C0 */
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

/* I2C1 */
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


/* I2C2 */
struct i2c_pads_info i2c_pad_info2 = {
	.scl = {
		.i2c_mode = SAC58R_PAD_PA9_I2C2_SCL,
		.gpio_mode = SAC58R_PAD_PA9_GPIO_9,
		.gp = 9
	},
	.sda = {
		.i2c_mode = SAC58R_PAD_PA8_I2C2_SDA,
		.gpio_mode = SAC58R_PAD_PA8_GPIO_8,
		.gp = 8
	}
};

/* I2C3 */
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


#ifdef CONFIG_RUN_FROM_IRAM_ONLY
/* Run DDR configuration if we run from IRAM only */

/* Micron MT41J64M16 @ 416 MHz*/
/* 13 row addr, 10 col addr, burst length 8, data size 32 */
#define MDCTL_CONFIG 	0x02190000

void ddr_ctrl_init(void)
{
	writel(0x00008000, 0x4016901C); //MDSCR

	/* MDCFG0: tRFC=48,tXS=52,tXP=3,tXPDLL=10,tFAW=30,tCL=6 */
	writel(0x303475D3, 0x4016900C); //MDCFG0

	/* MDCFG1:  tRCD=6,tRP=6,tRC=20,tRAS=15,tRPA=1,tWR=6,tMRD=4,tCWL=5 */
	writel(0xB66E8A83, 0x40169010); //MDCFG1

	/* MDCFG2: tDLLK=512,tRTP=4,tWTR=4,tRRD=4 */
	writel(0x01FF00DB, 0x40169014); //MDCFG2

	/*MDOTC:  tAOFPD=1,tAONPD=1,tANPD=4,tAXPD=4,tODTLon=5,tODT_idle_off=0 */
	writel(0x11335030, 0x40169008); //MDOTC (timing param)

	/* MDMISC: WALAT=1, BI bank interleave on, MIF3=3, RALAT=1, 8 banks, DDR3 */
	writel(0x00011640, 0x40169018); //MDMISC 

	/* MDOR: WALAT=1, BI bank interleave on, MIF3=3, RALAT=1, 8 banks, DDR3 */
	writel(0x00341023, 0x40169030); //MDOR
	writel(MDCTL_CONFIG, 0x40169000); //MDCTL

	/* Perform ZQ calibration */
	writel(0xA1390003, 0x40169800); //MPZQHWCTRL

	/* Enable MMDC with CS0 */
	writel(MDCTL_CONFIG + 0x80000000, 0x40169000); //MDCTL

	/*** Complete the initialization sequence as defined by JEDEC */
	/* Configure MR2: CWL=5, self-refresh=off, self-refresh temp=normal */
	writel(0x00008032, 0x4016901C);
	/* Configure MR3: normal operation */
	writel(0x00008033, 0x4016901C);
	/* Configure MR1: enable DLL, drive strength=40R, AL off, ODT=40R, write levelling off, TDQS=0, Qoff=on */
	writel(0x00448031, 0x4016901C);
	/* Configure MR0: BL=8, CL=6, DLL reset, write recovery=6, precharge PD off */
	writel(0x05208030, 0x4016901C);
	/* DDR ZQ calibration */
	writel(0x04008040, 0x4016901C);

	writel(0x0000004F, 0x40169040); //MDASP: 256 MB memory

	/* Configure the power down and self-refresh entry and exit parameters */
	writel(0x453E4942, 0x40169848); //MPRDDLCTL, 
	writel(0x30353033, 0x40169850); //MPWRDLCTL0
	
	writel(0x41720154, 0x4016983C); //MPDGCTRL0
	writel(0x016A014F, 0x40169840); //MPDGCTRL1
	writel(0x000026D2, 0x4016902C); //MDRWD

	writel(0x22334010, 0x40169008); //MDOTC (timing param)

	writel(0x00020024, 0x40169004); //MDPDC
	writel(0x30B01800, 0x40169020); //MDREF
	writel(0x0003333F, 0x40169818); //MPODTCTRL

	writel(0x00000000, 0x4016901C); //MDSCR 1
}
#endif


int dram_init(void)
{
#ifdef CONFIG_RUN_FROM_IRAM_ONLY
	/* When booting from IRAM, we need to enable the MMDC controller */
	ddr_ctrl_init();
#endif

	gd->ram_size = ((ulong)CONFIG_DDR_MB * SZ_1M);

	return 0;
}

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

static void setup_iomux_enet(void)
{
}

#ifdef CONFIG_I2C_MXC
static void setup_board_i2c(void)
{
	setup_i2c(0, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info0);
	setup_i2c(1, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info1);
	setup_i2c(2, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info2);
	setup_i2c(3, CONFIG_SYS_I2C_SPEED, 0x7f, &i2c_pad_info3);
}
#endif


#ifdef CONFIG_SYS_USE_NAND
void setup_iomux_nfc(void)
{
}
#endif

#ifdef CONFIG_FSL_ESDHC
struct fsl_esdhc_cfg esdhc_cfg[CONFIG_SYS_FSL_ESDHC_NUM] = {
	{USDHC0_BASE_ADDR},
	{USDHC1_BASE_ADDR},
	{USDHC2_BASE_ADDR},
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

static void setup_iomux_gpio(void)
{
	static const iomux_v3_cfg_t gpio_pads[] = {

		SAC58R_PAD_PF17_GPIO_177, /* USB OTG VBUS ENABLE */
		SAC58R_PAD_PF16_USB0_ID, /* USB OTG ID muxing*/
		SAC58R_PAD_PH21_GPIO_245, /* USB HOST VBUS Enable*/
	};

	imx_iomux_v3_setup_multiple_pads(
		gpio_pads, ARRAY_SIZE(gpio_pads));
}

void setup_iomux_audio(void)
{
	static const iomux_v3_cfg_t audio_pads[] = {
		/*SPDIF0 */
		SAC58R_PAD_PG4__SPDIF0_IN1,
		SAC58R_PAD_PG5__SPDIF0_OUT1,

		/*SPDIF1 */
		SAC58R_PAD_PG1__SPDIF1_IN1,
		SAC58R_PAD_PG2__SPDIF1_OUT1,
	};

	imx_iomux_v3_setup_multiple_pads(
		audio_pads, ARRAY_SIZE(audio_pads));
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

static void spdif_clock_init(void)
{
	struct ccm_reg *ccm = (struct ccm_reg *)CCM_BASE_ADDR;

	/* SPDIFs will use AUDIO0_PLL_DIV input
		as defined below:
		PLL8_FREQ = 1179 MHz
		CCM_AUDIO0_CLK_CTL = PLL4/48

		AUDIO0_PLL_DIV = 24,576 MHz
	*/

	/* CCM_AUDIO0_CLK_CTL = AUDIO0_PLL/48 = 1179/48 MHz */
	writel(CCM_MODULE_ENABLE_CTL_EN | (23 << CCM_PREDIV_CTRL_OFFSET) |
		   (0x1 << CCM_MUX_CTL_OFFSET),
			&ccm->AUDIO0_pll_div_clk);


	/* SPDIF0 input clock selection = AUDIO0_PLL_DIV */
	writel(0x3 << CCM_MUX_CTL_OFFSET, &ccm->spdif_tx_clk);

	/* SPDIF1 input clock selection = AUDIO0_PLL_DIV */
	writel(0x3 << CCM_MUX_CTL_OFFSET, &ccm->spdif1_tx_clk);

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
	enable_periph_clk(AIPS0, AIPS0_OFF_WKUP);
	enable_periph_clk(AIPS0, AIPS0_OFF_PIT);
	enable_periph_clk(AIPS2, AIPS2_OFF_SDHC0);
	enable_periph_clk(AIPS1, AIPS1_OFF_UART0);
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

	/* Set PLL ARM = 1200 MHz and enable it */
	config_pll(PLL_ARM, 50, 0, 1);
	enable_pll(PLL_ARM);

	/* configure ARM A7 clock => From PLL1 (PLL_CORE) = 1200/2 = 600MHz */
	writel( (0x1 << CCM_PREDIV_CTRL_OFFSET) | (0x3 << CCM_MUX_CTL_OFFSET), &ccm->a7_clk);

	/* SDHC0,1,2 clocks => from PLL_SYS/5 = 480/5 = 96 MHz */
	writel(CCM_MODULE_ENABLE_CTL_EN | (0x4<< CCM_PREDIV_CTRL_OFFSET)
		| (0x3 << CCM_MUX_CTL_OFFSET), &ccm->uSDHC0_perclk);
	writel(CCM_MODULE_ENABLE_CTL_EN | (0x4<< CCM_PREDIV_CTRL_OFFSET)
		| (0x3 << CCM_MUX_CTL_OFFSET), &ccm->uSDHC1_perclk);
	writel(CCM_MODULE_ENABLE_CTL_EN | (0x4<< CCM_PREDIV_CTRL_OFFSET)
		| (0x3 << CCM_MUX_CTL_OFFSET), &ccm->uSDHC2_perclk);

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
		=> DIV_SELECT = 49
		=> PLLx_NUM   =  152
		=> PLLx_DENOM = 1000
	*/
	config_pll(PLL_AUDIO0, 49, 152, 1000);
	config_pll(PLL_AUDIO1, 49, 152, 1000);
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

int board_eth_init(bd_t *bis)
{
	setup_iomux_enet();

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}

int board_early_init_f(void)
{
	clock_init();
	mscm_init();

	setup_iomux_gpio();

	setup_iomux_uart();

#ifdef CONFIG_SYS_USE_NAND
	setup_iomux_nfc();
#endif

	setup_iomux_audio();

	return 0;
}

int board_init(void)
{
	/* address of boot parameters */
	gd->bd->bi_boot_params = PHYS_SDRAM + 0x100;

#ifdef CONFIG_I2C_MXC
	setup_board_i2c();
#endif

	audiocodec_clock_init();
	spdif_clock_init();

	return 0;
}

int checkboard(void)
{
	puts("Board: SAC58R Functional Validation Board\n");

	return 0;
}
