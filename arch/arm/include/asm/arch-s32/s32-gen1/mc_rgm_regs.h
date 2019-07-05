/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2016-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MACH_S32GEN1_MCRGM_REGS_H__
#define __ARCH_ARM_MACH_S32GEN1_MCRGM_REGS_H__

#define RGM_DES				(MC_RGM_BASE_ADDR + 0x0)
#define RGM_DES_POR			(0x00000001)

#define RGM_FES				(MC_RGM_BASE_ADDR + 0x8)
#define RGM_FES_EXT			(0x00000001)

#define RGM_PRST(per)			(MC_RGM_BASE_ADDR + 0x40 + (per) * 0x8)
#define RGM_PSTAT(per)			(MC_RGM_BASE_ADDR + 0x140 + (per) * 0x8)

#define RGM_CORES_RESET_GROUP		1

#define RGM_CORE_RST(num)               (0x1 << ((num) + 1))

#endif /* __ARCH_ARM_MACH_S32GEN1_MCRGM_REGS_H__ */

