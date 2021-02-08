// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2021 NXP
 */

#include <common.h>
#include <asm/arch/lpddr2.h>
#include <campps32v2.h>

// The magic values used below are generated using a DDR Tool

static const struct lpddr2_config campps32v2_lpddr2_config_1 = {
	.mdasp_module0 = 0x0000004F,
	.mdasp_module1 = 0x0000006F,
	.mdcfg0 = 0x33374133,
	.mdcfg1 = 0xDAF00A82,
	.mdcfg2 = 0x00000093,
	.mdcfg3lp = 0x00170777,
	.mdctl = 0x03110000,
	.mdmisc = 0x00001688,
	.mdscr_mr2 = 0x04028030,
	.mdscr_mr3 = 0x04038030,
	.mprddlctl_module0 = 0x42464242,
	.mprddlctl_module1 = 0x3E423E3E,
	.mpwrdlctl_module0 = 0x44464444,
	.mpwrdlctl_module1 = 0x3E3E3C40,
	.mpdgctrl0_module0 = 0x2C7C0C7C,
	.mpdgctrl1_module0 = 0x0C7C0C7C,
	.mpdgctrl0_module1 = 0x2C7C0C7C,
	.mpdgctrl1_module1 = 0x0C7C0C7C,
	.frequency = 400,
};

static const struct lpddr2_config campps32v2_lpddr2_config_2 = {
	.mdasp_module0 = 0x0000004F,
	.mdasp_module1 = 0x0000006F,
	.mdcfg0 = 0x454F61A5,
	.mdcfg1 = 0x93F60EA3,
	.mdcfg2 = 0x000000DD,
	.mdcfg3lp = 0x001F0999,
	.mdctl = 0x03110000,
	.mdmisc = 0x00001688,
	.mdscr_mr2 = 0x06028030,
	.mdscr_mr3 = 0x06038030,
	.mprddlctl_module0 = 0x46464042,
	.mprddlctl_module1 = 0x4648443E,
	.mpwrdlctl_module0 = 0x4242443E,
	.mpwrdlctl_module1 = 0x4240403C,
	.mpdgctrl0_module0 = 0x2C7C0C7C,
	.mpdgctrl1_module0 = 0x0C7C0C7C,
	.mpdgctrl0_module1 = 0x2C7C0C7C,
	.mpdgctrl1_module1 = 0x0C7C0C7C,
	.frequency = 533,
};

static const struct lpddr2_config campps32v2_lpddr2_config_3 = {
	.mdasp_module0 = 0x0000004F,
	.mdasp_module1 = 0x0000006F,
	.mdcfg0 = 0x454F61A5,
	.mdcfg1 = 0x93F60EA3,
	.mdcfg2 = 0x000000DD,
	.mdcfg3lp = 0x00240AAD,
	.mdctl = 0x03110000,
	.mdmisc = 0x00001688,
	.mdscr_mr2 = 0x06028030,
	.mdscr_mr3 = 0x06038030,
	.mprddlctl_module0 = 0x44464042,
	.mprddlctl_module1 = 0x44484444,
	.mpwrdlctl_module0 = 0x42424040,
	.mpwrdlctl_module1 = 0x423E3A3E,
	.mpdgctrl0_module0 = 0x2C7C0C7C,
	.mpdgctrl1_module0 = 0x0C7C0C7C,
	.mpdgctrl0_module1 = 0x2C7C0C7C,
	.mpdgctrl1_module1 = 0x0C7C0C7C,
	.frequency = 533,
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
	.mdscr_mr3 = 0x01038030,
	.mprddlctl_module0 = 0x42463E40,
	.mprddlctl_module1 = 0x42444240,
	.mpwrdlctl_module0 = 0x42444242,
	.mpwrdlctl_module1 = 0x3E403E40,
	.mpdgctrl0_module0 = 0x2C7C0C7C,
	.mpdgctrl1_module0 = 0x0C7C0C7C,
	.mpdgctrl0_module1 = 0x2C7C0C7C,
	.mpdgctrl1_module1 = 0x0C7C0C7C,
	.frequency = 400,
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
	.mdscr_mr3 = 0x01038030,
	.mprddlctl_module0 = 0x44483E46,
	.mprddlctl_module1 = 0x4648423E,
	.mpwrdlctl_module0 = 0x484A4844,
	.mpwrdlctl_module1 = 0x3C3E3C3C,
	.mpdgctrl0_module0 = 0x2C7C0C7C,
	.mpdgctrl1_module0 = 0x0C7C0C7C,
	.mpdgctrl0_module1 = 0x2C7C0C7C,
	.mpdgctrl1_module1 = 0x0C7C0C7C,
	.frequency = 533,
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
	.mdscr_mr3 = 0x01038030,
	.mprddlctl_module0 = 0x42443E44,
	.mprddlctl_module1 = 0x464A4444,
	.mpwrdlctl_module0 = 0x48484444,
	.mpwrdlctl_module1 = 0x403E3E40,
	.mpdgctrl0_module0 = 0x2C7C0C7C,
	.mpdgctrl1_module0 = 0x0C7C0C7C,
	.mpdgctrl0_module1 = 0x2C7C0C7C,
	.mpdgctrl1_module1 = 0x0C7C0C7C,
	.frequency = 533,
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
