/*
 * (C) Copyright 2015 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#ifndef __ARCH_ARM_MACH_MAC57D5XH_SIUL_H__
#define __ARCH_ARM_MACH_MAC57D5XH_SIUL_H__

#define SIUL2_MIDR1					(SIUL2_BASE_ADDR + 0x00000004)
#define SIUL2_MIDR2					(SIUL2_BASE_ADDR + 0x00000008)
#define SIUL2_DISR0					(SIUL2_BASE_ADDR + 0x00000010)
#define SIUL2_DIRER0				(SIUL2_BASE_ADDR + 0x00000018)
#define SIUL2_DIRSR0				(SIUL2_BASE_ADDR + 0x00000020)
#define SIUL2_IREER0				(SIUL2_BASE_ADDR + 0x00000028)
#define SIUL2_IFEER0				(SIUL2_BASE_ADDR + 0x00000030)
#define SIUL2_IFER0					(SIUL2_BASE_ADDR + 0x00000038)

#define SIUL2_IFMCR_BASE			(SIUL2_BASE_ADDR + 0x00000040)
#define SIUL2_IFMCRn(i)				(SIUL2_IFMCR_BASE + 4 * (i))

#define SIUL2_IFCPR					(SIUL2_BASE_ADDR + 0x000000C0)

/* SIUL2_MSCR specifications as stated in Reference Manual:
 * 0 - 359 Output Multiplexed Signal Configuration Registers
 * 512- 1023 Input Multiplexed Signal Configuration Registers */
#define SIUL2_MSCR_BASE				(SIUL2_BASE_ADDR + 0x00000240)
#define SIUL2_MSCRn(i)				(SIUL2_MSCR_BASE + 4 * (i))

#define SIUL2_GRP0					(SIUL2_BASE_ADDR + 0x00001240)
#define SIUL2_GPR1					(SIUL2_BASE_ADDR + 0x00001244)
#define SIUL2_GPR2					(SIUL2_BASE_ADDR + 0x00001248)

#define SIUL2_GPDO_BASE				(SIUL2_BASE_ADDR + 0x00001300)
#define SIUL2_GPDOn(i)				(SIUL2_GPDO_BASE + 4 * (i))

#define SIUL2_GPDI_BASE				(SIUL2_BASE_ADDR + 0x00001500)
#define SIUL2_GPDIn(i)				(SIUL2_GPDI_BASE + 4 * (i))

#define SIUL2_PGPDO_BASE			(SIUL2_BASE_ADDR + 0x00001700)
#define SIUL2_PGPDOn(i)				(SIUL2_PGPDO_BASE +  2 * ((i) % 2 ? ((i) - 1) : ((i) + 1)))

#define SIUL2_PGPDI_BASE			(SIUL2_BASE_ADDR + 0x00001740)
#define SIUL2_PGPDIn(i)				(SIUL2_PGPDI_BASE + 2 * ((i) % 2 ? ((i) - 1) : ((i) + 1)))

#define SIUL2_MPGPDO_BASE			(SIUL2_BASE_ADDR + 0x00001780)
#define SIUL2_MPGPDOn(i)			(SIUL2_MPGPDO_BASE + 4 * (i))


/* SIUL2_MSCR masks */
#define SIUL2_MSCR_SRE(v)			((v) & 0x30000000)

#define SIUL2_MSCR_OBE(v)			((v) & 0x02000000)
#define SIUL2_MSCR_OBE_EN			(1 << 25)

#define SIUL2_MSCR_ODE(v)			((v) & 0x01000000)
#define SIUL2_MSCR_ODE_EN			(1 << 24)

#define SIUL2_MSCR_SMCTL(v)			((v) & 0x00800000)
#define SIUL2_MSCR_APC(v)			((v) & 0x00400000)
#define SIUL2_MSCR_TTL(v)			((v) & 0x00100000)

#define SIUL2_MSCR_IBE(v)			((v) & 0x00080000)
#define SIUL2_MSCR_IBE_EN			(1 << 19)

#define SIUL2_MSCR_HYS(v)			((v) & 0x00400000)
#define SIUL2_MSCR_HYS_EN			(1 << 18)

#define SIUL2_MSCR_PUS(v)			((v) & 0x00200000)
#define SIUL2_MSCR_PUS_UP			(1 << 17)

#define SIUL2_MSCR_PUE(v)			((v) & 0x00100000)
#define SIUL2_MSCR_PUE_EN			(1 << 16)

#define SIUL2_MSCR_SSS(v)			((v) & 0x0000000f)

/* Configure SIUL2 for UART */
#define SIUL2_MSCR_SSS_UART			(0x3)

/*
 * See IO_Signal_Description_and_Multiplexing_Tables.xlsx document for
 * UART2 configuration details:
 * 714 - LinFlex2_Receive_Input PAD
 * 173 - General Purpose I/O PAD
 */
#define SIUL2_MSCR_UART_Rx_OUT_2		714
#define SIUL2_MSCR_UART_Rx_IN_2			173

#define SIUL2_MSCR_UART_Tx_OUT_2		174

#define SIUL2_MSCR_UART_Rx(N)												\
	writel(SIUL2_MSCR_SSS_UART, SIUL2_MSCRn(SIUL2_MSCR_UART_Rx_OUT_##N));	\
	writel(SIUL2_MSCR_IBE_EN, SIUL2_MSCRn(SIUL2_MSCR_UART_Rx_IN_##N))

/* 174 - General Purpose I/O PAD */
#define SIUL2_MSCR_UART_Tx(N)\
	writel(SIUL2_MSCR_OBE_EN | SIUL2_MSCR_PUS_UP | SIUL2_MSCR_PUE_EN | SIUL2_MSCR_SSS_UART, SIUL2_MSCRn(SIUL2_MSCR_UART_Tx_OUT_##N))


#define SIUL2_MSCR_LINFLEX0_Rx		SIUL2_MSCRn(712)

#define SIUL2_MSCR_LINFLEX1_Rx		SIUL2_MSCRn(713)

#endif /*__ARCH_ARM_MACH_MAC57D5XH_SIUL_H__ */
