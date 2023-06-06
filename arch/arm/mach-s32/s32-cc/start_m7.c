// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2022-2023 NXP
 */

#include <common.h>
#include <command.h>
#include <misc.h>
#include <soc.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <s32-cc/s32cc_soc.h>

#define S32CC_SRAM_6M	(6 * SZ_1M)
#define S32CC_SRAM_8M	(8 * SZ_1M)
#define S32CC_SRAM_15M	(15 * SZ_1M)
#define S32CC_SRAM_20M	(20 * SZ_1M)

#define MC_ME_BASE_ADDR			(0x40088000)
#define MC_RGM_BASE_ADDR		(0x40078000)

#define RGM_PRST(MC_RGM, per)		((uintptr_t)(MC_RGM) + 0x40 + \
					 ((per) * 0x8))

#define MC_RGM_PRST_CM7			(0)
#define PRST_PERIPH_n_RST(n)		BIT(n)
#define PRST_PERIPH_CM7n_RST(n)		PRST_PERIPH_n_RST(n)

#define RGM_PSTAT(rgm, per)		((uintptr_t)(rgm) + 0x140 + \
					 ((per) * 0x8))
#define MC_RGM_PSTAT_CM7		(0)
#define PSTAT_PERIPH_n_STAT(n)		BIT(n)
#define PSTAT_PERIPH_CM7n_STAT(n)	PSTAT_PERIPH_n_STAT(n)

/* MC_ME registers. */
#define MC_ME_CTL_KEY(MC_ME)		((uintptr_t)(MC_ME) + 0x0)
#define MC_ME_CTL_KEY_KEY		(0x00005AF0)
#define MC_ME_CTL_KEY_INVERTEDKEY	(0x0000A50F)

/* MC_ME partition 1 m M definitions. */
#define MC_ME_PRTN_PART(PART, PRTN)	(MC_ME_BASE_ADDR + 0x140UL + \
					 (PART) * 0x200UL + \
					 (PRTN) * 0x20UL)
#define MC_ME_PRTN_N_CORE_M(n, m)      \
	MC_ME_PRTN_PART(n, m)

#define MC_ME_PRTN_N_PCONF_OFF	(0x0)
#define MC_ME_PRTN_N_PUPD_OFF	(0x4)
#define MC_ME_PRTN_N_STAT_OFF	(0x8)
#define MC_ME_PRTN_N_ADDR_OFF	(0xC)

#define MC_ME_PRTN_N_CORE_M_PCONF(n, m)	(MC_ME_PRTN_N_CORE_M(n, m))
#define MC_ME_PRTN_N_CORE_M_PUPD(n, m)	(MC_ME_PRTN_N_CORE_M(n, m) +\
					 MC_ME_PRTN_N_PUPD_OFF)
#define MC_ME_PRTN_N_CORE_M_STAT(n, m)	(MC_ME_PRTN_N_CORE_M(n, m) +\
					 MC_ME_PRTN_N_STAT_OFF)
#define MC_ME_PRTN_N_CORE_M_ADDR(n, m)	(MC_ME_PRTN_N_CORE_M(n, m) +\
					 MC_ME_PRTN_N_ADDR_OFF)

/* MC_ME_PRTN_N_CORE_M_* registers fields. */
#define MC_ME_PRTN_N_CORE_M_PCONF_CCE		BIT(0)
#define MC_ME_PRTN_N_CORE_M_PUPD_CCUPD		BIT(0)
#define MC_ME_PRTN_N_CORE_M_STAT_CCS		BIT(0)

#define MC_ME_CM7_PRTN		(0)

struct s32cc_soc_sram_size {
	u32 sram_size;
};

static const struct soc_attr s32cc_soc_sram_size_data[] = {
	{
		.machine = SOC_MACHINE_S32G233A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_6M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G254A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_8M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G274A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_8M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G358A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_15M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G359A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_20M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G378A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_15M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G379A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_20M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G398A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_15M,
		},
	},
	{
		.machine = SOC_MACHINE_S32G399A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_20M,
		},
	},
	{
		.machine = SOC_MACHINE_S32R455A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_8M,
		},
	},
	{
		.machine = SOC_MACHINE_S32R458A,
		.data = &(struct s32cc_soc_sram_size) {
			.sram_size = S32CC_SRAM_8M,
		},
	},
	{ /* sentinel */ }
};

static int get_sram_size(u32 *sram_size)
{
	const struct soc_attr *soc_match_data;
	const struct s32cc_soc_sram_size *s32cc_match_data;

	soc_match_data = soc_device_match(s32cc_soc_sram_size_data);
	if (!soc_match_data)
		return -EINVAL;

	s32cc_match_data = (struct s32cc_soc_sram_size *)soc_match_data->data;
	*sram_size = s32cc_match_data->sram_size;

	debug("%s: SRAM size: %u\n", __func__, *sram_size);

	return 0;
}

static int do_startm7(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	u32 coreid = 0, sram_size;
	unsigned long addr;
	char *ep = NULL;
	int ret;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], &ep, 16);
	if (ep == argv[1] || *ep != '\0')
		return CMD_RET_USAGE;

	ret = get_sram_size(&sram_size);
	if (ret) {
		printf("ERROR: Could not get SRAM size\n");
		return CMD_RET_FAILURE;
	}

	if (addr < S32CC_SRAM_BASE || addr >= S32CC_SRAM_BASE + sram_size) {
		printf("ERROR: Address 0x%08lX is not in internal SRAM ...\n",
		       addr);
		return CMD_RET_USAGE;
	}

	printf("Starting CM7_%d core at SRAM address 0x%08lX ... ",
	       coreid, addr);

	writel(readl(RGM_PRST(MC_RGM_BASE_ADDR, MC_RGM_PRST_CM7)) |
	       PRST_PERIPH_CM7n_RST(coreid),
	       RGM_PRST(MC_RGM_BASE_ADDR, MC_RGM_PRST_CM7));
	while (!(readl(RGM_PSTAT(MC_RGM_BASE_ADDR, MC_RGM_PSTAT_CM7)) &
		 PSTAT_PERIPH_CM7n_STAT(coreid)))
		;

	/* Run in Thumb mode by setting BIT(0) of the address*/
	writel(addr | BIT(0), MC_ME_PRTN_N_CORE_M_ADDR(MC_ME_CM7_PRTN, coreid));

	writel(MC_ME_PRTN_N_CORE_M_PCONF_CCE,
	       MC_ME_PRTN_N_CORE_M_PCONF(MC_ME_CM7_PRTN, coreid));
	writel(MC_ME_PRTN_N_CORE_M_PUPD_CCUPD,
	       MC_ME_PRTN_N_CORE_M_PUPD(MC_ME_CM7_PRTN, coreid));
	writel(MC_ME_CTL_KEY_KEY, (MC_ME_BASE_ADDR));
	writel(MC_ME_CTL_KEY_INVERTEDKEY, (MC_ME_BASE_ADDR));
	while (!(readl(MC_ME_PRTN_N_CORE_M_STAT(MC_ME_CM7_PRTN, coreid)) &
		 MC_ME_PRTN_N_CORE_M_STAT_CCS))
		;

	writel(readl(RGM_PRST(MC_RGM_BASE_ADDR, MC_RGM_PRST_CM7)) &
	       (~PRST_PERIPH_CM7n_RST(coreid)),
	       RGM_PRST(MC_RGM_BASE_ADDR, MC_RGM_PRST_CM7));
	while (readl(RGM_PSTAT(MC_RGM_BASE_ADDR, MC_RGM_PSTAT_CM7)) &
	       PSTAT_PERIPH_CM7n_STAT(coreid))
		;

	printf("done.\n");

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(startm7,	2,	1,	do_startm7,
	   "start CM7_0 core from SRAM address",
	   "<start_address>"
);
