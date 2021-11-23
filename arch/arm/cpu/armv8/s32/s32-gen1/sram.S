/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2021 NXP
 */

#include <config.h>
#include <linux/linkage.h>

#define S32_STACK_GUARD	0xDEAD

#ifndef S32_CRASH_STACK_SIZE
#define S32_CRASH_STACK_SIZE	0x400
#endif

.section .data.s32_crash_reg_stash
	.align 3
	s32_crash_reg_stash: .quad 0, 0, 0, 0, 0

/**
 * Use a local buffer as stack for Linflex crash callbacks and SRAM
 * initialization
 */
.section .data.s32_plat_data_stack
	.balign 16
	s32_plat_data_stack: .skip S32_CRASH_STACK_SIZE

/**
 * Save intra-procedure-call scratch, frame pointer and
 * link registers (x16, x17, x29, x30)
 *
 * Clobbers: reg1, reg2
 */
.macro	s32_save_regs reg1=x9, reg2=x10
	mov	\reg1, sp
	adrp	\reg2, s32_crash_reg_stash
	add	\reg2, \reg2, :lo12:s32_crash_reg_stash
	stp	x16, x17, [\reg2]
	stp	x29, x30, [\reg2, #16]
	str	x9, [\reg2, #32]
.endm

/**
 * Restore intra-procedure-call scratch, frame pointer
 * and link registers (x16, x17, x29, x30)
 *
 * Clobbers: reg1, reg2
 */
.macro	s32_restore_regs reg1=x9, reg2=x10
	adrp	\reg2, s32_crash_reg_stash
	add	\reg2, \reg2, :lo12:s32_crash_reg_stash
	ldp	x16, x17, [\reg2]
	ldp	x29, x30, [\reg2, #16]
	ldr	x9, [\reg2, #32]
	mov	sp, \reg1
.endm

.macro	s32_init_local_stack
	mov	w10, #S32_STACK_GUARD
	adrp	x9, s32_plat_data_stack
	add	x9, x9, :lo12:s32_plat_data_stack
	/* Color the end of the stack */
	str	w10, [x9]
	add	x9, x9, #S32_CRASH_STACK_SIZE
	mov	sp, x9
.endm

.macro s32_check_stack_guard
	mov	w10, #S32_STACK_GUARD
	adrp	x9, s32_plat_data_stack
	add	x9, x9, :lo12:s32_plat_data_stack
	ldrh	w9, [x9]
	cmp	w9, w10
	b.ne	panic
.endm

/**
 * Clear SRAM region using SRAMC
 *
 * x0: start address of memory area to clear
 * x1: size of the memory area to be cleared
 * x0: return the size of the cleared memory
 *
 * Clobber list: x0,x1,x9,x10
 */
ENTRY(sram_clr)
	add x1, x0, x1
	s32_save_regs
	s32_init_local_stack

	bl s32_sram_clear

	s32_check_stack_guard
	s32_restore_regs
	ret
ENDPROC(sram_clr)
