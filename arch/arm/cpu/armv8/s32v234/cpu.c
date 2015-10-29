/*
 * Copyright 2014 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/io.h>
#include <asm/arch/mc_me_regs.h>
#include "cpu.h"
#include "mp.h"

DECLARE_GLOBAL_DATA_PTR;

u32 cpu_mask(void)
{
	return readl(MC_ME_CS);
}

/*
 * Return the number of cores on this SOC.
 */
int cpu_numcores(void)
{
	int numcores;
	u32 mask;

	mask = cpu_mask();
	numcores = hweight32(cpu_mask());

	/* Verify if M4 is deactivated */
	if (mask & 0x1)
		numcores--;

	return numcores;
}

#if defined(CONFIG_ARCH_EARLY_INIT_R)
int arch_early_init_r(void)
{
	int rv;
	rv = fsl_s32v234_wake_seconday_cores();

	if (rv)
		printf("Did not wake secondary cores\n");

	return 0;
}
#endif /* CONFIG_ARCH_EARLY_INIT_R */
