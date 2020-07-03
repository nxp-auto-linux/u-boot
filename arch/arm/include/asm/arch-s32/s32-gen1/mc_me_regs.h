/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017-2020 NXP
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
#define MC_ME_PRTN_N_COFB0_STAT(n)	(MC_ME_PRTN_N(n) + 0x10)
#define MC_ME_PRTN_N_COFB0_CLKEN(n)	(MC_ME_PRTN_N(n) + 0x30)

/* MC_ME_PRTN_N_* register fields */
#define MC_ME_PRTN_N_PCE		(1 << 0)
#define MC_ME_PRTN_N_PCUD		BIT(0)
#define MC_ME_PRTN_N_PCS		BIT(0)
#define MC_ME_PRTN_N_OSSE		(1 << 2)
#define MC_ME_PRTN_N_OSSUD		BIT(2)
#define MC_ME_PRTN_N_OSSS		BIT(2)
#define MC_ME_PRTN_N_BLOCK(n)		BIT(n)
#define MC_ME_PRTN_N_REQ(n)		BIT(n)

/* MC_ME partition 1 m M definitions. */
#define MC_ME_PRTN_N_CORE_M(n, m)	(MC_ME_BASE_ADDR + 0x140 + \
						(n) * 0x200 + (m) * 0x20)
#define MC_ME_PRTN_N_CORE_M_PCONF(n, m)	(MC_ME_PRTN_N_CORE_M(n, m))
#define MC_ME_PRTN_N_CORE_M_PUPD(n, m)	(MC_ME_PRTN_N_CORE_M(n, m) + 0x4)
#define MC_ME_PRTN_N_CORE_M_STAT(n, m)	(MC_ME_PRTN_N_CORE_M(n, m) + 0x8)
#define MC_ME_PRTN_N_CORE_M_ADDR(n, m)	(MC_ME_PRTN_N_CORE_M(n, m) + 0xC)

/* MC_ME_PRTN_N_CORE_M_* registers fields. */
#define MC_ME_PRTN_N_CORE_M_PCONF_CCE		BIT(0)
#define MC_ME_PRTN_N_CORE_M_PUPD_CCUPD		BIT(0)
#define MC_ME_PRTN_N_CORE_M_STAT_CCS		BIT(0)

/* MC_ME partition mapping */
#define MC_ME_CORES_PRTN	(1)
#define MC_ME_USDHC_PRTN	(0)
#define MC_ME_DDR_0_PRTN	(0)
#define MC_ME_PFE_PRTN		(2)
#define MC_ME_CM7_PRTN		(0)
#define MC_ME_LAX_PRTN		(2)
#define MC_ME_SPT_PRTN		(3)

#define MC_ME_USDHC_REQ		(0)
#define MC_ME_DDR_0_REQ		(1)
#define MC_ME_PFE_EMAC0_REQ	(1 << 0)
#define MC_ME_PFE_EMAC1_REQ	(1 << 1)
#define MC_ME_PFE_EMAC2_REQ	(1 << 2)
#define MC_ME_PFE_TS_REQ	(1 << 3)
#define MC_ME_PFE_REQ_GROUP	(MC_ME_PFE_EMAC0_REQ | MC_ME_PFE_EMAC1_REQ | \
				 MC_ME_PFE_EMAC2_REQ | MC_ME_PFE_TS_REQ)
#define MC_ME_MIPICSI2_0_REQ	(5)
#define MC_ME_MIPICSI2_1_REQ	(6)
#define MC_ME_MIPICSI2_2_REQ	(7)
#define MC_ME_MIPICSI2_3_REQ	(8)
#define MC_ME_FDMA_REQ	(9)
#define MC_ME_FLEXCAN4_REQ	(11)
#define MC_ME_FLEXCAN5_REQ	(12)
#define MC_ME_FLEXCAN6_REQ	(13)
#define MC_ME_FLEXCAN7_REQ	(14)

/* S32R45 partition 2 */
#define MC_ME_LAX1_REQ		(0)
#define MC_ME_LAX2_REQ		(1)

/* S32R45 partition 3 */
#define MC_ME_CTE_XBAR3_REQ	(2)
#define MC_ME_EIM_DSP_REQ	(3)
#define MC_ME_BBE32EP_REQ	(4)
#define MC_ME_SPT_REQ		(5)

/* Reset domain definitions */

#define RDC_RD_2_CTRL		(RDC_BASE_ADDR + 0x8)
#define RDC_RD_2_STAT		(RDC_BASE_ADDR + 0x88)
#define RDC_RD_N_CTRL(N)	(RDC_BASE_ADDR + (0x4 * (N)))
#define RDC_RD_N_STATUS(N)	(RDC_BASE_ADDR + 0x80 + (0x4 * (N)))
#define RD_CTRL_UNLOCK_MASK	(0x80000000)
#define RD_XBAR_DISABLE_MASK	(0x00000008)
#define RDC_RD_CTRL_UNLOCK	(1 << 31)
#define RDC_RD_INTERCONNECT_DISABLE (1 << 3)
#define RDC_RD_INTERCONNECT_DISABLE_REQ_STAT (1 << 3)
#define RDC_RD_INTERCONNECT_DISABLE_STAT (1 << 4)

#define RDC_RD_STAT_XBAR_DISABLE_MASK	BIT(4)

#endif

#endif /*__ARCH_ARM_MACH_S32GEN1_MCME_REGS_H__ */
