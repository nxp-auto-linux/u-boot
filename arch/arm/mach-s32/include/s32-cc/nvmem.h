/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2022 NXP
 */
#ifndef NVMEM_H
#define NVMEM_H

struct udevice;

struct nvmem_cell {
	struct udevice *dev;
	int offset;
	int size;
};

int nvmem_cell_get_by_offset(struct udevice *dev, int offset,
			     struct nvmem_cell *cell);

int nvmem_cell_get_by_index(struct udevice *dev, int index,
			    struct nvmem_cell *cell);

int nvmem_cell_get(struct udevice *dev, const char *name,
		   struct nvmem_cell *cell);

int nvmem_cell_read(struct nvmem_cell *cell, void *buf, size_t size);
#endif
