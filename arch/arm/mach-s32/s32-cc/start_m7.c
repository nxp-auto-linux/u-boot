// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2022 NXP
 */

#include <common.h>
#include <command.h>
#include <misc.h>
#include <asm/io.h>
#include <dm/uclass.h>
#include <s32-cc/nvmem.h>
#include <dt-bindings/nvmem/s32cc-siul2-nvmem.h>

#define S32_SRAM_6M	(6 * SZ_1M)
#define S32_SRAM_8M	(8 * SZ_1M)
#define S32_SRAM_15M	(15 * SZ_1M)
#define S32_SRAM_20M	(20 * SZ_1M)

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

static u32 get_sram_size(void)
{
	int ret;
	const char *dev_name = "siul2_0_nvram";
	struct udevice *siul20_nvmem = NULL;
	u32 letter = 0, part_number = 0;
	struct nvmem_cell cell;

	ret = uclass_get_device_by_name(UCLASS_MISC, dev_name,
					&siul20_nvmem);
	if (ret)
		goto nvmem_err;

	ret = nvmem_cell_get_by_offset(siul20_nvmem,
				       S32CC_SOC_LETTER,
				       &cell);
	if (ret)
		goto nvmem_err;

	ret = nvmem_cell_read(&cell, &letter, sizeof(letter));
	if (ret)
		goto nvmem_err;

	ret = nvmem_cell_get_by_offset(siul20_nvmem,
				       S32CC_SOC_PART_NO,
				       &cell);
	if (ret)
		goto nvmem_err;

	ret = nvmem_cell_read(&cell, &part_number, sizeof(part_number));
	if (ret)
		goto nvmem_err;

	if ((char)letter == 'G') {
		switch (part_number) {
		case 233:
			return S32_SRAM_6M;

		case 274:
		case 254:
			return S32_SRAM_8M;

		case 398:
		case 378:
			return S32_SRAM_15M;

		case 399:
		case 379:
			return S32_SRAM_20M;

		default:
			printf("%s: %u is not a valid part number\n",
			       __func__, part_number);
			return 0;
		}
	} else if ((char)letter == 'R') {
		return S32_SRAM_8M;
	}

nvmem_err:
	printf("%s: Failed to read SIUL2 NVMEM (err = %d)\n",
	       __func__, ret);
	return 0;
}

static int do_startm7(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 coreid = 0, sram_size;
	unsigned long addr;
	char *ep = NULL;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], &ep, 16);
	if (ep == argv[1] || *ep != '\0')
		return CMD_RET_USAGE;

	sram_size = get_sram_size();
	if (!sram_size) {
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
