// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2018,2020-2021 NXP
 */

#include <asm/arch-s32/soc.h>
#include <common.h>
#include <cpu_func.h>
#include <linux/kernel.h>
#include <sram.h>

static int do_init_sram(cmd_tbl_t *cmdtp, int flag, int argc,
			char * const argv[])
{
	unsigned long addr;
	unsigned int size, max_size, ret;
	char *ep;

	if (argc < 3)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], &ep, 16);
	if (ep == argv[1] || *ep != '\0')
		return CMD_RET_USAGE;

	size = simple_strtoul(argv[2], &ep, 16);
	if (ep == argv[2] || *ep != '\0')
		return CMD_RET_USAGE;

	if (!IS_ALIGNED(addr, 32)) {
		printf("ERROR: Address 0x%08lX is not 32 byte aligned ...\n",
		       addr);
		return CMD_RET_USAGE;
	}

	if (!IS_ALIGNED(size, 32)) {
		printf("ERROR: size 0x%08X is not a 32 byte multiple ...\n",
		       size);
		return CMD_RET_USAGE;
	}

	if (!is_addr_in_sram(addr)) {
		printf("ERROR: Address 0x%08lX not in internal SRAM ...\n",
		       addr);
		return CMD_RET_USAGE;
	}

	max_size = get_sram_size() - (addr - S32_SRAM_BASE);
	if (size > max_size) {
		printf("WARNING: given size exceeds SRAM boundaries.\n");
		size = max_size;
	}

	invalidate_dcache_range(addr, addr + size);
	ret = sram_clr(addr, size);
	if (!ret) {
		printf("Init SRAM failed\n");
		return CMD_RET_FAILURE;
	}

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
		initsram,	3,	1,	do_init_sram,
		"Initialize SRAM from address",
		"startAddress[hex] size[hex]"
	  );

