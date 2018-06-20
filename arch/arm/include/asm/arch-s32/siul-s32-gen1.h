/*
 * (C) Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MACH_S32G1_SIUL_H__
#define __ARCH_ARM_MACH_S32G1_SIUL_H__

/* SIUL2_MSCR specifications as stated in Reference Manual: */

#define SIUL2_MSCR_BASE			(SIUL2_BASE_ADDR + 0x00000240)
#define SIUL2_MSCRn(i)			(SIUL2_MSCR_BASE + 4 * (i))
#define SIUL2_IMCR_BASE			(SIUL2_BASE_ADDR + 0x00000A40)
#define SIUL2_IMCRn(i)			(SIUL2_IMCR_BASE +  4 * (i))

#define SIUL2_MSCR_MUX_MODE(v)		((v) & 0x0000000f)
#define SIUL2_MSCR_MUX_MODE_ALT0	(0x0)
#define SIUL2_MSCR_MUX_MODE_ALT1	(0x1)
#define SIUL2_MSCR_MUX_MODE_ALT2	(0x2)
#define SIUL2_MSCR_MUX_MODE_ALT3	(0x3)
#define SIUL2_MSCR_MUX_MODE_ALT4	(0x4)

/* S32-GEN1 SIUL2_MSCR masks */
#define SIUL2_MSCR_S32_G1_OBE(v)		((v) & 0x00200000)
#define SIUL2_MSCR_S32_G1_OBE_EN		(1 << 21)

#define SIUL2_MSCR_S32_G1_ODE(v)		((v) & 0x00100000)
#define SIUL2_MSCR_S32_G1_ODE_EN		(1 << 20)

#define SIUL2_MSCR_S32_G1_IBE(v)		((v) & 0x00080000)
#define SIUL2_MSCR_S32_G1_IBE_EN		(1 << 19)

#define SIUL2_MSCR_S32_G1_INV(v)		((v) & 0x00020000)
#define SIUL2_MSCR_S32_G1_INV_EN		(1 << 17)

#define SIUL2_MSCR_S32_G1_SRC(v)		((v) & 0x0001C000)
#define SIUL2_MSCR_S32_G1_SRC_208MHz		(0 << 14)
#define SIUL2_MSCR_S32_G1_SRC_150MHz		(4 << 14)
#define SIUL2_MSCR_S32_G1_SRC_100MHz		(5 << 14)
#define SIUL2_MSCR_S32_G1_SRC_50MHz		(6 << 14)
#define SIUL2_MSCR_S32_G1_SRC_25MHz		(7 << 14)

#define SIUL2_MSCR_S32_G1_PUE(v)		((v) & 0x00002000)
#define SIUL2_MSCR_S32_G1_PUE_EN		(1 << 13)

#define SIUL2_MSCR_S32_G1_PUS(v)		((v) & 0x00001000)
#define SIUL2_MSCR_S32_G1_PUS_EN		(1 << 12)

#define SIUL2_MSCR_S32_G1_RCVR(v)		((v) & 0x00000400)
#define SIUL2_MSCR_S32_G1_RCVR_DBL		(0 << 10)
#define SIUL2_MSCR_S32_G1_RCVR_SNGL		(1 << 10)

#define SIUL2_MSCR_S32_G1_SMC(v)		((v) & 0x00000020)
#define SIUL2_MSCR_S32_G1_SMC_DIS		(1 << 5)

#endif /*__ARCH_ARM_MACH_S32G1_SIUL_H__ */
