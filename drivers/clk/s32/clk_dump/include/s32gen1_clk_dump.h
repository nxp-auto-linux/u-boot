/* SPDX-License-Identifier: BSD-3-Clause */
/*
 * Copyright 2020 NXP
 */
#ifndef S32GEN1_CLK_DUMP_H
#define S32GEN1_CLK_DUMP_H

#include <compiler.h>

#define S32GEN1_CLK_PREFIX	"S32GEN1_CLK_"

#define S32GEN1_CLK_NAME_INIT(ID, NAME) \
{                                       \
	.id = (ID),                     \
	.name = (NAME),                 \
}                                       \

struct s32gen1_clk_blk {
	ulong id;
	const char *name;
};

struct s32gen1_clk_blk *s32gen1_get_clk_blk(u32 id);
const char *s32gen1_get_clock_name(struct s32gen1_clk_blk *clk);

struct s32gen1_clk_blk *s32gen1_get_plat_clk_blk(u32 id);
const char *s32gen1_get_plat_clock_name(struct s32gen1_clk_blk *clk);

#endif
