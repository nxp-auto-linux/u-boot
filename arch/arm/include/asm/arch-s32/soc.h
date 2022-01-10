/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor
 * Copyright 2017-2018,2020-2022 NXP
 */
#ifndef __ARCH_S32_SOC_H
#define __ARCH_S32_SOC_H

#include "s32-gen1/s32-gen1-regs.h"
#include <asm/arch/siul.h>
#include <asm/arch/clock.h>
#include <asm/arch/mc_cgm_regs.h>
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/mc_rgm_regs.h>
#include <asm/arch/mmdc.h>

void setup_iomux_enet(void);

#ifdef CONFIG_DCU_QOS_FIX
int board_dcu_qos(void);
#endif

u32 get_sram_size(void);

static inline bool is_addr_in_sram(uintptr_t addr)
{
	return addr >= S32_SRAM_BASE && addr < S32_SRAM_BASE + get_sram_size();
}

void cpu_pci_clock_init(const int clockexternal);

#endif /* __ARCH_S32_SOC_H */
