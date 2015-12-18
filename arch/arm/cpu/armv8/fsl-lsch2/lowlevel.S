/*
 * (C) Copyright 2014 Freescale Semiconductor
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * Extracted from armv8/start.S
 */

#include <config.h>
#include <linux/linkage.h>
#include <asm/gic.h>
#include <asm/macro.h>
#include "mp.h"

ENTRY(lowlevel_init)
	mov	x29, lr			/* Save LR */

	/* Set the SMMU page size in the sACR register */
	ldr	x1, =SMMU_BASE
	ldr	w0, [x1, #0x10]
	orr	w0, w0, #1 << 16  /* set sACR.pagesize to indicate 64K page */
	str	w0, [x1, #0x10]

	/* Initialize GIC Secure Bank Status */
#if defined(CONFIG_GICV2) || defined(CONFIG_GICV3)
	branch_if_slave x0, 1f
	ldr	x0, =GICD_BASE
	bl	gic_init_secure
1:
#ifdef CONFIG_GICV3
	ldr	x0, =GICR_BASE
	bl	gic_init_secure_percpu
#elif defined(CONFIG_GICV2)
	ldr	x0, =GICD_BASE
	ldr	x1, =GICC_BASE
	bl	gic_init_secure_percpu
#endif
#endif

	branch_if_master x0, x1, 1f

	ldr	x0, =(SECONDARY_CPU_BOOT_PAGE)
	ldr	x1, =secondary_boot_func
	ldr	x2, =secondary_boot_page
	sub	x1, x1, x2
	add	x0, x0, x1
	blr	x0

1:
	/* Set Non Secure access for all devices protected via TZPC */
	ldr	x1, =TZPCDECPROT_0_SET_BASE /* Decode Protection-0 Set Reg */
	mov	w0, #0xFF		/* Set decode region to NS, Bits[7:0] */
	str	w0, [x1]

	ldr	x1, =TZPCDECPROT_1_SET_BASE /* Decode Protection-1 Set Reg */
	mov	w0, #0xFF		/* Set decode region to NS, Bits[7:0] */
	str	w0, [x1]

	ldr	x1, =TZPCDECPROT_2_SET_BASE /* Decode Protection-2 Set Reg */
	mov	w0, #0xFF		/* Set decode region to NS, Bits[7:0] */
	str	w0, [x1]

	/* Entire SRAM as NS */
	ldr	x1, =TZPCR0SIZE_BASE	/* Secure RAM region size Reg */
	mov	w0, #0x00000000		/* 0x00000000 = no secure region */
	str	w0, [x1]

	/* Set TZASC so that:
	 * a. We use only Region0 whose global secure write/read is EN
	 * b. We use only Region0 whose NSAID write/read is EN
	 *
	 * NOTE: As per the CCSR map doc, TZASC 3 and TZASC 4 are just
	 * 	 placeholders.
	 */
	ldr	x1, =TZASC_GATE_KEEPER(0)
	ldr	x0, [x1]		/* Filter 0 Gate Keeper Register */
	orr	x0, x0, #1 << 0		/* Set open_request for Filter 0 */
	str	x0, [x1]

	ldr	x1, =TZASC_GATE_KEEPER(1)
	ldr	x0, [x1]		/* Filter 0 Gate Keeper Register */
	orr	x0, x0, #1 << 0		/* Set open_request for Filter 0 */
	str	x0, [x1]

	ldr	x1, =TZASC_REGION_ATTRIBUTES_0(0)
	ldr	x0, [x1]		/* Region-0 Attributes Register */
	orr	x0, x0, #1 << 31	/* Set Sec global write en, Bit[31] */
	orr	x0, x0, #1 << 30	/* Set Sec global read en, Bit[30] */
	str	x0, [x1]

	ldr	x1, =TZASC_REGION_ATTRIBUTES_0(1)
	ldr	x0, [x1]		/* Region-1 Attributes Register */
	orr	x0, x0, #1 << 31	/* Set Sec global write en, Bit[31] */
	orr	x0, x0, #1 << 30	/* Set Sec global read en, Bit[30] */
	str	x0, [x1]

	ldr	x1, =TZASC_REGION_ID_ACCESS_0(0)
	ldr	w0, [x1]		/* Region-0 Access Register */
	mov	w0, #0xFFFFFFFF		/* Set nsaid_wr_en and nsaid_rd_en */
	str	w0, [x1]

	ldr	x1, =TZASC_REGION_ID_ACCESS_0(1)
	ldr	w0, [x1]		/* Region-1 Attributes Register */
	mov	w0, #0xFFFFFFFF		/* Set nsaid_wr_en and nsaid_rd_en */
	str	w0, [x1]
2:
	mov	lr, x29			/* Restore LR */
	ret
ENDPROC(lowlevel_init)

	/* Keep literals not used by the secondary boot page outside it */
	.ltorg

	.align 4
	.global secondary_boot_page
secondary_boot_page:
	.global __spin_table
__spin_table:
	.space CONFIG_MAX_CPUS*ENTRY_SIZE

	.align 4
	/* Secondary Boot Page starts here */
ENTRY(secondary_boot_func)
	/*
	 * PIR calculation from MPIDR_EL1
	 * MPIDR[1:0] = AFF0_CPUID <- Core ID (0,1)
	 * MPIDR[7:2] = AFF0_RES
	 * MPIDR[15:8] = AFF1_CLUSTERID <- Cluster ID (0,1,2,3)
	 * MPIDR[23:16] = AFF2_CLUSTERID
	 * MPIDR[24] = MT
	 * MPIDR[29:25] =RES
	 * MPIDR[30] = U
	 * MPIDR[31] = ME
	 * MPIDR[39:32] = AFF3
	 * We only use AFF0_CPUID and AFF1_CLUSTERID for now
	 * until AFF2_CLUSTERID and AFF3 have non-zero values.
	 */
	mrs	x0, mpidr_el1
	ubfm	x1, x0, #8, #15
	ubfm	x2, x0, #0, #1
	orr	x10, x2, x1, lsl #2	/* x10 has PIR */
	ubfm	x9, x0, #0, #15		/* w9 has 16-bit original PIR */
	lsl	x1, x10, #6	/* spin table is padded to 64 byte each core */
	ldr	x0, =(SECONDARY_CPU_BOOT_PAGE)
	ldr	x3, =__spin_table
	ldr	x4, =secondary_boot_page
	sub	x3, x3, x4
	add	x0, x0, x3
	add	x11, x1, x0

	str	x9, [x11, #16]	/* ENTRY_PIR */
	mov	x4, #1
	str	x4, [x11]	/* ENTRY_ADDR */
	dsb	sy
	isb
#if defined(CONFIG_GICV3)
	gic_wait_for_interrupt_m x0
#endif

	bl secondary_switch_to_el2
#ifdef CONFIG_ARMV8_SWITCH_TO_EL1
	secondary_switch_to_el1
#endif

slave_cpu:
	wfe
#ifdef CONFIG_FSL_SMP_RELEASE_ALL
	ldr	x1, =CPU_RELEASE_ADDR
	ldr	x0, [x1]
#else
	ldr	x0, [x11]
	tbnz	x0, #0, slave_cpu
#endif
	cbz	x0, slave_cpu
	br	x0			/* branch to the given address */
ENDPROC(secondary_boot_func)

ENTRY(secondary_switch_to_el2)
	switch_el x0, 1f, 0f, 0f
0:	ret
1:	armv8_switch_to_el2_m x0
ENDPROC(secondary_switch_to_el2)

ENTRY(secondary_switch_to_el1)
	switch_el x0, 0f, 1f, 0f
0:	ret
1:	armv8_switch_to_el1_m x0, x1
ENDPROC(secondary_switch_to_el1)

	/* Ensure that the literals used by the secondary boot page are
	 * assembled within it
	 */
	.ltorg

	.align 4
	.globl __secondary_boot_page_size
	.type __secondary_boot_page_size, %object
	/* Secondary Boot Page ends here */
__secondary_boot_page_size:
	.quad .-secondary_boot_page
