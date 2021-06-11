// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2021 NXP
 */
#include <common.h>
#include <linux/sizes.h>
#include <asm/arch/cpu.h>
#include <asm/arch/siul.h>

u32 cpu_pos_mask_cluster0(void)
{
	switch (get_s32g2_derivative()) {
	case S32G274A_DERIV:
		return CPUMASK_CLUSTER0;
	case S32G254A_DERIV:
	case S32G233A_DERIV:
		return BIT(0);
	default:
		return 0;
	}

	return 0;
}

u32 cpu_pos_mask_cluster1(void)
{
	switch (get_s32g2_derivative()) {
	case S32G274A_DERIV:
		return CPUMASK_CLUSTER1;
	case S32G254A_DERIV:
	case S32G233A_DERIV:
		return BIT(2);
	default:
		return 0;
	}

	return 0;
}

u32 get_sram_size(void)
{
	switch (get_s32g2_derivative()) {
	case S32G274A_DERIV:
	case S32G254A_DERIV:
		return S32_SRAM_SIZE;
	case S32G233A_DERIV:
		return 6 * SZ_1M;
	default:
		return 0;
	}

	return 0;
}
