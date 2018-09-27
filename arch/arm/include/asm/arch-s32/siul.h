/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017-2018 NXP
 * (C) Copyright 2017 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MACH_S32_SIUL_H__
#define __ARCH_ARM_MACH_S32_SIUL_H__

#include <asm/io.h>
#include "ddr.h"

#define SIUL2_MIDR1				(SIUL2_BASE_ADDR + 0x00000004)
#define SIUL2_MIDR2				(SIUL2_BASE_ADDR + 0x00000008)
#define SIUL2_DISR0				(SIUL2_BASE_ADDR + 0x00000010)
#define SIUL2_DIRER0				(SIUL2_BASE_ADDR + 0x00000018)
#define SIUL2_DIRSR0				(SIUL2_BASE_ADDR + 0x00000020)
#define SIUL2_IREER0				(SIUL2_BASE_ADDR + 0x00000028)
#define SIUL2_IFEER0				(SIUL2_BASE_ADDR + 0x00000030)
#define SIUL2_IFER0				(SIUL2_BASE_ADDR + 0x00000038)

#define SIUL2_IFMCR_BASE			(SIUL2_BASE_ADDR + 0x00000040)
#define SIUL2_IFMCRn(i)				(SIUL2_IFMCR_BASE + 4 * (i))

#define SIUL2_IFCPR				(SIUL2_BASE_ADDR + 0x000000C0)

/* SIUL2_MIDR1 masks */
#define SIUL2_MIDR1_MINOR_MASK		(0xF << 0)
#define SIUL2_MIDR1_MAJOR_SHIFT		(4)
#define SIUL2_MIDR1_MAJOR_MASK		(0xF << SIUL2_MIDR1_MAJOR_SHIFT)

#if defined(CONFIG_S32V234)
#include "siul-s32v234.h"
#elif defined(CONFIG_S32_GEN1)
#include "siul-s32-gen1.h"
#else
#error "Incomplete platform definition"
#endif

#define TREERUNNER_GENERATION_2_MAJOR	1

static inline int get_siul2_midr1_minor(void) {
	return (readl(SIUL2_MIDR1) & SIUL2_MIDR1_MINOR_MASK);
}

static inline int get_siul2_midr1_major(void) {
	return ((readl(SIUL2_MIDR1) & SIUL2_MIDR1_MAJOR_MASK)
			>> SIUL2_MIDR1_MAJOR_SHIFT);
}

#endif /*__ARCH_ARM_MACH_S32V234_SIUL_H__ */
