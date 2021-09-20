/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017-2020 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MACH_S32GEN1_MCME_REGS_H__
#define __ARCH_ARM_MACH_S32GEN1_MCME_REGS_H__

#ifndef __ASSEMBLY__
#include <common.h>

/* MC_ME registers. */
#define MC_ME_CTL_KEY(MC_ME)		(UPTR(MC_ME) + 0x0)
#define MC_ME_CTL_KEY_KEY		(0x00005AF0)
#define MC_ME_CTL_KEY_INVERTEDKEY	(0x0000A50F)

#define MC_ME_MODE_CONF(MC_ME)		(UPTR(MC_ME) + 0x00000004)
#define MC_ME_MODE_CONF_FUNC_RST	(0x1 << 1)

#define MC_ME_MODE_UPD(MC_ME)		(UPTR(MC_ME) + 0x00000008)
#define MC_ME_MODE_UPD_UPD		(0x1 << 0)

#define MC_ME_MODE_STAT(MC_ME)		(UPTR(MC_ME) + 0x0000000C)
#define MC_ME_MODE_STAT_PREVMODE	(0x1 << 0)

/* MC_ME partition definitions */
#define MC_ME_PRTN_N(MC_ME, n)			(UPTR(MC_ME) + 0x100 + \
						 (n) * 0x200)
#define MC_ME_PRTN_N_PCONF(MC_ME, n)		(MC_ME_PRTN_N(MC_ME, n))
#define MC_ME_PRTN_N_PUPD(MC_ME, n)		(MC_ME_PRTN_N(MC_ME, n) + 0x4)
#define MC_ME_PRTN_N_STAT(MC_ME, n)		(MC_ME_PRTN_N(MC_ME, n) + 0x8)
#define MC_ME_PRTN_N_COFB0_STAT(MC_ME, n)	(MC_ME_PRTN_N(MC_ME, n) + 0x10)
#define MC_ME_PRTN_N_COFB0_CLKEN(MC_ME, n)	(MC_ME_PRTN_N(MC_ME, n) + 0x30)

/* MC_ME_PRTN_N_* register fields */
#define MC_ME_PRTN_N_PCE		(1 << 0)
#define MC_ME_PRTN_N_PCUD		BIT(0)
#define MC_ME_PRTN_N_PCS		BIT(0)
#define MC_ME_PRTN_N_OSSE		BIT(2)
#define MC_ME_PRTN_N_OSSUD		BIT(2)
#define MC_ME_PRTN_N_OSSS		BIT(2)
#define MC_ME_PRTN_N_BLOCK(n)		BIT(n)
#define MC_ME_PRTN_N_REQ(n)		BIT(n)

/* MC_ME partition 1 m M definitions. */
#define MC_ME_PRTN_N_CORE_M(n, m)	(MC_ME_BASE_ADDR + 0x140 + \
					 (n) * 0x200 + \
					 mc_me_core2prtn_core_id(n, m) * 0x20)
#define MC_ME_PRTN_N_CORE_M_PCONF(n, m)	(MC_ME_PRTN_N_CORE_M(n, m))
#define MC_ME_PRTN_N_CORE_M_PUPD(n, m)	(MC_ME_PRTN_N_CORE_M(n, m) + 0x4)
#define MC_ME_PRTN_N_CORE_M_STAT(n, m)	(MC_ME_PRTN_N_CORE_M(n, m) + 0x8)
#define MC_ME_PRTN_N_CORE_M_ADDR(n, m)	(MC_ME_PRTN_N_CORE_M(n, m) + 0xC)

/* MC_ME_PRTN_N_CORE_M_* registers fields. */
#define MC_ME_PRTN_N_CORE_M_PCONF_CCE		BIT(0)
#define MC_ME_PRTN_N_CORE_M_PUPD_CCUPD		BIT(0)
#define MC_ME_PRTN_N_CORE_M_STAT_CCS		BIT(0)

#define MC_ME_CM7_PRTN		(0)
#define MC_ME_CORES_PRTN	(1)

/* Reset domain definitions */

#define RDC_RD_N_CTRL(RDC, N)	(UPTR(RDC) + (0x4 * (N)))
#define RDC_RD_N_STATUS(RDC, N)	(UPTR(RDC) + 0x80 + (0x4 * (N)))
#define RD_CTRL_UNLOCK_MASK	(0x80000000)
#define RD_XBAR_DISABLE_MASK	(0x00000008)
#define RDC_RD_CTRL_UNLOCK	(1 << 31)
#define RDC_RD_INTERCONNECT_DISABLE (1 << 3)
#define RDC_RD_INTERCONNECT_DISABLE_REQ_STAT (1 << 3)
#define RDC_RD_INTERCONNECT_DISABLE_STAT (1 << 4)

#define RDC_RD_STAT_XBAR_DISABLE_MASK	BIT(4)

u8 mc_me_core2prtn_core_id(u8 part, u8 id);

#endif

#endif /*__ARCH_ARM_MACH_S32GEN1_MCME_REGS_H__ */
