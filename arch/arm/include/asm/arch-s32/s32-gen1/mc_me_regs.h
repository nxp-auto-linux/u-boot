/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017-2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MACH_S32GEN1_MCME_REGS_H__
#define __ARCH_ARM_MACH_S32GEN1_MCME_REGS_H__

#ifndef __ASSEMBLY__

/* MC_ME registers. */
#define MC_ME_CTL_KEY			(MC_ME_BASE_ADDR)
#define MC_ME_CTL_KEY_KEY		(0x00005AF0)
#define MC_ME_CTL_KEY_INVERTEDKEY	(0x0000A50F)

#define MC_ME_MODE_CONF			((MC_ME_BASE_ADDR) + 0x00000004)
#define MC_ME_MODE_CONF_FUNC_RST	(0x1 << 1)

#define MC_ME_MODE_UPD			((MC_ME_BASE_ADDR) + 0x00000008)
#define MC_ME_MODE_UPD_UPD		(0x1 << 0)

#define MC_ME_MODE_STAT			((MC_ME_BASE_ADDR) + 0x0000000C)
#define MC_ME_MODE_STAT_PREVMODE	(0x1 << 0)

/* MC_ME partition definitions */
#define MC_ME_PRTN_N(n)			(MC_ME_BASE_ADDR + 0x100 + (n) * 0x200)
#define MC_ME_PRTN_N_PCONF(n)		(MC_ME_PRTN_N(n))
#define MC_ME_PRTN_N_PUPD(n)		(MC_ME_PRTN_N(n) + 0x4)
#define MC_ME_PRTN_N_STAT(n)		(MC_ME_PRTN_N(n) + 0x8)

/* MC_ME_PRTN_N_* register fields */
#define MC_ME_PRTN_N_PCE		(1 << 0)
#define MC_ME_PRTN_N_OSSE		(1 << 2)

/* MC_ME partition 1 m M definitions. */
#define MC_ME_PRTN_N_CORE_M(n, m)	(MC_ME_BASE_ADDR + 0x140 + \
						(n) * 0x200 + (m) * 0x20)
#define MC_ME_PRTN_N_CORE_M_PCONF(n, m)	(MC_ME_PRTN_N_CORE_M(n, m))
#define MC_ME_PRTN_N_CORE_M_PUPD(n, m)	(MC_ME_PRTN_N_CORE_M(n, m) + 0x4)
#define MC_ME_PRTN_N_CORE_M_STAT(n, m)	(MC_ME_PRTN_N_CORE_M(n, m) + 0x8)
#define MC_ME_PRTN_N_CORE_M_ADDR(n, m)	(MC_ME_PRTN_N_CORE_M(n, m) + 0xC)


/* MC_ME_PRTN_N_CORE_M_* registers fields. */
#define MC_ME_PRTN_N_CORE_M_PCONF_CCE		(1 << 0)
#define MC_ME_PRTN_N_CORE_M_PUPD_CCUPD		(1 << 0)

#define MC_ME_CORES_PRTN		1

#endif

#endif /*__ARCH_ARM_MACH_S32GEN1_MCME_REGS_H__ */
