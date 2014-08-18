/*
 * Copyright 2013 Freescale Semiconductor, Inc.
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

#ifndef __ARCH_ARM_MACH_SAC58R_CCM_REGS_H__
#define __ARCH_ARM_MACH_SAC58R_CCM_REGS_H__

#ifndef __ASSEMBLY__

/* Clock Controller Module (CCM) */
struct ccm_reg {
	u32 muxed_fast_osc_clk; 
	u32 muxed_slow_osc_clk;
	u32 resv1;
	u32 a7_clk;
	u32 QoS_DDR_root;
	u32 QoS301_clk;
	u32 resv2;
	u32 BUS2x_clk;
	u32 BUS_clk;
	u32 PER_clk;
	u32 dap_clk;
	u32 resv3;
	u32 AUDIO0_pll_div_clk;
	u32 AUDIO1_pll_div_clk;
	u32 VIDEO_pll_div_clk;
	u32 VSPA_clk;
 	u32 AXIQ_clk;
	u32 resv0;
	u32 DCU_LDI_pix_clk_src;
	u32 GCC_clk2x;
	u32 anacatum_proc_clk;
	u32 resv4;
	u32 resv5;
	u32 resv6;
	u32 flextimer0_extclk;
	u32 flextimer0_ff_clk;
	u32 enet_time_clk;
	u32 resv7;
	u32 uSDHC0_perclk;
	u32 uSDHC1_perclk;
	u32 uSDHC2_perclk;
	u32 qspi_4x_clk;
	u32 nfc_flash_clk_div;
	u32 AUD_CLK_0;
	u32 AUD_CLK_1;
	u32 sai0_mclk;
	u32 sai1_mclk;
	u32 sai2_mclk;
	u32 sai3_mclk;
	u32 esai0_mclk;
	u32 esai1_mclk;
	u32 spdif_tx_clk;
	u32 tun12_rx_bclk_div;
	u32 tun34_rx_bclk_div;
	u32 asrc0_mux_clk;
	u32 asrc1_mux_clk;
	u32 OBS_CLK_OUT0;
	u32 OBS_CLK_OUT1;
	u32 trace_clk;
	u32 codec_sai_bclk0;
	u32 codec_sai_bclk1;
	u32 codec_dac_tdm_src0;
	u32 codec_dac_tdm_src1;
	u32 codec_mclk;
	u32 viu_clk;
	u32 vpu_clk;
	u32 tuner_mclk_sai8;
	u32 tuner_mclk_sai9;
	u32 tuner_mclk_sai10;
	u32 tuner_mclk_sai11;
	u32 audio_codec_ipg_clk;
	u32 mipi_24M_ref_clk;
	u32 mipi_escape_mode_clk;
	u32 mipi_viu_intf_clk;
	u32 spdif1_tx_clk;
	u32 adc_conv_clk;
	u32 xrdc_ipg_clk;
	u32 ccr;
	u32 csr;
	u32 ccsr;
	u32 clpcr;
};

/* Analog components control digital interface (ANADIG) */
struct anadig_reg {
	u32 resv0[4];
	u32 pll3_ctrl;
	u32 resv1[3];
	u32 pll7_ctrl;
	u32 resv2[3];
	u32 pll2_ctrl;
	u32 resv3[3];
	u32 pll2_ss;
	u32 resv4[3];
	u32 pll2_num;
	u32 resv5[3];
	u32 pll2_denom;
	u32 resv6[3];
	u32 pll4_ctrl;
	u32 resv7[3];
	u32 pll4_num;
	u32 resv8[3];
	u32 pll4_denom;
	u32 resv9[3];
	u32 pll6_ctrl;
	u32 resv10[3];
	u32 pll6_num;
	u32 resv11[3];
	u32 pll6_denom;
	u32 resv12[7];
	u32 pll5_ctrl;
	u32 resv13[3];
	u32 pll3_pfd;
	u32 resv14[3];
	u32 pll2_pfd;
	u32 resv15[3];
	u32 reg_1p1;
	u32 resv16[3];
	u32 reg_3p0;
	u32 resv17[3];
	u32 reg_2p5;
	u32 resv18[7];
	u32 ana_misc0;
	u32 resv19[3];
	u32 ana_misc1;
	u32 resv20[63];
	u32 digprog;
	u32 resv21[3];
	u32 pll1_ctrl;
	u32 resv22[3];
	u32 pll1_num;
	u32 resv23[3];
	u32 pll1_denom;
	u32 resv24[11];
	u32 pll_lock;
	u32 resv25[3];
	u32 pll8_ctrl;
	u32 resv26[3];
	u32 pll8_num;
	u32 resv27[3];
	u32 pll8_denom;
};

/* Global Power Controller (GPC) */
struct gpc_reg {
	u32 reserved[16];
	u32 lpmr;
	u32 imr1;
	u32 imr2;
	u32 imr3;
	u32 imr4;
	u32 isr0;
	u32 isr1;
	u32 isr2;
	u32 isr3;
	u32 resv0[7];
	u32 aips0_onpf_pctl0;
	u32 aips0_onpf_pctl1;
	u32 aips0_offpf_pctl0;
	u32 aips0_offpf_pctl1;
	u32 aips0_offpf_pctl2;
	u32 aips0_offpf_pctl3;
	u32 aips0_offpf_pctl4;
	u32 resv1[25];
	u32 aips1_onpf_pctl0;
	u32 aips1_onpf_pctl1;
	u32 aips1_offpf_pctl0;
	u32 aips1_offpf_pctl1;
	u32 aips1_offpf_pctl2;
	u32 aips1_offpf_pctl3;
	u32 aips1_offpf_pctl4;
	u32 resv2[25];
	u32 aips2_onpf_pctl0;
	u32 aips2_onpf_pctl1;
	u32 aips2_offpf_pctl0;
	u32 aips2_offpf_pctl1;
	u32 aips2_offpf_pctl2;
	u32 aips2_offpf_pctl3;
	u32 aips2_offpf_pctl4;
	u32 aips2_offpf_pctl5;
	u32 resv3[24];
	u32 pgcr;
	u32 pupscr;
	u32 pdnscr;
	u32 pgsr;
	u32 pgdr;
	u32 pmc_ctrl;
	u32 hpreg_ctrl;
	u32 hpvd_ctrl;
	u32 resv4[1];
	u32 lpvd_ctrl;
	u32 resv5[1];
	u32 ulpvd_ctrl;
	u32 rstcnt_ctrl;
	u32 lpm_ctrl;
	u32 mem_mon_ctrl;
};

/* Slow Clock Source Controller (SCSC) */
struct scsc_reg {
	u32 irc_ctrl;
	u32 osc_ctrl;
	u32 clk_src_ctrl_stat;
};
#endif

/* CCM register fields */
#define CCM_MODULE_ENABLE_CTL_OFFSET	31
#define CCM_MODULE_ENABLE_CTL_EN	(1 << 31)

#define CCM_PREDIV_UPD_IN_PROGRESS_CTRL_OFFSET	28
#define CCM_PREDIV_UPD_IN_PROGRESS_CTRL_MASK	(0x1 << 28)

#define CCM_PREDIV_FRAC_DIV_CTRL_OFFSET	27
#define CCM_PREDIV_FRAC_DIV_CTRL_MASK	(0x1 << 27)
#define CCM_PREDIV_FRAC_DIV_CTRL(v)	(((v) & 0x1) << 27)

#define CCM_PREDIV_CTRL_OFFSET		16
#define CCM_PREDIV2_CTRL_MASK		(0x3 << 16)
#define CCM_PREDIV2_CTRL(v)		(((v) & 0x3) << 16)
#define CCM_PREDIV3_CTRL_MASK		(0x7 << 16)
#define CCM_PREDIV3_CTRL(v)		(((v) & 0x7) << 16)
#define CCM_PREDIV4_CTRL_MASK		(0xf << 16)
#define CCM_PREDIV4_CTRL(v)		(((v) & 0xf) << 16)
#define CCM_PREDIV5_CTRL_MASK		(0x1f << 16)
#define CCM_PREDIV5_CTRL(v)		(((v) & 0x1f) << 16)
#define CCM_PREDIV8_CTRL_MASK		(0xff << 16)
#define CCM_PREDIV8_CTRL(v)		(((v) & 0xff) << 16)
#define CCM_PREDIV10_CTRL_MASK		(0x3FF << 16)
#define CCM_PREDIV10_CTRL(v)		(((v) & 0x3FF) << 16)

#define CCM_MUX_CTL_OFFSET	0
#define CCM_MUX1_CTL_MASK	0x1
#define CCM_MUX1_CTL(v)		(v & 0x1)
#define CCM_MUX2_CTL_MASK	0x3
#define CCM_MUX2_CTL(v)		(v & 0x3)
#define CCM_MUX3_CTL_MASK	0x7
#define CCM_MUX3_CTL(v)		(v & 0x7)
#define CCM_MUX4_CTL_MASK	0xf
#define CCM_MUX4_CTL(v)		(v & 0xf)
#define CCM_MUX5_CTL_MASK	0x1f
#define CCM_MUX5_CTL(v)		(v & 0x1f)
#define CCM_MUX6_CTL_MASK	0x3f
#define CCM_MUX6_CTL(v)		(v & 0x3f)

/* Anadig register fields    */

/* PLL definition            */
/* PLL1 => CORE_PLL/ARM_PLL  */
/* PLL2 => SYS_PLL           */
/* PLL3 => USB0_PLL          */
/* PLL4 => USB1_PLL          */
/* PLL5 => AUDIO0_PLL        */
/* PLL6 => AUDIO1_PLL        */
/* PLL7 => VIDEO_PLL         */
/* PLL8 => ENET_PLL          */

#define ANADIG_PLL_CTRL_LOCK			(1 << 31)
#define ANADIG_PLL_CTRL_BYPASS			(1 << 16)
#define ANADIG_PLL_CTRL_ENABLE			(1 << 13)
#define ANADIG_PLL_CTRL_POWERDOWN		(1 << 12)
#define ANADIG_PLL1_CTRL_DIV_SELECT_MASK	0x7F
#define ANADIG_PLL2_CTRL_DIV_SELECT_MASK	1
#define ANADIG_PLL2_CTRL_DIV_SELECT		1
#define ANADIG_PLL3_CTRL_DIV_SELECT		1
#define ANADIG_PLL3_CTRL_DIV_SELECT_MASK	1
#define ANADIG_PLL4_CTRL_DIV_SELECT_MASK	0x7F
#define ANADIG_PLL5_CTRL_DIV_SELECT_MASK	3
#define ANADIG_PLL6_CTRL_DIV_SELECT_MASK	0x7F
#define ANADIG_PLL7_CTRL_DIV_SELECT_MASK	1
#define ANADIG_PLL8_CTRL_DIV_SELECT_MASK	0x7F
#define ANADIG_PLL_PFD4_CLKGATE_MASK	(0x1 << 31)
#define ANADIG_PLL_PFD3_CLKGATE_MASK	(0x1 << 23)
#define ANADIG_PLL_PFD2_CLKGATE_MASK	(0x1 << 15)
#define ANADIG_PLL_PFD1_CLKGATE_MASK	(0x1 << 7)
#define ANADIG_PLL_PFD4_FRAC_MASK		0x3F000000
#define ANADIG_PLL_PFD3_FRAC_MASK		0x003F0000
#define ANADIG_PLL_PFD2_FRAC_MASK		0x00003F00
#define ANADIG_PLL_PFD1_FRAC_MASK		0x0000003F
#define ANADIG_PLL_PFD4_OFFSET			24
#define ANADIG_PLL_PFD3_OFFSET			16
#define ANADIG_PLL_PFD2_OFFSET			8
#define ANADIG_PLL_PFD1_OFFSET			0
#define ANADIG_PLL_NUM_MASK				0x3FFFFFFF
#define ANADIG_PLL_DENOM_MASK			0x3FFFFFFF


#define SCSC_IRC_FIR_EN		(1 << 16)
#define SCSC_IRC_SIRC_EN	(1)
#define SCSC_CTRL_FXOSC_RDY_MASK	(0x1 << 20)

/* AIPS numbers */
#define AIPS0		0
#define AIPS1		1
#define AIPS2		2

/* AIPS Off platform Ids */
#define AIPS0_OFF_GPC	32
#define AIPS0_OFF_SRC	34
#define AIPS0_OFF_CCM	35
#define AIPS0_OFF_SCSC	36
#define AIPS0_OFF_CMU	37
#define AIPS0_OFF_ANADIG	38
#define AIPS0_OFF_IOMUXC	40
#define AIPS0_OFF_GPIOC	41
#define AIPS0_OFF_PORTA	42
#define AIPS0_OFF_PORTB	43
#define AIPS0_OFF_PORTC	44
#define AIPS0_OFF_PORTD	45
#define AIPS0_OFF_PORTE	46
#define AIPS0_OFF_PORTF	47
#define AIPS0_OFF_PORTG	48
#define AIPS0_OFF_PORTH	49
#define AIPS0_OFF_PORTJ	51
#define AIPS0_OFF_PORTK	52
#define AIPS0_OFF_PORTL	53
#define AIPS0_OFF_OCOTP0	60
#define AIPS0_OFF_OCOTP1	61
#define AIPS0_OFF_SNVS	62
#define AIPS0_OFF_SNVS_WDOG	63
#define AIPS0_OFF_FLEXCAN0	64
#define AIPS0_OFF_FLEXCAN1	65
#define AIPS0_OFF_FLEXCAN2	66
#define AIPS0_OFF_SPI0	69
#define AIPS0_OFF_UART2	72
#define AIPS0_OFF_DMA_CHMUX0	76
#define AIPS0_OFF_DMA_CHMUX1	77
#define AIPS0_OFF_WKUP	81
#define AIPS0_OFF_CRC	83
#define AIPS0_OFF_LPTIMER	86
#define AIPS0_OFF_PIT	88
#define AIPS0_OFF_GPADC	91
#define AIPS0_OFF_FTM	94
#define AIPS0_OFF_PDB	97
#define AIPS0_OFF_REG_BANK_PD0	100
#define AIPS0_OFF_REG_BANK_PD1	101

#define AIPS1_OFF_AUD_ADC_DAC0	32
#define AIPS1_OFF_AUD_ADC_DAC1	33
#define AIPS1_OFF_AUD_ADC_DAC2	34
#define AIPS1_OFF_AUD_ADC_DAC3	35
#define AIPS1_OFF_SAI4	38
#define AIPS1_OFF_SAI5	39
#define AIPS1_OFF_SAI6	44
#define AIPS1_OFF_SAI7	45
#define AIPS1_OFF_REG_BANK_PD2	48
#define AIPS1_OFF_MISC_PIN_CONTROL	49
#define AIPS1_OFF_ESAI0	51
#define AIPS1_OFF_MLB0	54
#define AIPS1_OFF_SPI1	56
#define AIPS1_OFF_UART0	59
#define AIPS1_OFF_UART1	60
#define AIPS1_OFF_I2C0	63
#define AIPS1_OFF_I2C1	64
#define AIPS1_OFF_VSPA	68
#define AIPS1_OFF_FECA	76
#define AIPS1_OFF_CAAM0	80
#define AIPS1_OFF_CAAM1	81
#define AIPS1_OFF_CAAM2	82
#define AIPS1_OFF_CAAM3	83
#define AIPS1_OFF_CAAM4	84
#define AIPS1_OFF_CAAM5	85
#define AIPS1_OFF_CAAM6	86
#define AIPS1_OFF_CAAM7	87
#define AIPS1_OFF_CAAM8	88
#define AIPS1_OFF_CAAM9	89
#define AIPS1_OFF_CAAM10	90
#define AIPS1_OFF_CAAM11	91
#define AIPS1_OFF_CAAM12	92
#define AIPS1_OFF_CAAM13	93
#define AIPS1_OFF_CAAM14	94
#define AIPS1_OFF_CAAM15	95
#define AIPS1_OFF_SAI8	104
#define AIPS1_OFF_SAI9	105
#define AIPS1_OFF_SAI10	106
#define AIPS1_OFF_SAI11	107
#define AIPS1_OFF_JESD204B0	110
#define AIPS1_OFF_JESD204B1	111

#define AIPS2_OFF_UART3	32
#define AIPS2_OFF_UART4	33
#define AIPS2_OFF_UART5	34
#define AIPS2_OFF_I2C2	36
#define AIPS2_OFF_I2C3	37
#define AIPS2_OFF_ESAI1	39
#define AIPS2_OFF_SAI0	41
#define AIPS2_OFF_SAI1	42
#define AIPS2_OFF_SAI2	43
#define AIPS2_OFF_SAI3	44
#define AIPS2_OFF_SPDIF0	48
#define AIPS2_OFF_SPDIF1	49
#define AIPS2_OFF_ASRC0	50
#define AIPS2_OFF_ASRC1	51
#define AIPS2_OFF_VADC	54
#define AIPS2_OFF_VDEC	55
#define AIPS2_OFF_VIU3	56
#define AIPS2_OFF_MIPI2	60
#define AIPS2_OFF_DCU0	64
#define AIPS2_OFF_DCU1	65
#define AIPS2_OFF_DCU2	66
#define AIPS2_OFF_DCU3	67
#define AIPS2_OFF_DCU4	68
#define AIPS2_OFF_DCU5	69
#define AIPS2_OFF_DCU6	70
#define AIPS2_OFF_DCU7	71
#define AIPS2_OFF_TCON	80
#define AIPS2_OFF_LDB	82
#define AIPS2_OFF_RLE	84
#define AIPS2_OFF_VPU	88
#define AIPS2_OFF_GC400T0	92
#define AIPS2_OFF_GC400T1	93
#define AIPS2_OFF_GC400T2	94
#define AIPS2_OFF_GC400T3	95
#define AIPS2_OFF_ENET	96
#define AIPS2_OFF_USB0	99
#define AIPS2_OFF_USB1	100
#define AIPS2_OFF_USB2	101
#define AIPS2_OFF_DMA_CHMUX2	103
#define AIPS2_OFF_DMA_CHMUX3	104
#define AIPS2_OFF_MMDC	105
#define AIPS2_OFF_QSPI	106
#define AIPS2_OFF_SDHC0	108
#define AIPS2_OFF_SDHC1	109
#define AIPS2_OFF_SDHC2	110
#define AIPS2_OFF_NFC0	120
#define AIPS2_OFF_NFC1	121
#define AIPS2_OFF_NFC2	122
#define AIPS2_OFF_NFC3	123

/* GPC register fields */
#define GPC_OFFPF_PCTL_OCOTP0_MASK	0x3 << 12
#define GPC_OFFPF_PCTL_OCOTP1_MASK	0x3 << 13

#define FIRC_CLK_FREQ		24000000
#define FAST_CLK_FREQ		24000000
#define SLOW_CLK_FREQ		32000
#define PLL8_MAIN_FREQ		500000000


#define ENET_EXTERNAL_CLK	50000000
#define AUDIO_EXTERNAL_CLK	24576000

#endif /*__ARCH_ARM_MACH_SAC58R_CCM_REGS_H__ */

