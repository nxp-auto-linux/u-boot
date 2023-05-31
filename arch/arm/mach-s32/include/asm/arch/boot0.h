/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2023 NXP
 */

#ifndef __BOOT0_H
#define __BOOT0_H

	ldr x0, =__bss_start
	ldr x1, =__bss_end
clear_loop:
	str xzr, [x0], #8
	cmp x0, x1
	b.lo    clear_loop

	b reset

#endif /* __BOOT0_H */
