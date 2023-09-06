// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2022-2023 NXP
 */

#include <common.h>
#include <command.h>
#include <elf.h>
#include <misc.h>
#include <asm/io.h>
#include <dm/uclass.h>

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

static void kick_off_m7(u32 coreid, unsigned long addr)
{
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
}

static int do_startm7(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	u32 coreid = 0;
	unsigned long addr;
	char *ep = NULL;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], &ep, 16);
	if (ep == argv[1] || *ep != '\0')
		return CMD_RET_USAGE;

	printf("Starting CM7_%d core at SRAM address 0x%08lX ... ",
	       coreid, addr);

	kick_off_m7(coreid, addr);

	printf("done.\n");

	return CMD_RET_SUCCESS;
}

static int do_bootm7(struct cmd_tbl *cmdtp, int flag, int argc, char * const argv[])
{
	Elf32_Ehdr *ehdr;
	const char *address_argv = argv[1];
	const char *entry_argv = argv[2];
	unsigned long addr, entry, elf_entry;

	if (argc < 2)
		return CMD_RET_USAGE;

	if (strict_strtoul(address_argv, 16, &addr) == -EINVAL) {
		printf("Invalid ELF address: %s\n", address_argv);
		return CMD_RET_FAILURE;
	}

	if (!valid_elf_image(addr)) {
		printf("Cannot find ELF headers @0x%08lx\n", addr);
		return CMD_RET_FAILURE;
	}

	ehdr = (Elf32_Ehdr *)addr;
	if (ehdr->e_ident[EI_CLASS] == ELFCLASS64) {
		printf("Cannot boot an AARCH64 ELF on the CM7_0 core.\n");
		return CMD_RET_FAILURE;
	}

	printf("## Loading ELF from 0x%08lx ...\n", addr);

	elf_entry = load_elf_image_phdr_skip_empty(addr, true);
	if (argc == 2) {
		entry = elf_entry;
	} else if (argc == 3) {
		/* Entry as address */
		if (strict_strtoul(entry_argv, 16, &entry) == -EINVAL) {
			/* Entry as a symbol */
			if (elf32_symbol_lookup(addr, entry_argv, &entry)) {
				printf("Failed to identify %s symbol\n",
				       entry_argv);
				return CMD_RET_FAILURE;
			}
		}
	}

	printf("## Starting CM7_0 core using 0x%08lx ...\n", entry);

	kick_off_m7(0, entry);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(startm7,	2,	1,	do_startm7,
	   "start CM7_0 core from SRAM address",
	   "<start_address>"
);

U_BOOT_CMD(bootm7, 3, 0, do_bootm7,
	   "Boot from an ELF image in memory using CM7_0 core",
	   "[address] [entry]\n"
	   "\t- load ELF image stored at [address] via program headers and\n"
	   "\t  kick-off the CM7_0 core using the [entry].\n"
	   "\t- [entry] can be the name of a symbol part of the elf or an address\n"
);
