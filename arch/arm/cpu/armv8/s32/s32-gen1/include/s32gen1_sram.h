/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2021 NXP
 */
#ifndef S32GEN1_SRAMC_H
#define S32GEN1_SRAMC_H

#include <linux/types.h>

#define SRAMC0_BASE_ADDR        0x4019C000
#define SRAMC1_BASE_ADDR        0x401A0000
#define SRAMC_SIZE              0x3000

#ifndef __ASSEMBLER__

struct sram_ctrl {
	uintptr_t base_addr;
	u32 min_sram_addr;
	u32 max_sram_addr;
	/**
	 * Translate an A53 SRAM address to SRAM controller offset
	 * associated to that memory region.
	 * This algorithm is platform specific.
	 */
	uintptr_t (*a53_to_sramc_offset)(uintptr_t addr);
};

int s32_sram_clear(uintptr_t start, uintptr_t end);
void s32_ssram_clear(void);
void s32_get_sramc(struct sram_ctrl **ctrls, size_t *size);
#endif
#endif

