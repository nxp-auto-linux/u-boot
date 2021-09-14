/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2021 NXP
 */
#ifndef S32GEN1_OCOTP_H
#define S32GEN1_OCOTP_H

#define S32GEN1_OCOTP_BANK_OFFSET	0x200U
#define S32GEN1_OCOTP_BANK_SIZE		0x20U
#define S32GEN1_OCOTP_WORD_SIZE		0x4U

/* Bank 0, word 6 */
#define S32GEN1_OCOTP_DIE_PROCESS_ADDR	0x218
#define S32GEN1_OCOTP_DIE_PROCESS_MASK	0x3

#define S32GEN1_OCOTP_READ_FUSE_CMD	0x1
#define S32GEN1_OCOTP_WRITE_FUSE_CMD	0x2

struct s32gen1_ocotp_cmd {
	int offset;
	void *buf;
	int size;
};

#endif
