// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <common.h>
#include <asm/arch/lpddr2.h>

/* The magic values used below are generated using DDR Tool */

static const struct lpddr2_config s32v_lpddr2_config = {
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
	.mprddlctl_module0 = 0x44463E3E,
	.mprddlctl_module1 = 0x4444403E,
	.mpwrdlctl_module0 = 0x42464246,
	.mpwrdlctl_module1 = 0x3A3E3C3C,
	.mpdgctrl0_module0 = 0x20000000,
	.mpdgctrl1_module0 = 0x00000000,
	.mpdgctrl0_module1 = 0x20000000,
	.mpdgctrl1_module1 = 0x00000000,
	.frequency = 533,
};

const struct lpddr2_config *s32_get_lpddr2_config(void)
{
	return &s32v_lpddr2_config;
}
