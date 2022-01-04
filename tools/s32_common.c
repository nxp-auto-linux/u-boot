/* SPDX-License-Identifier: GPL-2.0+ */
/* Copyright 2019-2020 NXP */

#include <image.h>
#include <generated/autoconf.h>
#include <config.h>
#include "s32_common.h"

#define S32_AUTO_OFFSET ((size_t)(-1))

int image_parts_comp(const void *p1, const void *p2)
{
	const struct image_comp **part1 = (typeof(part1))p1;
	const struct image_comp **part2 = (typeof(part2))p2;

	if (!*part1 && !*part2)
		return 0;

	if (!*part1)
		return -1;

	if (!*part2)
		return 1;

	if ((*part2)->offset > (*part1)->offset)
		return -1;

	if ((*part2)->offset < (*part1)->offset)
		return 1;

	return 0;
}

void check_overlap(struct image_comp *comp1,
		   struct image_comp *comp2)
{
	size_t end1 = comp1->offset + comp1->size;
	size_t end2 = comp2->offset + comp2->size;

	if (end1 > comp2->offset && end2 > comp1->offset) {
		fprintf(stderr, "Detected overlap between 0x%zx@0x%zx and "
				"0x%zx@0x%zx\n",
				comp1->size, comp1->offset,
				comp2->size, comp2->offset);
		exit(EXIT_FAILURE);
	}
}

void s32_compute_dyn_offsets(struct image_comp **parts, size_t n_parts)
{
	size_t i;
	size_t start_index = 0U;

	/* Skip empty entries */
	while (!parts[start_index])
		start_index++;

	for (i = start_index; i < n_parts; i++) {
		if (parts[i]->offset == S32_AUTO_OFFSET) {
			if (i == start_index) {
				parts[i]->offset = 0U;
				continue;
			}

			parts[i]->offset = parts[i - 1]->offset +
			    parts[i - 1]->size;
		}

		/* Apply alignment constraints */
		if (parts[i]->alignment != 0U)
			parts[i]->offset = ROUND(parts[i]->offset,
						 parts[i]->alignment);

		if (i != start_index)
			check_overlap(parts[i - 1], parts[i]);
	}
}

void s32_check_env_overlap(size_t image_size)
{
#ifdef CONFIG_ENV_OFFSET
	if (image_size > CONFIG_ENV_OFFSET) {
		fprintf(stderr, "This image of size 0x%x would be overwritten"
				" by environment at 0x%p\n",
			(unsigned int)(image_size),
			(void *)CONFIG_ENV_OFFSET);
		exit(EXIT_FAILURE);
	}
#endif
}
