/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor
 * (C) Copyright 2017-2018,2020 NXP
 */
#ifndef __ARCH_S32_SOC_H
#define __ARCH_S32_SOC_H

#include <asm/arch/imx-regs.h>
#include <asm/arch/siul.h>
#include <asm/arch/clock.h>
#include <asm/arch/xrdc.h>
#include <asm/arch/mc_cgm_regs.h>
#include <asm/arch/mc_me_regs.h>
#include <asm/arch/mc_rgm_regs.h>
#include <asm/arch/src.h>
#include <asm/arch/mmdc.h>
#include <asm/arch/ddr.h>
#if defined(CONFIG_S32_LPDDR2)
#include <asm/arch/lpddr2.h>
#elif defined(CONFIG_S32_DDR3)
#include <asm/arch/ddr3.h>
#elif defined(CONFIG_S32_LPDDR4)
#include <asm/arch/lpddr4.h>
#else
#error "Please define the DDR type!"
#endif


void setup_iomux_enet(void);

#ifdef CONFIG_FSL_DCU_FB
void setup_iomux_dcu(void);
#endif

#ifdef CONFIG_DCU_QOS_FIX
int board_dcu_qos(void);
#endif

static inline bool is_addr_in_sram(uintptr_t addr)
{
	return addr >= S32_SRAM_BASE && addr < S32_SRAM_BASE + S32_SRAM_SIZE;
}

void cpu_pci_clock_init(const int clockexternal);

#endif /* __ARCH_S32_SOC_H */
