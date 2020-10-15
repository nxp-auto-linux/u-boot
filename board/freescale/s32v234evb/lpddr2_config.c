// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <asm/arch/lpddr2.h>

static const struct lpddr2_config s32v_lpddr2_config = {
	.mdasp_module0 = 0x00000048,
	.mdasp_module1 = 0x00000068,
	.mdcfg0 = 0x464F61A5,
	.mdcfg1 = 0x00180E63,
	.mdcfg2 = 0x000000DD,
	.mdcfg3lp = 0x001F099B,
	.mdctl = 0x03010000,
	.mdmisc = 0x000017C8,
	.mdscr_mr2 = 0x06028030,
	.mprddlctl_module0 = 0x4D4B4F4B,
	.mprddlctl_module1 = 0x49484848,
	.mpwrdlctl_module0 = 0x38383737,
	.mpwrdlctl_module1 = 0x3E403E3F,
};

const struct lpddr2_config *s32_get_lpddr2_config(void)
{
	return &s32v_lpddr2_config;
}
