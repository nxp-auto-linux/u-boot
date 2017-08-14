/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_IMX_REGS_H__
#define __ASM_ARCH_IMX_REGS_H__

#define ARCH_MXC

#define IRAM_BASE_ADDR		0x38000000  /* internal ram */
#define IRAM_SIZE		0x00400000  /* 4MB */
#define IS_ADDR_IN_IRAM(addr) \
	((addr) >= (IRAM_BASE_ADDR) && \
	(addr) <= (IRAM_BASE_ADDR) + (IRAM_SIZE))

#define PER_GROUP0_BASE				(0x40000000UL)
#define PER_GROUP1_BASE				(0x40100000UL)
#define PER_GROUP2_BASE				(0x40200000)
#define PER_GROUP3_BASE				(0x40300000)
#define PER_GROUP8_BASE				(0x44000000)

/* Peripheral group 0 */
#define POST_BASE_ADDR				(PER_GROUP0_BASE)
#define STCU_BASE_ADDR				(PER_GROUP0_BASE + 0x0002000)
#define MTR_BASE_ADDR				(PER_GROUP0_BASE + 0x0003000)
#define ATX_BASE_ADDR				(PER_GROUP0_BASE + 0x0004000)
#define STAM_BASE_ADDR				(PER_GROUP0_BASE + 0x0005000)
#define TMU_BASE_ADDR				(PER_GROUP0_BASE + 0x0006000)
#define ADC_BIST0_BASE_ADDR			(PER_GROUP0_BASE + 0x0008000)
#define ADC_BIST1_BASE_ADDR			(PER_GROUP0_BASE + 0x0009000)
#define MC_CGM0_BASE_ADDR			(PER_GROUP0_BASE + 0x0010000)
#define MC_CGM1_BASE_ADDR			(PER_GROUP0_BASE + 0x0014000)
#define SIRC_BASE_ADDR				(PER_GROUP0_BASE + 0x002C000)
#define XOSC_BASE_ADDR				(PER_GROUP0_BASE + 0x0030000)
#define CMU_BASE_ADDR				(PER_GROUP0_BASE + 0x003C000)
#define RTC_BASE_ADDR				(PER_GROUP0_BASE + 0x0040000)
#define FIRC_BASE_ADDR				(PER_GROUP0_BASE + 0x0044000)
#define MC_RGM_BASE_ADDR			(PER_GROUP0_BASE + 0x0050000)
#define SRC_SOC_BASE_ADDR			(PER_GROUP0_BASE + 0x0054000)
#define RDC_BASE_ADDR				(PER_GROUP0_BASE + 0x0058000)
#define MC_ME_BASE_ADDR				(PER_GROUP0_BASE + 0x0060000)
#define PMC_BASE_ADDR				(PER_GROUP0_BASE + 0x0064000)
#define WKPU_BASE_ADDR				(PER_GROUP0_BASE + 0x0068000)
#define SIUL2_BASE_ADDR				(PER_GROUP0_BASE + 0x0070000)

/* Peripheral group 1 */
#define SWT0_BASE_ADDR				(PER_GROUP1_BASE)
#define SWT1_BASE_ADDR				(PER_GROUP1_BASE + 0x00001000)
#define SWT2_BASE_ADDR				(PER_GROUP1_BASE + 0x00002000)
#define SWT3_BASE_ADDR				(PER_GROUP1_BASE + 0x00003000)
#define STM0_BASE_ADDR				(PER_GROUP1_BASE + 0x00004000)
#define STM1_BASE_ADDR				(PER_GROUP1_BASE + 0x00005000)
#define STM2_BASE_ADDR				(PER_GROUP1_BASE + 0x00006000)
#define STM3_BASE_ADDR				(PER_GROUP1_BASE + 0x00007000)
#define DMAMUX0_BASE_ADDR			(PER_GROUP1_BASE + 0x00008000)
#define DMAMUX1_BASE_ADDR			(PER_GROUP1_BASE + 0x00009000)
#define DMACR0_BASE_ADDR			(PER_GROUP1_BASE + 0x0000C000)
#define EDMA0_CONTROL_BASE_ADDR			(PER_GROUP1_BASE + 0x00010000)
#define EDMA0_CHANNEL_0_BASE_ADDR		(PER_GROUP1_BASE + 0x00011000)
#define EDMA0_CHANNEL_1_BASE_ADDR		(PER_GROUP1_BASE + 0x00012000)
#define EDMA0_CHANNEL_2_BASE_ADDR		(PER_GROUP1_BASE + 0x00013000)
#define EDMA0_CHANNEL_3_BASE_ADDR		(PER_GROUP1_BASE + 0x00014000)
#define EDMA0_CHANNEL_4_BASE_ADDR		(PER_GROUP1_BASE + 0x00015000)
#define EDMA0_CHANNEL_5_BASE_ADDR		(PER_GROUP1_BASE + 0x00016000)
#define EDMA0_CHANNEL_6_BASE_ADDR		(PER_GROUP1_BASE + 0x00017000)
#define EDMA0_CHANNEL_7_BASE_ADDR		(PER_GROUP1_BASE + 0x00018000)
#define EDMA0_CHANNEL_8_BASE_ADDR		(PER_GROUP1_BASE + 0x00019000)
#define EDMA0_CHANNEL_9_BASE_ADDR		(PER_GROUP1_BASE + 0x0001A000)
#define EDMA0_CHANNEL_10_BASE_ADDR		(PER_GROUP1_BASE + 0x0001B000)
#define EDMA0_CHANNEL_11_BASE_ADDR		(PER_GROUP1_BASE + 0x0001C000)
#define EDMA0_CHANNEL_12_BASE_ADDR		(PER_GROUP1_BASE + 0x0001D000)
#define EDMA0_CHANNEL_13_BASE_ADDR		(PER_GROUP1_BASE + 0x0001E000)
#define EDMA0_CHANNEL_14_BASE_ADDR		(PER_GROUP1_BASE + 0x0001F000)
#define EDMA0_CHANNEL_15_BASE_ADDR		(PER_GROUP1_BASE + 0x00020000)
#define EDMA0_CHANNEL_16_BASE_ADDR		(PER_GROUP1_BASE + 0x00021000)
#define EDMA0_CHANNEL_17_BASE_ADDR		(PER_GROUP1_BASE + 0x00022000)
#define EDMA0_CHANNEL_18_BASE_ADDR		(PER_GROUP1_BASE + 0x00023000)
#define EDMA0_CHANNEL_19_BASE_ADDR		(PER_GROUP1_BASE + 0x00024000)
#define EDMA0_CHANNEL_20_BASE_ADDR		(PER_GROUP1_BASE + 0x00025000)
#define EDMA0_CHANNEL_21_BASE_ADDR		(PER_GROUP1_BASE + 0x00026000)
#define EDMA0_CHANNEL_22_BASE_ADDR		(PER_GROUP1_BASE + 0x00027000)
#define EDMA0_CHANNEL_23_BASE_ADDR		(PER_GROUP1_BASE + 0x00028000)
#define EDMA0_CHANNEL_24_BASE_ADDR		(PER_GROUP1_BASE + 0x00029000)
#define EDMA0_CHANNEL_25_BASE_ADDR		(PER_GROUP1_BASE + 0x0002A000)
#define EDMA0_CHANNEL_26_BASE_ADDR		(PER_GROUP1_BASE + 0x0002B000)
#define EDMA0_CHANNEL_27_BASE_ADDR		(PER_GROUP1_BASE + 0x0002C000)
#define EDMA0_CHANNEL_28_BASE_ADDR		(PER_GROUP1_BASE + 0x0002D000)
#define EDMA0_CHANNEL_29_BASE_ADDR		(PER_GROUP1_BASE + 0x0002E000)
#define EDMA0_CHANNEL_30_BASE_ADDR		(PER_GROUP1_BASE + 0x0002F000)
#define EDMA0_CHANNEL_31_BASE_ADDR		(PER_GROUP1_BASE + 0x00030000)
#define PIT0_BASE_ADDR				(PER_GROUP1_BASE + 0x00051000)
#define CRC0_BASE_ADDR				(PER_GROUP1_BASE + 0x00054000)
#define MSCM_BASE_ADDR				(PER_GROUP1_BASE + 0x00058000)
#define SRAM_CTL_BASE_ADDR			(PER_GROUP1_BASE + 0x00059000)
#define XRDC_BASE_ADDR				(PER_GROUP1_BASE + 0x0005C000)
#define CAN_FD0_BASE_ADDR			(PER_GROUP1_BASE + 0x00060000)
#define CAN_FD1_BASE_ADDR			(PER_GROUP1_BASE + 0x00061000)
#define LINFLEXD0_BASE_ADDR			(PER_GROUP1_BASE + 0x00068000)
#define LINFLEXD1_BASE_ADDR			(PER_GROUP1_BASE + 0x00069000)
#define DSPL0_BASE_ADDR				(PER_GROUP1_BASE + 0x0006C000)
#define DSPL1_BASE_ADDR				(PER_GROUP1_BASE + 0x0006D000)
#define DSPL2_BASE_ADDR				(PER_GROUP1_BASE + 0x0006E000)
#define I2C2_0_BASE_ADDR			(PER_GROUP1_BASE + 0x00070000)
#define I2C2_1_BASE_ADDR			(PER_GROUP1_BASE + 0x00071000)
#define I2C2_2_BASE_ADDR			(PER_GROUP1_BASE + 0x00072000)
#define FLEXTIMER0_BASE_ADDR			(PER_GROUP1_BASE + 0x00074000)
#define SARADC0_BASE_ADDR			(PER_GROUP1_BASE + 0x00075000)
#define CTU_BASE_ADDR				(PER_GROUP1_BASE + 0x00076000)
#define QUADSPI_BASE_ADDR			(PER_GROUP1_BASE + 0x00077000)

/* Peripheral group 2 */
#define SWT4_BASE_ADDR				(PER_GROUP2_BASE)
#define SWT5_BASE_ADDR				(PER_GROUP2_BASE + 0x00001000)
#define SWT6_BASE_ADDR				(PER_GROUP2_BASE + 0x00002000)
#define SWT7_BASE_ADDR				(PER_GROUP2_BASE + 0x00003000)
#define STM4_BASE_ADDR				(PER_GROUP2_BASE + 0x00004000)
#define STM5_BASE_ADDR				(PER_GROUP2_BASE + 0x00005000)
#define STM6_BASE_ADDR				(PER_GROUP2_BASE + 0x00006000)
#define STM7_BASE_ADDR				(PER_GROUP2_BASE + 0x00007000)
#define DMAMUX2_BASE_ADDR			(PER_GROUP2_BASE + 0x00008000)
#define DMAMUX3_BASE_ADDR			(PER_GROUP2_BASE + 0x00009000)
#define DMACR1_BASE_ADDR			(PER_GROUP2_BASE + 0x0000C000)
#define EDMA1_CONTROL_BASE_ADDR			(PER_GROUP2_BASE + 0x00010000)
#define EDMA1_CHANNEL_0_BASE_ADDR		(PER_GROUP2_BASE + 0x00011000)
#define EDMA1_CHANNEL_1_BASE_ADDR		(PER_GROUP2_BASE + 0x00012000)
#define EDMA1_CHANNEL_2_BASE_ADDR		(PER_GROUP2_BASE + 0x00013000)
#define EDMA1_CHANNEL_3_BASE_ADDR		(PER_GROUP2_BASE + 0x00014000)
#define EDMA1_CHANNEL_4_BASE_ADDR		(PER_GROUP2_BASE + 0x00015000)
#define EDMA1_CHANNEL_5_BASE_ADDR		(PER_GROUP2_BASE + 0x00016000)
#define EDMA1_CHANNEL_6_BASE_ADDR		(PER_GROUP2_BASE + 0x00017000)
#define EDMA1_CHANNEL_7_BASE_ADDR		(PER_GROUP2_BASE + 0x00018000)
#define EDMA1_CHANNEL_8_BASE_ADDR		(PER_GROUP2_BASE + 0x00019000)
#define EDMA1_CHANNEL_9_BASE_ADDR		(PER_GROUP2_BASE + 0x0001A000)
#define EDMA1_CHANNEL_10_BASE_ADDR		(PER_GROUP2_BASE + 0x0001B000)
#define EDMA1_CHANNEL_11_BASE_ADDR		(PER_GROUP2_BASE + 0x0001C000)
#define EDMA1_CHANNEL_12_BASE_ADDR		(PER_GROUP2_BASE + 0x0001D000)
#define EDMA1_CHANNEL_13_BASE_ADDR		(PER_GROUP2_BASE + 0x0001E000)
#define EDMA1_CHANNEL_14_BASE_ADDR		(PER_GROUP2_BASE + 0x0001F000)
#define EDMA1_CHANNEL_15_BASE_ADDR		(PER_GROUP2_BASE + 0x00020000)
#define EDMA1_CHANNEL_16_BASE_ADDR		(PER_GROUP2_BASE + 0x00021000)
#define EDMA1_CHANNEL_17_BASE_ADDR		(PER_GROUP2_BASE + 0x00022000)
#define EDMA1_CHANNEL_18_BASE_ADDR		(PER_GROUP2_BASE + 0x00023000)
#define EDMA1_CHANNEL_19_BASE_ADDR		(PER_GROUP2_BASE + 0x00024000)
#define EDMA1_CHANNEL_20_BASE_ADDR		(PER_GROUP2_BASE + 0x00025000)
#define EDMA1_CHANNEL_21_BASE_ADDR		(PER_GROUP2_BASE + 0x00026000)
#define EDMA1_CHANNEL_22_BASE_ADDR		(PER_GROUP2_BASE + 0x00027000)
#define EDMA1_CHANNEL_23_BASE_ADDR		(PER_GROUP2_BASE + 0x00028000)
#define EDMA1_CHANNEL_24_BASE_ADDR		(PER_GROUP2_BASE + 0x00029000)
#define EDMA1_CHANNEL_25_BASE_ADDR		(PER_GROUP2_BASE + 0x0002A000)
#define EDMA1_CHANNEL_26_BASE_ADDR		(PER_GROUP2_BASE + 0x0002B000)
#define EDMA1_CHANNEL_27_BASE_ADDR		(PER_GROUP2_BASE + 0x0002C000)
#define EDMA1_CHANNEL_28_BASE_ADDR		(PER_GROUP2_BASE + 0x0002D000)
#define EDMA1_CHANNEL_29_BASE_ADDR		(PER_GROUP2_BASE + 0x0002E000)
#define EDMA1_CHANNEL_30_BASE_ADDR		(PER_GROUP2_BASE + 0x0002F000)
#define EDMA1_CHANNEL_31_BASE_ADDR		(PER_GROUP2_BASE + 0x00030000)
#define PIT1_BASE_ADDR				(PER_GROUP2_BASE + 0x00051000)
#define SEMA4_BASE_ADDR				(PER_GROUP2_BASE + 0x00058000)
#define CAN_FD3_BASE_ADDR			(PER_GROUP2_BASE + 0x00060000)
#define CAN_FD4_BASE_ADDR			(PER_GROUP2_BASE + 0x00061000)
#define LINFLEXD2_BASE_ADDR			(PER_GROUP2_BASE + 0x00068000)
#define DSPL3_BASE_ADDR				(PER_GROUP2_BASE + 0x0006C000)
#define DSPL4_BASE_ADDR				(PER_GROUP2_BASE + 0x0006D000)
#define DSPL5_BASE_ADDR				(PER_GROUP2_BASE + 0x0006E000)
#define I2C2_3_BASE_ADDR			(PER_GROUP2_BASE + 0x00070000)
#define I2C2_4_BASE_ADDR			(PER_GROUP2_BASE + 0x00071000)
#define FLEXTIMER1_BASE_ADDR			(PER_GROUP2_BASE + 0x00074000)
#define SARADC1_BASE_ADDR			(PER_GROUP2_BASE + 0x00075000)
#define USDHC_BASE_ADDR				(PER_GROUP2_BASE + 0x00078000)

/* Peripheral group 8 */
#define MC_CGM2_BASE_ADDR			(PER_GROUP8_BASE + 0x000C0000)

/* TODO Remove this after the IOMUX framework is implemented */
#define IOMUXC_BASE_ADDR SIUL2_BASE_ADDR

/* MUX mode and PAD ctrl are in one register */
#define CONFIG_IOMUX_SHARE_CONF_REG

#define FEC_QUIRK_ENET_MAC
#define I2C_QUIRK_REG

/* MSCM interrupt router */
#define MSCM_IRSPRC_CPn_EN		3
#define MSCM_IRSPRC_NUM			176
#define MSCM_CPXTYPE_RYPZ_MASK		0xFF
#define MSCM_CPXTYPE_RYPZ_OFFSET	0
#define MSCM_CPXTYPE_PERS_MASK		0xFFFFFF00
#define MSCM_CPXTYPE_PERS_OFFSET	8
#define MSCM_CPXTYPE_PERS_A53		0x413533
#define MSCM_CPXTYPE_PERS_CM4		0x434d34

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

/* System Reset Controller (SRC) */
struct src {
	u32 bmr1;
	u32 bmr2;
	u32 gpr1_boot;
	u32 reserved_0x00C[61];
	u32 gpr1;
	u32 gpr2;
	u32 gpr3;
	u32 gpr4;
	u32 gpr5;
	u32 gpr6;
	u32 gpr7;
	u32 reserved_0x11C[2];
	u32 gpr10;
	u32 gpr11;
	u32 gpr12;
	u32 gpr13;
	u32 gpr14;
	u32 gpr15;
	u32 gpr16;
	u32 reserved_0x140[1];
	u32 gpr18;
	u32 gpr19;
	u32 gpr20;
	u32 gpr21;
	u32 gpr22;
	u32 gpr23;
	u32 gpr24;
	u32 gpr25;
	u32 gpr26;
	u32 gpr27;
	u32 reserved_0x16C[5];
	u32 pcie_config1;
	u32 ddr_self_ref_ctrl;
	u32 pcie_config0;
	u32 reserved_0x18C[3];
	u32 soc_misc_config2;
};

/* SRC registers definitions */

/* SRC_GPR1 */
#define SRC_GPR1_PLL_SOURCE(pll,val)( ((val) & SRC_GPR1_PLL_SOURCE_MASK) << \
										(SRC_GPR1_PLL_OFFSET + (pll)) )
#define SRC_GPR1_PLL_SOURCE_MASK	(0x1)

#define SRC_GPR1_PLL_OFFSET			(27)
#define SRC_GPR1_FIRC_CLK_SOURCE	(0x0)
#define SRC_GPR1_XOSC_CLK_SOURCE	(0x1)

/* SRC_GPR3 */
#define SRC_GPR3_ENET_MODE			(1<<1)

/* Periodic Interrupt Timer (PIT) */
struct pit_reg {
	u32 mcr;
	u32 recv0[55];
	u32 ltmr64h;
	u32 ltmr64l;
	u32 recv1[6];
	u32 ldval0;
	u32 cval0;
	u32 tctrl0;
	u32 tflg0;
	u32 ldval1;
	u32 cval1;
	u32 tctrl1;
	u32 tflg1;
	u32 ldval2;
	u32 cval2;
	u32 tctrl2;
	u32 tflg2;
	u32 ldval3;
	u32 cval3;
	u32 tctrl3;
	u32 tflg3;
	u32 ldval4;
	u32 cval4;
	u32 tctrl4;
	u32 tflg4;
	u32 ldval5;
	u32 cval5;
	u32 tctrl5;
	u32 tflg5;
};

/* Watchdog Timer (WDOG) */
struct wdog_regs {
	u32 cr;
	u32 ir;
	u32 to;
	u32 wn;
	u32 sr;
	u32 co;
	u32 sk;
};

/* UART */
struct linflex_fsl {
	u32 lincr1;
	u32 linier;
	u32 linsr;
	u32 linesr;
	u32 uartcr;
	u32 uartsr;
	u32 lintcsr;
	u32 linocr;
	u32 lintocr;
	u32 linfbrr;
	u32 linibrr;
	u32 lincfr;
	u32 lincr2;
	u32 bidr;
	u32 bdrl;
	u32 bdrm;
	u32 ifer;
	u32 ifmi;
	u32 ifmr;
#ifdef CONFIG_LINFLEX_MASTER_SLAVE_MODE
	u32 ifcr0;
	u32 ifcr1;
	u32 ifcr2;
	u32 ifcr3;
	u32 ifcr4;
	u32 ifcr5;
	u32 ifcr6;
	u32 ifcr7;
	u32 ifcr8;
	u32 ifcr9;
	u32 ifcr10;
	u32 ifcr11;
	u32 ifcr12;
	u32 ifcr13;
	u32 ifcr14;
	u32 ifcr15;
#endif
	u32 gcr;
	u32 uartpto;
	u32 uartcto;
	u32 dmatxe;
	u32 dmarxe;
};

/* MSCM Interrupt Router */
struct mscm_ir {
	u32 cpxtype;		/* Processor x Type Register			*/
	u32 cpxnum;			/* Processor x Number Register			*/
	u32 cpxmaster;		/* Processor x Master Number Register	*/
	u32 cpxcount;		/* Processor x Count Register			*/
	u32 cpxcfg0;		/* Processor x Configuration 0 Register */
	u32 cpxcfg1;		/* Processor x Configuration 1 Register */
	u32 cpxcfg2;		/* Processor x Configuration 2 Register */
	u32 cpxcfg3;		/* Processor x Configuration 3 Register */
	u32 cp0type;		/* Processor 0 Type Register			*/
	u32 cp0num;			/* Processor 0 Number Register			*/
	u32 cp0master;		/* Processor 0 Master Number Register	*/
	u32 cp0count;		/* Processor 0 Count Register			*/
	u32 cp0cfg0;		/* Processor 0 Configuration 0 Register	*/
	u32 cp0cfg1;		/* Processor 0 Configuration 1 Register	*/
	u32 cp0cfg2;		/* Processor 0 Configuration 2 Register	*/
	u32 cp0cfg3;		/* Processor 0 Configuration 3 Register	*/
	u32 cp1type;		/* Processor 1 Type Register			*/
	u32 cp1num;			/* Processor 1 Number Register			*/
	u32 cp1master;		/* Processor 1 Master Number Register	*/
	u32 cp1count;		/* Processor 1 Count Register			*/
	u32 cp1cfg0;		/* Processor 1 Configuration 0 Register	*/
	u32 cp1cfg1;		/* Processor 1 Configuration 1 Register	*/
	u32 cp1cfg2;		/* Processor 1 Configuration 2 Register	*/
	u32 cp1cfg3;		/* Processor 1 Configuration 3 Register	*/
	u32 reserved_0x060[232];
	u32 ocmdr0;			/* On-Chip Memory Descriptor Register	*/
	u32 reserved_0x404[2];
	u32 ocmdr3;			/* On-Chip Memory Descriptor Register	*/
	u32 reserved_0x410[28];
	u32 tcmdr[4];		/* Generic Tightly Coupled Memory Descriptor Register	*/
	u32 reserved_0x490[28];
	u32 cpce0;			/* Core Parity Checking Enable Register 0				*/
	u32 reserved_0x504[191];
	u32 ircp0ir;		/* Interrupt Router CP0 Interrupt Register				*/
	u32 ircp1ir;		/* Interrupt Router CP1 Interrupt Register				*/
	u32 reserved_0x808[6];
	u32 ircpgir;		/* Interrupt Router CPU Generate Interrupt Register		*/
	u32 reserved_0x824[23];
	u16 irsprc[176];	/* Interrupt Router Shared Peripheral Routing Control Register	*/
	u32 reserved_0x9e0[136];
	u32 iahbbe0;		/* Gasket Burst Enable Register							*/
	u32 reserved_0xc04[63];
	u32 ipcge;			/* Interconnect Parity Checking Global Enable Register	*/
	u32 reserved_0xd04[3];
	u32 ipce[4];		/* Interconnect Parity Checking Enable Register			*/
	u32 reserved_0xd20[8];
	u32 ipcgie;			/* Interconnect Parity Checking Global Injection Enable Register	*/
	u32 reserved_0xd44[3];
	u32 ipcie[4];		/* Interconnect Parity Checking Injection Enable Register	*/
};

#endif	/* __ASSEMBLER__*/

#endif	/* __ASM_ARCH_IMX_REGS_H__ */
