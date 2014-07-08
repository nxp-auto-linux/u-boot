/*
 *
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 *
 * Author: Gilles Talis <gilles.talis@freescale.com>
 *
 * Based on iomux-v3.h
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

#ifndef __MACH_DMACHMUX_H__
#define __MACH_DMACHMUX_H__

#include <common.h>

/*
 *	Build DMAMUX structure
 *
 * This DMAMUX scheme allows configuring the DMA MUX components
 * by allocating a specific DMA request source to a specific DMA MUX channel
 *
 * - Each DMA MUX handles a number of channels
 * - Each channel has a configuration register (DMAMUX_CHCFG)
 *
 * DMAMUX/channel Bit field definitions
 *
 * DMAMUX_SHIFT:  0..3 (4 bits) - This is the DMAMUX (0-3)
 * DMAMUX_CHAN_SHIFT:	4..7 (4 bits) - This is DMAMUX channel (0-15)
 * DMACHANNEL_SRC_SHIFT:	16..21 (6 bits) - This is DMA Request source number (0-63)
 * DMACH_ENABLE_SHIFT:	   22  (1 bit) - This enables/disables DMAMUX channel
 * DMACH_TRIG_SHIFT: 23 (1 bit) - This enables/disables periodic trigger capability
*/

#define DMAMUX_SHIFT			0
#define DMAMUX_MASK				(0x0f << DMAMUX_SHIFT)
#define DMAMUX_CHAN_SHIFT		4
#define DMAMUX_CHAN_MASK		(0x0f << DMAMUX_CHAN_SHIFT)
#define DMACH_SRC_SHIFT			16
#define DMACH_SRC_MASK			(0x3f << DMACH_SRC_SHIFT)
#define DMACH_ENABLE_SHIFT		22
#define DMACH_ENABLE_MASK		(0x1 << DMACH_ENABLE_SHIFT)
#define DMACH_TRIG_SHIFT		23
#define DMACH_TRIG_MASK			(0x1 << DMACH_TRIG_SHIFT)

#define DMAMUX_CHANNEL_ENABLE		1
#define DMAMUX_CHANNEL_DISABLE		0
#define DMAMUX_CHANNEL_TRIG_ON		1
#define DMAMUX_CHANNEL_TRIG_OFF		0

#define DMAMUX_CHANNEL_NUMBER	16

#define DMAMUX_CHANNEL(dma_mux, channel, src, trigger, enable)	\
	((dma_mux	<< DMAMUX_SHIFT)	|	\
	(channel		<< DMAMUX_CHAN_SHIFT)	|	\
	(src  			<< DMACH_SRC_SHIFT)		|	\
	(trigger		<< DMACH_TRIG_SHIFT)		|	\
	(enable 		<< DMACH_ENABLE_SHIFT))

enum {
	DMAMUX_0,
	DMAMUX_1,
	DMAMUX_2,
	DMAMUX_3,
	DMAMUX_MAX,
};

typedef u32 dmamux_cfg_t;

int imx_dmamux_setup_channel(dmamux_cfg_t channel);
int imx_dmamux_setup_multiple_channels(dmamux_cfg_t const *channel_list,
				     unsigned count);

#endif



