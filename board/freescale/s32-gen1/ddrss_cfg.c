// SPDX-License-Identifier: GPL-2.0+
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

#include "ddr_init.h"

struct ddrss_config configs[] = {
	{
		.memory_type = LPDDR4,
		.ddrc_cfg = &ddrc_cfg[0],
		.ddrc_cfg_size = 0,
		.dq_swap_cfg = &dq_swap_cfg[0],
		.dq_swap_cfg_size = 0,
		.phy_cfg = &phy_cfg[0],
		.phy_cfg_size = 0,
		.imem_1d = &imem_1d[0],
		.imem_1d_size = 0,
		.dmem_1d = &dmem_1d[0],
		.dmem_1d_size = 0,
		.imem_2d = &imem_2d[0],
		.imem_2d_size = 0,
		.dmem_2d = &dmem_2d[0],
		.dmem_2d_size = 0,
		.pie_cfg = &pie_cfg[0],
		.pie_cfg_size = 0,
	}
};

void init_image_sizes(void)
{
	size_t i;

	for (i = 0; i < ddrss_config_size; i++) {
		configs[i].ddrc_cfg_size = ddrc_cfg_size;
		configs[i].dq_swap_cfg_size = dq_swap_cfg_size;
		configs[i].phy_cfg_size = phy_cfg_size;
		configs[i].imem_1d_size = imem_1d_size;
		configs[i].dmem_1d_size = dmem_1d_size;
		configs[i].imem_2d_size = imem_2d_size;
		configs[i].dmem_2d_size = dmem_2d_size;
		configs[i].pie_cfg_size = pie_cfg_size;
	}
}

size_t ddrss_config_size = sizeof(configs) / sizeof(struct ddrss_config);
