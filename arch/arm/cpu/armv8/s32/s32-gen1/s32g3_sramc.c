// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2021 NXP
 */
#include <config.h>
#include <linux/kernel.h>
#include <s32gen1_sram.h>

#define SRAMC2_BASE_ADDR        0x4055A000
#define SRAMC3_BASE_ADDR        0x4055E000

#define SRAM_BANK_SIZE          (S32_SRAM_SIZE / 4)

#define SRAM_BANK_MIN(N)        (S32_SRAM_BASE + (N) * SRAM_BANK_SIZE)
#define SRAM_BANK_MAX(N)        (S32_SRAM_BASE + ((N) + 1) * SRAM_BANK_SIZE - 1)

static uintptr_t a53_to_sramc_offset(uintptr_t addr)
{
	/**
	 * mem_addr[16:0] = { (bus_addr[24:20] modulo 5),
	 *                    bus_addr[19:8], bus_addr[5:4]};
	 */
	addr = ((addr & 0x30) >> 4) |
	    (((addr & 0xFFF00) >> 8) << 2) |
	    (((addr & 0x1F00000) >> 20) % 5) << 14;

	return addr;
}

void s32_get_sramc(struct sram_ctrl **ctrls, size_t *size)
{
	static struct sram_ctrl controllers[] = {
		{
			.base_addr = SRAMC0_BASE_ADDR,
			.min_sram_addr = SRAM_BANK_MIN(0),
			.max_sram_addr = SRAM_BANK_MAX(0),
			.a53_to_sramc_offset = a53_to_sramc_offset,
		},
		{
			.base_addr = SRAMC1_BASE_ADDR,
			.min_sram_addr = SRAM_BANK_MIN(1),
			.max_sram_addr = SRAM_BANK_MAX(1),
			.a53_to_sramc_offset = a53_to_sramc_offset,
		},
		{
			.base_addr = SRAMC2_BASE_ADDR,
			.min_sram_addr = SRAM_BANK_MIN(2),
			.max_sram_addr = SRAM_BANK_MAX(2),
			.a53_to_sramc_offset = a53_to_sramc_offset,
		},
		{
			.base_addr = SRAMC3_BASE_ADDR,
			.min_sram_addr = SRAM_BANK_MIN(3),
			.max_sram_addr = SRAM_BANK_MAX(3),
			.a53_to_sramc_offset = a53_to_sramc_offset,
		},
	};

	*ctrls = &controllers[0];
	*size = ARRAY_SIZE(controllers);
}
