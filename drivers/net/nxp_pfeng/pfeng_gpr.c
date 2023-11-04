// SPDX-License-Identifier: GPL-2.0+
/*
 * NXP S32G PFE Ethernet accelerator (base) driver
 *
 * Copyright 2023 NXP
 */

#include <common.h>
#include <dm.h>
#include <nvmem.h>
#include <dm/device_compat.h>

#define PFE_COH_PORTS_MASK_HIF_0_3      GENMASK(4, 1)

int pfeng_write_nvmem_cell(struct udevice *dev, const char *cell_name,
			   u32 value)
{
	struct nvmem_cell c;
	int ret;

	ret = nvmem_cell_get_by_name(dev, cell_name, &c);
	if (ret) {
		dev_err(dev, "Failed to get cell '%s' (err = %d)\n", cell_name,
			ret);
		return ret;
	}

	ret = nvmem_cell_write(&c, &value, sizeof(value));
	if (ret) {
		dev_err(dev, "Failed to write cell '%s' (err = %d)\n",
			cell_name, ret);
		return ret;
	}

	return 0;
}

int pfeng_read_nvmem_cell(struct udevice *dev, const char *cell_name,
			  u32 *value)
{
	struct nvmem_cell c;
	int ret;

	ret = nvmem_cell_get_by_name(dev, cell_name, &c);
	if (ret) {
		dev_err(dev, "Failed to get cell '%s' (err = %d)\n", cell_name,
			ret);
		return ret;
	}

	ret = nvmem_cell_read(&c, value, sizeof(*value));
	if (ret) {
		dev_err(dev, "Failed to read cell '%s' (err = %d)\n",
			cell_name, ret);
		return ret;
	}

	return 0;
}

int pfeng_set_port_coherency_nvmem(struct udevice *dev)
{
	int ret;

	ret = pfeng_write_nvmem_cell(dev, "pfe_coh_en",
				     PFE_COH_PORTS_MASK_HIF_0_3);
	if (ret) {
		dev_err(dev, "Failed to set PFE HIF coherency\n");
		return ret;
	}

	return 0;
}

int pfeng_clear_port_coherency_nvmem(struct udevice *dev)
{
	int ret;
	u32 value = 0;

	ret = pfeng_read_nvmem_cell(dev, "pfe_coh_en", &value);
	if (ret) {
		dev_err(dev, "Failed to read PFE HIF coherency\n");
		return ret;
	}

	if (!(value & PFE_COH_PORTS_MASK_HIF_0_3)) {
		dev_dbg(dev, "PFE port coherency already cleared\n");
		return 0;
	}

	value &= ~((u32)PFE_COH_PORTS_MASK_HIF_0_3);
	ret = pfeng_write_nvmem_cell(dev, "pfe_coh_en", value);
	if (ret) {
		dev_err(dev, "Failed to clear PFE HIF coherency\n");
		return ret;
	}

	return 0;
}

int pfeng_is_ip_ready_get_nvmem_cell(struct udevice *dev, bool *is_ready)
{
	u32 value = 0;
	int ret;

	ret = pfeng_read_nvmem_cell(dev, "pfe_genctrl3", &value);
	if (ret)
		return ret;

	*is_ready = !!(value);

	return 0;
}
