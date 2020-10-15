// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */

#include <common.h>
#include <asm/arch/lpddr2.h>
#include <campps32v2.h>

static const struct lpddr2_config campps32v2_lpddr2_config_1 = {
	.mdasp_module0 = 0x0000004F,
	.mdasp_module1 = 0x0000006F,
	.mdcfg0 = 0x454F61A5,
	.mdcfg1 = 0x93F60EA3,
	.mdcfg2 = 0x000000DD,
	.mdcfg3lp = 0x001F0999,
	.mdctl = 0x03110000,
	.mdmisc = 0x00001688,
	.mdscr_mr2 = 0x06028030,
	.mprddlctl_module0 = 0x46484646,
	.mprddlctl_module1 = 0x42464042,
	.mpwrdlctl_module0 = 0x46484240,
	.mpwrdlctl_module1 = 0x3C3E403E,
};

static const struct lpddr2_config campps32v2_lpddr2_config_2 = {
	.mdasp_module0 = 0x0000004F,
	.mdasp_module1 = 0x0000006F,
	.mdcfg0 = 0x33374133,
	.mdcfg1 = 0xDAF00A82,
	.mdcfg2 = 0x00000093,
	.mdcfg3lp = 0x00170777,
	.mdctl = 0x03110000,
	.mdmisc = 0x00001688,
	.mdscr_mr2 = 0x04028030,
	.mprddlctl_module0 = 0x46464040,
	.mprddlctl_module1 = 0x46484042,
	.mpwrdlctl_module0 = 0x44444442,
	.mpwrdlctl_module1 = 0x423E3E3C,
};

static const struct lpddr2_config campps32v2_lpddr2_config_3 = {
	.mdasp_module0 = 0x0000004F,
	.mdasp_module1 = 0x0000006F,
	.mdcfg0 = 0x33374133,
	.mdcfg1 = 0xDAF00A82,
	.mdcfg2 = 0x00000093,
	.mdcfg3lp = 0x00170777,
	.mdctl = 0x03110000,
	.mdmisc = 0x00001688,
	.mdscr_mr2 = 0x04028030,
	.mprddlctl_module0 = 0x42444042,
	.mprddlctl_module1 = 0x42464644,
	.mpwrdlctl_module0 = 0x44444444,
	.mpwrdlctl_module1 = 0x4040403E,
};

static const struct lpddr2_config campps32v2_lpddr2_config_4 = {
	.mdasp_module0 = 0x0000004F,
	.mdasp_module1 = 0x0000006F,
	.mdcfg0 = 0x33374133,
	.mdcfg1 = 0xDAF00A82,
	.mdcfg2 = 0x00000093,
	.mdcfg3lp = 0x00170777,
	.mdctl = 0x03110000,
	.mdmisc = 0x00001688,
	.mdscr_mr2 = 0x04028030,
	.mprddlctl_module0 = 0x44463E42,
	.mprddlctl_module1 = 0x40444442,
	.mpwrdlctl_module0 = 0x42444244,
	.mpwrdlctl_module1 = 0x403E3E3C,
};

static const struct lpddr2_config campps32v2_lpddr2_config_5 = {
	.mdasp_module0 = 0x0000004F,
	.mdasp_module1 = 0x0000006F,
	.mdcfg0 = 0x454F61A5,
	.mdcfg1 = 0x93F60EA3,
	.mdcfg2 = 0x000000DD,
	.mdcfg3lp = 0x001F0999,
	.mdctl = 0x03110000,
	.mdmisc = 0x00001688,
	.mdscr_mr2 = 0x06028030,
	.mprddlctl_module0 = 0x46464244,
	.mprddlctl_module1 = 0x42463E3E,
	.mpwrdlctl_module0 = 0x484A4846,
	.mpwrdlctl_module1 = 0x3C3C3C3E,
};

static const struct lpddr2_config campps32v2_lpddr2_config_6 = {
	.mdasp_module0 = 0x0000004F,
	.mdasp_module1 = 0x0000006F,
	.mdcfg0 = 0x454F61A5,
	.mdcfg1 = 0x93F60EA3,
	.mdcfg2 = 0x000000DD,
	.mdcfg3lp = 0x001F0999,
	.mdctl = 0x03110000,
	.mdmisc = 0x00001688,
	.mdscr_mr2 = 0x06028030,
	.mprddlctl_module0 = 0x46484848,
	.mprddlctl_module1 = 0x484A4444,
	.mpwrdlctl_module0 = 0x46464240,
	.mpwrdlctl_module1 = 0x3C3C3C3E,
};

const struct lpddr2_config *s32_get_lpddr2_config(void)
{
	switch (campps32v2_get_device_id()) {
	case 1:
		return &campps32v2_lpddr2_config_1;
	case 2:
		return &campps32v2_lpddr2_config_2;
	case 3:
		return &campps32v2_lpddr2_config_3;
	case 4:
		return &campps32v2_lpddr2_config_4;
	case 5:
		return &campps32v2_lpddr2_config_5;
	case 6:
		return &campps32v2_lpddr2_config_6;
	default:
		pr_err("DDR: config not fund\nInvalid SoC ID: %d",
		       campps32v2_get_device_id());
		return NULL;
	}
}
