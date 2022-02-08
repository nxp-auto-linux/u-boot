/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * (C) Copyright 2014 Freescale Semiconductor
 * Copyright 2017-2022 NXP
 *
 * Extracted from armv8/start.S
 */

/* Allow inclusion assembly macros */
#define __INCLUDE_ASSEMBLY_MACROS__

#include <config.h>
#include <linux/linkage.h>
#include <asm/armv8/mmu.h>
#include <asm/macro.h>
#include <asm/gic.h>
#include "mp.h"

ENTRY(lowlevel_init)

	mov	x29, lr			/* Save LR */

#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
#endif

/* turn on a53 slave cores from a53 master */
/* deassert cores on reset */
start_slave_cores:

#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
	ldr	x0, =GICD_BASE
	bl	gic_init_secure

1:
#if defined(CONFIG_GICV3)
	ldr	x0, =GICR_BASE
	bl	gic_init_secure_percpu
#elif defined(CONFIG_GICV2)
	ldr	x0, =GICD_BASE
	ldr	x1, =GICC_BASE
	bl	gic_init_secure_percpu
#endif
#endif

#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	mrs	x0, S3_1_c15_c2_1
	orr	x0, x0, #(1 << 6)
	msr	S3_1_c15_c2_1, x0
	isb
#endif
	branch_if_master x0, x1, 2f

	/*
	 * All slaves will enter EL2 and optionally EL1.
	 */
	adr	x4, lowlevel_in_el2
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el2

lowlevel_in_el2:
#ifdef CONFIG_ARMV8_SWITCH_TO_EL1
	adr	x4, lowlevel_in_el1
	ldr	x5, =ES_TO_AARCH64
	bl	armv8_switch_to_el1

lowlevel_in_el1:
#endif

2:
	mov	lr, x29			/* Restore LR */
	ret

ENDPROC(lowlevel_init)

	/* Keep literals not used by the secondary boot page outside it */
	.ltorg

	/* 64 bit alignment for elements accessed as data */
	.align 4
	.global __real_cntfrq
__real_cntfrq:
	.quad COUNTER_FREQUENCY

