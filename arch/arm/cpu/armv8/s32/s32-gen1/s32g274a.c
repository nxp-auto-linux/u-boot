// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2021-2022 NXP
 */
#include <common.h>
#include <dm.h>
#include <linux/sizes.h>
#include <misc.h>
#include <asm/arch/cpu.h>
#include <s32gen1_siul2_nvram.h>

enum s32g2_derivative {
	S32G274A_DERIV,
	S32G254A_DERIV,
	S32G233A_DERIV,
	S32G2_INVAL_DERIV,
};

static inline int get_s32g2_derivative(void)
{
	struct udevice *siul20_nvmem = NULL;
	u32 deriv = 0;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_MISC, "siul2_0_nvram",
					&siul20_nvmem);
	if (ret < 0)
		return ret;

	ret = misc_read(siul20_nvmem, S32GEN1_SOC_PART_NO, &deriv,
			sizeof(deriv));
	if (ret < 0)
		return ret;

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
