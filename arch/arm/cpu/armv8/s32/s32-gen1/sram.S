/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2020-2021 NXP
 *
 * Extracted from armv8/start.S
 */

/* Allow inclusion assembly macros */
#define __INCLUDE_ASSEMBLY_MACROS__

#include <config.h>
#include <linux/linkage.h>
#include "s32g_sramc.h"

/*
 * Translate a bus address in SRAM controller initialization address based on:
 * On S32G2 and S32R45
 * mem_addr[16:0] = {bus_addr[23:9], bus_addr[5:4]}
 *
 * addr := ((addr >> 9) << 2) | ((addr >> 4) & 0x3);
 *
 * On S32G3
 * mem_addr[16:0] = { (bus_addr[24:20] modulo 5), bus_addr[19:8], bus_addr[5:4]}
 *
 * addr := ((addr & 0x30) >> 4) |
 *         (((addr & 0xFFF00) >> 8) << 2) |
 *         (((addr & 0x1F00000) >> 20) % 5) << 14
 *
 * x0: SRAM bus address
 * ret: x0: Initialization address
 *
 * Clobber list: x0,x4,x5,x6
 */
ENTRY(calc_sramc_addr)
#if defined(CONFIG_NXP_S32G2XX) || defined(CONFIG_NXP_S32R45)
	lsr	x4, x0, #9
	lsl	x4, x4, #2
	lsr	x5, x0, #4
	and	x5, x5, #3
	orr	x0, x4, x5
#else /* CONFIG_NXP_S32G3XX */
	ubfx	x5, x0, #20, #5
	mov	w4, #0xcccd
	movk	w4, #0xcccc, lsl #16
	lsr	w6, w0, #6
	and	w6, w6, #0x3ffc
	ubfx	x0, x0, #4, #2
	umull	x4, w5, w4
	orr	w0, w0, w6
	lsr	x4, x4, #32
	lsr	w4, w4, #2
	add	w4, w4, w4, lsl #2
	sub	w4, w5, w4
	orr	w0, w0, w4, lsl #14
#endif
	ret
ENDPROC(calc_sramc_addr)

/**
 * Check if two intervals are in overlap
 *
 * x0: start index of the first interval
 * x1: end index of the first interval
 * x2: start index of the second interval
 * x3: end index of the second interval
 *
 * x0: return 0 if the intervals do not overlap and 1 otherwise
 * Clobber list: x0,x1,x2,x3,x4,x5,x6
 */
ENTRY(in_overlap)
	/* x4 = max */
	cmp	x1, x3
	b.gt	1f
	mov	x4, x3
	b	2f
1:
	mov	x4, x1
2:
	/* x5 = min */
	cmp	x0, x2
	b.lt	3f
	mov	x5, x2
	b	4f
3:
	mov	x5, x0
4:
	/* x5 = max - min */
	sub	x5, x4, x5
	/* x4 = first interval size */
	sub	x4, x1, x0
	/* x6 = second interval size */
	sub	x6, x3, x2
	/* x4 = len1 + len2 */
	add	x4, x4, x6
	cmp	x4, x5
	bgt	overlap
	mov	x0, #0
	ret
overlap:
	mov	x0, #1
	ret
ENDPROC(in_overlap)

/**
 * Clear SRAM range using SRAM controller
 *
 * x0: SRAM controller address
 * x1: Initialization Start Address
 * x2: Initialization End Address
 *
 * Clobber list: x0,x1,x2,x3
 */
ENTRY(clear_sramc_range)
	/* Disable the module */
	mov	x3, #0
	str	w3, [x0, #SRAMC_PRAMCR_OFFSET]

	/* Address range */
	str	w1, [x0, #SRAMC_PRAMIAS_OFFSET]
	str	w2, [x0, #SRAMC_PRAMIAE_OFFSET]

	/* Request initialization */
	mov	x3, #SRAMC_PRAMCR_INITREQ
	str	w3, [x0, #SRAMC_PRAMCR_OFFSET]

wait_init:
	ldr	w3, [x0, #SRAMC_PRAMSR_OFFSET]
	and	w3, w3, #SRAMC_PRAMSR_IDONE
	cbz	w3, wait_init

	/* Clear status */
	mov	w3, #SRAMC_PRAMSR_IDONE
	str	w3, [x0, #SRAMC_PRAMSR_OFFSET]

	ret
ENDPROC(clear_sramc_range)

/**
 * Clear SRAM range using (current) A53 core
 *
 * x0: start address of memory area to clear
 * x1: end address of memory area to clear
 *
 * Clobber list: x0,x1,x9,x10
 */
ENTRY(a53_sram_clr)
	mov	x10, #0x0
	cmp	x1, x0
	bgt	pos_step
	mov	x9, #-8
	b	clr_loop
pos_step:
	mov	x9, #8
clr_loop:
	cmp	x1, x0
	beq	finish
	str	x10, [x0]
	add	x0, x0, x9
	b	clr_loop
finish:
	ret
ENDPROC(a53_sram_clr)

/**
 * Clear unaligned (to SRAMC granularity) SRAM range using current core
 *
 * x0: start address of memory area to clear
 * x1: end address of memory area to clear
 *
 * Clobber list: x0,x1,x9,x10,x11
 */
ENTRY(clear_unaligned_ends)
	/* Save x30 */
	mov	x12, x30
	/*
	 * Obtain SRAM addresses
	 */
	mov	x9, #S32G_SRAM_BASE
	add	x11, x1, x9
	add	x9, x0, x9

	and	x1, x9, #SRAM_INV_BLOCK_MASK
	cmp	x1, x9
	beq	clear_end
	add	x1, x1, SRAM_BLOCK
	mov	x0, x9
	bl	a53_sram_clr

clear_end:
	and	x0, x11, #SRAM_INV_BLOCK_MASK
	cmp	x0, x11
	beq	clr_exit
	mov	x1, x11
	bl	a53_sram_clr
clr_exit:
	mov	x30, x12
	ret
ENDPROC(clear_unaligned_ends)

/**
 * Clear SRAM region using SRAMC & current core fo unaligned ends
 *
 * x0: start address of memory area to clear
 * x1: end address of memory area to clear
 * x0: return 0 on error or size of memory cleared on success
 *
 * Clobber list: x0,x1,x2,x3,x4,x5,x6,x7,x9,x10,x11,x12,x13,x14,x15,x16
 */
ENTRY(_s32g_sram_clr)
	mov	x16, x1
	sub	x16, x16, x0
	mov	x13, x0
	mov	x14, x1
	bl	clear_unaligned_ends
	mov	x0, x13
	mov	x1, x14

	/* Align to SRAM blocks */
	and	x9, x0, #SRAM_INV_BLOCK_MASK
	cmp	x0, x9
	beq	align_high_addr
	add	x0, x9, SRAM_BLOCK

align_high_addr:
	and	x1, x1, #SRAM_INV_BLOCK_MASK
	cbz	x1, sram_exit

	sub	x1, x1, #1
	/* Low address */
	bl	calc_sramc_addr
	mov	x7, x0
	mov	x0, x1

	/* High address */
	bl	calc_sramc_addr

	/**
	 * x7 - Low initialization address
	 * x10 - High initialization address
	 */
	mov	x10, x0
	mov	x9, #SRAMC1_MAX_ADDR
	cmp	x10, x9
	bgt	error

	mov	x0, #SRAMC0_MIN_ADDR
	mov	x1, #SRAMC0_MAX_ADDR
	mov	x2, x7
	mov	x3, x10
	bl	in_overlap
	cbz	x0, check_sramc1

init_sramc0:
	mov	x0, #SRAMC0_BASE_ADDR_L
	movk	x0, #SRAMC0_BASE_ADDR_H, lsl #16
	mov	x1, x7
	mov	x2, x10
	mov	x9, #SRAMC0_MAX_ADDR
	cmp	x2, x9
	b.gt	1f
	b	2f
1:
	mov	x2, #SRAMC0_MAX_ADDR
2:
	bl	clear_sramc_range

check_sramc1:
	mov	x0, #SRAMC1_MIN_ADDR
	mov	x1, #SRAMC1_MAX_ADDR
	mov	x2, x7
	mov	x3, x10
	bl	in_overlap
#if defined(CONFIG_NXP_S32G2XX) || defined(CONFIG_NXP_S32R45)
	cbz	x0, sram_exit
#else /* CONFIG_NXP_S32G3XX */
	cbz	x0, check_sramc2
#endif

init_sramc1:
	mov	x0, #SRAMC1_BASE_ADDR
	movk	x0, #SRAMC1_BASE_ADDR_H, lsl #16
	cmp	x7, #SRAMC1_MIN_ADDR
	b.lt	3f
	sub	x1, x7, #SRAMC1_MIN_ADDR
	b	4f
3:
	mov	x1, #0
4:
	sub	x2, x10, #SRAMC1_MIN_ADDR
	bl	clear_sramc_range

#ifdef CONFIG_NXP_S32G3XX
check_sramc2:
	mov	x0, #SRAMC2_MIN_ADDR
	mov	x1, #SRAMC2_MAX_ADDR_L
	movk	x1, #SRAMC2_MAX_ADDR_H, lsl #16
	mov	x2, x7
	mov	x3, x10
	bl	in_overlap
	cbz	x0, check_sramc3

init_sramc2:
	mov	x0, #SRAMC2_BASE_ADDR_L
	movk	x0, #SRAMC2_BASE_ADDR_H, lsl #16
	cmp	x7, #SRAMC2_MIN_ADDR
	b.lt	5f
	sub	x1, x7, #SRAMC2_MIN_ADDR
	b	6f
5:
	mov	x1, #0
6:
	sub	x2, x10, #SRAMC2_MIN_ADDR
	bl	clear_sramc_range

check_sramc3:
	mov	x0, #SRAMC3_MIN_ADDR
	mov	x1, #SRAMC3_MAX_ADDR_L
	movk	x1, #SRAMC3_MAX_ADDR_H, lsl #16
	mov	x2, x7
	mov	x3, x10
	bl	in_overlap
	cbz	x0, sram_exit

init_sramc3:
	mov	x0, #SRAMC2_BASE_ADDR_L
	movk	x0, #SRAMC2_BASE_ADDR_H, lsl #16
	cmp	x7, #SRAMC2_MIN_ADDR
	b.lt	7f
	sub	x1, x7, #SRAMC2_MIN_ADDR
	b	8f
7:
	mov	x1, #0
8:
	sub	x2, x10, #SRAMC2_MIN_ADDR
	bl	clear_sramc_range
#endif

	mov	x0, #0

sram_exit:
	/* Restore x30 */
	mov	x30, x15
	mov	x0, x16
	ret

error:
	/* Restore x30 */
	mov	x30, x15
	mov	x0, #0
	ret
ENDPROC(_s32g_sram_clr)

/**
 * Clear SRAM region using SRAMC
 *
 * x0: start address of memory area to clear
 * x1: size of the memory area to be cleared
 * x0: return the size of the cleared memory
 *
 * Clobber list: x0,x1,x2,x3,x4,x5,x6,x7,x9,x10,x11,x12,x13,x14,x15,x16
 */
ENTRY(sram_clr)
	/* Save x30 */
	mov	x15, x30

	/* To bus addresses */
	mov	x2, #S32G_SRAM_BASE
	sub	x0, x0, x2
	add	x1, x1, x0

	b	_s32g_sram_clr
ENDPROC(sram_clr)
