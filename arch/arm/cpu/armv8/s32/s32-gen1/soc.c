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
#include <asm/arch/s32-gen1/mc_me_regs.h>
#include <asm/arch/s32-gen1/mc_rgm_regs.h>
#include <board_common.h>
#ifdef CONFIG_SAF1508BET_USB_PHY
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <generic-phy.h>
#endif
#include <dt-bindings/clock/s32gen1-clock.h>
#include <s32gen1_clk_utils.h>

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

__weak u8 mc_me_core2prtn_core_id(u8 part, u8 id)
{
	return id;
}

__weak u8 get_rgm_a53_bit(u8 core)
{
	/**
	 * Bit corresponding to CA53_n in the cores'
	 * RGM reset partition (n=0..3)
	 */
	return core + 1;
}

__weak u32 mc_me_get_cluster_ptrn(u32 core)
{
	/**
	 * For S32G2/S32R45 we have the following mapping:
	 *     MC_ME_PRTN1_CORE0_* -> CA53 cluster0 core0/1
	 *     MC_ME_PRTN1_CORE2_* -> CA53 cluster1 core0/1
	 */
	return (core % 4) & ~1;
}
