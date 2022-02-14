// SPDX-License-Identifier:     GPL-2.0+
/*
 * Copyright 2017-2022 NXP
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <mmc.h>
#include <div64.h>
#include <errno.h>
#include <hang.h>
#include <asm/arch/cpu.h>
#include <asm/arch/s32-gen1/a53_cluster_gpr.h>
#include <board_common.h>
#ifdef CONFIG_SAF1508BET_USB_PHY
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <generic-phy.h>
#endif

#define RGM_PRST(MC_RGM, per)		(UPTR(MC_RGM) + 0x40 + \
					 ((per) * 0x8))

#define MC_RGM_PRST_CM7			(0)
#define PRST_PERIPH_n_RST(n)		BIT(n)
#define PRST_PERIPH_CM7n_RST(n)		PRST_PERIPH_n_RST(n)

#define RGM_PSTAT(rgm, per)		(UPTR(rgm) + 0x140 + \
					 ((per) * 0x8))
#define MC_RGM_PSTAT_CM7		(0)
#define PSTAT_PERIPH_n_STAT(n)		BIT(n)
#define PSTAT_PERIPH_CM7n_STAT(n)	PSTAT_PERIPH_n_STAT(n)

#define RGM_CORES_RESET_GROUP		1

/* MC_ME registers. */
#define MC_ME_CTL_KEY(MC_ME)		(UPTR(MC_ME) + 0x0)
#define MC_ME_CTL_KEY_KEY		(0x00005AF0)
#define MC_ME_CTL_KEY_INVERTEDKEY	(0x0000A50F)

/* MC_ME partition 1 m M definitions. */
#define MC_ME_PRTN_PART(PART, PRTN)	(MC_ME_BASE_ADDR + 0x140UL + \
					 (PART) * 0x200UL + \
					 (PRTN) * 0x20UL)
#define MC_ME_PRTN_N_CORE_M(n, m)      \
	MC_ME_PRTN_PART(n, m)

#define MC_ME_PRTN_N_PCONF_OFF	0x0
#define MC_ME_PRTN_N_PUPD_OFF	0x4
#define MC_ME_PRTN_N_STAT_OFF	0x8
#define MC_ME_PRTN_N_ADDR_OFF	0xC

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

DECLARE_GLOBAL_DATA_PTR;

__weak u32 cpu_pos_lockstep_mask(void)
{
	return CPUMASK_LOCKSTEP;
}

__weak u32 cpu_pos_mask(void)
{
	if (is_a53_lockstep_enabled())
		return cpu_pos_lockstep_mask();

	return cpu_pos_mask_cluster0() | cpu_pos_mask_cluster1();
}

__weak u32 cpu_pos_mask_cluster0(void)
{
	return CPUMASK_CLUSTER0;
}

__weak u32 cpu_pos_mask_cluster1(void)
{
	return CPUMASK_CLUSTER1;
}

u32 cpu_mask(void)
{
	u32 rgm_stat = readl(RGM_PSTAT(MC_RGM_BASE_ADDR,
				       RGM_CORES_RESET_GROUP));
	/* 0 means out of reset. */
	/* Bit 0 corresponds to cluster reset and is 0 if any
	 * of the other bits 1-4 are 0.
	 */
	return ((~(rgm_stat)) >> 1) & cpu_pos_mask();
}

/*
 * Return the number of cores on this SOC.
 */
int cpu_numcores(void)
{
	return hweight32(cpu_mask());
}

__weak int dram_init(void)
{
	return 0;
}

static int do_startm7(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u32 coreid = 0;
	unsigned long addr;
	char *ep;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], &ep, 16);
	if (ep == argv[1] || *ep != '\0')
		return CMD_RET_USAGE;

	if (!is_addr_in_sram(addr)) {
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

U_BOOT_CMD(
		startm7,	2,	1,	do_startm7,
		"start CM7_0 core from SRAM address",
		"<start_address>"
	  );

#ifdef CONFIG_SAF1508BET_USB_PHY
static int enable_saf1508bet(void)
{
	int ret = 0;
	struct udevice *dev;
	struct phy phy;
	struct uclass *uc;
	struct udevice *bus;

	ret = uclass_get_device(UCLASS_USB, 0, &dev);
	if (ret) {
		pr_err("%s: Cannot find USB device\n", __func__);
		return ret;
	}
	ret = uclass_get(UCLASS_USB, &uc);
	if (ret)
		return ret;

	/* Probe USB controller */
	uclass_foreach_dev(bus, uc) {
		ret = device_probe(bus);
		if (ret == -ENODEV) {	/* No such device. */
			puts("Port not available.\n");
			continue;
		}
	}

	/* SAF1508BET PHY */
	ret = generic_phy_get_by_index(dev, 0, &phy);
	if (ret) {
		pr_err("failed to get %s USB PHY\n", dev->name);
		return ret;
	}

	ret = generic_phy_power_on(&phy);
	if (ret) {
		pr_err("failed to get %s USB PHY\n", dev->name);
		return ret;
	}

	return ret;
}
#endif

int arch_misc_init(void)
{
#ifdef CONFIG_SAF1508BET_USB_PHY
	/* The usb phy must be probed in u-boot in order to have a working USB
	 * interface in linux.
	 */
	enable_saf1508bet();
#endif
	return 0;
}

__weak u32 get_sram_size(void)
{
	return S32_SRAM_SIZE;
}
