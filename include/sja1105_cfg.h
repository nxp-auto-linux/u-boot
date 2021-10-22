/* SPDX-License-Identifier:    GPL-2.0+
 *
 * Copyright 2019-2021 NXP
 *
 */


struct sja1105_cfgs_s {
	u32 devid;
	u32 cs;
	u32 bin_len;
	const u8 *cfg_bin;
	bool loaded;
};

