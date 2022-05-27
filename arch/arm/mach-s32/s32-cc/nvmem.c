// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022 NXP
 */
#include <common.h>
#include <misc.h>
#include <dm/device.h>
#include <dm/of_access.h>
#include <dm/ofnode.h>
#include <dm/read.h>
#include <s32-cc/nvmem.h>

int nvmem_cell_get_by_offset(struct udevice *dev, int offset,
			     struct nvmem_cell *cell)
{
	fdt_addr_t cell_addr, cell_size;
	ofnode subnode;
	bool found = false;

	if (!cell)
		return -EINVAL;

	ofnode_for_each_subnode(subnode, dev_ofnode(dev)) {
		cell_addr = ofnode_get_addr_size_index_notrans(subnode, 0,
							       &cell_size);
		if (cell_addr == FDT_ADDR_T_NONE)
			continue;

		if ((int)cell_addr == offset) {
			found = true;
			break;
		}
	}

	if (!found)
		return -EINVAL;

	cell->size = (int)cell_size;
	cell->offset = offset;
	cell->dev = dev;

	return 0;
}

int nvmem_cell_get_by_index(struct udevice *dev, int index,
			    struct nvmem_cell *cell)
{
	int ret = 0;
	struct ofnode_phandle_args args;
	fdt_addr_t cell_addr, cell_size;
	ofnode parent_ofnode;

	debug("%s(dev=%p, index=%d, cell=%p)\n", __func__, dev, index, cell);

	if (!cell)
		return -EINVAL;

	cell->dev = NULL;

	ret = dev_read_phandle_with_args(dev, "nvmem-cells", NULL,
					 0, index, &args);
	if (ret) {
		debug("%s: dev_read_phandle_with_args failed: err=%d\n",
		      __func__, ret);
		return ret;
	}

	parent_ofnode = ofnode_get_parent(args.node);
	ret = uclass_get_device_by_ofnode(UCLASS_MISC, parent_ofnode,
					  &cell->dev);
	if (ret) {
		debug("%s: uclass_get_device_by_ofnode failed: err=%d\n",
		      __func__, ret);
		return ret;
	}

	cell_addr = ofnode_get_addr_size_index_notrans(args.node, 0,
						       &cell_size);
	if (cell_addr == FDT_ADDR_T_NONE)
		return -ENOENT;

	cell->offset = (int)cell_addr;
	cell->size = (int)cell_size;

	return 0;
}

int nvmem_cell_get(struct udevice *dev, const char *name,
		   struct nvmem_cell *cell)
{
	int index;

	index = dev_read_stringlist_search(dev, "nvmem-cell-names", name);
	if (index < 0) {
		pr_err("dev_read_stringlist_search() failed: %d\n", index);
		return index;
	}

	return nvmem_cell_get_by_index(dev, index, cell);
}

int nvmem_cell_read(struct nvmem_cell *cell, void *buf, size_t size)
{
	int ret;

	if (!cell || !buf)
		return -EINVAL;

	if (size != cell->size)
		return -EINVAL;

	ret = misc_read(cell->dev, cell->offset, buf, cell->size);
	if (ret != cell->size)
		return -EINVAL;

	return 0;
}

