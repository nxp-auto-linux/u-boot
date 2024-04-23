// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2024 NXP
 */

#include <common.h>
#include <clk.h>
#include <cpu.h>
#include <dm.h>
#include <misc.h>
#include <asm/system.h>
#include <asm/armv8/cpu.h>
#include <dm/uclass.h>

struct cpu_s32n_plat {
	const char *name;
	u32 variant;
	u32 revision;
	u32 max_freq;
	u32 mpidr;
};

static int cpu_s32n_get_desc(const struct udevice *dev, char *buf, int size)
{
	struct cpu_s32n_plat *plat = dev_get_plat(dev);
	int ret;

	ret = snprintf(buf, size,
		       "ARM Cortex-%s r%up%u @ max %u MHz",
		       plat->name, plat->variant, plat->revision,
		       plat->max_freq);
	if (ret >= size || ret < 0)
		return -ENOSPC;

	return 0;
}

static int cpu_s32n_get_info(const struct udevice *dev, struct cpu_info *info)
{
	info->cpu_freq = 0;
	info->features = BIT(CPU_FEAT_L1_CACHE) | BIT(CPU_FEAT_MMU);

	return info->cpu_freq;
}

static int cpu_s32n_get_count(const struct udevice *dev)
{
	/* One CPU per instance */
	return 1;
}

static int cpu_s32n_get_vendor(const struct udevice *dev,  char *buf, int size)
{
	int ret;

	ret = snprintf(buf, size, "NXP");
	if (ret >= size || ret < 0)
		return -ENOSPC;

	return 0;
}

static int cpu_s32n_is_current(struct udevice *dev)
{
	struct cpu_s32n_plat *plat = dev_get_plat(dev);

	if (plat->mpidr == (read_mpidr() & 0xffff))
		return 1;

	return 0;
}

static const struct cpu_ops cpu_s32n_ops = {
	.get_desc	= cpu_s32n_get_desc,
	.get_info	= cpu_s32n_get_info,
	.get_count	= cpu_s32n_get_count,
	.get_vendor	= cpu_s32n_get_vendor,
	.is_current	= cpu_s32n_is_current,
};

static const struct udevice_id cpu_s32n_ids[] = {
	{ .compatible = "arm,cortex-a78-ae" },
	{ }
};

static const char *s32n_get_core_name(void)
{
	if (is_cortex_a78ae())
		return "A78AE";

	return "?";
}

static int s32n_cpu_probe(struct udevice *dev)
{
	struct cpu_s32n_plat *plat = dev_get_plat(dev);

	if (!plat)
		return -EINVAL;

	plat->name = s32n_get_core_name();
	plat->variant = read_core_midr_variant();
	plat->revision = read_core_midr_revision();

	plat->mpidr = dev_read_addr(dev);
	if (plat->mpidr == FDT_ADDR_T_NONE) {
		printf("Failed to get CPU reg property\n");
		return -EINVAL;
	}

	return 0;
}

static int s32n_cpu_bind(struct udevice *dev)
{
	struct cpu_plat *plat = dev_get_parent_plat(dev);

	plat->cpu_id = dev_read_u32_default(dev, "reg", 0);

	return 0;
}

U_BOOT_DRIVER(cpu_s32n_drv) = {
	.name		= "s32n_cpu",
	.id		= UCLASS_CPU,
	.of_match	= cpu_s32n_ids,
	.ops		= &cpu_s32n_ops,
	.bind		= s32n_cpu_bind,
	.probe		= s32n_cpu_probe,
	.plat_auto	= sizeof(struct cpu_s32n_plat),
	.flags		= DM_FLAG_PRE_RELOC,
};
