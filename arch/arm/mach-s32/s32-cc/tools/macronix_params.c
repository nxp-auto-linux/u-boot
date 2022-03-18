// SPDX-License-Identifier: GPL-2.0+
/* Copyright 2019-2022 NXP */

#include <s32cc_image_params.h>

static struct qspi_params macronix_qspi_conf = {
	.header   = 0x5a5a5a5a,
	.mcr      = 0x030f00cc,
	.flshcr   = 0x00010303,
	.bufgencr = 0x00000000,
	.dllcr    = 0xc280000c,
	.paritycr = 0x00000000,
	.sfacr    = 0x00020000,
	.smpr     = 0x44000000,
	.dlcr     = 0x40ff40ff,
	.sflash_1_size = 0x20000000,
	.sflash_2_size = 0x20000000,
	.dlpr = 0xaa553443,
	.sfar = 0x00000000,
	.ipcr = 0x00000000,
	.tbdr = 0x00000000,
	.dll_bypass_en   = 0x00,
	.dll_slv_upd_en  = 0x00,
	.dll_auto_upd_en = 0x01,
	.ipcr_trigger_en = 0x00,
	.sflash_clk_freq = 200,
	.reserved = {0x00, 0x00, 0x00},
	/* Macronix read - 8DTRD */
	.command_seq = {0x471147ee,
			0x0f142b20,
			0x00003b10},
	.writes = {
		{
			/* Write enable */
			.config = {
				.valid_addr = 0,
				.cdata_size = 0,
				.addr_size = 0,
				.pad = 0,
				.reserved = 0,
				.opcode = 6,
			},
			.addr = 0,
			.data = 0,
		},
		{
			/* WRCR2 - DTR OPI */
			.config = {
				.valid_addr = 1,
				.cdata_size = 1,
				.addr_size = 32,
				.pad = 0,
				.reserved = 0,
				.opcode = 0x72,
			},
			.addr = 0x0,
			.data = 0x2,
		},
	},
};

struct qspi_params *get_macronix_qspi_conf(void)
{
	return &macronix_qspi_conf;
}
