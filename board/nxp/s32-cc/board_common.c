// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022-2023 NXP
 */
#include <common.h>
#include <fdtdec.h>
#include <image.h>
#include <asm/u-boot.h>
#include <s32-cc/scmi_reset_agent.h>

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

void board_cleanup_before_linux(void)
{
	int ret;

	ret = scmi_reset_agent();
	if (ret)
		pr_err("Failed to reset SCMI agent's settings\n");
}
