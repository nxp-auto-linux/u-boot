// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2019 NXP
 */
#ifndef S32GEN1_CLOCK_H
#define S32GEN1_CLOCK_H

void s32gen1_setup_fxosc(void);
void s32gen1_enable_partition_blocks(u32 partition_n, u32 *blocks,
				     size_t n_blocks);
int s32gen1_program_pll(enum pll_type pll, u32 refclk_freq, u32 phi_nr,
			u64 freq[], u32 dfs_nr, u32 dfs[][DFS_PARAMS_Nr],
			u32 plldv_rdiv, u32 plldv_mfi, u32 pllfd_mfn);

#endif
