// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <cpu.h>
#include <dm.h>
#include <misc.h>
#include <dm/uclass.h>
#include <s32-cc/siul2_nvram.h>

struct cpu_s32cc_plat {
	struct udevice *siul20_nvmem;
	struct udevice *siul21_nvmem;
};

static int cpu_s32cc_get_desc(const struct udevice *dev, char *buf, int size)
{
	bool has_subminor = false;
	u32 letter, part_number, major, minor, subminor;
	struct cpu_s32cc_plat *plat = dev_get_plat(dev);
	int ret;

	ret = misc_read(plat->siul20_nvmem, S32CC_SOC_LETTER, &letter,
			sizeof(letter));
	if (ret != sizeof(letter)) {
		printf("%s: Failed to read SoC's letter (err = %d)\n",
		       __func__, ret);
		return -EINVAL;
	}

	ret = misc_read(plat->siul20_nvmem, S32CC_SOC_PART_NO, &part_number,
			sizeof(part_number));
	if (ret != sizeof(part_number)) {
		printf("%s: Failed to read SoC's part number (err = %d)\n",
		       __func__, ret);
		return -EINVAL;
	}

	ret = misc_read(plat->siul20_nvmem, S32CC_SOC_MAJOR, &major,
			sizeof(major));
	if (ret != sizeof(major)) {
		printf("%s: Failed to read SoC's major (err = %d)\n",
		       __func__, ret);
		return -EINVAL;
	}

	ret = misc_read(plat->siul20_nvmem, S32CC_SOC_MINOR, &minor,
			sizeof(minor));
	if (ret != sizeof(minor)) {
		printf("%s: Failed to read SoC's minor (err = %d)\n",
		       __func__, ret);
		return -EINVAL;
	}

	/* It might be unavailable */
	ret = misc_read(plat->siul21_nvmem, S32CC_SOC_SUBMINOR, &subminor,
			sizeof(subminor));
	if (ret == sizeof(subminor))
		has_subminor = true;

	ret = snprintf(buf, size, "NXP S32%c%uA rev. %u.%u",
		       (char)letter, part_number, major, minor);

	if (has_subminor)
		snprintf(buf + ret, size - ret, ".%u", subminor);

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

static const struct cpu_ops cpu_s32cc_ops = {
	.get_desc	= cpu_s32cc_get_desc,
	.get_info	= cpu_s32cc_get_info,
	.get_count	= cpu_s32cc_get_count,
	.get_vendor	= cpu_s32cc_get_vendor,
};

static const struct udevice_id cpu_s32cc_ids[] = {
	{ .compatible = "arm,cortex-a53" },
	{ }
};

static int s32cc_cpu_probe(struct udevice *dev)
{
	struct cpu_s32cc_plat *plat = dev_get_plat(dev);
	struct udevice *siul20_nvmem, *siul21_nvmem;
	int ret;

	ret = uclass_get_device_by_name(UCLASS_MISC, "siul2_0_nvram",
					&siul20_nvmem);
	if (ret) {
		printf("%s: No SIUL20 NVMEM (err = %d)\n", __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_name(UCLASS_MISC, "siul2_1_nvram",
					&siul21_nvmem);
	if (ret) {
		printf("%s: No SIUL21 NVMEM (err = %d)\n", __func__, ret);
		return ret;
	}

	plat->siul20_nvmem = siul20_nvmem;
	plat->siul21_nvmem = siul21_nvmem;

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

