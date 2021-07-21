/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * (C) Copyright 2014 Freescale Semiconductor
 * Copyright 2017-2021 NXP
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

#define SWT_SR(SWTn_BASE_ADDR) (SWTn_BASE_ADDR + 0x10)


ENTRY(lowlevel_init)

	mov	x29, lr			/* Save LR */

#if defined(CONFIG_S32_GEN1) && !defined(CONFIG_S32_ATF_BOOT_FLOW)
reset_registers_for_lockstep:
	/*
	 * Timers reset must be done when lockstep is enabled to avoid RCCU
	 * mismatch errors. Reset should be executed as early as possible
	 * before any read access to these counters. Resetting them for all boot
	 * flows assures consistent values
	 * This must be done in EL3 and executed for all cores.
	 */

	mov x0, #0x0
	msr cntkctl_el1, x0

	msr cntp_tval_el0, x0
	msr cntp_ctl_el0, x0
	msr cntp_cval_el0, x0

	msr cntv_tval_el0, x0
	msr cntv_cval_el0, x0
	msr cntv_ctl_el0, x0

	msr cntvoff_el2, x0
	msr cnthctl_el2, x0

	msr cnthp_tval_el2, x0
	msr cnthp_ctl_el2, x0
	msr cnthp_cval_el2, x0

	msr cntps_tval_el1, x0
	msr cntps_ctl_el1, x0
	msr cntps_cval_el1, x0

	/*
	 * Lockstep sync GPR registers: write x19-x28 callee-saved registers
	 * as defined in procedure call standard for the ARM 64-bit. These
	 * registers may be saved to stack without being initialized, setting
	 * them is needed to avoid lockstep errors.
	 */

	mov x19, #0
	mov x20, #0
	mov x21, #0
	mov x22, #0
	mov x23, #0
	mov x24, #0
	mov x25, #0
	mov x26, #0
	mov x27, #0
	mov x28, #0
#endif

#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
#endif

#if defined(CONFIG_S32V234)
watchdog_cortexm_disable:
	/* disable SWT4 watchdog*/
	ldr x0, =SWT_SR(SWT4_BASE_ADDR)
	ldr w1, =0xC520
	str w1, [x0]
	ldr w1, =0xD928
	str w1, [x0]
	ldr x0, =SWT4_BASE_ADDR
	ldr x1, [x0]
	and x1, x1, 0xFFFFFFFE
	str x1, [x0]
	ldr x1, [x0]
	orr x1, x1, 0x40
	str x1, [x0]
#endif

/* Skip SRAM initialization if running with ATF and U-Boot in DDR */
#if !(defined(CONFIG_S32_ATF_BOOT_FLOW) && defined(CONFIG_S32_SKIP_RELOC))
sram_init:
	/* Clear stack region */
	ldr x0,  =(CONFIG_SYS_INIT_SP_ADDR - CONFIG_SYS_INIT_SP_OFFSET)
	ldr x1,  =CONFIG_SYS_INIT_SP_OFFSET
	bl sram_clr

	/* Start address of the SRAM memory to init */
	ldr x0,  =__bss_start
#ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
	ldr x1, =__bss_end
#elif defined CONFIG_S32_GEN1
	ldr x1, =CONFIG_SYS_TEXT_BASE
	add x1, x1, #CONFIG_UBOOT_SRAM_FOOTPRINT
#else	/* S32V234 */
	ldr x1, = S32_SRAM_BASE
	add x1, x1, S32_SRAM_SIZE
#endif
	sub x1, x1, x0

	bl sram_clr
	/* turn on a53 slave cores from a53 master */
	/* deassert cores on reset */
#endif

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
	 * Slave should wait for master clearing spin table and
	 * the mmu page tables.
	 * This sync prevent salves observing incorrect
	 * value of spin table and jumping to wrong place.
	 */

	wfe
	tlbi	alle3
	dsb	sy
	isb
	isb

	ldr	x0, =s32_tlb_addr
	ldr	x0, [x0]
	msr	ttbr0_el3, x0
	msr	ttbr0_el2, x0

	ldr	x0, =s32_tcr
	ldr	x0, [x0]
	msr	tcr_el3, x0
	msr	tcr_el2, x0

	ldr	x0,=MEMORY_ATTRIBUTES
	msr	mair_el3, x0
	msr	mair_el2, x0

	ldr	x0, =__real_cntfrq
	ldr     x0, [x0]
	msr	cntfrq_el0, x0	/* set with real frequency */
	dsb	sy
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

