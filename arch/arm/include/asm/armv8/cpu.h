/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2018,2023-2024 NXP
 */

#define MIDR_PARTNUM_CORTEX_A35		0xD04
#define MIDR_PARTNUM_CORTEX_A53		0xD03
#define MIDR_PARTNUM_CORTEX_A55		0xD05
#define MIDR_PARTNUM_CORTEX_A57		0xD07
#define MIDR_PARTNUM_CORTEX_A72		0xD08
#define MIDR_PARTNUM_CORTEX_A73		0xD09
#define MIDR_PARTNUM_CORTEX_A75		0xD0A
#define MIDR_PARTNUM_CORTEX_A76		0xD0B
#define MIDR_PARTNUM_CORTEX_A720	0xD81
#define MIDR_PARTNUM_CORTEX_A78AE	0xD42
#define MIDR_PARTNUM_SHIFT		0x4
#define MIDR_PARTNUM_MASK		(0xFFF << MIDR_PARTNUM_SHIFT)
#define MIDR_REVISION_SHIFT	0x0
#define MIDR_REVISION_MASK	(0xF << MIDR_REVISION_SHIFT)

#define MIDR_VARIANT_SHIFT	0x14
#define MIDR_VARIANT_MASK	(0xF << MIDR_VARIANT_SHIFT)

static inline unsigned int read_midr(void)
{
	unsigned long val;

	asm volatile("mrs %0, midr_el1" : "=r" (val));

	return val;
}

#define is_cortex_a(__n)					\
	static inline int is_cortex_a##__n(void)		\
	{							\
		unsigned int midr = read_midr();		\
		midr &= MIDR_PARTNUM_MASK;			\
		midr >>= MIDR_PARTNUM_SHIFT;			\
		return midr == MIDR_PARTNUM_CORTEX_A##__n;	\
	}

is_cortex_a(35)
is_cortex_a(53)
is_cortex_a(55)
is_cortex_a(57)
is_cortex_a(72)
is_cortex_a(73)
is_cortex_a(75)
is_cortex_a(76)
is_cortex_a(720)
#define is_cortex_a35() (((read_midr() & MIDR_PARTNUM_MASK) >> \
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A35)
#define is_cortex_a53() (((read_midr() & MIDR_PARTNUM_MASK) >> \
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A53)
#define is_cortex_a72() (((read_midr() & MIDR_PARTNUM_MASK) >>\
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A72)
#define is_cortex_a78ae() (((read_midr() & MIDR_PARTNUM_MASK) >>\
			 MIDR_PARTNUM_SHIFT) == MIDR_PARTNUM_CORTEX_A78AE)

#define read_core_midr_revision()	((read_midr() & MIDR_REVISION_MASK) >>\
					 MIDR_REVISION_SHIFT)
#define read_core_midr_variant()	((read_midr() & MIDR_VARIANT_MASK) >> \
					 MIDR_VARIANT_SHIFT)
