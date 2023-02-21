/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2022-2023 NXP
 */
#ifndef S32CC_SIUL2_NVRAM_H
#define S32CC_SIUL2_NVRAM_H

/* siul2_0 */
#define S32CC_SOC_LETTER	0x0
#define S32CC_SOC_PART_NO	0x4
#define S32CC_SOC_MAJOR	0x8
#define S32CC_SOC_MINOR	0xc
#define S32CC_MAX_CORE_FREQ	0x10
#define S32CC_MAX_A53_CORES_PER_CLUSTER	0x14

/* Used for getting a mask where the state of a bit
 * on position N corresponds to Core N,
 * 0 = defeatured
 * 1 = available
 */
#define S32CC_A53_CORES_MASK	0x18

#define S32CC_OVERWRITE_PCIE_DEV_ID		0x1C

/* siul2_1 */
#define S32CC_SERDES_PRESENCE	0x100
#define S32CC_LAX_PRESENCE	0x104
#define S32CC_SOC_SUBMINOR	0x108

#define S32CC_CELL_SIZE		0x4

#endif
