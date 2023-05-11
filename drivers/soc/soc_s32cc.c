// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2023 NXP
 */

#include <common.h>
#include <dm.h>
#include <nvmem.h>
#include <soc.h>

struct soc_s32cc_plat {
	u32 letter;
	u32 part_number;
	u32 major;
	u32 minor;
	u32 subminor;
	bool has_subminor;
};

struct soc_nvmem_cell {
	const char *name;
	u32 *data;
};

static int soc_s32cc_get_machine(struct udevice *dev, char *buf, int size)
{
	struct soc_s32cc_plat *plat = dev_get_platdata(dev);
	int ret;

	ret = snprintf(buf, size, "%dA", plat->part_number);
	if (ret >= size || ret < 0)
		return -ENOSPC;

	return 0;
}

static int soc_s32cc_get_revision(struct udevice *dev, char *buf, int size)
{
	struct soc_s32cc_plat *plat = dev_get_platdata(dev);
	int ret;

	ret = snprintf(buf, size, "%u.%u", plat->major, plat->minor);
	if (ret >= size || ret < 0)
		return -ENOSPC;

	size -= ret;
	if (plat->has_subminor) {
		ret = snprintf(buf + ret, size, ".%u", plat->subminor);
		if (ret >= size || ret < 0)
			return -ENOSPC;
	}

	return 0;
}

static int soc_s32cc_get_family(struct udevice *dev, char *buf, int size)
{
	struct soc_s32cc_plat *plat = dev_get_platdata(dev);
	int ret;

	ret = snprintf(buf, size, "NXP S32%c", (char)plat->letter);
	if (ret >= size || ret < 0)
		return -ENOSPC;

	return 0;
}

static const struct soc_ops soc_s32cc_ops = {
	.get_machine = soc_s32cc_get_machine,
	.get_revision = soc_s32cc_get_revision,
	.get_family = soc_s32cc_get_family,
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

static int soc_s32cc_probe(struct udevice *dev)
{
	struct soc_s32cc_plat *plat = dev_get_platdata(dev);
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

	return 0;
}

static const struct udevice_id soc_s32cc_ids[] = {
	{ .compatible = "nxp,s32cc-soc" },
	{}
};

U_BOOT_DRIVER(soc_s32cc_drv) = {
	.name = "soc_s32cc",
	.id = UCLASS_SOC,
	.ops = &soc_s32cc_ops,
	.of_match = soc_s32cc_ids,
	.probe = soc_s32cc_probe,
	.platdata_auto_alloc_size = sizeof(struct soc_s32cc_plat),
	.flags = DM_FLAG_PRE_RELOC,
};
