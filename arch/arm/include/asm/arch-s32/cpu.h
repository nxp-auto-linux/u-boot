/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2017,2019-2021 NXP
 *
 */
#ifndef _FSL_S32_CPU_H
#define _FSL_S32_CPU_H

#include <linux/types.h>
#include <asm/armv8/mmu.h>
#include <config.h>

#ifndef CONFIG_SYS_DCACHE_OFF
#define TCR_EL2_PS_40BIT	(2 << 16)

#define CONFIG_SYS_FSL_PERIPH_BASE      0x40000000

#if defined(CONFIG_S32V234)
#define CONFIG_SYS_FSL_PERIPH_SIZE      0x40000000
#elif defined(CONFIG_S32_GEN1)
#define CONFIG_SYS_FSL_PERIPH_SIZE      0x20000000
#endif

#ifdef CONFIG_PCIE_S32GEN1
/* TODO: These should go to defconfig, or even better,
 * be read from device tree
 */
#define CONFIG_SYS_PCIE0_PHYS_ADDR_LO		0x53000000ULL
#define CONFIG_SYS_PCIE1_PHYS_ADDR_LO		0x45000000ULL
#define CONFIG_SYS_PCIE0_PHYS_SIZE_LO		0x1000000
#define CONFIG_SYS_PCIE1_PHYS_SIZE_LO		0x1000000
#define CONFIG_SYS_PCIE0_PHYS_ADDR_HI		0x5800000000ULL
#define CONFIG_SYS_PCIE1_PHYS_ADDR_HI		0x4800000000ULL
#define CONFIG_SYS_PCIE0_PHYS_SIZE_HI		0x0800000000ULL
#define CONFIG_SYS_PCIE1_PHYS_SIZE_HI		0x0800000000ULL
#endif

#endif

#if defined(CONFIG_NXP_S32G3XX)
#define CPUMASK_CLUSTER0	(BIT(0) | BIT(1) | BIT(4) | BIT(5))
#define CPUMASK_CLUSTER1	(BIT(2) | BIT(3) | BIT(6) | BIT(7))
#define CPUMASK_LOCKSTEP	(BIT(0) | BIT(1) | BIT(2) | BIT(3))
#elif defined(CONFIG_NXP_S32G2XX) || defined(CONFIG_NXP_S32R45)
#define CPUMASK_CLUSTER0	(BIT(0) | BIT(1))
#define CPUMASK_CLUSTER1	(BIT(2) | BIT(3))
#define CPUMASK_LOCKSTEP	CPUMASK_CLUSTER0
#endif

u32 cpu_mask(void);
int cpu_numcores(void);
u32 cpu_pos_mask_cluster0(void);
u32 cpu_pos_mask_cluster1(void);
u32 cpu_pos_lockstep_mask(void);

#endif /* _FSL_S32_CPU_H */
