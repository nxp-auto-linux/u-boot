/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <asm/io.h>
#include <asm/arch/cse.h>
#include <common.h>

#define CSE_TIMEOUT		1000000

static inline void cse_cancel_cmd(void)
{
	writel(CSE_CMD_CANCEL, CSE_CMD);
}

static inline int cse_wait(int timeout)
{
	int delay = timeout;

	while (delay--) {
		udelay(1);
		if (!(readl(CSE_SR) & CSE_SR_BSY))
			return 0;
	}

	printf("cse_init: Timed out while waiting for CSE command\n");
	return -1;
}

#ifdef CONFIG_SECURE_BOOT
int cse_auth(void)
{
	/* Not supported */
	printf("Secure boot authentication not supported\n");
	return -1;
}
#endif

#ifdef CONFIG_CSE3
int cse_init(void)
{
	uint32_t firmware;

	/* Initialise key image address */
	writel(KIA_BASE, CSE_KIA0);
	writel(KIA_BASE, CSE_KIA1);

	if (readl(CSE_SR) & CSE_SR_BSY) {
		cse_cancel_cmd();
		if (cse_wait(CSE_TIMEOUT))
			return -1;
	}

	/* Init CSE3 */
	writel(&firmware, CSE_P1);
	writel(CSE_CMD_INIT_CSE, CSE_CMD);

	if (cse_wait(CSE_TIMEOUT))
		return -1;
	if (readl(CSE_ECR))
		return -1;

	/* Init RNG */
	writel(CSE_CMD_INIT_RNG, CSE_CMD);

	if (cse_wait(CSE_TIMEOUT))
		return -1;
	if (readl(CSE_ECR))
		return -1;

	return 0;
}
#endif
