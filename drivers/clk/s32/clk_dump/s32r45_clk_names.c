// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2020-2021 NXP
 */
#include <dt-bindings/clock/s32r45-clock.h>
#include <linux/kernel.h>
#include <s32gen1_clk_dump.h>

#define S32R45_CLK_PREFIX	"S32R45_CLK_"

#define CLK_NAME(ID_DEF)     \
	S32GEN1_CLK_NAME_INIT(ID_DEF, #ID_DEF)

static struct s32gen1_clk_blk s32r45_clk_blks[] = {
	CLK_NAME(S32GEN1_CLK_PER),
	CLK_NAME(S32GEN1_CLK_CAN_PE),
	CLK_NAME(S32R45_CLK_ACCEL_PLL_PHI0),
	CLK_NAME(S32GEN1_CLK_ARM_PLL_DFS4),
	CLK_NAME(S32R45_CLK_ARM_PLL_DFS4_2),
	CLK_NAME(S32R45_CLK_GMAC1_EXT_TX),
	CLK_NAME(S32R45_CLK_GMAC1_EXT_RX),
	CLK_NAME(S32R45_CLK_GMAC1_EXT_REF),
	CLK_NAME(S32R45_CLK_SERDES1_LANE0_TX),
	CLK_NAME(S32R45_CLK_SERDES1_LANE0_CDR),
	CLK_NAME(S32R45_CLK_GMAC1_REF_DIV),
	CLK_NAME(S32R45_CLK_MC_CGM2_MUX0),
	CLK_NAME(S32R45_CLK_ACCEL3),
	CLK_NAME(S32R45_CLK_ACCEL3_DIV3),
	CLK_NAME(S32R45_CLK_MC_CGM2_MUX1),
	CLK_NAME(S32R45_CLK_ACCEL4),
	CLK_NAME(S32R45_CLK_MC_CGM2_MUX2),
	CLK_NAME(S32R45_CLK_GMAC1_TX),
	CLK_NAME(S32R45_CLK_MC_CGM2_MUX3),
	CLK_NAME(S32R45_CLK_MC_CGM2_MUX4),
	CLK_NAME(S32R45_CLK_GMAC1_RX),
};

const char *plat_clk_name(const char *name)
{
	static size_t s32gen1_len = sizeof(S32GEN1_CLK_PREFIX) - 1;
	static size_t s32r45_len = sizeof(S32R45_CLK_PREFIX) - 1;

	if (!name)
		return NULL;

	if (!strncmp(name, S32GEN1_CLK_PREFIX, s32gen1_len))
		return name + s32gen1_len;

	return name + s32r45_len;
}

struct s32gen1_clk_blk *s32gen1_get_plat_clk_blk(u32 id)
{
	if (id < ARRAY_SIZE(s32r45_clk_blks))
		return &s32r45_clk_blks[id];

	return NULL;
}

const char *s32gen1_get_plat_clock_name(struct s32gen1_clk_blk *clk)
{
	if (!clk)
		return NULL;

	return plat_clk_name(clk->name);
}

