// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2022, 2024 NXP
 */

#include <common.h>
#include <env.h>
#include <fdt_support.h>
#include <asm/global_data.h>
#include <dm/uclass.h>
#include <linux/ioport.h>
#include <linux/kernel.h>
#include <linux/libfdt.h>
#include <s32/fdt.h>

#define S32_DDR_LIMIT_VAR	"ddr_limitX"

static int apply_memory_fixups(void *blob, struct bd_info *bd)
{
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	int ret, bank, banks = 0;

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		if (!bd->bi_dram[bank].start && !bd->bi_dram[bank].size)
			continue;

		start[banks] = bd->bi_dram[bank].start;
		size[banks] = bd->bi_dram[bank].size;
		banks++;
	}

	ret = fdt_fixup_memory_banks(blob, start, size, banks);
	if (ret)
		log_err("s32-fdt: Failed to set memory banks\n");

	return ret;
}

static void apply_ddr_limits(struct bd_info *bd)
{
	static const size_t var_len = sizeof(S32_DDR_LIMIT_VAR);
	static const size_t digit_pos = var_len - 2;
	char ddr_limit[var_len], *var_val;
	u64 start, end, limit;
	int bank;

	memcpy(ddr_limit, S32_DDR_LIMIT_VAR, var_len);

	ddr_limit[digit_pos] = '0';
	while ((var_val = env_get(ddr_limit))) {
		limit = simple_strtoull(var_val, NULL, 16);

		for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
			start = bd->bi_dram[bank].start;
			end = start + bd->bi_dram[bank].size;

			if (limit >= start && limit < end)
				bd->bi_dram[bank].size = limit - start;
		}

		if (ddr_limit[digit_pos] >= '9')
			break;

		ddr_limit[digit_pos]++;
	};
}

int ft_fixup_memory(void *blob, struct bd_info *bd)
{
	apply_ddr_limits(bd);

	return apply_memory_fixups(blob, bd);
}

static int add_atf_reserved_memory(void *new_blob)
{
	int ret;
	struct fdt_memory carveout;
	struct resource reg = {0};
	ofnode node;

	if (fdt_check_header(new_blob)) {
		log_err("Invalid FDT Header for Linux DT Blob\n");
		return -EINVAL;
	}

	/* Get atf reserved-memory node offset */
	node = ofnode_path("/reserved-memory/atf");
	if (!ofnode_valid(node)) {
		log_err("Couldn't find 'atf' reserved-memory node\n");
		return -EINVAL;
	}

	ret = ofnode_read_resource(node, 0, &reg);
	if (ret) {
		log_err("Unable to get value of 'reg' prop of 'atf' node\n");
		return ret;
	}

	carveout.start = reg.start;
	carveout.end = reg.end;

	/* Increase Linux DT size before adding new node */
	ret = fdt_increase_size(new_blob, 512);
	if (ret < 0) {
		log_err("Could not increase size of Linux DT: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	/* Add 'atf' node to Linux DT */
	ret = fdtdec_add_reserved_memory(new_blob, "atf", &carveout,
					 NULL, 0, NULL,
					 FDTDEC_RESERVED_MEMORY_NO_MAP);
	if (ret < 0) {
		log_err("Unable to add 'atf' node to Linux DT\n");
		return ret;
	}

	return 0;
}

int ft_fixup_atf(void *new_blob)
{
	int ret = add_atf_reserved_memory(new_blob);

	if (ret)
		log_err("Copying 'atf' node from U-Boot DT to Linux DT failed!\n");

	return ret;
}
