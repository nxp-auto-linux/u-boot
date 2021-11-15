// SPDX-License-Identifier: GPL-2.0+
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

#include "ddr_init.h"

struct dqconf dq_swap_cfg[] = {
	{0x40394830, 0x00},
	{0x40394834, 0x01},
	{0x40394838, 0x02},
	{0x4039483c, 0x03},
	{0x40394840, 0x04},
	{0x40394844, 0x05},
	{0x40394848, 0x06},
	{0x4039484c, 0x07},
	{0x40396830, 0x00},
	{0x40396834, 0x01},
	{0x40396838, 0x02},
	{0x4039683c, 0x03},
	{0x40396840, 0x04},
	{0x40396844, 0x05},
	{0x40396848, 0x06},
	{0x4039684c, 0x07},
	{0x40398830, 0x00},
	{0x40398834, 0x01},
	{0x40398838, 0x02},
	{0x4039883c, 0x03},
	{0x40398840, 0x04},
	{0x40398844, 0x05},
	{0x40398848, 0x06},
	{0x4039884c, 0x07},
	{0x4039a830, 0x00},
	{0x4039a834, 0x01},
	{0x4039a838, 0x02},
	{0x4039a83c, 0x03},
	{0x4039a840, 0x04},
	{0x4039a844, 0x05},
	{0x4039a848, 0x06},
	{0x4039a84c, 0x07},
};

size_t dq_swap_cfg_size = ARRAY_SIZE(dq_swap_cfg);
