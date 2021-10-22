// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017, 2020-2021 NXP
 *
 * Contains firmware in octet string format for SJA1105.
 */

#include <common.h>
#include <errno.h>
#include <sja1105_cfg.h>

extern struct sja1105_cfgs_s *sja1105_cfgs;

int sja1105_get_cfg(u32 devid, u32 cs, u32 *bin_len, const u8 **cfg_bin)
{
	int i = 0;
	while (sja1105_cfgs[i].cfg_bin) {
		if (sja1105_cfgs[i].devid == devid &&
		    sja1105_cfgs[i].cs == cs) {
			*bin_len = sja1105_cfgs[i].bin_len;
			*cfg_bin = sja1105_cfgs[i].cfg_bin;
			return 0;
		}
		i++;
	}

	*bin_len = 0;
	*cfg_bin = NULL;

	printf("No matching device ID found for devid %X, cs %d.\n", devid, cs);

	return -EINVAL;
}

int sja1105_fw_set_load(u32 devid, u32 cs)
{
	int i = 0;

	while (sja1105_cfgs[i].cfg_bin) {
		if (sja1105_cfgs[i].devid == devid &&
		    sja1105_cfgs[i].cs == cs) {
			sja1105_cfgs[i].loaded = true;
			return 0;
		}
		i++;
	}

	printf("No matching device ID found for devid %X, cs %d.\n", devid, cs);

	return -EINVAL;
}

int sja1105_fw_load_check(u32 devid, u32 cs)
{
	int i = 0;

	while (sja1105_cfgs[i].cfg_bin) {
		if (sja1105_cfgs[i].devid == devid &&
		    sja1105_cfgs[i].cs == cs) {
			if (sja1105_cfgs[i].loaded)
				return 1;
			else
				return 0;
		}
		i++;
	}

	printf("No matching device ID found for devid %X, cs %d.\n", devid, cs);

	return -EINVAL;
}
