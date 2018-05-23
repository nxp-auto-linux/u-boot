/*
 * Copyright 2018 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __DMA_MACROS_H__
#define __DMA_MACROS_H__

/* eDMA controller */
#define DMA_CHANNEL_1		1
#define DMA_TCD_BASE_ADDRESS	(EDMA0_CONTROL_BASE_ADDR + 0x4000)
#define DMA_CHANNEL(channel)	(DMA_TCD_BASE_ADDRESS + 0x1000 * (channel))
#define DMA_CR				(EDMA0_CONTROL_BASE_ADDR)
#define DMA_ES				(EDMA0_CONTROL_BASE_ADDR + 0x4)
#define DMA_CH_N_CSR(channel)		(DMA_CHANNEL(channel))
#define DMA_CH_N_ES(channel)		(DMA_CHANNEL(channel) + 0x04)
#define DMA_TCD_N_SADDR(channel)	(DMA_CHANNEL(channel) + 0x20)
#define DMA_TCD_N_SOFF(channel)		(DMA_CHANNEL(channel) + 0x24)
#define DMA_TCD_N_NBYTES_MLNO(channel)	(DMA_CHANNEL(channel) + 0x28)
#define DMA_TCD_N_DADDR(channel)	(DMA_CHANNEL(channel) + 0x30)
#define DMA_TCD_N_DOFF(channel)		(DMA_CHANNEL(channel) + 0x34)
#define DMA_TCD_N_CITER_ELINKNO(channel)(DMA_CHANNEL(channel) + 0x36)
#define DMA_TCD_N_CSR(channel)		(DMA_CHANNEL(channel) + 0x3C)
#define DMA_TCD_N_BITER_ELINKNO(channel)(DMA_CHANNEL(channel) + 0x3E)

#ifdef __INCLUDE_ASSEMBLY_MACROS__
.macro check_done_bit
	ldr x9, =DMA_CH_N_CSR(DMA_CHANNEL_1)
	ldr w10, [x9]
	ldr x11, =0x40000000
	/* Check transfer done */
	and w10, w10, #0x40000000
	sub w10, w10, w11
.endm

.macro clear_done_bit
	ldr x9, =DMA_CH_N_CSR(DMA_CHANNEL_1)
	ldr w10, =0x40000000
	str w10, [x9]
.endm

.macro clear_channel_err
	/* CHn_ES */
	ldr x9, =DMA_CH_N_ES(DMA_CHANNEL_1)
	/* Clear error bit for channel */
	ldr x10, =0x80000000
	str w10, [x9]
.endm

#endif
#endif /* __DMA_MACROS_H__ */
