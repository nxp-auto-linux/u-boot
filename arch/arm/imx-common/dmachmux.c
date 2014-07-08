/*
 *
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 *
 * Author: Gilles Talis <gilles.talis@freescale.com>
 *
 * Based on iomux-v3.c
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
#include <common.h>
#include <asm/io.h>
#include <asm/arch/imx-regs.h>
#include <asm/imx-common/dmachmux.h>

static void *dmamux_base [] = {
		(void *) DMA_MUX0_BASE_ADDR,
		(void *) DMA_MUX1_BASE_ADDR,
		(void *) DMA_MUX2_BASE_ADDR,
		(void *) DMA_MUX3_BASE_ADDR,
	};

/*
 * configures a single channel in one of the DMA channel muxes
 */
int imx_dmamux_setup_channel(dmamux_cfg_t channel)
{
	void __iomem *base;

	u8 dmamux = (u8) ((channel & DMAMUX_MASK) >> DMAMUX_SHIFT);
	u8 dmamux_ch = (u8)((channel & DMAMUX_CHAN_MASK) >> DMAMUX_CHAN_SHIFT);
	u8 dmach_conf = (u8)(channel >> 16);

	if (dmamux >= DMAMUX_MAX) {
		return -1;
	}

	if (dmamux_ch > DMAMUX_CHANNEL_NUMBER) {
		return -1;
	}

	base = dmamux_base[dmamux];

	__raw_writeb(dmach_conf, base + dmamux_ch);

	return 0;
}


/*
 * configures multiple channels in any of the DMA muxes
 */
int imx_dmamux_setup_multiple_channels(dmamux_cfg_t const *channel_list,
				     unsigned count)
{
	dmamux_cfg_t const *p = channel_list;
	int i;
	int ret;

	for (i = 0; i < count; i++) {
		ret = imx_dmamux_setup_channel(*p);
		if (ret)
			return ret;
		p++;
	}
	return 0;
}

