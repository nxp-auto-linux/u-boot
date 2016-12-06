/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_IMX_REGS_H__
#define __ASM_ARCH_IMX_REGS_H__

#define ARCH_MXC

#define IRAM_BASE_ADDR		0x30000000  /* internal ram */
#define IRAM_SIZE		0x00400000  /* 5MB */
#define IS_ADDR_IN_IRAM(addr) \
	((addr) >= (IRAM_BASE_ADDR) && \
	(addr) <= (IRAM_BASE_ADDR) + (IRAM_SIZE))

#define PER_GROUP0_BASE				(0x40000000UL)
#define PER_GROUP1_BASE				(0x40080000UL)
#define PER_GROUP2_BASE				(0x40100000)
#define PER_GROUP3_BASE				(0x40180000UL)

/* Peripheral group 0 */
#define CSE3_BASE_ADDR				(PER_GROUP0_BASE + 0x0001000)
#define EDMA_CRC_BASE_ADDR			(PER_GROUP0_BASE + 0x0004000)
#define EDMA_CONTROL_BASE_ADDR			(PER_GROUP0_BASE + 0x0005000)
#define EDMA_CHANNEL_0_BASE_ADDR		(PER_GROUP0_BASE + 0x0006000)
#define EDMA_CHANNEL_1_BASE_ADDR		(PER_GROUP0_BASE + 0x0007000)
#define EDMA_CHANNEL_2_BASE_ADDR		(PER_GROUP0_BASE + 0x0008000)
#define EDMA_CHANNEL_3_BASE_ADDR		(PER_GROUP0_BASE + 0x0009000)
#define EDMA_CHANNEL_4_BASE_ADDR		(PER_GROUP0_BASE + 0x000A000)
#define EDMA_CHANNEL_5_BASE_ADDR		(PER_GROUP0_BASE + 0x000B000)
#define EDMA_CHANNEL_6_BASE_ADDR		(PER_GROUP0_BASE + 0x000C000)
#define EDMA_CHANNEL_7_BASE_ADDR		(PER_GROUP0_BASE + 0x000D000)
#define XRDC_BASE_ADDR				(PER_GROUP0_BASE + 0x0018000)
#define SWT0_BASE_ADDR				(PER_GROUP0_BASE + 0x001E000)
#define SWT1_BASE_ADDR				(PER_GROUP0_BASE + 0x001F000)
#define STM0_BASE_ADDR				(PER_GROUP0_BASE + 0x0022000)
#define ACE_BASE_ADDR				(PER_GROUP0_BASE + 0x0028000)
#define MIPI_CSI0_BASE_ADDR			(PER_GROUP0_BASE + 0x0030000)
#define DMAMUX0_BASE_ADDR			(PER_GROUP0_BASE + 0x0032000)
#define ENET0_BASE_ADDR				(PER_GROUP0_BASE + 0x0036000)
#define USDHC_BASE_ADDR				(PER_GROUP0_BASE + 0x0038000)
#define OCOTP CONTROLLER_BASE_ADDR		(PER_GROUP0_BASE + 0x003C000)
#define FLEXRAY_BASE_ADDR			(PER_GROUP0_BASE + 0x0040000)
#define DRAM_CONTROLLER0_BASE_ADDR		(PER_GROUP0_BASE + 0x0048000)
#define PIT0_BASE_ADDR				(PER_GROUP0_BASE + 0x0053000)
#define FLEXCAN0_BASE_ADDR			(PER_GROUP0_BASE + 0x0055000)
#define FLEXCAN4_BASE_ADDR			(PER_GROUP0_BASE + 0x0056000)
#define I2C0_BASE_ADDR				(PER_GROUP0_BASE + 0x005B000)
#define SPI0_BASE_ADDR				(PER_GROUP0_BASE + 0x005F000)
#define SPI4_BASE_ADDR				(PER_GROUP0_BASE + 0x0060000)
#define CRC0_BASE_ADDR				(PER_GROUP0_BASE + 0x0064000)
#define RDC_BASE_ADDR				(PER_GROUP0_BASE + 0x0065000)

/* Peripheral group 1 */
#define MC_CGM0_BASE_ADDR			(PER_GROUP1_BASE + 0x0004000)
#define MC_CGM1_BASE_ADDR			(PER_GROUP1_BASE + 0x0008000)
#define MC_CGM2_BASE_ADDR			(PER_GROUP1_BASE + 0x000C000)
#define MC_CGM3_BASE_ADDR			(PER_GROUP1_BASE + 0x0010000)
#define MC_CGM4_BASE_ADDR			(PER_GROUP1_BASE + 0x0014000)
#define MC_RGM_BASE_ADDR			(PER_GROUP1_BASE + 0x0018000)
#define MC_ME_BASE_ADDR				(PER_GROUP1_BASE + 0x001C000)
#define SAR_ADC_BASE_ADDR			(PER_GROUP1_BASE + 0x002B000)
#define FLEXTIMER0_BASE_ADDR			(PER_GROUP1_BASE + 0x002F000)
#define I2C1_BASE_ADDR				(PER_GROUP1_BASE + 0x0033000)
#define LINFLEXD0_BASE_ADDR			(PER_GROUP1_BASE + 0x0037000)
#define FLEXCAN1_BASE_ADDR			(PER_GROUP1_BASE + 0x003B000)
#define SPI1_BASE_ADDR				(PER_GROUP1_BASE + 0x003F000)
#define CRC1_BASE_ADDR				(PER_GROUP1_BASE + 0x0044000)
#define WKPU_BASE_ADDR				(PER_GROUP1_BASE + 0x0048000)
#define HPSMI_SRAM_CONTROLLER_BASE_ADDR		(PER_GROUP1_BASE + 0x004C000)
#define MIPI_CSI2_BASE_ADDR			(PER_GROUP1_BASE + 0x0050000)
#define SIPI_BASE_ADDR				(PER_GROUP1_BASE + 0x0054000)
#define LFAST_BASE_ADDR				(PER_GROUP1_BASE + 0x0058000)
#define SSE_BASE_ADDR				(PER_GROUP1_BASE + 0x0059000)
#define SRC_SOC_BASE_ADDR			(PER_GROUP1_BASE + 0x005C000)
#define ERM_BASE_ADDR				(PER_GROUP1_BASE + 0x0060000)
#define DRAM_PHY0_BASE_ADDR			(PER_GROUP1_BASE + 0x0068000)
#define EDMA_CHANNEL_8_BASE_ADDR		(PER_GROUP1_BASE + 0x0071000)
#define EDMA_CHANNEL_9_BASE_ADDR		(PER_GROUP1_BASE + 0x0072000)
#define EDMA_CHANNEL_10_BASE_ADDR		(PER_GROUP1_BASE + 0x0073000)
#define EDMA_CHANNEL_11_BASE_ADDR		(PER_GROUP1_BASE + 0x0074000)
#define EDMA_CHANNEL_12_BASE_ADDR		(PER_GROUP1_BASE + 0x0075000)
#define EDMA_CHANNEL_13_BASE_ADDR		(PER_GROUP1_BASE + 0x0076000)
#define EDMA_CHANNEL_14_BASE_ADDR		(PER_GROUP1_BASE + 0x0077000)
#define EDMA_CHANNEL_15_BASE_ADDR		(PER_GROUP1_BASE + 0x0078000)

/* Peripheral group 2 */
#define MSCM_BASE_ADDR				(PER_GROUP2_BASE + 0x00001000)
#define EDMA_CHANNEL_16_BASE_ADDR		(PER_GROUP2_BASE + 0x00003000)
#define EDMA_CHANNEL_17_BASE_ADDR		(PER_GROUP2_BASE + 0x00004000)
#define EDMA_CHANNEL_18_BASE_ADDR		(PER_GROUP2_BASE + 0x00005000)
#define EDMA_CHANNEL_19_BASE_ADDR		(PER_GROUP2_BASE + 0x00006000)
#define EDMA_CHANNEL_20_BASE_ADDR		(PER_GROUP2_BASE + 0x00007000)
#define EDMA_CHANNEL_21_BASE_ADDR		(PER_GROUP2_BASE + 0x00008000)
#define EDMA_CHANNEL_22_BASE_ADDR		(PER_GROUP2_BASE + 0x00009000)
#define EDMA_CHANNEL_23_BASE_ADDR		(PER_GROUP2_BASE + 0x0000A000)
#define SEMA42_BASE_ADDR			(PER_GROUP2_BASE + 0x00013000)
#define INTC_MON_BASE_ADDR			(PER_GROUP2_BASE + 0x00015000)
#define SWT2_BASE_ADDR				(PER_GROUP2_BASE + 0x00017000)
#define SWT3_BASE_ADDR				(PER_GROUP2_BASE + 0x00018000)
#define SWT4_BASE_ADDR				(PER_GROUP2_BASE + 0x00019000)
#define STM1_BASE_ADDR				(PER_GROUP2_BASE + 0x0001D000)
#define APB_BASE_ADDR				(PER_GROUP2_BASE + 0x00020000)
#define EIM_BASE_ADDR				(PER_GROUP2_BASE + 0x00030000)
#define MIPI_CSI1_BASE_ADDR			(PER_GROUP2_BASE + 0x00033000)
#define DMAMUX1_BASE_ADDR			(PER_GROUP2_BASE + 0x00037000)
#define DRAM_CONTROLLER1_BASE_ADDR		(PER_GROUP2_BASE + 0x00040000)
#define QUADSPI_BASE_ADDR			(PER_GROUP2_BASE + 0x0004B000)
#define PIT1_BASE_ADDR				(PER_GROUP2_BASE + 0x0004F000)
#define FCCU_BASE_ADDR				(PER_GROUP2_BASE + 0x00051000)
#define FLEXTIMER1_BASE_ADDR			(PER_GROUP2_BASE + 0x00055000)
#define I2C2_BASE_ADDR				(PER_GROUP2_BASE + 0x00057000)
#define LINFLEXD1_BASE_ADDR			(PER_GROUP2_BASE + 0x0005B000)
#define SIUL2_BASE_ADDR				(PER_GROUP2_BASE + 0x00060000)
#define FLEXCAN2_BASE_ADDR			(PER_GROUP2_BASE + 0x00068000)
#define SPI2_BASE_ADDR				(PER_GROUP2_BASE + 0x0006A000)

/* Peripheral group 2 */
#define RCCU_IPS_DEBUG_BASE_ADDR		(PER_GROUP3_BASE + 0x0001000)
#define EDMA_CHANNEL_24_BASE_ADDR		(PER_GROUP3_BASE + 0x0003000)
#define EDMA_CHANNEL_25_BASE_ADDR		(PER_GROUP3_BASE + 0x0004000)
#define EDMA_CHANNEL_26_BASE_ADDR		(PER_GROUP3_BASE + 0x0005000)
#define EDMA_CHANNEL_27_BASE_ADDR		(PER_GROUP3_BASE + 0x0006000)
#define EDMA_CHANNEL_28_BASE_ADDR		(PER_GROUP3_BASE + 0x0007000)
#define EDMA_CHANNEL_29_BASE_ADDR		(PER_GROUP3_BASE + 0x0008000)
#define EDMA_CHANNEL_30_BASE_ADDR		(PER_GROUP3_BASE + 0x0009000)
#define EDMA_CHANNEL_31_BASE_ADDR		(PER_GROUP3_BASE + 0x000A000)
#define TMU_BASE_ADDR				(PER_GROUP3_BASE + 0x0013000)
#define LG_KLT_BASE_ADDR			(PER_GROUP3_BASE + 0x0015000)
#define LG_PF_BASE_ADDR				(PER_GROUP3_BASE + 0x0016000)
#define LG_SGM_BASE_ADDR			(PER_GROUP3_BASE + 0x0017000)
#define H264_ENC_BASE_ADDR			(PER_GROUP3_BASE + 0x0020000)
#define MEMU_BASE_ADDR				(PER_GROUP3_BASE + 0x0024000)
#define STCU_BASE_ADDR				(PER_GROUP3_BASE + 0x0028000)
#define SLFTST_CTRL_BASE_ADDR			(PER_GROUP3_BASE + 0x002A000)
#define ENET1_BASE_ADDR				(PER_GROUP3_BASE + 0x0031000)
#define MBIST_CONTROLLER_BASE_ADDR		(PER_GROUP3_BASE + 0x0034000)
#define MIPI_CSI3_BASE_ADDR			(PER_GROUP3_BASE + 0x003D000)
#define DRAM_PHY1_BASE_ADDR 			(PER_GROUP3_BASE + 0x0040000)
#define FLEXCAN3_BASE_ADDR			(PER_GROUP3_BASE + 0x0052000)
#define I2C3_BASE_ADDR				(PER_GROUP3_BASE + 0x0056000)
#define SPI3_BASE_ADDR				(PER_GROUP3_BASE + 0x005A000)
#define CMU_BASE_ADDR				(PER_GROUP3_BASE + 0x005E000)
#define PMC_BASE_ADDR				(PER_GROUP3_BASE + 0x0062000)
#define ATX_DIG_BASE_ADDR			(PER_GROUP3_BASE + 0x0066000)

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
