/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2016-2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/soc.h>
#include <fsl_esdhc.h>
#include <mmc.h>
#include <netdev.h>
#include <div64.h>
#include <errno.h>
#include <asm/arch/cse.h>
#include <asm/arch/imx-regs.h>

#ifdef VIRTUAL_PLATFORM
#define UART_CLK_FREQ	133333333
#endif

DECLARE_GLOBAL_DATA_PTR;


u32 cpu_mask(void)
{
	return 0xF;
}

/*
 * Return the number of cores on this SOC.
 */
int cpu_numcores(void)
{
	return 4;
}

static u32 get_uart_clk(void)
{
	return UART_CLK_FREQ;
}

/* return clocks in Hz */
unsigned int mxc_get_clock(enum mxc_clock clk)
{
	switch (clk) {
	case MXC_UART_CLK:
		return get_uart_clk();
	default:
		break;
	}
	printf("Error: Unsupported function to read the frequency! Please define it correctly!");
	return 0;
}

/* Dump some core clocks */
int do_s32_showclocks(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	printf("Root clocks:\n");
	printf("UART clock:	%5d MHz\n",
	       mxc_get_clock(MXC_UART_CLK) / 1000000);

	return 0;
}

U_BOOT_CMD(
		clocks, CONFIG_SYS_MAXARGS, 1, do_s32_showclocks,
		"display clocks",
		""
	  );

#if defined(CONFIG_DISPLAY_CPUINFO)
static char *get_reset_cause(void)
{
	return "unknown reset";
}

void reset_cpu(ulong addr)
{
	printf("FATAL: Reset Failed!\n");
};

int print_cpuinfo(void)
{
	printf("CPU:   NXP S32X\n");
	printf("Reset cause: %s\n", get_reset_cause());

	return 0;
}
#endif


__weak int dram_init(void)
{
	gd->ram_size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

/* start M7 core */
static int do_start_m7(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	unsigned long addr;
	char *ep;

	if (argc < 2)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], &ep, 16);
	if (ep == argv[1] || *ep != '\0')
		return CMD_RET_USAGE;

	if (!IS_ADDR_IN_IRAM(addr)) {
		printf("ERROR: Address 0x%08lX not in internal SRAM ...\n",
		       addr);
		return CMD_RET_USAGE;
	}

	printf("Starting core M7 at SRAM address 0x%08lX ...\n", addr);
	printf("Current functionality isn't supported.\n");
	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
		startm7,	2,	1,	do_start_m7,
		"start M7 core from SRAM address",
		"startAddress"
	  );

extern void dma_mem_clr(int addr, int size);

static int do_init_sram(cmd_tbl_t *cmdtp, int flag, int argc,
		char * const argv[])
{
	unsigned long addr;
	unsigned size, max_size;
	char *ep;

	if (argc < 3)
		return CMD_RET_USAGE;

	addr = simple_strtoul(argv[1], &ep, 16);
	if (ep == argv[1] || *ep != '\0')
		return CMD_RET_USAGE;

	size = simple_strtoul(argv[2], &ep, 16);
	if (ep == argv[2] || *ep != '\0')
		return CMD_RET_USAGE;

	if (!IS_ADDR_IN_IRAM(addr)) {
		printf("ERROR: Address 0x%08lX not in internal SRAM ...\n",
		       addr);
		return CMD_RET_USAGE;
	}

	max_size = IRAM_SIZE - (addr - IRAM_BASE_ADDR);
	if (size > max_size) {
		printf("WARNING: given size exceeds SRAM boundaries.\n");
		size = max_size;
	}
	printf("Init SRAM region at address 0x%08lX, size 0x%X bytes ...\n",
	       addr, size);
	dma_mem_clr(addr, size);

	return CMD_RET_SUCCESS;
}

U_BOOT_CMD(
		initsram,	3,	1,	do_init_sram,
		"init SRAM from address",
		"startAddress[hex] size[hex]"
	  );

#ifdef CONFIG_ARCH_MISC_INIT
int arch_misc_init(void)
{
#ifdef CONFIG_FSL_CSE3
	int ret;
	ret = cse_init();
	if (ret && ret != -ENODEV)
		printf("Failed to initialize CSE3 security engine\n");
#endif
	return 0;
}
#endif
