/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2019-2020 NXP */

#ifndef S32_COMMON_H
#define S32_COMMON_H

#include <asm/types.h>
#include <generated/autoconf.h>

#define S32_AUTO_OFFSET ((size_t)(-1))

#define IVT_TAG				(0xd1)
#define DCD_MAXIMUM_SIZE		(8192)
#define DCD_HEADER_LENGTH_OFFSET	(1)

#define DCD_COMMAND_HEADER(tag, len, params) ((tag) | \
					      (cpu_to_be16((len)) << 8) | \
					      (params) << 24)
#define DCD_WRITE_TAG	(0xcc)
#define DCD_CHECK_TAG	(0xcf)
#define DCD_NOP_TAG	(0xc0)

#define PARAMS_DATA_SET		BIT(4)
#define PARAMS_DATA_MASK	BIT(3)
#define PARAMS_BYTES(x)		((x) & 0x7)

#define DCD_WRITE_HEADER(n, params)	DCD_COMMAND_HEADER(DCD_WRITE_TAG, \
							   4 + (n) * 8, \
							   (params))
#define DCD_CHECK_HEADER(params)	DCD_COMMAND_HEADER(DCD_CHECK_TAG, \
							   16, \
							   (params))
#define DCD_CHECK_HEADER_NO_COUNT(params) \
					DCD_COMMAND_HEADER(DCD_CHECK_TAG, \
							   12, \
							   (params))
#define DCD_NOP_HEADER			DCD_COMMAND_HEADER(DCD_NOP_TAG, 4, 0)

#define DCD_ADDR(x)	cpu_to_be32((x))
#define DCD_MASK(x)	cpu_to_be32((x))
#define DCD_COUNT(x)	cpu_to_be32((x))

struct image_comp {
	size_t offset;
	size_t size;
	size_t alignment;
	uint8_t *data;
};

int image_parts_comp(const void *p1, const void *p2);
void check_overlap(struct image_comp *comp1, struct image_comp *comp2);
void s32_compute_dyn_offsets(struct image_comp **parts, size_t n_parts);
void s32_check_env_overlap(size_t image_size);

#endif /* S32_COMMON_H */
