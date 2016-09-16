/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Configuration settings for all the Freescale S32V234 boards.
 */

#ifndef __S32V234_COMMON_H
#define __S32V234_COMMON_H

#include <asm/arch/imx-regs.h>

#define CONFIG_S32V234
#define CONFIG_DISPLAY_CPUINFO
#define CONFIG_DISPLAY_BOARDINFO

/* Init CSE3 from u-boot */
#ifdef CONFIG_FSL_CSE3

#define CONFIG_ARCH_MISC_INIT
#define KIA_BASE		0x3e805000UL
/* Secure Boot */
#ifdef CONFIG_SECURE_BOOT
#define SECURE_BOOT_KEY_ID	0x4UL
#endif
/* start address and size of firmware+keyimage binary blob */
#define CSE_BLOB_BASE		0x3e801000UL
#define CSE_BLOB_SIZE		0x00004500UL

#endif /* CONFIG_FSL_CSE3 */

/* Config GIC */
#define CONFIG_GICV2
#define GICD_BASE		0x7D001000
#define GICC_BASE		0x7D002000

#define CONFIG_REMAKE_ELF

/* Run by default from DDR1 */
#ifdef CONFIG_RUN_FROM_DDR0
#define DDR_BASE_ADDR		0x80000000
#else
#define DDR_BASE_ADDR		0xC0000000
#endif

/* DDR chips on S32V234 boards have 32 bits cells */
#define RAM_CELL_SIZE		32

#define CONFIG_SKIP_LOWLEVEL_INIT

/* Enable passing of ATAGs */
#define CONFIG_CMDLINE_TAG

/* SMP definitions */
#define CONFIG_MAX_CPUS			(4)
#define SECONDARY_CPU_BOOT_PAGE		(CONFIG_SYS_SDRAM_BASE)
#define CPU_RELEASE_ADDR		SECONDARY_CPU_BOOT_PAGE
#define CONFIG_FSL_SMP_RELEASE_ALL
#define CONFIG_ARMV8_SWITCH_TO_EL1

/* SMP Spin Table Definitions */
#define CONFIG_MP
#define CONFIG_OF_LIBFDT

/* Ramdisk name */
#define RAMDISK_NAME		rootfs.uimg

#ifdef CONFIG_RUN_FROM_DDR0
/* Flat device tree definitions */
#define FDT_ADDR		0x82000000

/*Kernel image load address */
#define LOADADDR		0x8007FFC0

/* Ramdisk load address */
#define RAMDISK_ADDR		0x84000000
#else
/* Flat device tree definitions */
#define FDT_ADDR		0xC2000000

/*Kernel image load address */
#define LOADADDR		0xC007FFC0

/* Ramdisk load address */
#define RAMDISK_ADDR		0xC4000000
#endif


/* Generic Timer Definitions */
#if defined(CONFIG_SYS_GENERIC_TIMER)
#define COUNTER_FREQUENCY               (12000000)     /* 12MHz */
#elif defined(CONFIG_SYS_PIT_TIMER)
#define COUNTER_FREQUENCY               (133056128)     /* 133MHz */
#else
#error "Unknown System Timer"
#endif

#define CONFIG_SYS_FSL_ERRATUM_A008585

/* Size of malloc() pool */
#ifdef CONFIG_RUN_FROM_IRAM_ONLY
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 1 * 1024 * 1024)
#else
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * 1024 * 1024)
#endif

#define CONFIG_BOARD_EARLY_INIT_F
#define CONFIG_ARCH_EARLY_INIT_R

#if CONFIG_FSL_LINFLEX_MODULE == 0
#define LINFLEXUART_BASE	LINFLEXD0_BASE_ADDR
#else
#define LINFLEXUART_BASE	LINFLEXD1_BASE_ADDR
#endif

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_UART_PORT		(1)

#undef CONFIG_CMD_IMLS

#define CONFIG_MMC
#define CONFIG_FSL_ESDHC
#define CONFIG_SYS_FSL_ESDHC_NUM	1

#define CONFIG_SYS_FSL_ERRATUM_ESDHC111

#define CONFIG_CMD_MMC
#define CONFIG_GENERIC_MMC
#define CONFIG_CMD_FAT  /* FAT support */
#define CONFIG_DOS_PARTITION

/* Ethernet config */
#define CONFIG_CMD_MII
#define CONFIG_FEC_MXC
#define CONFIG_MII
#define IMX_FEC_BASE            ENET_BASE_ADDR

#ifdef CONFIG_PHY_RGMII_DIRECT_CONNECTED
#define CONFIG_FEC_MXC_PHYADDR (0x484a53)
#define CONFIG_BCM_DUPLEX_MODE	DUPLEX_FULL
#endif

/* I2C Configs */
#define CONFIG_CMD_I2C
#define CONFIG_SYS_I2C
#define CONFIG_SYS_I2C_MXC
#define CONFIG_SYS_I2C_MXC_I2C1	/* enable I2C bus 1 */
#define CONFIG_SYS_I2C_SPEED	100000
#define CONFIG_SYS_I2C_SLAVE	0x8
#define CONFIG_SYS_SPD_BUS_NUM	0

#define CONFIG_S32V234_FLASH

/* memory mapped external flash */
#define CONFIG_SYS_FSL_FLASH0_BASE      0x20000000
#define CONFIG_SYS_FSL_FLASH0_SIZE      0x10000000
#define CONFIG_SYS_FSL_FLASH1_BASE      0x60000000
#define CONFIG_SYS_FSL_FLASH1_SIZE      0x10000000

/* QSPI/hyperflash configs */
#ifdef CONFIG_S32V234_FLASH

/* debug stuff for qspi/hyperflash */
#undef CONFIG_DEBUG_S32V234_QSPI_QSPI

/* flash comand disabled until implemented */
#undef CONFIG_CMD_FLASH

#define QSPI_BASE_ADDR		0x400A6000
#define FLASH_BASE_ADR		CONFIG_SYS_FSL_FLASH0_BASE
#define FLASH_BASE_ADR2		0x24000000

#endif

#define CONFIG_BOOTDELAY	3

#define CONFIG_BOOTARGS		"console=ttyLF"	__stringify(CONFIG_FSL_LINFLEX_MODULE) \
				" root=/dev/ram rw"

#define CONFIG_CMD_ENV

#define CONFIG_OF_LIBFDT
#define CONFIG_CMD_BOOTI
#define CONFIG_CMD_BOOTZ

#ifdef CONFIG_CMD_BOOTI

/*
 * Enable CONFIG_USE_BOOTI if the u-boot enviroment variables
 * specific for boot method have to be defined for booti by default.
 */
#define CONFIG_USE_BOOTI
#ifdef CONFIG_USE_BOOTI
#define IMAGE_NAME Image
#define BOOT_MTD booti
#else
#define IMAGE_NAME uImage
#define BOOT_MTD bootm
#endif

#endif

#ifndef CONFIG_BOARD_EXTRA_ENV_SETTINGS
#define CONFIG_BOARD_EXTRA_ENV_SETTINGS	""
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_BOARD_EXTRA_ENV_SETTINGS  \
	"script=boot.scr\0" \
	"boot_mtd=" __stringify(BOOT_MTD) "\0" \
	"image=" __stringify(IMAGE_NAME) "\0" \
	"ramdisk=" __stringify(RAMDISK_NAME) "\0"\
	"console=ttyLF" __stringify(CONFIG_FSL_LINFLEX_MODULE) "\0" \
	"fdt_high=0xffffffff\0" \
	"initrd_high=0xffffffff\0" \
	"fdt_file="  __stringify(FDT_FILE) "\0" \
	"fdt_addr=" __stringify(FDT_ADDR) "\0" \
	"ramdisk_addr=" __stringify(RAMDISK_ADDR) "\0" \
	"boot_fdt=try\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=" __stringify(CONFIG_MMC_PART) "\0" \
	"mmcroot=/dev/mmcblk0p2 rootwait rw\0" \
	"cse_addr=" __stringify(CSE_BLOB_BASE) "\0" \
	"cse_file=cse.bin\0" \
	"sec_boot_key_id=" __stringify(SECURE_BOOT_KEY_ID) "\0" \
	"update_sd_firmware_filename=u-boot.s32\0" \
	"update_sd_firmware=" \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"if mmc dev ${mmcdev}; then "	\
			"if ${get_cmd} ${update_sd_firmware_filename}; then " \
				"setexpr fw_sz ${filesize} / 0x200; " \
				"setexpr fw_sz ${fw_sz} + 1; "	\
				"mmc write ${loadaddr} 0x2 ${fw_sz}; " \
			"fi; "	\
		"fi\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate} " \
		"root=${mmcroot}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadramdisk=fatload mmc ${mmcdev}:${mmcpart} ${ramdisk_addr} ${ramdisk}\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}\0" \
	"jtagboot=echo Booting using jtag...; " \
		"${boot_mtd} ${loadaddr} ${ramdisk_addr} ${fdt_addr}\0" \
	"jtagsdboot=echo Booting loading Linux with ramdisk from SD...; " \
		"run loadimage; run loadramdisk; run loadfdt;"\
		"${boot_mtd} ${loadaddr} ${ramdisk_addr} ${fdt_addr}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if run loadfdt; then " \
				"${boot_mtd} ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"${boot_mtd}; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"${boot_mtd}; " \
		"fi;\0" \
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs " \
	"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp\0" \
		"netboot=echo Booting from net ...; " \
		"run netargs; " \
		"if test ${ip_dyn} = yes; then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"${get_cmd} ${image}; " \
		"if test ${boot_fdt} = yes || test ${boot_fdt} = try; then " \
			"if ${get_cmd} ${fdt_addr} ${fdt_file}; then " \
				"${boot_mtd} ${loadaddr} - ${fdt_addr}; " \
			"else " \
				"if test ${boot_fdt} = try; then " \
					"${boot_mtd}; " \
				"else " \
					"echo WARN: Cannot load the DT; " \
				"fi; " \
			"fi; " \
		"else " \
			"${boot_mtd}; " \
		"fi;\0"

#define CONFIG_BOOTCOMMAND \
	   "mmc dev ${mmcdev}; if mmc rescan; then " \
		   "if run loadimage; then " \
			   "run mmcboot; " \
		   "else run netboot; " \
		   "fi; " \
	   "else run netboot; fi"

/* Miscellaneous configurable options */
#define CONFIG_SYS_LONGHELP		/* undef to save memory */
#define CONFIG_SYS_HUSH_PARSER		/* use "hush" command parser */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_AUTO_COMPLETE
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		\
			(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE
#define CONFIG_CMDLINE_EDITING

#define CONFIG_CMD_MEMTEST
#define CONFIG_SYS_MEMTEST_START	(DDR_BASE_ADDR)
#define CONFIG_SYS_MEMTEST_END		(DDR_BASE_ADDR + (CONFIG_SYS_DDR_SIZE - 1))

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ				1000

#define CONFIG_SYS_TEXT_BASE        0x3E820000 /* SDRAM */
#define CONFIG_SYS_TEXT_OFFSET      0x00020000

#ifdef CONFIG_RUN_FROM_IRAM_ONLY
#define CONFIG_SYS_MALLOC_BASE		(DDR_BASE_ADDR)
#endif

/*
 * Stack sizes
 * The stack sizes are set up in start.S using the settings below
 */
#define CONFIG_STACKSIZE		(128 * 1024)	/* regular stack */

#if 0
/* Configure PXE */
#define CONFIG_CMD_PXE
#define CONFIG_BOOTP_PXE
#define CONFIG_BOOTP_PXE_CLIENTARCH	0x100
#endif

/* Physical memory map */
#define CONFIG_NR_DRAM_BANKS	1
#define PHYS_SDRAM			(DDR_BASE_ADDR)
#define PHYS_SDRAM_SIZE			(CONFIG_SYS_DDR_SIZE)

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM
#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

/* FLASH and environment organization */
#define CONFIG_SYS_NO_FLASH

#define CONFIG_ENV_SIZE			(8 * 1024)
#define CONFIG_ENV_IS_IN_MMC

#define CONFIG_ENV_OFFSET		(12 * 64 * 1024)
#define CONFIG_SYS_MMC_ENV_DEV		0
#define CONFIG_MMC_PART			1

#define CONFIG_BOOTP_BOOTFILESIZE
#define CONFIG_BOOTP_BOOTPATH
#define CONFIG_BOOTP_GATEWAY
#define CONFIG_BOOTP_HOSTNAME

#ifdef CONFIG_CMD_PCI
#define CONFIG_PCIE_S32V234
#define CONFIG_PCI
#define CONFIG_PCI_PNP
#define CONFIG_PCI_SCAN_SHOW
#endif

#endif
