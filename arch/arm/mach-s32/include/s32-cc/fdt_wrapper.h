/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2022 NXP
 */
#ifndef S32CC_FDT_WRAPPER_H
#define S32CC_FDT_WRAPPER_H

#include <linux/libfdt.h>

static inline int get_next_cpu(const void *blob, int off)
{
	return fdt_node_offset_by_prop_value(blob, off,
			"device_type", "cpu", 4);
}

#endif
