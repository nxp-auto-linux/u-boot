// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2021 NXP
 */

#include <asm/arch/s32-gen1/mc_me_regs.h>

u8 mc_me_core2prtn_core_id(u8 part, u8 id)
{
	/**
	 * A map where the key is core id obtained from MPIDR and the
	 * value represents the ID of the core in MC_ME.PRTN1_CORE*
	 */
	static const u8 mc_me_a53_core_id[] = {
		/* Cluster 0, core 0 */
		[0] = 0,
		/* Cluster 0, core 1 */
		[1] = 1,
		/* Cluster 0, core 2 */
		[2] = 4,
		/* Cluster 0, core 3 */
		[3] = 5,
		/* Cluster 1, core 0 */
		[4] = 2,
		/* Cluster 1, core 1 */
		[5] = 3,
		/* Cluster 1, core 2 */
		[6] = 6,
		/* Cluster 1, core 3 */
		[7] = 7,
	};

	static const u8 mc_me_m7_core_id[] = {
		/* Core 0 */
		[0] = 0,
		[1] = 1,
		[2] = 2,
		/* Core 3 */
		[3] = 4,
	};

	if (part == MC_ME_CORES_PRTN)
		return mc_me_a53_core_id[id];

	return mc_me_m7_core_id[id];
}

u8 get_rgm_a53_bit(u8 core)
{
	static u8 periph_rgm_cores[] = {
		/** Cluster 0, core 0*/
		[0] = 65,
		[1] = 66,
		[2] = 69,
		[3] = 70,
		/** Cluster 1, core 0*/
		[4] = 67,
		[5] = 68,
		[6] = 71,
		[7] = 72,
	};

	return BIT(periph_rgm_cores[core] % 64);
}
