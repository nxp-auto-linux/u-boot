// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2021 NXP
 */
#include <asm/io.h>
#include <cpu_func.h>
#include <errno.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <s32gen1_sram.h>

#define S32G_SRAM_END		(S32_SRAM_BASE + S32_SRAM_SIZE)

/* SRAM controller is able to erase 64 bits at once */
#define SRAM_BLOCK              512
#define SRAM_BLOCK_MASK         (SRAM_BLOCK - 1)

#define SRAMC_PRAMCR_OFFSET     0x0
#define SRAMC_PRAMCR_INITREQ    1
#define SRAMC_PRAMIAS_OFFSET    0x4
#define SRAMC_PRAMIAE_OFFSET    0x8
#define SRAMC_PRAMSR_OFFSET     0xC
#define SRAMC_PRAMSR_IDONE      1

static void a53_sram_init(void *start, size_t len)
{
	memset(start, 0, len);
	if (dcache_status())
		flush_dcache_range((uintptr_t)start, len);
}

static void clear_unaligned_ends(uintptr_t *start, uintptr_t *end)
{
	uintptr_t leftover;

	if (*start % SRAM_BLOCK) {
		leftover = SRAM_BLOCK - (*start & SRAM_BLOCK_MASK);

		a53_sram_init((void *)*start, round_up(leftover, 8));
		*start += leftover;
	}

	if (*end % SRAM_BLOCK) {
		leftover = *end & SRAM_BLOCK_MASK;

		a53_sram_init((void *)(*end - leftover), round_up(leftover, 8));
		*end -= leftover;
	}
}

static bool in_overlap(uintptr_t s1, uintptr_t e1, uintptr_t s2, uintptr_t e2)
{
	return max(s1, s2) <= min(e1, e2);
}

static void clear_sramc_range(uintptr_t base, u32 start_offset,
			      u32 end_offset)
{
	/* Disable the controller */
	writel(0x0, base + SRAMC_PRAMCR_OFFSET);

	/* Max range */
	writel(start_offset, base + SRAMC_PRAMIAS_OFFSET);
	writel(end_offset, base + SRAMC_PRAMIAE_OFFSET);

	/* Initialization request */
	writel(SRAMC_PRAMCR_INITREQ, base + SRAMC_PRAMCR_OFFSET);

	while (!(readl(base + SRAMC_PRAMSR_OFFSET) & SRAMC_PRAMSR_IDONE))
		;
	writel(SRAMC_PRAMSR_IDONE, base + SRAMC_PRAMSR_OFFSET);
}

static void clear_sram_range(struct sram_ctrl *c, uintptr_t start_addr,
			     uintptr_t end_addr)
{
	uintptr_t base = c->base_addr;
	u32 start_offset, end_offset;

	start_addr -= c->min_sram_addr;
	end_addr -= c->min_sram_addr;

	start_offset = c->a53_to_sramc_offset(start_addr);
	end_offset = c->a53_to_sramc_offset(end_addr) - 1;

	clear_sramc_range(base, start_offset, end_offset);
}

int s32_sram_clear(uintptr_t start, uintptr_t end)
{
	struct sram_ctrl *ctrls;
	struct sram_ctrl *c;
	size_t i, n_ctrls;
	uintptr_t s, e;

	if (start == end)
		return 1;

	if (end < start)
		return 0;

	if (start < S32_SRAM_BASE)
		return 0;

	if (end > S32G_SRAM_END)
		return 0;

	clear_unaligned_ends(&start, &end);

	s32_get_sramc(&ctrls, &n_ctrls);

	for (i = 0u; i < n_ctrls; i++) {
		c = &ctrls[i];

		if (!in_overlap(start, end, c->min_sram_addr, c->max_sram_addr))
			continue;

		/* Adapt the range to current controller */
		s = max(start, (uintptr_t)c->min_sram_addr);
		e = min(end, (uintptr_t)c->max_sram_addr);

		clear_sram_range(c, s, e);
	}

	return 1;
}
