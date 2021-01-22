// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2021 NXP
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 * contributors may be used to endorse or promote products derived from this
 * software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "ddr_density.h"
#include "ddr_utils.h"

#include <linux/bitops.h>
#include <bitfield.h>
#include <linux/kernel.h>

#define OFFSET_DDRC_START_ADDRMAP	0x21C

#define NO_ADDRMAP_REGS			8

/* ECC */
#define ECC_MODE_MASK	0x00000007
#define ECC_MODE_SHIFT	0x00000000
#define ECC_DISABLE	0x00000000
#define ECC_FLAG_MASK	0x00000001

enum addr_map_masks {
	ADDRMAP7 = 0x00000f0f,
	ADDRMAP6 = 0x0f0f0f0f,
	ADDRMAP5 = 0x0f0f0f00,
	ADDRMAP4 = 0x00001f1f,
	ADDRMAP3 = 0x1f1f1f1f,
	ADDRMAP2 = 0x0f0f1f0f,
	ADDRMAP1 = 0x003f3f3f,
	ADDRMAP0 = 0x0000001f,
};

enum addr_map_internal_base {
	ADDRMAP7_BASE = 0x00001716,
	ADDRMAP6_BASE = 0x15141312,
	ADDRMAP5_BASE = 0x11060706,
	ADDRMAP4_BASE = 0x00000b0a,
	ADDRMAP3_BASE = 0x09080706,
	ADDRMAP2_BASE = 0x05040302,
	ADDRMAP1_BASE = 0x00040302,
	ADDRMAP0_BASE = 0x00000006,
};

enum addr_map_shift {
	ADDRMAP7_SHIFT = 0x00000800,
	ADDRMAP6_SHIFT = 0x18100800,
	ADDRMAP5_SHIFT = 0x18100800,
	ADDRMAP4_SHIFT = 0x00000800,
	ADDRMAP3_SHIFT = 0x18100800,
	ADDRMAP2_SHIFT = 0x18100800,
	ADDRMAP1_SHIFT = 0x00100800,
	ADDRMAP0_SHIFT = 0x00000000,
};

static const enum addr_map_masks addr_map_masks_map[] = {
	ADDRMAP7, ADDRMAP6, ADDRMAP5, ADDRMAP4,
	ADDRMAP3, ADDRMAP2, ADDRMAP1, ADDRMAP0
};

static const enum addr_map_shift addr_map_shift_map[] = {
	ADDRMAP7_SHIFT, ADDRMAP6_SHIFT, ADDRMAP5_SHIFT,
	ADDRMAP4_SHIFT, ADDRMAP3_SHIFT, ADDRMAP2_SHIFT,
	ADDRMAP1_SHIFT, ADDRMAP0_SHIFT
};

static const enum addr_map_internal_base addr_map_internal_base_map[] = {
	ADDRMAP7_BASE, ADDRMAP6_BASE, ADDRMAP5_BASE, ADDRMAP4_BASE,
	ADDRMAP3_BASE, ADDRMAP2_BASE, ADDRMAP1_BASE, ADDRMAP0_BASE
};

static inline u32 get_byte(u32 val, u32 nr)
{
	return bitfield_extract(val, 8 * nr, 8);
}

void s32gen1_get_ddr_regions(struct s32_ddr_region
			     pages[S32GEN1_DDR_MAX_NO_PAGES],
			     int *active_pages)
{
	u32 i, sh, mk;
	u32 *reg = (u32 *)(DDRC_BASE_ADDR + OFFSET_DDRC_START_ADDRMAP);
	u32 idx, tmp, reg_val, max_hif = 0;
	struct s32_ddr_region curr_page = {
		.address = 0x800000000,
		/* Reset value */
		.flags = 0x0,
	};

	reg_val = readl(DDRC_BASE_ADDR + OFFSET_DDRC_ECCCFG0);
	reg_val = (reg_val & ECC_MODE_MASK) >> ECC_MODE_SHIFT;

	if (reg_val != ECC_DISABLE)
		curr_page.flags |= ECC_ON;

	/**
	 * Calculate the size.
	 * Use HIF address bit number to determine total density
	 */
	for (idx = 0; idx < NO_ADDRMAP_REGS ; idx++, reg--) {
		/* We start with highest reg */
		reg_val = readl(reg);

		if ((reg_val & addr_map_masks_map[idx]) ==
		    addr_map_masks_map[idx])
			continue;

		for (i = 0; i < 4; i++) {
			sh = get_byte(addr_map_shift_map[idx], i);
			mk = get_byte(addr_map_masks_map[idx], i);
			/**
			 * If is equals to mask value it means it is disabled so
			 * we don't need to process it
			 */
			tmp = ((reg_val >> sh) & mk);
			if (tmp != mk) {
				tmp += get_byte(addr_map_internal_base_map[idx],
						i);
				max_hif = max(tmp, max_hif);
			}
		}
	}

	max_hif++;
	curr_page.size =  (1 << max_hif);

	/**
	 * Convert to AXI
	 * See details in System Address to Physical Address Mapping
	 */
	curr_page.size <<= 2;

	/**
	 * If ECC is used we need to exclude the ECC region
	 * ECC is always the last 1/8 of the memory
	 */
	if (curr_page.flags & ECC_FLAG_MASK)
		curr_page.size = (curr_page.size * 7) / 8;

	*active_pages = 1;

	pages[0] =  curr_page;
}

