/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MACH_S32GEN1_MCME_REGS_H__
#define __ARCH_ARM_MACH_S32GEN1_MCME_REGS_H__

#ifndef __ASSEMBLY__

/* MC_ME registers. */
#define MC_ME_CTL_KEY				(MC_ME_BASE_ADDR)
#define MC_ME_CTL_KEY_KEY			(0x00005AF0)
#define MC_ME_CTL_KEY_INVERTEDKEY		(0x0000A50F)

/* MC_ME partition 1 m M definitions. */
#define MC_ME_PRTN_1_CORE_M(m)			(MC_ME_BASE_ADDR + \
							0x340 + (m) * 0x20)
#define MC_ME_PRTN_1_CORE_M_PCONF(m)		(MC_ME_PRTN_1_CORE_M(m))
#define MC_ME_PRTN_1_CORE_M_PUPD(m)		(MC_ME_PRTN_1_CORE_M(m) + 0x4)
#define MC_ME_PRTN_1_CORE_M_STAT(m)		(MC_ME_PRTN_1_CORE_M(m) + 0x8)
#define MC_ME_PRTN_1_CORE_M_ADDR(m)		(MC_ME_PRTN_1_CORE_M(m) + 0xC)


/* MC_ME_PRTN_N_CORE_M_* registers fields. */
#define MC_ME_PRTN_N_CORE_M_PCONF_CCE		(1 << 0)
#define MC_ME_PRTN_N_CORE_M_PUPD_CCUPD		(1 << 0)

#endif

#endif /*__ARCH_ARM_MACH_S32GEN1_MCME_REGS_H__ */
