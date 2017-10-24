/*
 * (C) Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ARCH_ARM_MACH_S32_MCME_REGS_H__
#define __ARCH_ARM_MACH_S32_MCME_REGS_H__

#include <config.h>

#if defined(CONFIG_S32V234)
#include "s32v234/mc_me_regs.h"
#elif defined(CONFIG_S32_GEN1)
#include "s32-gen1/mc_me_regs.h"
#else
#error "Incomplete platform definition"
#endif

#endif /*__ARCH_ARM_MACH_S32_MCME_REGS_H__ */

