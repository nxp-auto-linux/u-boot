// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022-2023 NXP
 */
#include <dm.h>
#include <log.h>
#include <misc.h>
#include <asm/io.h>
#include <dm/device.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <dt-bindings/nvmem/s32cc-siul2-nvmem.h>

#define MIDR1_OFF	0x4
#define MIDR2_OFF	0x8

/* SIUL2_0, MIDR1 */
#define MINOR_SHIFT	0
#define MINOR_MASK	GENMASK(3, MINOR_SHIFT)

#define MAJOR_SHIFT	4
#define MAJOR_MASK	GENMASK(7, MAJOR_SHIFT)

#define PART_NO_SHIFT	16
#define PART_NO_MASK	GENMASK(25, PART_NO_SHIFT)

#define LETTER_SHIFT	26
#define LETTER_MASK	GENMASK(31, LETTER_SHIFT)

/* SIUL2_0, MIDR2 */
#define CORE_FREQ_SHIFT	16
#define CORE_FREQ_MASK	GENMASK(19, CORE_FREQ_SHIFT)

/* SIUL2_1 MIDR1 */
#define SERDES_SHIFT	15
#define SERDES_MASK	BIT(SERDES_SHIFT)

#define LAX_SHIFT	12
#define LAX_MASK	BIT(LAX_SHIFT)

/* SIUL2_1 MIDR2 */
#define S32G2_SUBMINOR_SHIFT	26
#define S32G2_SUBMINOR_MASK	GENMASK(27, 26)

#define S32G_MIN_PART_NO	233
#define S32G_MAX_PART_NO	399
#define S32R_PART_NO_MIN	450

#define CPUMASK_S32G2				GENMASK(3, 0)
#define CPUMASK_S32G2_DERIVATIVE		(BIT(0) | BIT(2))
#define CPUMASK_S32G3				GENMASK(7, 0)
#define CPUMASK_S32G37X_DERIVATIVE		(GENMASK(5, 4) | GENMASK(1, 0))
#define CPUMASK_S32G35X_DERIVATIVE		(BIT(4) | BIT(0))
#define CPUMASK_S32R				GENMASK(3, 0)

struct siul2_nvram;

struct siul_mapping {
	u32 nvram_off;
	u32 siul2_off;
	u32 mask;
	u32 shift;
	u32 (*adjust_value)(u32 value, struct siul2_nvram *nvram);
};

struct siul_plat {
	const struct siul_mapping *mappings;
	size_t n_mappings;
	const struct siul_plat *next;
};

struct siul2_nvram {
	const struct siul_plat *plat;
	void __iomem *base;
};

static inline u32 get_second_digit(u32 val)
{
	return (val / 10) % 10;
}

static inline u32 get_third_digit(u32 val)
{
	return (val / 100) % 10;
}

static u32 adjust_letter(u32 value, struct siul2_nvram *nvram)
{
	return ('A' - 1 + value);
}

static u32 adjust_major(u32 value, struct siul2_nvram *nvram)
{
	return value + 1;
}

static u32 adjust_minor(u32 value, struct siul2_nvram *nvram)
{
	u32 minor = value & MINOR_MASK;

	if ((value & MAJOR_MASK) && minor)
		minor--;

	return minor;
}

static u32 adjust_freq(u32 value, struct siul2_nvram *nvram)
{
	static const u32 freqs[] = {
		[1] = 64,
		[2] = 80,
		[3] = 120,
		[4] = 160,
		[5] = 240,
		[6] = 320,
		[7] = 340,
		[8] = 400,
		[9] = 600,
		[10] = 800,
		[11] = 1000,
		[12] = 1200,
	};

	if (value < ARRAY_SIZE(freqs))
		return freqs[value];

	return 0;
}

static u32 adjust_s32g3_freq(u32 value, struct siul2_nvram *nvram)
{
	static const u32 freqs[] = {
		[1] = 64,
		[2] = 80,
		[3] = 120,
		[4] = 160,
		[5] = 240,
		[6] = 320,
		[7] = 340,
		[8] = 400,
		[9] = 600,
		[10] = 800,
		[11] = 1000,
		[12] = 1100,
		[14] = 1300,
		[15] = 1400,
	};

	if (value < ARRAY_SIZE(freqs))
		return freqs[value];

	return 0;
}

static u32 adjust_s32r_max_cores_per_cluster(u32 value,
					     struct siul2_nvram *nvram)
{
	u32 vert_slot, plat_gen;

	if (value < S32R_PART_NO_MIN)
		return 0;

	/* Get Platform Generation */
	plat_gen = get_third_digit(value);

	/* Get Number of Cores */
	vert_slot = get_second_digit(value);

	if (plat_gen == 4 && vert_slot == 5)
		return 2;

	return 0;
}

static u32 adjust_s32g_max_cores_per_cluster(u32 value,
					     struct siul2_nvram *nvram)
{
	u32 series;

	if (value < S32G_MIN_PART_NO || value > S32G_MAX_PART_NO)
		return 0;

	/* Get S32G platform flavour (S32G2/S32G3) */
	series = get_third_digit(value);

	if (series == 3)
		return 4;

	return 2;
}

static u32 adjust_s32r_derivative_cores(u32 value,
					struct siul2_nvram *nvram)
{
	u32 vert_slot, plat_gen;

	if (value < S32R_PART_NO_MIN)
		return 0;

	/* Get Platform Generation */
	plat_gen = get_third_digit(value);

	/* Get Number of Cores */
	vert_slot = get_second_digit(value);

	if (plat_gen == 4 && vert_slot == 5)
		return CPUMASK_S32R;

	return 0;
}

static u32 adjust_s32g_derivative_cores(u32 value,
					struct siul2_nvram *nvram)
{
	u32 series;
	u32 perf_id;

	if (value < S32G_MIN_PART_NO || value > S32G_MAX_PART_NO)
		return 0;

	perf_id = get_second_digit(value);

	/* Get S32G platform flavour (S32G2/S32G3) */
	series = get_third_digit(value);

	if (series == 3) {
		if (perf_id == 9)
			return CPUMASK_S32G3;
		else if (perf_id == 7)
			return CPUMASK_S32G37X_DERIVATIVE;
		else if (perf_id == 5)
			return CPUMASK_S32G35X_DERIVATIVE;
	} else if (series == 2) {
		if (perf_id == 3 || perf_id == 5)
			return CPUMASK_S32G2_DERIVATIVE;
		else if (perf_id == 7)
			return CPUMASK_S32G2;
	}

	return 0;
}

static u32 adjust_pcie_dev_id(u32 value,
			      struct siul2_nvram *nvram)
{
	/* Mapping between G3 variant ID and the PCIe Device ID,
	 * as described in the "S32G3 Reference Manual rev1.0,
	 * chapter SerDes Subsystem, section "Device and revision IDs",
	 * where: index = last 2 digits of the variant
	 *        value = last hex digit of the PCIe Device ID"
	 */
	static const u32 s32g3_variants[] = {
		[78] = 0x6,
		[79] = 0x4,
		[98] = 0x2,
		[99] = 0x0,
	};

	if (value / 10 == 45)
		return 0x4002;

	if (value / 100 == 2)
		return 0;

	value %= 100;

	if (value < ARRAY_SIZE(s32g3_variants))
		return s32g3_variants[value];

	return 0;
}

static const struct siul_mapping siul20_mappings[] = {
	{
		.nvram_off = S32CC_SOC_MAJOR,
		.siul2_off = MIDR1_OFF,
		.mask = MAJOR_MASK,
		.shift = MAJOR_SHIFT,
		.adjust_value = adjust_major,
	},
	{
		.nvram_off = S32CC_SOC_MINOR,
		.siul2_off = MIDR1_OFF,
		.mask = MINOR_MASK,
		.shift = MINOR_SHIFT,
	},
	{
		.nvram_off = S32CC_SOC_PART_NO,
		.siul2_off = MIDR1_OFF,
		.mask = PART_NO_MASK,
		.shift = PART_NO_SHIFT,
	},
	{
		.nvram_off = S32CC_SOC_LETTER,
		.siul2_off = MIDR1_OFF,
		.mask = LETTER_MASK,
		.shift = LETTER_SHIFT,
		.adjust_value = adjust_letter,
	},
	{
		.nvram_off = S32CC_MAX_CORE_FREQ,
		.siul2_off = MIDR2_OFF,
		.mask = CORE_FREQ_MASK,
		.shift = CORE_FREQ_SHIFT,
		.adjust_value = adjust_freq,
	},
	{
		.nvram_off = S32CC_OVERWRITE_PCIE_DEV_ID,
		.siul2_off = MIDR1_OFF,
		.mask = PART_NO_MASK,
		.shift = PART_NO_SHIFT,
		.adjust_value = adjust_pcie_dev_id,
	},
};

static const struct siul_mapping siul21_mappings[] = {
	{
		.nvram_off = S32CC_SERDES_PRESENCE,
		.siul2_off = MIDR2_OFF,
		.mask = SERDES_MASK,
		.shift = SERDES_SHIFT,
	},
	{
		.nvram_off = S32CC_LAX_PRESENCE,
		.siul2_off = MIDR2_OFF,
		.mask = LAX_MASK,
		.shift = LAX_SHIFT,
	},
};

static const struct siul_mapping s32g_siul20_mappings[] = {
	{
		.nvram_off = S32CC_MAX_A53_CORES_PER_CLUSTER,
		.siul2_off = MIDR1_OFF,
		.mask = PART_NO_MASK,
		.shift = PART_NO_SHIFT,
		.adjust_value = adjust_s32g_max_cores_per_cluster,
	},
	{
		.nvram_off = S32CC_A53_CORES_MASK,
		.siul2_off = MIDR1_OFF,
		.mask = PART_NO_MASK,
		.shift = PART_NO_SHIFT,
		.adjust_value = adjust_s32g_derivative_cores,
	},
};

static const struct siul_mapping s32r_siul20_mappings[] = {
	{
		.nvram_off = S32CC_MAX_A53_CORES_PER_CLUSTER,
		.siul2_off = MIDR1_OFF,
		.mask = PART_NO_MASK,
		.shift = PART_NO_SHIFT,
		.adjust_value = adjust_s32r_max_cores_per_cluster,
	},
	{
		.nvram_off = S32CC_A53_CORES_MASK,
		.siul2_off = MIDR1_OFF,
		.mask = PART_NO_MASK,
		.shift = PART_NO_SHIFT,
		.adjust_value = adjust_s32r_derivative_cores,
	},
};

static const struct siul_mapping s32g2_siul20_mappings[] = {
	{
		.nvram_off = S32CC_SOC_MINOR,
		.siul2_off = MIDR1_OFF,
		.mask = MAJOR_MASK | MINOR_MASK,
		.shift = MINOR_SHIFT,
		.adjust_value = adjust_minor,
	},
};

static const struct siul_mapping s32g2_siul21_mappings[] = {
	{
		.nvram_off = S32CC_SOC_SUBMINOR,
		.siul2_off = MIDR2_OFF,
		.mask = S32G2_SUBMINOR_MASK,
		.shift = S32G2_SUBMINOR_SHIFT,
	},
};

static const struct siul_mapping s32g3_siul20_mappings[] = {
	{
		.nvram_off = S32CC_MAX_CORE_FREQ,
		.siul2_off = MIDR2_OFF,
		.mask = CORE_FREQ_MASK,
		.shift = CORE_FREQ_SHIFT,
		.adjust_value = adjust_s32g3_freq,
	},

};

static const struct siul_plat siul20_plat = {
	.mappings = &siul20_mappings[0],
	.n_mappings = ARRAY_SIZE(siul20_mappings),
};

static const struct siul_plat siul21_plat = {
	.mappings = &siul21_mappings[0],
	.n_mappings = ARRAY_SIZE(siul21_mappings),
};

static const struct siul_plat s32g_siul20_plat = {
	.mappings = &s32g_siul20_mappings[0],
	.n_mappings = ARRAY_SIZE(s32g_siul20_mappings),
	.next = &siul20_plat,
};

static const struct siul_plat s32r_siul20_plat = {
	.mappings = &s32r_siul20_mappings[0],
	.n_mappings = ARRAY_SIZE(s32r_siul20_mappings),
	.next = &siul20_plat,
};

static const struct siul_plat s32g2_siul20_plat = {
	.mappings = &s32g2_siul20_mappings[0],
	.n_mappings = ARRAY_SIZE(s32g2_siul20_mappings),
	.next = &s32g_siul20_plat,
};

static const struct siul_plat s32g2_siul21_plat = {
	.mappings = &s32g2_siul21_mappings[0],
	.n_mappings = ARRAY_SIZE(s32g2_siul21_mappings),
	.next = &siul21_plat,
};

static const struct siul_plat s32g3_siul20_plat = {
	.mappings = &s32g3_siul20_mappings[0],
	.n_mappings = ARRAY_SIZE(s32g_siul20_mappings),
	.next = &s32g_siul20_plat,
};

static int siul2_nvram_read(struct udevice *dev, int offset,
			    void *buf, int size)
{
	struct siul2_nvram *nvram = dev_get_plat(dev);
	const struct siul_mapping *mapping = NULL;
	const struct siul_plat *plat = nvram->plat;
	size_t i;
	u32 val;

	/* All values are 4 bytes wide */
	if (size != sizeof(u32))
		return 0;

	while (plat) {
		for (i = 0u; i < plat->n_mappings; i++) {
			if (plat->mappings[i].nvram_off == offset) {
				mapping = &plat->mappings[i];
				break;
			}
		}

		if (mapping)
			break;

		plat = plat->next;
	}

	if (!mapping)
		return 0;

	val = readl(nvram->base + mapping->siul2_off);

	val = (val & mapping->mask) >> mapping->shift;
	if (mapping->adjust_value)
		val = mapping->adjust_value(val, nvram);

	*((u32 *)buf) = val;
	return size;
}

static const struct misc_ops siul2_nvram_ops = {
	.read = siul2_nvram_read,
};

static const struct udevice_id siul2_nvram_ids[] = {
	{
		.compatible = "nxp,s32g2-siul2_0-nvram",
		.data = (ulong)&s32g2_siul20_plat,
	},
	{
		.compatible = "nxp,s32g3-siul2_0-nvram",
		.data = (ulong)&s32g3_siul20_plat,
	},
	{
		.compatible = "nxp,s32r-siul2_0-nvram",
		.data = (ulong)&s32r_siul20_plat,
	},
	{
		.compatible = "nxp,s32g2-siul2_1-nvram",
		.data = (ulong)&s32g2_siul21_plat,
	},
	{
		.compatible = "nxp,s32g3-siul2_1-nvram",
		.data = (ulong)&siul21_plat,
	},
	{
		.compatible = "nxp,s32r-siul2_1-nvram",
		.data = (ulong)&siul21_plat,
	},
	{ /* sentinel */ }
};

static int siul2_nvram_set_plat(struct udevice *dev)
{
	struct siul2_nvram *nvram = dev_get_plat(dev);

	nvram->base = (void __iomem *)dev_read_addr_ptr(dev);
	if (!nvram->base)
		return -EINVAL;

	nvram->plat = (struct siul_plat *)dev_get_driver_data(dev);
	return 0;
}

U_BOOT_DRIVER(s32cc_siul2_nvram) = {
	.name = "s32cc_siul2_nvram",
	.id = UCLASS_MISC,
	.ops = &siul2_nvram_ops,
	.of_match = siul2_nvram_ids,
	.plat_auto = sizeof(struct siul2_nvram),
	.of_to_plat = siul2_nvram_set_plat,
};

