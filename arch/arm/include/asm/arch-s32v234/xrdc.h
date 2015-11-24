/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ASM_ARCH_XRDC_H
#define __ASM_ARCH_XRDC_H

#include "imx-regs.h"

#define XRDC_ADDR_MIN    (0x00000000)
#define XRDC_ADDR_MAX    (0xffffffff)
#define XRDC_VALID       (0x80000000)

#define XRDC_MRGD_W0_16   (XRDC_BASE_ADDR + 0x2200L)
#define XRDC_MRGD_W1_16   (XRDC_BASE_ADDR + 0x2204L)
#define XRDC_MRGD_W3_16   (XRDC_BASE_ADDR + 0x220CL)

#define XRDC_MRGD_W0_17   (XRDC_BASE_ADDR + 0x2220L)
#define XRDC_MRGD_W1_17   (XRDC_BASE_ADDR + 0x2224L)
#define XRDC_MRGD_W3_17   (XRDC_BASE_ADDR + 0x222CL)

#define XRDC_MRGD_W0_18   (XRDC_BASE_ADDR + 0x2240L)
#define XRDC_MRGD_W1_18   (XRDC_BASE_ADDR + 0x2244L)
#define XRDC_MRGD_W3_18   (XRDC_BASE_ADDR + 0x224CL)

#define XRDC_MRGD_W0_19   (XRDC_BASE_ADDR + 0x2260L)
#define XRDC_MRGD_W1_19   (XRDC_BASE_ADDR + 0x2264L)
#define XRDC_MRGD_W3_19   (XRDC_BASE_ADDR + 0x226CL)

#endif /* __ASM_ARCH_XRDC_H */
