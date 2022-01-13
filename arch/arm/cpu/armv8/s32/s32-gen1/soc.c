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
#if defined(CONFIG_SYS_FSL_DDRSS) && defined(CONFIG_TARGET_TYPE_S32GEN1_EMULATOR)
#include <asm/arch/s32-gen1/ddrss.h>
#elif defined(CONFIG_SYS_FSL_DDRSS) && defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
#include <ddr_init.h>
#endif
#include <board_common.h>
#ifdef CONFIG_SAF1508BET_USB_PHY
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <generic-phy.h>
#endif
#include <dt-bindings/clock/s32gen1-clock.h>
#include <s32gen1_clk_utils.h>
#include <misc.h>
#include <dm/uclass.h>
#include <s32gen1_siul2_nvram.h>

/* FCCU registers */
#ifndef CONFIG_S32_ATF_BOOT_FLOW
#define FCCU_NCF_S1	(FCCU_BASE_ADDR + 0x84)
#define FCCU_NCFK	(FCCU_BASE_ADDR + 0x90)
#define FCCU_NCFK_KEY	(0xAB3498FE)
#endif

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

int enable_i2c_clk(unsigned char enable, unsigned i2c_num)
{
	if (enable)
		return s32gen1_enable_plat_clk(S32GEN1_CLK_XBAR_DIV3);

	return 0;
}

/* return clocks in Hz */
unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_UART_CLK:
		return S32GEN1_LIN_BAUD_CLK_FREQ;
	case MXC_I2C_CLK:
		return s32gen1_get_plat_clk_rate(S32GEN1_CLK_XBAR_DIV3);
	case MXC_ESDHC_CLK:
		return s32gen1_get_plat_clk_rate(S32GEN1_CLK_SDHC);
	case MXC_DSPI_CLK:
		return s32gen1_get_plat_clk_rate(S32GEN1_CLK_SPI);
	default:
		break;
	}
	printf("Error: Unsupported function to read the frequency!\n");
	printf("Please define it correctly!\n");
	return 0;
}

#if defined(CONFIG_DISPLAY_CPUINFO)
static const char *get_reset_cause(void)
{
	u32 val;

	val = readl(RGM_DES(MC_RGM_BASE_ADDR));
	if (val & RGM_DES_POR) {
		/* Clear bit */
		writel(RGM_DES_POR, RGM_DES(MC_RGM_BASE_ADDR));
		return "Power-On Reset";
	}

	if (val) {
		writel(~RGM_DES_POR, RGM_DES(MC_RGM_BASE_ADDR));
		return "Destructive Reset";
	}

	val = readl(RGM_FES(MC_RGM_BASE_ADDR));
	if (val & RGM_FES_EXT) {
		writel(RGM_FES_EXT, RGM_FES(MC_RGM_BASE_ADDR));
		return "External Reset";
	}

	if (val) {
		writel(~RGM_FES_EXT, RGM_FES(MC_RGM_BASE_ADDR));
		return "Functional Reset";
	}

	val = readl(MC_ME_MODE_STAT(MC_ME_BASE_ADDR));
	if ((val & MC_ME_MODE_STAT_PREVMODE) == 0)
		return "Reset";

	return "unknown reset";
}

void reset_cpu(ulong addr)
{
	writel(MC_ME_MODE_CONF_FUNC_RST, MC_ME_MODE_CONF(MC_ME_BASE_ADDR));

	writel(MC_ME_MODE_UPD_UPD, MC_ME_MODE_UPD(MC_ME_BASE_ADDR));

	writel(MC_ME_CTL_KEY_KEY, MC_ME_CTL_KEY(MC_ME_BASE_ADDR));
	writel(MC_ME_CTL_KEY_INVERTEDKEY, MC_ME_CTL_KEY(MC_ME_BASE_ADDR));

	/* If we get there, we are not in good shape */
	mdelay(1000);
	printf("FATAL: Reset Failed!\n");
	hang();
}

int print_cpuinfo(void)
{
	struct udevice *siul20_nvmem, *siul21_nvmem;
	int ret;
	bool has_subminor = false;
	u32 letter, part_number, major, minor, subminor;

	ret = uclass_get_device_by_name(UCLASS_MISC, "siul2_0_nvram",
					&siul20_nvmem);
	if (ret) {
		printf("%s: No SIUL20 NVMEM (err = %d)\n", __func__, ret);
		return ret;
	}

	ret = uclass_get_device_by_name(UCLASS_MISC, "siul2_1_nvram",
					&siul21_nvmem);
	if (ret) {
		printf("%s: No SIUL21 NVMEM (err = %d)\n", __func__, ret);
		return ret;
	}

	ret = misc_read(siul20_nvmem, S32GEN1_SOC_LETTER, &letter,
			sizeof(letter));
	if (ret != sizeof(letter)) {
		printf("%s: Failed to read SoC's letter (err = %d)\n",
		       __func__, ret);
		return -EINVAL;
	}

	ret = misc_read(siul20_nvmem, S32GEN1_SOC_PART_NO, &part_number,
			sizeof(part_number));
	if (ret != sizeof(part_number)) {
		printf("%s: Failed to read SoC's part number (err = %d)\n",
		       __func__, ret);
		return -EINVAL;
	}

	ret = misc_read(siul20_nvmem, S32GEN1_SOC_MAJOR, &major,
			sizeof(major));
	if (ret != sizeof(major)) {
		printf("%s: Failed to read SoC's major (err = %d)\n",
		       __func__, ret);
		return -EINVAL;
	}

	ret = misc_read(siul20_nvmem, S32GEN1_SOC_MINOR, &minor,
			sizeof(minor));
	if (ret != sizeof(minor)) {
		printf("%s: Failed to read SoC's minor (err = %d)\n",
		       __func__, ret);
		return -EINVAL;
	}

	/* It might be unavailable */
	ret = misc_read(siul21_nvmem, S32GEN1_SOC_SUBMINOR, &subminor,
			sizeof(subminor));
	if (ret == sizeof(subminor))
		has_subminor = true;

	printf("CPU: NXP S32%c%uA rev. %u.%u", (char)letter, part_number,
	       major, minor);

	if (has_subminor)
		printf(".%u", subminor);

	puts("\n");

	printf("Reset cause: %s\n", get_reset_cause());

	return 0;
}
#endif

#if defined(CONFIG_SYS_FSL_DDRSS) && defined(CONFIG_TARGET_TYPE_S32GEN1_EMULATOR)
extern struct ddrss_conf ddrss_conf;
extern struct ddrss_firmware ddrss_firmware;
#endif

#if defined(CONFIG_SYS_FSL_DDRSS) && defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
void store_csr(void) {}
#endif

#ifdef CONFIG_S32_SKIP_RELOC
__weak int dram_init(void)
{
	return 0;
}
#else
__weak int dram_init(void)
{
#if defined(CONFIG_SYS_FSL_DDRSS) && defined(CONFIG_TARGET_TYPE_S32GEN1_EMULATOR)
	ddrss_init(&ddrss_conf, &ddrss_firmware);
#elif defined(CONFIG_SYS_FSL_DDRSS) && defined(CONFIG_S32_STANDALONE_BOOT_FLOW)
	uint32_t ret = 0;
	ret = ddr_init();
	if (ret) {
		printf("Error %d on ddr_init\n", ret);
		return ret;
	}
#endif

	return 0;
}
#endif

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

/**
 * Clear non-critical faults generated by SWT (software watchdog timer)
 * All SWT faults are placed in NCF_S1 (33-38)
 */
#ifndef CONFIG_S32_ATF_BOOT_FLOW
static void clear_swt_faults(void)
{
	u32 val = readl(FCCU_NCF_S1);

	if (val) {
		writel(FCCU_NCFK_KEY, FCCU_NCFK);
		writel(val, FCCU_NCF_S1);
	}
}
#endif

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
#ifndef CONFIG_S32_ATF_BOOT_FLOW
	clear_swt_faults();
#endif

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

__weak u64 fdt_to_cpu_id(u64 fdt_id)
{
	return (fdt_id & 0x3u) | (fdt_id >> 0x7u);
}
