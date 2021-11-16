/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2021 NXP
 */

#ifndef __ASM_ARCH_IMX_REGS_H__
#define __ASM_ARCH_IMX_REGS_H__
#include <config.h>

#define ARCH_MXC

#include "s32-gen1/s32-gen1-regs.h"

/* TODO Remove this after the IOMUX framework is implemented */
#define IOMUXC_BASE_ADDR SIUL2_BASE_ADDR

/* MUX mode and PAD ctrl are in one register */
#define CONFIG_IOMUX_SHARE_CONF_REG

#define I2C_QUIRK_REG

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

/* UART */
struct linflex_fsl {
	u32 lincr1;
	u32 linier;
	u32 linsr;
	u32 linesr;
	u32 uartcr;
	u32 uartsr;
	u32 lintcsr;
	u32 linocr;
	u32 lintocr;
	u32 linfbrr;
	u32 linibrr;
	u32 lincfr;
	u32 lincr2;
	u32 bidr;
	u32 bdrl;
	u32 bdrm;
	u32 reserved[3];
	u32 gcr;
	u32 uartpto;
	u32 uartcto;
	u32 dmatxe;
	u32 dmarxe;
};

#endif	/* __ASSEMBLER__*/

#endif	/* __ASM_ARCH_IMX_REGS_H__ */
