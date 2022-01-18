/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2017-2022 NXP
 */

/*
 * Configuration settings for all the Freescale S32 boards.
 */

#ifndef __S32_H
#define __S32_H

#define CONFIG_S32

#include <generated/autoconf.h>

#undef CONFIG_RUN_FROM_IRAM_ONLY

#define CONFIG_REMAKE_ELF

#define CONFIG_MACH_TYPE		4146

/* Config CACHE */
#define CONFIG_CMD_CACHE

/* Flat device tree definitions */
#define CONFIG_OF_FDT
#define CONFIG_OF_BOARD_SETUP

/* System Timer */
/* #define CONFIG_SYS_PIT_TIMER */

#define CONFIG_LOADADDR		LOADADDR

#define CONFIG_SKIP_LOWLEVEL_INIT

/* Enable passing of ATAGs */
#define CONFIG_CMDLINE_TAG

/* SMP definitions */
#define CONFIG_MAX_CPUS			(4)
#ifdef CONFIG_S32_SKIP_RELOC
#define SECONDARY_CPU_BOOT_PAGE		(S32_SRAM_BASE)
#else
#define SECONDARY_CPU_BOOT_PAGE		(CONFIG_SYS_FSL_DRAM_BASE1)
#endif
#define CPU_RELEASE_ADDR		SECONDARY_CPU_BOOT_PAGE
#define CONFIG_FSL_SMP_RELEASE_ALL
#ifndef CONFIG_XEN_SUPPORT
#define CONFIG_ARMV8_SWITCH_TO_EL1
#endif

/* SMP Spin Table Definitions */
#define CONFIG_MP

/* Ramdisk name */
#define RAMDISK_NAME		rootfs.uimg

/* Increase image size */
#define CONFIG_SYS_BOOTM_LEN    (64 << 20)

/* Flat device tree definitions */
#  ifdef CONFIG_TARGET_TYPE_S32GEN1_SIMULATOR
#      define FDT_ADDR		0x82000000
#   else
#      define FDT_ADDR		0x83E00000
#endif

/*Kernel image load address */
#  define LOADADDR		0x80000000

/* Ramdisk load address */
#  define RAMDISK_ADDR		0x84000000

#if defined(CONFIG_SPI_FLASH) && defined(CONFIG_FSL_QSPI)

/* Flash Size and Num need to be updated according to the board's flash type */
#define FSL_QSPI_FLASH_SIZE            SZ_128M
#define FSL_QSPI_FLASH_NUM             2

#define QSPI0_BASE_ADDR                QSPI_BASE_ADDR
#define QSPI0_AMBA_BASE                CONFIG_SYS_FSL_FLASH0_BASE

#else

/* Flash comand disabled until implemented */
#undef CONFIG_CMD_FLASH

#define FLASH_BASE_ADR2		(CONFIG_SYS_FSL_FLASH0_BASE + 0x4000000)

#endif

#define S32_LOAD_FLASH_IMAGES_CMD\
	"sf probe 6:0;"\
	"sf read ${loadaddr} ${kernel_flashaddr} ${kernel_maxsize};"\
	"sf read ${fdt_addr} ${fdt_flashaddr} ${fdt_maxsize};"\
	"sf read ${ramdisk_addr} ${ramdisk_flashaddr} "\
	" ${ramdisk_maxsize};"

/* Note: The *_FLASH_ADDR and *_FLASH_MAXSIZE macros are used
 * with the 'setexpr' command. Therefore ensure none of them expand
 * into operations with more than two operands and avoid unnecessary
 * parantheses. Also these should be kept in sync with
 * 'conf/machine/include/s32*flashmap.inc'.
 */
#define CONFIG_SYS_FLASH_BASE		CONFIG_SYS_FSL_FLASH0_BASE
#define KERNEL_FLASH_MAXSIZE		0xe00000
#define FDT_FLASH_MAXSIZE		0x100000
#define RAMDISK_FLASH_MAXSIZE		0x2000000
#define UBOOT_FLASH_ADDR		(CONFIG_SYS_FSL_FLASH0_BASE + 0x0)

#define KERNEL_FLASH_ADDR	(CONFIG_SYS_FSL_FLASH0_BASE + 0x1f0000)
#define FDT_FLASH_ADDR	(CONFIG_SYS_FSL_FLASH0_BASE + 0xff0000)
#define RAMDISK_FLASH_ADDR	(CONFIG_SYS_FSL_FLASH0_BASE + 0x10f0000)

#if defined(CONFIG_ENV_IS_IN_FLASH) || defined(CONFIG_ENV_IS_IN_SPI_FLASH)

#if defined(CONFIG_ENV_OFFSET)
#define ENV_FLASH_ADDR	(CONFIG_SYS_FSL_FLASH0_BASE + CONFIG_ENV_OFFSET)
#else
#define ENV_FLASH_ADDR	(CONFIG_ENV_ADDR)
#endif

#if (ENV_FLASH_ADDR + CONFIG_ENV_SIZE > KERNEL_FLASH_ADDR)
#  error "Environment and Kernel would overlap in flash memory"
#endif

#endif

#if (KERNEL_FLASH_ADDR + KERNEL_FLASH_MAXSIZE > FDT_FLASH_ADDR)
#error "Kernel and FDT would overlap in flash memory"
#endif
#if (FDT_FLASH_ADDR + FDT_FLASH_MAXSIZE > RAMDISK_FLASH_ADDR)
#error "FDT and Ramdisk would overlap in flash memory"
#endif

#define ENV_FDTCONTROLADDR \
			"fdtcontroladdr=" __stringify(CONFIG_DTB_SRAM_ADDR) "\0"

/* Generic Timer Definitions */
#if defined(CONFIG_SYS_ARCH_TIMER)
/* COUNTER_FREQUENCY value will be used at startup but will be replaced
 * if an older chip version is determined at runtime.
 */
#if defined(CONFIG_S32_GEN1)
/* FXOSC_CLK; this will be further divided by "GPR00[26:24] + 1"
 * Note: CONFIG_TARGET_S32G2XXAEVB is a per-board configuration, as the value of
 * FXOSC_CLK itself is board-specific.
 */
#define COUNTER_FREQUENCY		(40 * 1000 * 1000)
#elif defined(CONFIG_TARGET_TYPE_S32GEN1_EMULATOR)
#define COUNTER_FREQUENCY				(1000)		    /* 1Khz */
#endif
#elif defined(CONFIG_SYS_PIT_TIMER)
#define COUNTER_FREQUENCY               (133056128)     /* 133MHz */
#else
#error "Unknown System Timer"
#endif

/* Size of malloc() pool */
#if defined(CONFIG_RUN_FROM_IRAM_ONLY) || defined(CONFIG_S32_SKIP_RELOC)
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 512 * 1024)
#else
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + 2 * 1024 * 1024)
#endif

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SYS_UART_PORT		(1)

#undef CONFIG_CMD_IMLS

#define CONFIG_HWCONFIG

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

#ifndef CONFIG_DCU_EXTRA_ENV_SETTINGS
#define CONFIG_DCU_EXTRA_ENV_SETTINGS	""
#endif

#ifndef S32_DEFAULT_IP
#define S32_DEFAULT_IP "10.0.0.100\0"
#endif

/*
 * Enable CONFIG_BOARD_USE_RAMFS_IN_NFSBOOT if u-boot should use a ramdisk
 * for nfsbooting.
 */
#ifdef CONFIG_BOARD_USE_RAMFS_IN_NFSBOOT
#define NFSRAMFS_ADDR "${ramdisk_addr}"
#define NFSRAMFS_TFTP_CMD "run loadtftpramdisk; "
#else
#define NFSRAMFS_ADDR "-"
#define NFSRAMFS_TFTP_CMD ""
#endif

#ifdef CONFIG_TARGET_S32R45EVB
#define GMAC1_ENABLE_VAR_VALUE \
	"s32cc_gmac1_mode=enable\0"
#else
#define GMAC1_ENABLE_VAR_VALUE ""
#endif /* CONFIG_TARGET_S32R45EVB */

#ifdef CONFIG_DWC_ETH_QOS_S32CC
#define GMAC_EXTRA_ENV_SETTINGS \
	"s32cc_gmac_mode=enable\0"\
	GMAC1_ENABLE_VAR_VALUE
#else
#define GMAC_EXTRA_ENV_SETTINGS ""
#endif /* CONFIG_DWC_ETH_QOS_S32CC */

#ifdef CONFIG_FSL_PFENG
#define PFENG_EXTRA_BOOT_ARGS " nohz=off coherent_pool=64M "
#if defined(CONFIG_TARGET_S32G2XXAEVB) || defined(CONFIG_TARGET_S32G3XXAEVB)
#define PFENG_MODE "enable,none,rgmii,rgmii"
#define PFENG_EMAC "1"
#endif
#ifdef CONFIG_TARGET_S32G274ARDB
#define PFENG_MODE "enable,sgmii,none,rgmii"
#define PFENG_EMAC "2"
#endif
#ifdef CONFIG_TARGET_S32G274ABLUEBOX3
#define PFENG_MODE "enable,sgmii,none,none"
#define PFENG_EMAC "0"
#endif
#define PFE_EXTRA_ENV_SETTINGS \
	"pfeng_mode=" PFENG_MODE "\0" \
	"pfeaddr=00:01:be:be:ef:11\0" \
	"pfe1addr=00:01:be:be:ef:22\0" \
	"pfe2addr=00:01:be:be:ef:33\0" \
	"ethact=eth_pfeng\0" \
	"pfengemac=" PFENG_EMAC "\0"
#define PFE_INIT_CMD "pfeng stop; "
#else
#define PFENG_EXTRA_BOOT_ARGS ""
#define PFE_EXTRA_ENV_SETTINGS ""
#define PFE_INIT_CMD ""
#endif

#if !defined(PCIE_EXTRA_ENV_SETTINGS)
#if defined(CONFIG_PCIE_S32GEN1) || defined(CONFIG_FSL_PFENG)
#define PCIE_EXTRA_ENV_SETTINGS \
	"hwconfig=" CONFIG_S32GEN1_HWCONFIG "\0"
#else
#define PCIE_EXTRA_ENV_SETTINGS ""
#endif
#endif

#define FDT_ENABLE_HS400ES \
	"fdt_enable_hs400es=" \
		"fdt addr ${fdt_addr}; " \
		"fdt rm /usdhc no-1-8-v; " \
		"fdt resize; \0" \

#define CONFIG_FLASHBOOT_RAMDISK " ${ramdisk_addr} "

#ifdef CONFIG_XEN_SUPPORT
#define XEN_EXTRA_ENV_SETTINGS \
	"script_addr=0x80200000\0" \
	"mmcpart_ext=" __stringify(MMC_PART_EXT) "\0" \

#define XEN_BOOTCMD \
	"ext4load mmc ${mmcdev}:${mmcpart_ext} ${script_addr} " \
		"boot/${script}; source ${script_addr}; " \
		"booti ${xen_addr} - ${fdt_addr};"
#else
#define XEN_EXTRA_ENV_SETTINGS  ""
#endif

#define INITRD_HIGH_DEFAULT 0xffffffff

/* Leave room for TF-A & OPTEE */
#if defined(CONFIG_S32_ATF_BOOT_FLOW)
#define INITRD_HIGH 0xFE1FFFFF
#else
#define INITRD_HIGH INITRD_HIGH_DEFAULT
#endif

#define CONFIG_EXTRA_ENV_SETTINGS \
	CONFIG_BOARD_EXTRA_ENV_SETTINGS  \
	CONFIG_DCU_EXTRA_ENV_SETTINGS \
	"ipaddr=" S32_DEFAULT_IP \
	ENV_FDTCONTROLADDR \
	"serverip=10.0.0.1\0" \
	"netmask=255.255.255.0\0" \
	"nfsbootargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs rw " \
		"ip=${ipaddr}:${serverip}::${netmask}::" CONFIG_BOARD_NFS_BOOT_INTERFACE ":off " \
		"nfsroot=${serverip}:/tftpboot/rfs,nolock,v3,tcp " \
		"earlycon " PFENG_EXTRA_BOOT_ARGS "\0" \
	"loadtftpimage=tftp ${loadaddr} ${image};\0" \
	"loadtftpramdisk=tftp ${ramdisk_addr} ${ramdisk};\0" \
	"loadtftpfdt=tftp ${fdt_addr} ${fdt_file};\0" \
	"nfsboot=echo Booting from net using tftp and nfs...; " \
		"run nfsbootargs;"\
		"run loadtftpimage; " NFSRAMFS_TFTP_CMD "run loadtftpfdt;"\
		"${boot_mtd} ${loadaddr} " NFSRAMFS_ADDR " ${fdt_addr};\0" \
	"script=boot.scr\0" \
	"boot_mtd=" __stringify(BOOT_MTD) "\0" \
	"image=" __stringify(IMAGE_NAME) "\0" \
	"ramdisk=" __stringify(RAMDISK_NAME) "\0"\
	"console=ttyLF" __stringify(CONFIG_FSL_LINFLEX_MODULE) "\0" \
	"fdt_high=0xa0000000\0" \
	"initrd_high=" __stringify(INITRD_HIGH) "\0" \
	"fdt_file="  __stringify(FDT_FILE) "\0" \
	"fdt_addr=" __stringify(FDT_ADDR) "\0" \
	"ramdisk_addr=" __stringify(RAMDISK_ADDR) "\0" \
	"boot_fdt=try\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=" __stringify(CONFIG_MMC_PART) "\0" \
	"mmcroot=/dev/mmcblk0p2 rootwait rw\0" \
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
				"mmc write ${loadaddr} 0x8 ${fw_sz}; " \
			"fi; "	\
		"fi\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate}" \
		" root=${mmcroot} earlycon " \
		PFENG_EXTRA_BOOT_ARGS "\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadramdisk=fatload mmc ${mmcdev}:${mmcpart} ${ramdisk_addr} ${ramdisk}\0" \
	FDT_ENABLE_HS400ES \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}; " \
		 "${fdt_override}\0" \
	"jtagboot=echo Booting using jtag...; " \
		"${boot_mtd} ${loadaddr} ${ramdisk_addr} ${fdt_addr}\0" \
	"jtagsdboot=echo Booting loading Linux with ramdisk from SD...; " \
		"run loadimage; run loadramdisk; run loadfdt;"\
		"${boot_mtd} ${loadaddr} ${ramdisk_addr} ${fdt_addr}\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if run loadfdt; then " \
			"${boot_mtd} ${loadaddr} - ${fdt_addr}; " \
		"else " \
			"echo WARN: Cannot load the DT; " \
		"fi;\0" \
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp " \
		"earlycon " PFENG_EXTRA_BOOT_ARGS "\0" \
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
		"fi;\0" \
	"flashbootargs=setenv bootargs console=${console},${baudrate}" \
		" root=/dev/ram rw earlycon " \
		PFENG_EXTRA_BOOT_ARGS ";" \
		"setexpr uboot_flashaddr " __stringify(UBOOT_FLASH_ADDR) ";" \
		"setexpr kernel_flashaddr " __stringify(KERNEL_FLASH_ADDR) ";" \
		"setenv kernel_maxsize " __stringify(KERNEL_FLASH_MAXSIZE) ";" \
		"setexpr fdt_flashaddr " __stringify(FDT_FLASH_ADDR) ";" \
		"setenv fdt_maxsize " __stringify(FDT_FLASH_MAXSIZE) ";" \
		"setexpr ramdisk_flashaddr " \
				__stringify(RAMDISK_FLASH_ADDR) ";" \
		"setenv ramdisk_maxsize " \
				__stringify(RAMDISK_FLASH_MAXSIZE) ";\0" \
	"flashboot=echo Booting from flash...; " \
		"run flashbootargs;"\
		S32_LOAD_FLASH_IMAGES_CMD\
		"${boot_mtd} ${loadaddr}" CONFIG_FLASHBOOT_RAMDISK \
		"${fdt_addr};\0" \
	XEN_EXTRA_ENV_SETTINGS \
	GMAC_EXTRA_ENV_SETTINGS \
	PFE_EXTRA_ENV_SETTINGS \
	PCIE_EXTRA_ENV_SETTINGS \

#undef CONFIG_BOOTCOMMAND

#if defined(CONFIG_TARGET_TYPE_S32GEN1_EMULATOR) || \
	defined(CONFIG_TARGET_TYPE_S32GEN1_SIMULATOR)
#  define CONFIG_BOOTCOMMAND \
		"${boot_mtd} ${loadaddr} - ${fdt_addr}"
#elif defined(CONFIG_QSPI_BOOT)
#  define CONFIG_BOOTCOMMAND \
	PFE_INIT_CMD "run flashboot"
#elif defined(CONFIG_SD_BOOT)
#ifdef CONFIG_XEN_SUPPORT
#  define CONFIG_BOOTCOMMAND XEN_BOOTCMD
#else
#  define CONFIG_BOOTCOMMAND \
	PFE_INIT_CMD "mmc dev ${mmcdev}; if mmc rescan; then " \
		   "if run loadimage; then " \
			   "run mmcboot; " \
		   "else run netboot; " \
		   "fi; " \
	   "else run netboot; fi"
#endif
#endif

/* Miscellaneous configurable options */
#define CONFIG_SYS_PROMPT_HUSH_PS2	"> "
#define CONFIG_SYS_PROMPT		"=> "
#define CONFIG_SYS_CBSIZE		256	/* Console I/O Buffer Size */
#define CONFIG_SYS_PBSIZE		\
			(CONFIG_SYS_CBSIZE + sizeof(CONFIG_SYS_PROMPT) + 16)
#define CONFIG_SYS_MAXARGS		16	/* max number of command args */
#define CONFIG_SYS_BARGSIZE		CONFIG_SYS_CBSIZE

/* Limit mtest to first DDR bank if no arguments are given */
#define CONFIG_SYS_MEMTEST_START	(CONFIG_SYS_FSL_DRAM_BASE1)
#define CONFIG_SYS_MEMTEST_END		(CONFIG_SYS_FSL_DRAM_BASE1 + \
					 CONFIG_SYS_FSL_DRAM_SIZE1)

#define CONFIG_SYS_LOAD_ADDR		CONFIG_LOADADDR
#define CONFIG_SYS_HZ			1000

#ifdef CONFIG_RUN_FROM_IRAM_ONLY
#define CONFIG_SYS_MALLOC_BASE		(CONFIG_SYS_FSL_DRAM_BASE1)
#endif

#if defined(CONFIG_S32_SKIP_RELOC) && !defined(CONFIG_S32_ATF_BOOT_FLOW)
#define CONFIG_SYS_SDRAM_BASE		S32_SRAM_BASE
#else
#define CONFIG_SYS_SDRAM_BASE		CONFIG_SYS_FSL_DRAM_BASE1
#endif

#define CONFIG_SYS_INIT_RAM_ADDR	IRAM_BASE_ADDR
#define CONFIG_SYS_INIT_RAM_SIZE	IRAM_SIZE

/* PGTABLE_SIZE offset */
#define CONFIG_SYS_INIT_SP_OFFSET	(16 * 1024)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_INIT_RAM_ADDR + CONFIG_SYS_INIT_SP_OFFSET)

#define S32_MMU_TABLES_OFFSET		CONFIG_SYS_INIT_SP_OFFSET
#define S32_IRAM_MMU_TABLES_BASE	(IRAM_BASE_ADDR + S32_MMU_TABLES_OFFSET)

#define CONFIG_SYS_MMC_ENV_DEV		0
#define MMC_PART_FAT			1
#define MMC_PART_EXT			2
#define CONFIG_MMC_PART			MMC_PART_FAT

#ifdef CONFIG_S32_GEN1
#define FLASH_SECTOR_SIZE               (64 * 1024) /* 64 KB */
#else
#define FLASH_SECTOR_SIZE		0x40000 /* 256 KB */
#endif

#if defined(CONFIG_QSPI_BOOT)
#define CONFIG_SYS_MAX_FLASH_BANKS	1
#define CONFIG_SYS_MAX_FLASH_SECT 	(0x4000000 / CONFIG_ENV_SECT_SIZE)
#endif

#define CONFIG_BOOTP_BOOTFILESIZE

#define CONFIG_SYS_PCI_64BIT

#define CONFIG_SYS_LDSCRIPT  "arch/arm/cpu/armv8/u-boot.lds"

#endif /* __S32_H */
