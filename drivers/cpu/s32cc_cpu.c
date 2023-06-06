// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022-2023 NXP
 */

#include <common.h>
#include <clk.h>
#include <cpu.h>
#include <dm.h>
#include <misc.h>
#include <nvmem.h>
#include <asm/system.h>
#include <asm/armv8/cpu.h>
#include <dm/uclass.h>
#include <dt-bindings/clock/s32cc-scmi-clock.h>

struct cpu_s32cc_plat {
	const char *name;
	u32 variant;
	u32 revision;
	u32 max_freq;
	u32 mpidr;
};

static struct udevice *get_clk_device(void)
{
	static struct udevice *dev;
	int ret;

	if (dev)
		return dev;

	ret = uclass_get_device_by_driver(UCLASS_CLK, DM_DRIVER_GET(scmi_clock),
					  &dev);
	if (ret)
		return NULL;

	return dev;
}

static int cpu_s32cc_get_desc(const struct udevice *dev, char *buf, int size)
{
	struct cpu_s32cc_plat *plat = dev_get_plat(dev);
	int ret;

	ret = snprintf(buf, size,
		       "ARM Cortex-%s r%up%u @ max %u MHz",
		       plat->name, plat->variant, plat->revision,
		       plat->max_freq);
	if (ret >= size || ret < 0)
		return -ENOSPC;

	return 0;
}

static int cpu_s32cc_get_info(const struct udevice *dev, struct cpu_info *info)
{
	int ret;
	ulong freq;
	struct udevice *clk_dev;
	struct clk a53_clock = {
		.id = S32CC_SCMI_CLK_A53,
	};

	clk_dev = get_clk_device();
	if (!clk_dev) {
		printf("%s: Failed to get clock device\n", __func__);
		return -ENODEV;
	}

	ret = clk_request(clk_dev, &a53_clock);
	if (ret) {
		printf("%s: Failed to request clock for CPU (err = %d)\n",
		       __func__, ret);
		return ret;
	}

	freq = clk_get_rate(&a53_clock);
	if (!freq) {
		printf("%s: Failed to get clock freq for CPU\n", __func__);
		goto exit;
	}

	info->cpu_freq = freq;
	info->features = BIT(CPU_FEAT_L1_CACHE) | BIT(CPU_FEAT_MMU);

exit:
	ret = clk_free(&a53_clock);
	if (ret) {
		printf("%s: Failed to free clock for CPU (err = %d)\n",
		       __func__, ret);
		return ret;
	}

	return freq ? 0 : -EINVAL;
}

static int cpu_s32cc_get_count(const struct udevice *dev)
{
	/* One CPU per instance */
	return 1;
}

static int cpu_s32cc_get_vendor(const struct udevice *dev,  char *buf, int size)
{
	int ret;

	ret = snprintf(buf, size, "NXP");
	if (ret >= size || ret < 0)
		return -ENOSPC;

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

static const char *s32cc_get_core_name(void)
{
	if (is_cortex_a53())
		return "A53";

	return "?";
}

static int s32cc_cpu_probe(struct udevice *dev)
{
	struct cpu_s32cc_plat *plat = dev_get_plat(dev);
	struct nvmem_cell cell;
	const char *max_freq_name = "core_max_freq";
	int ret;

	if (!plat)
		return -EINVAL;

	plat->name = s32cc_get_core_name();
	plat->variant = read_core_midr_variant();
	plat->revision = read_core_midr_revision();

	ret = nvmem_cell_get_by_name(dev, max_freq_name, &cell);
	if (ret) {
		printf("Failed to get '%s' cell\n", max_freq_name);
		return ret;
	}

	ret = nvmem_cell_read(&cell, &plat->max_freq, sizeof(plat->max_freq));
	if (ret) {
		printf("%s: Failed to read cell '%s' (err = %d)\n",
		       __func__, max_freq_name, ret);
		return ret;
	}

	plat->mpidr = dev_read_addr(dev);
	if (plat->mpidr == FDT_ADDR_T_NONE) {
		printf("Failed to get CPU reg property\n");
		return -EINVAL;
	}

	return 0;
}

static int s32cc_cpu_bind(struct udevice *dev)
{
	struct cpu_plat *plat = dev_get_parent_plat(dev);

	plat->cpu_id = dev_read_u32_default(dev, "reg", 0);

	return 0;
}

U_BOOT_DRIVER(cpu_s32cc_drv) = {
	.name		= "s32cc_cpu",
	.id		= UCLASS_CPU,
	.of_match	= cpu_s32cc_ids,
	.ops		= &cpu_s32cc_ops,
	.bind		= s32cc_cpu_bind,
	.probe		= s32cc_cpu_probe,
	.plat_auto	= sizeof(struct cpu_s32cc_plat),
	.flags		= DM_FLAG_PRE_RELOC,
};
