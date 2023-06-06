/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018,2023 NXP
 */

#define MIDR_REVISION_SHIFT	0x0
#define MIDR_REVISION_MASK	(0xF << MIDR_REVISION_SHIFT)

#define MIDR_PARTNUM_CORTEX_A35	0xD04
#define MIDR_PARTNUM_CORTEX_A53	0xD03
#define MIDR_PARTNUM_CORTEX_A72	0xD08
#define MIDR_PARTNUM_SHIFT	0x4
#define MIDR_PARTNUM_MASK	(0xFFF << 0x4)

#define MIDR_VARIANT_SHIFT	0x14
#define MIDR_VARIANT_MASK	(0xF << MIDR_VARIANT_SHIFT)

static inline unsigned int read_midr(void)
{
	unsigned long val;

	asm volatile("mrs %0, midr_el1" : "=r" (val));

	return val;
}

#define is_cortex_a35() (((read_midr() & MIDR_PARTNUM_MASK) >> \
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A35)
#define is_cortex_a53() (((read_midr() & MIDR_PARTNUM_MASK) >> \
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A53)
#define is_cortex_a72() (((read_midr() & MIDR_PARTNUM_MASK) >>\
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A72)

#define read_core_midr_revision()	((read_midr() & MIDR_REVISION_MASK) >>\
					 MIDR_REVISION_SHIFT)
#define read_core_midr_variant()	((read_midr() & MIDR_VARIANT_MASK) >> \
					 MIDR_VARIANT_SHIFT)
