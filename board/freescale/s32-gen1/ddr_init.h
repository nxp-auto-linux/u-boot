/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2020-2021 NXP
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

#ifndef DDR_INIT_H_
#define DDR_INIT_H_

#include <stdlib.h>
#include "ddr_utils.h"

#define APBONLY_MICRORESET   0x40380420
#define MASTER_PLLCTRL1      0x403816f0
#define MASTER_PLLTESTMODE   0x40381708
#define MASTER_PLLCTRL4      0x4038171c
#define MASTER_PLLCTRL2      0x403816dc

#define MICROCONT_MUX_SEL 0x40380400
#define LOCK_CSR_ACCESS   0x00000001
#define UNLOCK_CSR_ACCESS 0x00000000

/* Enum for DRAM Type */
enum dram_type {
	DDR4,
	DDR3,
	LPDDR4,
	LPDDR3,
	DDR5
};

struct regconf {
	u32 addr;
	u32 data;
};

struct dqconf {
	u32 addr;
	u8 data;
};

struct ddrss_config {
	uint8_t memory_type;
	struct regconf *ddrc_cfg;
	size_t ddrc_cfg_size;
	struct dqconf *dq_swap_cfg;
	size_t dq_swap_cfg_size;
	struct regconf *phy_cfg;
	size_t phy_cfg_size;
	u16 *imem_1d;
	size_t imem_1d_size;
	u16 *dmem_1d;
	size_t dmem_1d_size;
	u16 *imem_2d;
	size_t imem_2d_size;
	u16 *dmem_2d;
	size_t dmem_2d_size;
	struct regconf *pie_cfg;
	size_t pie_cfg_size;
};

extern struct regconf ddrc_cfg[];
extern size_t ddrc_cfg_size;
extern struct dqconf dq_swap_cfg[];
extern size_t dq_swap_cfg_size;
extern struct regconf phy_cfg[];
extern size_t phy_cfg_size;
extern u16 imem_1d[];
extern size_t imem_1d_size;
extern u16 dmem_1d[];
extern size_t dmem_1d_size;
extern u16 imem_2d[];
extern size_t imem_2d_size;
extern u16 dmem_2d[];
extern size_t dmem_2d_size;
extern struct regconf pie_cfg[];
extern size_t pie_cfg_size;
extern struct ddrss_config configs[];
extern size_t ddrss_config_size;

u32 ddr_init(void);
void init_image_sizes(void);

#endif /* DDR_INIT_H */
