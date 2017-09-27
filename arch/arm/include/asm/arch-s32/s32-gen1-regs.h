/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_S32_GEN1_REGS_H__
#define __ASM_ARCH_S32_GEN1_REGS_H__

#define IRAM_BASE_ADDR		0x38000000  /* internal ram */
#define IRAM_SIZE		0x00400000  /* 4MB */

#if !defined(__ASSEMBLER__)
#define PER_GROUP0_BASE				(0x40000000UL)
#define PER_GROUP1_BASE				(0x40100000UL)
#define PER_GROUP2_BASE				(0x40200000UL)
#define PER_GROUP3_BASE				(0x40300000UL)
#define PER_GROUP8_BASE				(0x44000000UL)
#else
#define PER_GROUP0_BASE				(0x40000000)
#define PER_GROUP1_BASE				(0x40100000)
#define PER_GROUP2_BASE				(0x40200000)
#define PER_GROUP3_BASE				(0x40300000)
#define PER_GROUP8_BASE				(0x44000000)
#endif

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

/* MSCM interrupt router */
#define MSCM_IRSPRC_CPn_EN		0xF
#define MSCM_IRSPRC_NUM			248

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

/* MSCM Interrupt Router */
struct mscm_ir {
	u32 cpxtype;		/* Processor x Type Register			*/
	u32 cpxnum;		/* Processor x Number Register			*/
	u32 cpxmaster;		/* Processor x Master Number Register	*/
	u32 cpxcount;		/* Processor x Count Register			*/
	u32 cpxcfg0;		/* Processor x Configuration 0 Register */
	u32 cpxcfg1;		/* Processor x Configuration 1 Register */
	u32 cpxcfg2;		/* Processor x Configuration 2 Register */
	u32 cpxcfg3;		/* Processor x Configuration 3 Register */
	u32 cpntype;		/* Processor 0 Type Register			*/
	u32 cpnnum;		/* Processor 0 Number Register			*/
	u32 cpnmaster;		/* Processor 0 Master Number Register	*/
	u32 cpncount;		/* Processor 0 Count Register			*/
	u32 cpncfg0;		/* Processor 0 Configuration 0 Register	*/
	u32 cpncfg1;		/* Processor 0 Configuration 1 Register	*/
	u32 cpncfg2;		/* Processor 0 Configuration 2 Register	*/
	u32 cpncfg3;		/* Processor 0 Configuration 3 Register	*/
	u32 cpntype_1;		/* Processor 1 Type Register			*/
	u32 cpnnum_1;		/* Processor 1 Number Register			*/
	u32 cpnmaster_1;	/* Processor 1 Master Number Register	*/
	u32 cpncount_1;		/* Processor 1 Count Register			*/
	u32 cpncfg0_1;		/* Processor 1 Configuration 0 Register	*/
	u32 cpncfg1_1;		/* Processor 1 Configuration 1 Register	*/
	u32 cpncfg2_1;		/* Processor 1 Configuration 2 Register	*/
	u32 cpncfg3_1;		/* Processor 1 Configuration 3 Register	*/
	u32 cpntype_2;		/* Processor 2 Type Register			*/
	u32 cpnnum_2;		/* Processor 2 Number Register			*/
	u32 cpnmaster_2;	/* Processor 2 Master Number Register	*/
	u32 cpncount_2;		/* Processor 2 Count Register			*/
	u32 cpncfg0_2;		/* Processor 2 Configuration 0 Register	*/
	u32 cpncfg1_2;		/* Processor 2 Configuration 1 Register	*/
	u32 cpncfg2_2;		/* Processor 2 Configuration 2 Register	*/
	u32 cpncfg3_2;		/* Processor 2 Configuration 3 Register	*/
	u32 cpntype_3;		/* Processor 3 Type Register			*/
	u32 cpnnum_3;		/* Processor 3 Number Register			*/
	u32 cpnmaster_3;	/* Processor 3 Master Number Register	*/
	u32 cpncount_3;		/* Processor 3 Count Register			*/
	u32 cpncfg0_3;		/* Processor 3 Configuration 0 Register	*/
	u32 cpncfg1_3;		/* Processor 3 Configuration 1 Register	*/
	u32 cpncfg2_3;		/* Processor 3 Configuration 2 Register	*/
	u32 cpncfg3_3;		/* Processor 3 Configuration 3 Register	*/
	u32 reserved_0x060[216];
	u32 ocmdr0;		/* On-Chip Memory Descriptor Register	*/
	u32 ocmdr1;		/* On-Chip Memory Descriptor Register	*/
	u32 ocmdr2;		/* On-Chip Memory Descriptor Register	*/
	u32 ocmdr3;		/* On-Chip Memory Descriptor Register	*/
	u32 reserved_0x410[28];
	u32 tcmdr[4];		/* Generic Tightly Coupled Memory Descriptor Register	*/
	u32 reserved_0x490[28];
	u32 cpce0;		/* Core Parity Checking Enable Register 0	*/
	u32 reserved_0x504[191];
	u32 ircp0ir;		/* Interrupt Router CP0 Interrupt Register		*/
	u32 ircp1ir;		/* Interrupt Router CP1 Interrupt Register		*/
	u32 ircp2ir;		/* Interrupt Router CP2 Interrupt Register		*/
	u32 ircp3ir;		/* Interrupt Router CP3 Interrupt Register		*/
	u32 reserved_0x810[4];
	u32 ircpgir;		/* Interrupt Router CPU Generate Interrupt Register		*/
	u32 reserved_0x824[23];
	u16 irsprc[MSCM_IRSPRC_NUM];	/* Interrupt Router Shared Peripheral Routing Control Register	*/
	u32 reserved_0xa70[93];
	u32 iahbbe0;		/* Gasket Burst Enable Register							*/
	u32 reserved_0xc04[63];
	u32 ipcge;			/* Interconnect Parity Checking Global Enable Register	*/
	u32 reserved_0xd04[3];
	u32 ipce[4];		/* Interconnect Parity Checking Enable Register			*/
	u32 reserved_0xd20[8];
	u32 ipcgie;			/* Interconnect Parity Checking Global Injection Enable Register	*/
	u32 reserved_0xd44[3];
	u32 ipcie[4];           /* Interconnect Parity Checking Injection Enable Register       */
};
#endif
#endif	/* __ASM_ARCH_S32XXXX_GEN1_REGS_H__ */
