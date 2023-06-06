// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022-2023 NXP
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <misc.h>
#include <nvmem.h>
#include <asm/system.h>
#include <dm/uclass.h>

struct cpu_s32cc_plat {
	u32 letter;
	u32 part_number;
	u32 major;
	u32 minor;
	u32 subminor;
	bool has_subminor;
	u32 mpidr;
};

struct soc_nvmem_cell {
	const char *name;
	u32 *data;
};

static int cpu_s32cc_get_desc(const struct udevice *dev, char *buf, int size)
{
	struct cpu_s32cc_plat *plat = dev_get_plat(dev);
	int ret;

	ret = snprintf(buf, size, "NXP S32%c%uA rev. %u.%u",
		       (char)plat->letter, plat->part_number, plat->major,
		       plat->minor);

	if (plat->has_subminor)
		snprintf(buf + ret, size - ret, ".%u", plat->subminor);

	return 0;
}

static int cpu_s32cc_get_info(const struct udevice *dev, struct cpu_info *info)
{
	info->features = BIT(CPU_FEAT_L1_CACHE) | BIT(CPU_FEAT_MMU);

	return 0;
}

static int cpu_s32cc_get_count(const struct udevice *dev)
{
	/* One CPU per instance */
	return 1;
}

static int cpu_s32cc_get_vendor(const struct udevice *dev,  char *buf, int size)
{
	snprintf(buf, size, "NXP");
	return 0;
}

static int cpu_s32cc_is_current(struct udevice *dev)
{
	struct cpu_s32cc_plat *plat = dev_get_plat(dev);

	if (plat->mpidr == (read_mpidr() & 0xffff))
		return 1;

	return 0;
}

static const struct cpu_ops cpu_s32cc_ops = {
	.get_desc	= cpu_s32cc_get_desc,
	.get_info	= cpu_s32cc_get_info,
	.get_count	= cpu_s32cc_get_count,
	.get_vendor	= cpu_s32cc_get_vendor,
	.is_current	= cpu_s32cc_is_current,
};

static const struct udevice_id cpu_s32cc_ids[] = {
	{ .compatible = "arm,cortex-a53" },
	{ }
};

static int read_soc_nvmem_cell(struct udevice *dev, struct soc_nvmem_cell *cell)
{
	struct nvmem_cell c;
	int ret;

	ret = nvmem_cell_get_by_name(dev, cell->name, &c);
	if (ret) {
		printf("Failed to get '%s' cell\n", cell->name);
		return ret;
	}

	ret = nvmem_cell_read(&c, cell->data, sizeof(*cell->data));
	if (ret) {
		printf("%s: Failed to read cell '%s' (err = %d)\n",
		       __func__, cell->name, ret);
		return ret;
	}

	return 0;
}

static int s32cc_cpu_probe(struct udevice *dev)
{
	struct cpu_s32cc_plat *plat = dev_get_plat(dev);
	struct nvmem_cell cell;
	struct soc_nvmem_cell cells[] = {
		{ .name = "soc_letter", .data = &plat->letter },
		{ .name = "part_no", .data = &plat->part_number },
		{ .name = "soc_major", .data = &plat->major },
		{ .name = "soc_minor", .data = &plat->minor },
	};
	const char *subminor = "soc_subminor";
	int ret;
	size_t i;

	if (!plat)
		return -EINVAL;

	for (i = 0u; i < ARRAY_SIZE(cells); i++) {
		ret = read_soc_nvmem_cell(dev, &cells[i]);
		if (ret)
			return ret;
	}

	ret = nvmem_cell_get_by_name(dev, subminor, &cell);
	if (ret) {
		printf("Failed to get '%s' cell", subminor);
		return ret;
	}

	ret = nvmem_cell_read(&cell, &plat->subminor,
			      sizeof(plat->subminor));
	if (ret)
		plat->has_subminor = false;

	plat->mpidr = dev_read_addr(dev);
	if (plat->mpidr == FDT_ADDR_T_NONE) {
		printf("Failed to get CPU reg property\n");
		return -EINVAL;
	}

	return 0;
}

U_BOOT_DRIVER(cpu_s32cc_drv) = {
	.name		= "s32cc_cpu",
	.id		= UCLASS_CPU,
	.of_match	= cpu_s32cc_ids,
	.ops		= &cpu_s32cc_ops,
	.probe		= s32cc_cpu_probe,
	.plat_auto	= sizeof(struct cpu_s32cc_plat),
	.flags		= DM_FLAG_PRE_RELOC,
};
