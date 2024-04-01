// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022-2024 NXP
 */
#include <common.h>
#include <fdtdec.h>
#include <image.h>

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	return fdtdec_setup_mem_size_base();
}

int dram_init_banksize(void)
{
	return fdtdec_setup_memory_banksize();
}

void *board_fdt_blob_setup(int *err)
{
	void *dtb;

	dtb = (void *)(CONFIG_SYS_TEXT_BASE - CONFIG_S32_MAX_DTB_SIZE);

	*err = 0;
	if (fdt_magic(dtb) != FDT_MAGIC)
		*err = -EFAULT;

	return dtb;
}
