// SPDX-License-Identifier: BSD-3-Clause
/*
 * HSE ABI for secure boot in u-boot
 *
 * Copyright 2020 NXP
 */

#include <hse/hse_abi.h>

/* see include/mmc.h and cmd/mmc.c */
static int curr_device;

struct mmc *hse_init_mmc_device(int dev, bool force_init)
{
	struct mmc *mmc;

	mmc = find_mmc_device(dev);

	if (!mmc) {
		log_err("ERROR: no mmc device at slot %x!\n", dev);
		return NULL;
	}

	if (force_init)
		mmc->has_init = 0;

	if (mmc_init(mmc))
		return NULL;

	return mmc;
}

int hse_mmc_read(void *addr, u32 blk, u32 cnt)
{
	struct mmc *mmc;
	u32 n;

	curr_device = 0;

	mmc = hse_init_mmc_device(curr_device, false);
	if (!mmc) {
		log_err("ERROR: MMC init failed!\n");
		return CMD_RET_FAILURE;
	}

	printf("\tMMC read: dev # %d, block # %d, count %d ...",
	       curr_device, blk, cnt);

	n = blk_dread(mmc_get_blk_desc(mmc), blk, cnt, addr);
	printf("%d blocks read: %s\n", n, (n == cnt) ? "OK" : "ERROR");

	return (n == cnt) ? 0 : -1;
}

int hse_mmc_write(void *addr, u32 blk, u32 cnt)
{
	struct mmc *mmc;
	u32 n;

	mmc = hse_init_mmc_device(curr_device, false);
	if (!mmc) {
		log_err("ERROR: MMC init failed!\n");
		return -1;
	}

	printf("\tMMC write: dev # %d, block # %d, count %d...",
	       curr_device, blk, cnt);

	if (mmc_getwp(mmc) == 1) {
		log_err("ERROR: card is write protected!\n");
		return -1;
	}

	n = blk_dwrite(mmc_get_blk_desc(mmc), blk, cnt, addr);
	printf("%d blocks written: %s\n", n, (n == cnt) ? "OK" : "ERROR");

	return (n == cnt) ? 0 : -1;
}

