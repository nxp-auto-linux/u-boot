// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2021 NXP
 */
#include <dt-bindings/clock/s32g-clock.h>
#include <dt-bindings/clock/s32g3-clock.h>
#include <linux/kernel.h>
#include <s32g_clk_dump.h>

#define CLK_NAME(ID_DEF)     \
	S32GEN1_CLK_NAME_INIT(ID_DEF, #ID_DEF)

static struct s32gen1_clk_blk s32g398a_clk_blks[] = {
	CLK_NAME(S32G_CLK_MC_CGM6_MUX0),
	CLK_NAME(S32G_CLK_MC_CGM6_MUX1),
	CLK_NAME(S32G_CLK_MC_CGM6_MUX2),
	CLK_NAME(S32G_CLK_MC_CGM6_MUX3),
};

struct s32gen1_clk_blk *s32g_get_plat_clk_blk(u32 id)
{
	if (id < ARRAY_SIZE(s32g398a_clk_blks))
		return &s32g398a_clk_blks[id];

	return NULL;
}

