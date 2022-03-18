// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022 NXP
 */
#include <common.h>
#include <asm/u-boot.h>
#include <fdtdec.h>

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

	dtb = (void *)(CONFIG_SYS_TEXT_BASE - CONFIG_S32CC_MAX_DTB_SIZE);

	if (fdt_magic(dtb) != FDT_MAGIC)
		*err = -EFAULT;

	return dtb;
}
