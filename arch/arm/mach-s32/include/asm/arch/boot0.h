/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2023-2025 NXP
 */

#ifndef __BOOT0_H
#define __BOOT0_H

#if CONFIG_POSITION_INDEPENDENT
	adrp	x9, __bss_start    /* x9 <- Runtime &__bss_start */
	add	x9, x9, #:lo12:__bss_start
	adrp	x10, __bss_end     /* x10 <- Runtime &__bss_end */
	add	x10, x10, #:lo12:__bss_end
#else
	ldr x9, =__bss_start
	ldr x10, =__bss_end
#endif
clear_loop:
	str xzr, [x9], #8
	cmp x9, x10
	b.lo    clear_loop

	b reset

#endif /* __BOOT0_H */
