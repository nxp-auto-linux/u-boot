/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_S32V234_REGS_H__
#define __ASM_ARCH_S32V234_REGS_H__

#define IRAM_BASE_ADDR		0x3E800000  /* internal ram */
#define IRAM_SIZE		0x00400000  /* 4MB */

#if !defined(__ASSEMBLER__)
#define AIPS0_BASE_ADDR		(0x40000000UL)
#define AIPS1_BASE_ADDR		(0x40080000UL)
#else
#define AIPS0_BASE_ADDR		(0x40000000)
#define AIPS1_BASE_ADDR		(0x40080000)
#endif

/* AIPS 0 */
#define AXBS_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00000000)
#define CSE3_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00001000)
#define EDMA_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00002000)
#define XRDC_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00004000)
#define SWT0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0000A000)
#define SWT1_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0000B000)
#define STM0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0000D000)
#define NIC301_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00010000)
#define GC3000_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00020000)
#define DEC200_DECODER_BASE_ADDR			(AIPS0_BASE_ADDR + 0x00026000)
#define DEC200_ENCODER_BASE_ADDR			(AIPS0_BASE_ADDR + 0x00027000)
#define TWOD_ACE_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00028000)
#define MIPI_CSI0_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00030000)
#define DMAMUX0_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00031000)
#define ENET_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00032000)
#define FLEXRAY_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00034000)
#define MMDC0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00036000)
#define MEW0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00037000)
#define MONITOR_DDR0_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00038000)
#define MONITOR_CCI0_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00039000)
#define PIT0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0003A000)
#define MC_CGM0_BASE_ADDR				(AIPS0_BASE_ADDR + 0x0003C000)
#define MC_CGM1_BASE_ADDR				(AIPS0_BASE_ADDR + 0x0003F000)
#define MC_CGM2_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00042000)
#define MC_CGM3_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00045000)
#define MC_RGM_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00048000)
#define MC_ME_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0004A000)
#define MC_PCU_BASE_ADDR				(AIPS0_BASE_ADDR + 0x0004B000)
#define ADC0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0004D000)
#define FLEXTIMER_BASE_ADDR				(AIPS0_BASE_ADDR + 0x0004F000)
#define I2C1_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00051000)
#define LINFLEXD0_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00053000)
#define FLEXCAN0_BASE_ADDR				(AIPS0_BASE_ADDR + 0x00055000)
#define SPI0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00057000)
#define SPI2_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00059000)
#define CRC0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0005B000)
#define USDHC_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0005D000)
#define OCOTP_CONTROLLER_BASE_ADDR			(AIPS0_BASE_ADDR + 0x0005F000)
#define WKPU_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00063000)
#define VIU0_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00064000)
#define HPSMI_SRAM_CONTROLLER_BASE_ADDR			(AIPS0_BASE_ADDR + 0x00068000)
#define SIUL2_BASE_ADDR					(AIPS0_BASE_ADDR + 0x0006C000)
#define SIPI_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00074000)
#define LFAST_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00078000)
#define SSE_BASE_ADDR					(AIPS0_BASE_ADDR + 0x00079000)
#define SRC_SOC_BASE_ADDR				(AIPS0_BASE_ADDR + 0x0007C000)


/* AIPS 1 */
#define ERM_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00000000)
#define MSCM_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00001000)
#define SEMA42_BASE_ADDR				(AIPS1_BASE_ADDR + 0x00002000)
#define INTC_MON_BASE_ADDR				(AIPS1_BASE_ADDR + 0x00003000)
#define SWT2_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00004000)
#define SWT3_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00005000)
#define SWT4_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00006000)
#define STM1_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00007000)
#define EIM_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00008000)
#define APB_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00009000)
#define XBIC_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00012000)
#define MIPI_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00020000)
#define DMAMUX1_BASE_ADDR				(AIPS1_BASE_ADDR + 0x00021000)
#define MMDC1_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00022000)
#define MEW1_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00023000)
#define DDR1_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00024000)
#define CCI1_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00025000)
#define QUADSPI0_BASE_ADDR				(AIPS1_BASE_ADDR + 0x00026000)
#define PIT1_BASE_ADDR					(AIPS1_BASE_ADDR + 0x0002A000)
#define FCCU_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00030000)
#define FLEXTIMER_FTM1_BASE_ADDR			(AIPS1_BASE_ADDR + 0x00036000)
#define I2C2_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00038000)
#define I2C3_BASE_ADDR					(AIPS1_BASE_ADDR + 0x0003A000)
#define LINFLEXD1_BASE_ADDR				(AIPS1_BASE_ADDR + 0x0003C000)
#define FLEXCAN1_BASE_ADDR				(AIPS1_BASE_ADDR + 0x0003E000)
#define SPI1_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00040000)
#define SPI3_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00042000)
#define IPL_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00043000)
#define CGM_CMU_BASE_ADDR				(AIPS1_BASE_ADDR + 0x00044000)
#define PMC_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00048000)
#define CRC1_BASE_ADDR					(AIPS1_BASE_ADDR + 0x0004C000)
#define TMU_BASE_ADDR					(AIPS1_BASE_ADDR + 0x0004E000)
#define VIU1_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00050000)
#define JPEG_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00054000)
#define H264_DEC_BASE_ADDR				(AIPS1_BASE_ADDR + 0x00058000)
#define H264_ENC_BASE_ADDR				(AIPS1_BASE_ADDR + 0x0005C000)
#define MEMU_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00060000)
#define STCU_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00064000)
#define SLFTST_CTRL_BASE_ADDR				(AIPS1_BASE_ADDR + 0x00066000)
#define MCT_BASE_ADDR					(AIPS1_BASE_ADDR + 0x00068000)
#define REP_BASE_ADDR					(AIPS1_BASE_ADDR + 0x0006A000)
#define MBIST_CONTROLLER_BASE_ADDR			(AIPS1_BASE_ADDR + 0x0006C000)
#define BOOT_LOADER_BASE_ADDR				(AIPS1_BASE_ADDR + 0x0006F000)

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
	u16 irsprc[MSCM_IRSPRC_NUM];	/* Interrupt Router Shared Peripheral Routing Control Register	*/
	u32 reserved_0x9e0[136];
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
#include "dma_macros.h"

#endif	/* __ASM_ARCH_S32V234_REGS_H__ */
