// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2019,2021 NXP
 */

#include <stdio.h>
#include <linux/types.h>
#include <asm/io.h>
#include <common.h>
#include <asm/arch/cpu.h>
#include <asm/arch/s32-gen1/a53_cluster_gpr.h>
#include <asm/arch/s32-gen1/ncore.h>

#define NCORE_BASE_ADDR		(0x50400000UL)

#define A53_CLUSTER0_CAIU	(0)
#define A53_CLUSTER1_CAIU	(1)

#define DIRU(n)			(NCORE_BASE_ADDR + 0x80000 + (n) * 0x1000)
#define DIRUSFER(n)		(DIRU(n) + 0x10)
#define DIRUSFER_SFEN		BIT(0)
#define DIRUCASER(n) (DIRU(n) + 0x40)
#define DIRUCASER_CASNPEN(caiu)	BIT(caiu)
#define DIRUSFMAR(n)		(DIRU(n) + 0x84)
#define DIRUSFMAR_MNTOPACTV	BIT(0)
#define DIRUSFMCR(n)		(DIRU(n) + 0x80)
#define DIRUSFMCR_SFID(sf)	((sf) << 16)
#define DIRUSFMCR_SFMNTOP_INITIALIZE_ALL_ENTRIES	(0x0)

#define CAIU(n)                 (NCORE_BASE_ADDR + (n * 0x1000))
#define CAIUIDR(n)		(CAIU(n) + 0xffc)
#define CAIUIDR_TYPE		(0xf << 16)
#define CAIUIDR_TYPE_ACE_DVM	(0x0 << 16)

#define CSR			(NCORE_BASE_ADDR + 0xff000)
#define CSADSER			(CSR + 0x40)
#define CSADSER_DVMSNPEN(caiu)	BIT(caiu)
#define CSIDR			(CSR + 0xffc)
#define CSIDR_NUMSFS_SHIFT	(18)
#define CSIDR_NUMSFS_MASK	(0x1f << CSIDR_NUMSFS_SHIFT)
#define CSIDR_NUMSFS(csidr)	(((csidr) & CSIDR_NUMSFS_MASK) \
				 >> CSIDR_NUMSFS_SHIFT)
#define CSUIDR			(CSR + 0xff8)
#define CSUIDR_NUMCMIUS_SHIFT	(24)
#define CSUIDR_NUMCMIUS_MASK	(0x3f << CSUIDR_NUMCMIUS_SHIFT)
#define CSUIDR_NUMCMIUS(csuidr)	(((csuidr) & CSUIDR_NUMCMIUS_MASK) \
				 >> CSUIDR_NUMCMIUS_SHIFT)
#define CSUIDR_NUMDIRUS_SHIFT	(16)
#define CSUIDR_NUMDIRUS_MASK	(0x3f << CSUIDR_NUMDIRUS_SHIFT)
#define CSUIDR_NUMDIRUS(csuidr)	(((csuidr) & CSUIDR_NUMDIRUS_MASK) \
				 >> CSUIDR_NUMDIRUS_SHIFT)
#define CSUIDR_NUMNCBUS_SHIFT	(8)
#define CSUIDR_NUMNCBUS_MASK	(0x3f << CSUIDR_NUMNCBUS_SHIFT)
#define CSUIDR_NUMNCBUS(csuidr)	(((csuidr) & CSUIDR_NUMNCBUS_MASK) \
				 >> CSUIDR_NUMNCBUS_SHIFT)

static void ncore_diru_online(uint32_t diru)
{
	int numsfs, sf;

	numsfs = CSIDR_NUMSFS(readl(CSIDR)) + 1;
	for (sf = 0; sf < numsfs; sf++) {
		writel(DIRUSFMCR_SFID(sf) |
		       DIRUSFMCR_SFMNTOP_INITIALIZE_ALL_ENTRIES,
		       DIRUSFMCR(diru));
		while (readl(DIRUSFMAR(diru)) & DIRUSFMAR_MNTOPACTV)
			;
		writel(readl(DIRUSFER(diru)) | DIRUSFER_SFEN,
		       DIRUSFER(diru));
	}
}

static void ncore_cmiu_online(uint32_t cmiu)
{
	/* Nothing to be done since the hardware implementation
	 * does not have a coherent memory cache
	 */
}

static void ncore_ncbu_online(uint32_t ncbu)
{
	/* Nothing to be done since the hardware implementation
	 * does not have a proxy cache
	 */
}

static void set_caiu(uint32_t caiu, bool on)
{
	int numdirus, diru;
	uint32_t dirucase, csadser, caiuidr;

	numdirus = CSUIDR_NUMDIRUS(readl(CSUIDR));
	for (diru = 0; diru < numdirus; diru++) {
		dirucase = readl(DIRUCASER(diru));

		if (on)
			dirucase |= DIRUCASER_CASNPEN(caiu);
		else
			dirucase &= ~DIRUCASER_CASNPEN(caiu);

		writel(dirucase, DIRUCASER(diru));
	}

	caiuidr = readl(CAIUIDR(caiu));

	if ((caiuidr & CAIUIDR_TYPE) == CAIUIDR_TYPE_ACE_DVM) {
		csadser = readl(CSADSER);

		if (on)
			csadser |= CSADSER_DVMSNPEN(caiu);
		else
			csadser &= ~CSADSER_DVMSNPEN(caiu);

		writel(csadser, CSADSER);
	}
}

static void ncore_caiu_online(uint32_t caiu)
{
	set_caiu(caiu, true);
}

static void ncore_common_init(void)
{
	uint32_t numdirus, diru;
	uint32_t numcmius, cmiu;
	uint32_t numncbus, ncbu;
	uint32_t csuidr = readl(CSUIDR);

	numdirus = CSUIDR_NUMDIRUS(csuidr);
	for (diru = 0; diru < numdirus; diru++)
		ncore_diru_online(diru);

	numcmius = CSUIDR_NUMCMIUS(csuidr);
	for (cmiu = 0; cmiu < numcmius; cmiu++)
		ncore_cmiu_online(cmiu);

	numncbus = CSUIDR_NUMNCBUS(csuidr);
	for (ncbu = 0; ncbu < numncbus; ncbu++)
		ncore_ncbu_online(ncbu);
}

void ncore_init(u32 cpumask)
{

	static bool is_common_init;

	if (!is_common_init) {
		is_common_init = true;
		ncore_common_init();
	}

	ncore_caiu_online(A53_CLUSTER0_CAIU);

	if (((cpumask & cpu_pos_mask_cluster0()) == cpumask) ||
	    ((cpumask & cpu_pos_mask_cluster1()) == cpumask) ||
	    is_a53_lockstep_enabled())
		return;

	ncore_caiu_online(A53_CLUSTER1_CAIU);
}
