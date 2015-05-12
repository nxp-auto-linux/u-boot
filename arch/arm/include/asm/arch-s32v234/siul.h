/*
 * Copyright 2015 Freescale Semiconductor, Inc.
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


#ifndef __ARCH_ARM_MACH_S32V234_SIUL_H__
#define __ARCH_ARM_MACH_S32V234_SIUL_H__

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

#define SIUL2_IMCR_BASE				(SIUL2_BASE_ADDR + 0x00000A40)
#define SIUL2_IMCRn(i)				(SIUL2_IMCR_BASE +  4 * (i))

#define SIUL2_GPDO_BASE				(SIUL2_BASE_ADDR + 0x00001300)
#define SIUL2_GPDOn(i)				(SIUL2_GPDO_BASE + 4 * (i))

#define SIUL2_GPDI_BASE				(SIUL2_BASE_ADDR + 0x00001500)
#define SIUL2_GPDIn(i)				(SIUL2_GPDI_BASE + 4 * (i))

#define SIUL2_PGPDO_BASE			(SIUL2_BASE_ADDR + 0x00001700)
#define SIUL2_PGPDOn(i)				(SIUL2_PGPDO_BASE +  2 * (i))

#define SIUL2_PGPDI_BASE			(SIUL2_BASE_ADDR + 0x00001740)
#define SIUL2_PGPDIn(i)				(SIUL2_PGPDI_BASE + 2 * (i))

#define SIUL2_MPGPDO_BASE			(SIUL2_BASE_ADDR + 0x00001780)
#define SIUL2_MPGPDOn(i)			(SIUL2_MPGPDO_BASE + 4 * (i))


/* SIUL2_MSCR masks */
#define SIUL2_MSCR_DDR_DO_TRIM(v)	((v) & 0xC0000000)
#define SIUL2_MSCR_DDR_INPUT(v)		((v) & 0x20000000)
#define SIUL2_MSCR_DDR_SEL(v)		((v) & 0x18000000)
#define SIUL2_MSCR_DDR_ODT(v)		((v) & 0x07000000)
#define SIUL2_MSCR_DCYCLE_TRIM(v)	((v) & 0x00C00000)

#define SIUL2_MSCR_OBE(v)			((v) & 0x00200000)
#define SIUL2_MSCR_OBE_EN			(1 << 21)

#define SIUL2_MSCR_ODE(v)			((v) & 0x00100000)
#define SIUL2_MSCR_ODE_EN			(1 << 20)

#define SIUL2_MSCR_IBE(v)			((v) & 0x00010000)
#define SIUL2_MSCR_IBE_EN			(1 << 19)

#define SIUL2_MSCR_HYS(v)			((v) & 0x00400000)
#define SIUL2_MSCR_HYS_EN			(1 << 18)

#define SIUL2_MSCR_INV(v)			((v) & 0x00020000)
#define SIUL2_MSCR_INV_EN			(1 << 17)

#define SIUL2_MSCR_PKE(v)			((v) & 0x00010000)
#define SIUL2_MSCR_PKE_EN			(1 << 16)

#define SIUL2_MSCR_SRE(v)			((v) & 0x0000C000)

#define SIUL2_MSCR_PUE(v)			((v) & 0x00002000)
#define SIUL2_MSCR_PUE_EN			(1 << 13)

#define SIUL2_MSCR_PUS(v)			((v) & 0x00001800)

#define SIUL2_MSCR_DSE(v)			((v) & 0x00000700)
#define SIUL2_MSCR_CRPOINT_TRIM(v)	((v) & 0x000000C0)
#define SIUL2_MSCR_SMC(v)			((v) & 0x00000020)
#define SIUL2_MSCR_MUX_MODE(v)		((v) & 0x0000000f)


/* Configure SIUL2 for UART */
#define SIUL2_MSCR_MUX_MODE_UART_Tx		(0x2)
#define SIUL2_MSCR_MUX_MODE_UART_Rx		(0x1)
#define SIUL2_MSCR_PUS_50KOHM_UP		(0x800)


#define SIUL2_MSCR_UART_Rx_OUT_1		714
#define SIUL2_MSCR_UART_Rx_IN_1			13

#define SIUL2_MSCR_UART_Tx_OUT_1		14

/*
 * 714 - UART 1 Receive Data PAD
 *  13 - General Purpose I/O PAD
 */
#define SIUL2_MSCR_UART_Rx(N)									\
	writel(SIUL2_MSCR_MUX_MODE_UART_Rx, SIUL2_MSCRn(SIUL2_MSCR_UART_Rx_OUT_##N));	\
	writel(SIUL2_MSCR_IBE_EN, SIUL2_MSCRn(SIUL2_MSCR_UART_Rx_IN_##N))

/*
 * 14 - General Purpose I/O PAD
 */
#define SIUL2_MSCR_UART1_Tx(N)	\
	writel(SIUL2_MSCR_OBE_EN | SIUL2_MSCR_PUE_EN | SIUL2_MSCR_PUS_50KOHM_UP | SIUL2_MSCR_MUX_MODE_UART_Tx, SIUL2_MSCRn(SIUL2_MSCR_UART_Tx_OUT_##N))


#endif /*__ARCH_ARM_MACH_S32V234_SIUL_H__ */
