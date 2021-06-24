// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2021 NXP
 */
#include "s32gen1_ocotp.h"
#include <dm/device.h>
#include <dm/fdtaddr.h>
#include <linux/bitops.h>
#include <linux/errno.h>
#include <log.h>
#include <misc.h>
#include <asm/io.h>

#define OCOTP_WORD(X)		BIT(X)
#define OCOTP_WORD_RANGE(L, H)	GENMASK(H, L)

#define CTRL_SYS		0x0
#define   CTRL_AUTH_KEY		(0x12 << 16)
#define   CTRL_RD_WR(X)		((X) << 2)
#define   CTRL_READ_FUSE	1
#define ADDR_SYS		0x4
#define RDATA_SYS		0xC
#define STATUS_SYS		0x50
#define   STATUS_BUSY		BIT(0)
#define   STATUS_CRC_FAIL	BIT(1)
#define   STATUS_ERROR		BIT(2)

struct s32gen1_fuse_map {
	const ulong *map;
	size_t n_banks;
};

struct s32gen1_ocotp_platdata {
	fdt_addr_t base;
	const struct s32gen1_fuse_map *fuses;
};

static const ulong s32g_fuse_map[] = {
	[0] = OCOTP_WORD_RANGE(2, 6),
	[1] = OCOTP_WORD_RANGE(5, 7),
	[2] = OCOTP_WORD_RANGE(0, 4),
	[4] = OCOTP_WORD(6),
	[5] = OCOTP_WORD_RANGE(1, 2),
	[6] = OCOTP_WORD(7),
	[7] = OCOTP_WORD_RANGE(0, 1),
	[11] = OCOTP_WORD_RANGE(0, 7),
	[12] = OCOTP_WORD_RANGE(0, 2) | OCOTP_WORD(7),
	[13] = OCOTP_WORD_RANGE(2, 4),
	[14] = OCOTP_WORD(1) | OCOTP_WORD(4) | OCOTP_WORD(5),
	[15] = OCOTP_WORD_RANGE(5, 7),
};

static const struct s32gen1_fuse_map s32g_map = {
	.map = s32g_fuse_map,
	.n_banks = ARRAY_SIZE(s32g_fuse_map),
};

static const ulong s32r45_fuse_map[] = {
	[0] = OCOTP_WORD_RANGE(2, 6),
	[1] = OCOTP_WORD_RANGE(5, 7),
	[2] = OCOTP_WORD_RANGE(0, 4),
	[5] = OCOTP_WORD_RANGE(1, 2),
	[6] = OCOTP_WORD(7),
	[7] = OCOTP_WORD_RANGE(0, 1),
	[11] = OCOTP_WORD_RANGE(0, 7),
	[12] = OCOTP_WORD_RANGE(0, 2) | OCOTP_WORD(7),
	[13] = OCOTP_WORD_RANGE(2, 4),
	[14] = OCOTP_WORD(1) | OCOTP_WORD(4) | OCOTP_WORD(5),
	[15] = OCOTP_WORD_RANGE(5, 7),
};

static const struct s32gen1_fuse_map s32r45_map = {
	.map = s32r45_fuse_map,
	.n_banks = ARRAY_SIZE(s32r45_fuse_map),
};

static u32 get_bank_index(int offset)
{
	return (offset - S32GEN1_OCOTP_BANK_OFFSET) / S32GEN1_OCOTP_BANK_SIZE;
}

static u32 get_word_index(int offset)
{
	return offset % S32GEN1_OCOTP_BANK_SIZE / S32GEN1_OCOTP_WORD_SIZE;
}

static bool is_valid_word(const struct s32gen1_fuse_map *map,
			  u32 bank, u32 word)
{
	if (bank >= map->n_banks)
		return false;

	return !!(map->map[bank] & OCOTP_WORD(word));
}

static u32 wait_if_busy(fdt_addr_t base)
{
	u32 status;

	do {
		status = readl(base + STATUS_SYS);
	} while (status & STATUS_BUSY);

	return status;
}

static int read_ocotp(fdt_addr_t base, u32 reg, u32 *val)
{
	u32 status;

	status = wait_if_busy(base);

	if (status & STATUS_ERROR) {
		writel(status, base + STATUS_SYS);

		status = wait_if_busy(base);
	}

	status = readl(base + STATUS_SYS);
	if (status & STATUS_ERROR) {
		log_err("Failed to clear OCOTP\n");
		return -EIO;
	}

	writel(reg, base + ADDR_SYS);
	writel(CTRL_AUTH_KEY | CTRL_RD_WR(CTRL_READ_FUSE), base + CTRL_SYS);

	status = wait_if_busy(base);
	if (status & STATUS_ERROR)
		return -EIO;

	*val = readl(base + RDATA_SYS);

	return 0;
}

static int s32gen1_ocotp_read(struct udevice *dev, int offset,
			      void *buf, int size)
{
	struct s32gen1_ocotp_platdata *plat = dev_get_platdata(dev);
	u32 bank, word;

	if (offset < 0)
		return -EINVAL;

	if (size != 4)
		return -EPERM;

	bank = get_bank_index(offset);
	word = get_word_index(offset);

	if (!is_valid_word(plat->fuses, bank, word)) {
		log_err("OCOTP: [bank %u word %u] is not a valid fuse\n",
			bank, word);
		return -EINVAL;
	}

	return read_ocotp(plat->base, offset, buf);
}

static int s32gen1_ocotp_platdata(struct udevice *bus)
{
	struct s32gen1_ocotp_platdata *plat = dev_get_platdata(bus);

	plat->base = devfdt_get_addr(bus);
	if (plat->base == FDT_ADDR_T_NONE) {
		log_err("OCOTP: Can't get base address or size\n");
		return -ENOMEM;
	}

	plat->fuses = (const struct s32gen1_fuse_map *)dev_get_driver_data(bus);

	return 0;
}

static const struct misc_ops s32gen1_ocotp_ops = {
	.read = s32gen1_ocotp_read,
};

static const struct udevice_id s32gen1_ocotp_ids[] = {
	{ .compatible = "fsl,s32g-ocotp",  .data = (ulong)&s32g_map },
	{ .compatible = "fsl,s32r45-ocotp",  .data = (ulong)&s32r45_map },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(s32gen1_ocotp) = {
	.name = "s32gen1-ocotp",
	.id = UCLASS_MISC,
	.ops = &s32gen1_ocotp_ops,
	.of_match = s32gen1_ocotp_ids,
	.platdata_auto_alloc_size = sizeof(struct s32gen1_ocotp_platdata),
	.ofdata_to_platdata = s32gen1_ocotp_platdata,
};

