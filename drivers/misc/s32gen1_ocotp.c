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
#define   CTRL_WRITE_FUSE	2
#define ADDR_SYS		0x4
#define WRDATA_SYS		0x8
#define RDATA_SYS		0xC
#define STATUS_SYS		0x50
#define   STATUS_BUSY		BIT(0)
#define   STATUS_CRC_FAIL	BIT(1)
#define   STATUS_ERROR		BIT(2)

/* Write is allowed without a lock */
#define NO_LOCK			0xFF
/* Read only */
#define RO_ONLY_LOCK		0

#define GP1_LOCK		1
#define GP2_LOCK		3
#define BOOT_CFG_LOCK		5
#define MISC_CONF_LOCK		7
#define MAC_ADDR_LOCK		9
#define GP5_LOCK		11
#define LOCK_CUSTOMER_LOCK	31
#define GP6_LOCK		33

struct s32gen1_fuse {
	u8 bank;
	u8 words_mask;
	u8 lock;
};

struct s32gen1_fuse_map {
	const struct s32gen1_fuse *map;
	size_t n_entries;
};

struct s32gen1_ocotp_platdata {
	fdt_addr_t base;
	const struct s32gen1_fuse_map *fuses;
};

static const struct s32gen1_fuse s32gen1_fuse_map[] = {
	{ .bank = 0, .words_mask = OCOTP_WORD_RANGE(2, 6),
		.lock = LOCK_CUSTOMER_LOCK },
	{ .bank = 1, .words_mask = OCOTP_WORD_RANGE(5, 7),
		.lock = BOOT_CFG_LOCK },
	{ .bank = 2, .words_mask = OCOTP_WORD_RANGE(0, 1),
		.lock = MISC_CONF_LOCK },
	{ .bank = 2, .words_mask = OCOTP_WORD_RANGE(2, 4),
		.lock = MAC_ADDR_LOCK },
	{ .bank = 4, .words_mask = OCOTP_WORD(6),
		.lock = NO_LOCK },
	{ .bank = 5, .words_mask = OCOTP_WORD(1),
		.lock = GP1_LOCK },
	{ .bank = 5, .words_mask = OCOTP_WORD(2),
		.lock = GP2_LOCK },
	{ .bank = 6, .words_mask = OCOTP_WORD(7),
		.lock = RO_ONLY_LOCK },
	{ .bank = 7, .words_mask = OCOTP_WORD_RANGE(0, 1),
		.lock = RO_ONLY_LOCK },
	{ .bank = 11, .words_mask = OCOTP_WORD_RANGE(0, 5),
		.lock = GP5_LOCK },
	{ .bank = 11, .words_mask = OCOTP_WORD_RANGE(6, 7),
		.lock = GP6_LOCK },
	{ .bank = 12, .words_mask = OCOTP_WORD_RANGE(0, 2),
		.lock = GP6_LOCK },
	{ .bank = 12, .words_mask = OCOTP_WORD(7),
		.lock = RO_ONLY_LOCK },
	{ .bank = 13, .words_mask = OCOTP_WORD_RANGE(2, 4),
		.lock = RO_ONLY_LOCK },
	{ .bank = 14,
		.words_mask = OCOTP_WORD(1) | OCOTP_WORD(4) | OCOTP_WORD(5),
		.lock = RO_ONLY_LOCK },
	{ .bank = 15, .words_mask = OCOTP_WORD_RANGE(5, 7),
		.lock = RO_ONLY_LOCK },
};

static const struct s32gen1_fuse_map s32gen1_map = {
	.map = s32gen1_fuse_map,
	.n_entries = ARRAY_SIZE(s32gen1_fuse_map),
};

static u32 get_bank_index(int offset)
{
	return (offset - S32GEN1_OCOTP_BANK_OFFSET) / S32GEN1_OCOTP_BANK_SIZE;
}

static u32 get_word_index(int offset)
{
	return offset % S32GEN1_OCOTP_BANK_SIZE / S32GEN1_OCOTP_WORD_SIZE;
}

static const struct s32gen1_fuse *get_fuse(const struct s32gen1_fuse_map *map,
					   u32 bank, u32 word)
{
	size_t i;

	for (i = 0; i < map->n_entries; i++) {
		if (map->map[i].bank == bank &&
		    (map->map[i].words_mask & OCOTP_WORD(word)))
			return &map->map[i];
	}

	return NULL;
}

static bool is_valid_word(const struct s32gen1_fuse_map *map,
			  u32 bank, u32 word)
{
	if (bank >= map->n_entries)
		return false;

	if (get_fuse(map, bank, word))
		return true;

	return false;
}

static u32 wait_if_busy(fdt_addr_t base)
{
	u32 status;

	do {
		status = readl(base + STATUS_SYS);
	} while (status & STATUS_BUSY);

	return status;
}

static u32 wait_and_clear_err(fdt_addr_t base)
{
	u32 status = wait_if_busy(base);

	if (status & STATUS_ERROR) {
		writel(status, base + STATUS_SYS);

		status = wait_if_busy(base);
	}

	return readl(base + STATUS_SYS);
}

static int read_ocotp(fdt_addr_t base, u32 reg, u32 *val)
{
	u32 status;

	status = wait_and_clear_err(base);
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

static int init_word_bank(struct s32gen1_ocotp_platdata *plat, int offset,
			  int size, u32 *bank, u32 *word)
{
	if (offset < 0)
		return -EINVAL;

	if (size != 4)
		return -EPERM;

	*bank = get_bank_index(offset);
	*word = get_word_index(offset);

	if (!is_valid_word(plat->fuses, *bank, *word)) {
		log_err("OCOTP: [bank %u word %u] is not a valid fuse\n",
			*bank, *word);
		return -EINVAL;
	}

	return 0;
}

static int s32gen1_ocotp_shadow_read(struct udevice *dev, int offset,
				     void *buf, int size)
{
	struct s32gen1_ocotp_platdata *plat = dev_get_platdata(dev);
	u32 bank, word;
	int ret;

	ret = init_word_bank(plat, offset, size, &bank, &word);
	if (ret)
		return ret;

	*(u32 *)buf = readl(plat->base + offset);

	return size;
}

static int s32gen1_ocotp_shadow_write(struct udevice *dev, int offset,
				      const void *buf, int size)
{
	struct s32gen1_ocotp_platdata *plat = dev_get_platdata(dev);
	u32 bank, word, *val;
	int ret;

	ret = init_word_bank(plat, offset, size, &bank, &word);
	if (ret)
		return ret;

	val = (u32 *)buf;
	writel(*val, plat->base + offset);

	if (readl(plat->base + offset) != *val)
		return -EIO;

	return size;
}

static int s32gen1_ocotp_fuse_read(struct s32gen1_ocotp_platdata *plat,
				   int offset, void *buf, int size)
{
	u32 bank, word;
	int ret;

	ret = init_word_bank(plat, offset, size, &bank, &word);
	if (ret)
		return ret;

	return read_ocotp(plat->base, offset, buf);
}

static int check_write_lock(struct s32gen1_ocotp_platdata *plat,
			    const struct s32gen1_fuse *fuse)
{
	int ret;
	u32 offset, value, lock_mask, word;

	if (fuse->lock == NO_LOCK)
		return 0;

	if (fuse->lock == RO_ONLY_LOCK)
		return -EPERM;

	/* All locks are placed in bank 0, words 2-3 */
	word = 2 + fuse->lock / S32GEN1_OCOTP_BANK_SIZE;

	offset = S32GEN1_OCOTP_BANK_OFFSET + word * S32GEN1_OCOTP_WORD_SIZE;

	ret = s32gen1_ocotp_fuse_read(plat, offset, &value, sizeof(value));
	if (ret)
		return ret;

	lock_mask = BIT(fuse->lock % S32GEN1_OCOTP_BANK_SIZE);

	if (!(value & lock_mask))
		return 0;

	return -EPERM;
}

static int write_ocotp(struct s32gen1_ocotp_platdata *plat, u32 reg, u32 bank,
		       u32 word, u32 val)
{
	const struct s32gen1_fuse *fuse = get_fuse(plat->fuses, bank, word);
	fdt_addr_t base = plat->base;
	u32 status;
	int ret;

	if (!fuse)
		return -EINVAL;

	status = wait_and_clear_err(base);
	if (status & STATUS_ERROR) {
		log_err("Failed to clear OCOTP\n");
		return -EIO;
	}

	ret = check_write_lock(plat, fuse);
	if (ret) {
		log_err("The write of %u:%u is locked\n", bank, word);
		return ret;
	}

	writel(reg, base + ADDR_SYS);
	writel(val, base + WRDATA_SYS);
	writel(CTRL_AUTH_KEY | CTRL_RD_WR(CTRL_WRITE_FUSE), base + CTRL_SYS);

	status = wait_if_busy(base);
	if (status & STATUS_ERROR)
		return -EIO;

	return 0;
}

static int s32gen1_ocotp_fuse_write(struct s32gen1_ocotp_platdata *plat,
				    int offset, const void *buf, int size)
{
	u32 bank, word;
	int ret;

	ret = init_word_bank(plat, offset, size, &bank, &word);
	if (ret)
		return ret;

	return write_ocotp(plat, offset, bank, word, *(const u32 *)buf);
}

static int s32gen1_ocotp_ioctl(struct udevice *dev, unsigned long request,
			       void *buf)
{
	struct s32gen1_ocotp_platdata *plat = dev_get_platdata(dev);
	struct s32gen1_ocotp_cmd *cmd = buf;

	if (request == S32GEN1_OCOTP_READ_FUSE_CMD) {
		return s32gen1_ocotp_fuse_read(plat, cmd->offset,
					       cmd->buf, cmd->size);
	} else if (request == S32GEN1_OCOTP_WRITE_FUSE_CMD) {
		return s32gen1_ocotp_fuse_write(plat, cmd->offset,
						cmd->buf, cmd->size);
	}

	return -EINVAL;
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
	.read = s32gen1_ocotp_shadow_read,
	.write = s32gen1_ocotp_shadow_write,
	.ioctl = s32gen1_ocotp_ioctl,
};

static const struct udevice_id s32gen1_ocotp_ids[] = {
	{ .compatible = "fsl,s32g-ocotp",  .data = (ulong)&s32gen1_map },
	{ .compatible = "fsl,s32r45-ocotp",  .data = (ulong)&s32gen1_map },
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

