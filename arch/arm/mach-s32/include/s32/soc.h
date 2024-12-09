/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2024 NXP
 */

#ifndef __S32_SOC_H__
#define __S32_SOC_H__

#define DDR_ATTRS	(PTE_BLOCK_MEMTYPE(MT_NORMAL) | \
			 PTE_BLOCK_OUTER_SHARE | \
			 PTE_BLOCK_NS)

struct mm_region *get_mm_region(u64 phys_base);

#endif
