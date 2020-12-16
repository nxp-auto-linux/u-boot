/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2020 NXP
 */

/* Allow inclusion assembly macros */
#define __INCLUDE_ASSEMBLY_MACROS__

#include <config.h>
#include <linux/linkage.h>

/*************************************************************************
 *
 * void sram_clr(start, size);
 *
 * Clear a memory region of size 'size' starting with address 'start'
 * (using DMA)
 *
 *************************************************************************/

ENTRY(sram_clr)
	/*
	 * x0: start address of memory to clear
	 * x1: size of memory area to clear
	 * x0: return 0 on error or size of memory cleared on success
	 */

	/* DMA_TCDn_SADDR */
	ldr x9, =DMA_TCD_N_SADDR(DMA_CHANNEL_1)
	ldr x10, =initvar
	/* Point DMA source address to initialization data block */
	str w10, [x9]

	/* DMA_TCDn_SOFF */
	/* DMA_TCDn_ATTR */
	ldr x9, =DMA_TCD_N_SOFF(DMA_CHANNEL_1)
	/* no offset reread same source address each interation */
	/* 32 byte burst */
	ldr x10, =0x05050000
	str w10, [x9]

	/* DMA_TCDn_NBYTES_MLNO */
	ldr x9, =DMA_TCD_N_NBYTES_MLNO(DMA_CHANNEL_1)
	/* load the size */
	mov x10, x1
	str x10, [x9]

	/* DMA_TCDn_DADDR */
	ldr x9, =DMA_TCD_N_DADDR(DMA_CHANNEL_1)
	/* load the start address */
	mov x10, x0
	str x10, [x9]

	/* DMA_TCDn_DOFF          */
	/* DMA_TCDn_CITER_ELINKNO */
	ldr x9, =DMA_TCD_N_DOFF(DMA_CHANNEL_1)
	/* Increment destination address by 0x20 each interation */
	/* ELINKNO CITER = 0X0001 single service request */
	ldr w10, =0x00010020
	str w10, [x9]

	/* DMA_TCDn_BITER_ELINKNO */
	/* ELINKNO BITER = 0X0001 single service request */
	ldr x9, =DMA_TCD_N_BITER_ELINKNO(DMA_CHANNEL_1)
	ldr w10, =0x1
	strb w10, [x9]

	/* Start transfer */
	/* DMA_TCDn_CSR */
	ldr x9, =DMA_TCD_N_CSR(DMA_CHANNEL_1)
	ldr w10, =0x1
	strb w10, [x9]

	/* loop until write is done */
ctrl_status:
	/* DMA_ES */
	ldr x9, =DMA_ES
	ldr w10, [x9]
	ldr x11, =0x80000000
	/* Check error bit */
	and w10, w10, #0x80000000
	sub w10, w10, w11
	cbz w10, ret_error
	/* Check transfer done */
	check_done_bit
	cbnz w10, ctrl_status
	/* clear DONE bit */
	clear_done_bit
	/* return bytes written */
	mov x0, x1

	ret

ret_error:
	/* error return zero bytes written */
	ldr x0, =0x0
	clear_channel_err
	ret

ENDPROC(sram_clr)

	.data
initvar:
	.quad 0x0
	.quad 0x0
	.quad 0x0
	.quad 0x0

