/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017 MicroSys Electronics GmbH
 * Copyright 2017-2022 NXP
 *
 */

#ifndef __ARCH_ARM_MACH_S32_SIUL_H__
#define __ARCH_ARM_MACH_S32_SIUL_H__

#include "s32-gen1/s32-gen1-regs.h"
#include <asm/io.h>
#include <linux/bitops.h>

#define SIUL2_MIDR1				(SIUL2_0_BASE_ADDR + 0x00000004)
#define SIUL2_MIDR2				(SIUL2_0_BASE_ADDR + 0x00000008)

/* SIUL2_MIDR2 masks */
#define SIUL2_MIDR2_FREQ_SHIFT		(16)
#define SIUL2_MIDR2_FREQ_MASK		(0xF << SIUL2_MIDR2_FREQ_SHIFT)

#define SIUL2_MIDR1_DERIV_MASK		(0xFFFF0000U)
#define SIUL2_MIDR1_OFF			(16U)

#ifdef CONFIG_NXP_S32G2XX
enum s32g2_derivative {
	S32G274A_DERIV,
	S32G254A_DERIV,
	S32G233A_DERIV,
	S32G2_INVAL_DERIV,
};

static inline enum s32g2_derivative get_s32g2_derivative(void)
{
	u32 deriv;

	deriv = readl(SIUL2_MIDR1) & SIUL2_MIDR1_DERIV_MASK;
	deriv >>= SIUL2_MIDR1_OFF;

	switch (deriv) {
	case 0x1D12U:
		return S32G274A_DERIV;
	case 0x1CFEU:
		return S32G254A_DERIV;
	case 0x1CE9U:
		return S32G233A_DERIV;
	};

	pr_err("Invalid S32G2 derivative: 0x%x\n", deriv);
	return S32G2_INVAL_DERIV;
}

static inline const char *get_s32g2_deriv_name(void)
{
	enum s32g2_derivative deriv;

	deriv = get_s32g2_derivative();
	switch (deriv) {
	case S32G274A_DERIV:
		return "S32G274A";
	case S32G254A_DERIV:
		return "S32G254A";
	case S32G233A_DERIV:
		return "S32G233A";
	case S32G2_INVAL_DERIV:
		return "INVAL";
	}

	return "INVAL";
}
#endif

static inline u32 get_siul2_midr2_freq(void)
{
	return ((readl(SIUL2_MIDR2) & SIUL2_MIDR2_FREQ_MASK)
			>> SIUL2_MIDR2_FREQ_SHIFT);
}

#endif /*____ARCH_ARM_MACH_S32_SIUL_H__ */
