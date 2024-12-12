/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2023-2024 NXP
 */

#ifndef __BOOT0_H
#define __BOOT0_H

#if CONFIG_POSITION_INDEPENDENT
	adrp	x0, __bss_start     /* x0 <- Runtime &__bss_start */
	add	x0, x0, #:lo12:__bss_start
	adrp	x1, __bss_end     /* x0 <- Runtime &__bss_end */
	add	x1, x1, #:lo12:__bss_end
#else
	ldr x0, =__bss_start
	ldr x1, =__bss_end
#endif
clear_loop:
	str xzr, [x0], #8
	cmp x0, x1
	b.lo    clear_loop

	b reset

#endif /* __BOOT0_H */
