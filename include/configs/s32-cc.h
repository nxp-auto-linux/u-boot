/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017-2022 NXP
 *
 */

/*
 * Configuration settings for the NXP S32 CC SoC.
 */

#ifndef __S32CC_H__
#define __S32CC_H__

#include <linux/sizes.h>
#include <generated/autoconf.h>

/* memory mapped external flash */
#define CONFIG_SYS_FLASH_BASE		0x0UL
#define CONFIG_SYS_FLASH_SIZE		(SZ_512M)

#define PHYS_SDRAM_1			0x80000000UL
#ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
#define PHYS_SDRAM_1_SIZE		(SZ_1G)
#else
#define PHYS_SDRAM_1_SIZE		(SZ_2G)
#define PHYS_SDRAM_2			0x880000000UL
#define PHYS_SDRAM_2_SIZE		(SZ_2G)
#endif

/* DDR images layout */
#define S32CC_FDT_ADDR			0x83E00000
#define S32CC_RAMDISK_ADDR		0x84000000
#define S32CC_FDT_HIGH_ADDR		0xa0000000
#define S32CC_INITRD_HIGH_ADDR		0xfe1fffff

#define CONFIG_SYS_INIT_SP_OFFSET	(SZ_16K)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_DATA_BASE + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_MMC_ENV_DEV		0
#define MMC_PART_FAT			1
#define MMC_PART_EXT			2

#ifdef CONFIG_CMD_IRQ
#  define CONFIG_GICSUPPORT
#  define CONFIG_USE_IRQ
#endif

#define CONFIG_SKIP_LOWLEVEL_INIT

/* Enable passing of ATAGs */
#define CONFIG_CMDLINE_TAG

#ifndef CONFIG_XEN_SUPPORT
#  define CONFIG_ARMV8_SWITCH_TO_EL1
#endif

/* Increase image size */
#define CONFIG_SYS_BOOTM_LEN		(SZ_64M)


/* Note: The *_FLASH_ADDR and *_FLASH_MAXSIZE macros are used
 * with the 'setexpr' command. Therefore ensure none of them expand
 * into operations with more than two operands and avoid unnecessary
 * parantheses. Also these should be kept in sync with
 * 'conf/machine/include/s32*flashmap.inc'.
 */
#define KERNEL_FLASH_MAXSIZE		0xe00000
#define FDT_FLASH_MAXSIZE		0x100000
#define RAMDISK_FLASH_MAXSIZE		0x2000000
#define FIP_FLASH_ADDR			CONFIG_SYS_FLASH_BASE

#define KERNEL_FLASH_ADDR		(CONFIG_SYS_FLASH_BASE + 0x1f0000UL)
#define FDT_FLASH_ADDR			(CONFIG_SYS_FLASH_BASE + 0xff0000UL)
#define RAMDISK_FLASH_ADDR		(CONFIG_SYS_FLASH_BASE + 0x10f0000UL)

#if defined(CONFIG_ENV_IS_IN_SPI_FLASH)
#  if (CONFIG_ENV_OFFSET + CONFIG_ENV_SIZE > KERNEL_FLASH_ADDR)
#    error "Environment and Kernel would overlap in flash memory"
#  endif
#endif
#if (KERNEL_FLASH_ADDR + KERNEL_FLASH_MAXSIZE > FDT_FLASH_ADDR)
#  error "Kernel and FDT would overlap in flash memory"
#endif
#if (FDT_FLASH_ADDR + FDT_FLASH_MAXSIZE > RAMDISK_FLASH_ADDR)
#  error "FDT and Ramdisk would overlap in flash memory"
#endif

/* Size of malloc() pool */
#define CONFIG_SYS_MALLOC_LEN		(CONFIG_ENV_SIZE + SZ_2M)

/* Allow to overwrite serial and ethaddr */
#define CONFIG_ENV_OVERWRITE

#ifdef CONFIG_CMD_IMLS
#undef CONFIG_CMD_IMLS
#endif

#define CONFIG_HWCONFIG

#define S32CC_DEFAULT_IP		"10.0.0.100"
#define S32CC_NETMASK			"255.255.255.0"

/*
 * In case U-Boot needs a ramdisk for booting over NFS
 */
#define NFSRAMFS_ADDR			"-"
#define NFSRAMFS_TFTP_CMD		""

#ifdef CONFIG_DWC_ETH_QOS_S32CC
#  define GMAC_EXTRA_ENV_SETTINGS	"s32cc_gmac_mode=enable\0"\
					GMAC1_ENABLE_VAR_VALUE
#else
#  define GMAC_EXTRA_ENV_SETTINGS	""
#endif

#if defined(CONFIG_S32GEN1_HWCONFIG)
#  define PCIE_EXTRA_ENV_SETTINGS "hwconfig=" CONFIG_S32GEN1_HWCONFIG "\0"
#else
#  define PCIE_EXTRA_ENV_SETTINGS ""
#endif

#ifdef CONFIG_XEN_SUPPORT
#  define XEN_EXTRA_ENV_SETTINGS \
	"script_addr=0x80200000\0" \
	"mmcpart_ext=" __stringify(MMC_PART_EXT) "\0" \

#  define XEN_BOOTCMD \
	"ext4load mmc ${mmcdev}:${mmcpart_ext} ${script_addr} " \
		"boot/${script}; source ${script_addr}; " \
		"booti ${xen_addr} - ${fdt_addr};"
#else
#  define XEN_EXTRA_ENV_SETTINGS  ""
#endif

#define S32CC_ENV_SETTINGS \
	"boot_fdt=try\0" \
	"boot_mtd=booti\0" \
	"bootscript=echo Running bootscript from mmc ...; " \
		"source\0" \
	"console=ttyLF0\0" \
	"fdt_addr=" __stringify(S32CC_FDT_ADDR) "\0" \
	"fdt_enable_hs400es=" \
		"fdt addr ${fdt_addr}; " \
		"fdt rm /usdhc no-1-8-v; " \
		"fdt resize; \0" \
	"fdt_file=" FDT_FILE "\0" \
	"fdt_high=" __stringify(S32CC_FDT_HIGH_ADDR) "\0" \
	"fdt_override=;\0" \
	"flashboot=echo Booting from flash...; " \
		"run flashbootargs;"\
		"sf probe 6:0;"\
		"sf read ${loadaddr} ${kernel_flashaddr} ${kernel_maxsize};"\
		"sf read ${fdt_addr} ${fdt_flashaddr} ${fdt_maxsize};"\
		"sf read ${ramdisk_addr} ${ramdisk_flashaddr} "\
		" ${ramdisk_maxsize};" \
		"${boot_mtd} ${loadaddr} ${ramdisk_addr} ${fdt_addr};\0" \
	"flashbootargs=setenv bootargs console=${console},${baudrate}" \
		" root=/dev/ram rw earlycon " EXTRA_BOOT_ARGS ";"\
		"setexpr fip_flashaddr " __stringify(FIP_FLASH_ADDR) ";" \
		"setexpr kernel_flashaddr " __stringify(KERNEL_FLASH_ADDR) ";" \
		"setenv kernel_maxsize " __stringify(KERNEL_FLASH_MAXSIZE) ";" \
		"setexpr fdt_flashaddr " __stringify(FDT_FLASH_ADDR) ";" \
		"setenv fdt_maxsize " __stringify(FDT_FLASH_MAXSIZE) ";" \
		"setexpr ramdisk_flashaddr " \
				__stringify(RAMDISK_FLASH_ADDR) ";" \
		"setenv ramdisk_maxsize " \
				__stringify(RAMDISK_FLASH_MAXSIZE) ";\0" \
	"image=Image\0" \
	"initrd_high=" __stringify(S32CC_INITRD_HIGH_ADDR) "\0" \
	"ipaddr=" S32CC_DEFAULT_IP "\0"\
	"jtagboot=echo Booting using jtag...; " \
		"${boot_mtd} ${loadaddr} ${ramdisk_addr} ${fdt_addr}\0" \
	"jtagsdboot=echo Booting loading Linux with ramdisk from SD...; " \
		"run loadimage; run loadramdisk; run loadfdt;"\
		"${boot_mtd} ${loadaddr} ${ramdisk_addr} ${fdt_addr}\0" \
	"loadbootscript=" \
		"fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${script};\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}; " \
		 "run fdt_override;\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
	"loadtftpfdt=tftp ${fdt_addr} ${fdt_file};\0" \
	"loadtftpimage=tftp ${loadaddr} ${image};\0" \
	"mmcargs=setenv bootargs console=${console},${baudrate}" \
		" root=${mmcroot} earlycon " EXTRA_BOOT_ARGS "\0" \
	"mmcboot=echo Booting from mmc ...; " \
		"run mmcargs; " \
		"if run loadfdt; then " \
			"${boot_mtd} ${loadaddr} - ${fdt_addr}; " \
		"else " \
			"echo WARN: Cannot load the DT; " \
		"fi;\0" \
	"mmcdev=" __stringify(CONFIG_SYS_MMC_ENV_DEV) "\0" \
	"mmcpart=" __stringify(MMC_PART_FAT) "\0" \
	"mmcroot=/dev/mmcblk0p2 rootwait rw\0" \
	"netargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs " \
		"ip=dhcp nfsroot=${serverip}:${nfsroot},v3,tcp " \
		"earlycon " EXTRA_BOOT_ARGS "\0" \
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
	"netmask=" S32CC_NETMASK "\0" \
	"nfsboot=echo Booting from net using tftp and nfs...; " \
		"run nfsbootargs;"\
		"run loadtftpimage; " NFSRAMFS_TFTP_CMD "run loadtftpfdt;"\
		"${boot_mtd} ${loadaddr} " NFSRAMFS_ADDR " ${fdt_addr};\0" \
	"nfsbootargs=setenv bootargs console=${console},${baudrate} " \
		"root=/dev/nfs rw " \
		"ip=${ipaddr}:${serverip}::${netmask}::" \
			CONFIG_BOARD_NFS_BOOT_INTERFACE ":off " \
		"nfsroot=${serverip}:/tftpboot/rfs,nolock,v3,tcp " \
		"earlycon " EXTRA_BOOT_ARGS "\0" \
	"ramdisk_addr=" __stringify(S32CC_RAMDISK_ADDR) "\0" \
	"script=boot.scr\0" \
	"serverip=10.0.0.1\0" \
	"update_sd_firmware_filename=fip.s32\0" \
	"update_sd_firmware=" \
		"if test ${ip_dyn} = yes; " \
		"then " \
			"setenv get_cmd dhcp; " \
		"else " \
			"setenv get_cmd tftp; " \
		"fi; " \
		"if mmc dev ${mmcdev}; then "	\
			"if ${get_cmd} ${update_sd_firmware_filename}; then " \
				"setexpr fw_sz ${filesize} / 0x200; " \
				"setexpr fw_sz ${fw_sz} - 7; "	\
				"setexpr loadaddr ${loadaddr} + 0x1000; " \
				"mmc write ${loadaddr} 0x8 ${fw_sz}; " \
			"fi; "	\
		"fi\0" \
	GMAC_EXTRA_ENV_SETTINGS \
	PCIE_EXTRA_ENV_SETTINGS \
	XEN_EXTRA_ENV_SETTINGS \

#if defined(CONFIG_TARGET_TYPE_S32GEN1_EMULATOR)
#  define BOOTCOMMAND "${boot_mtd} ${loadaddr} - ${fdt_addr}"
#elif defined(CONFIG_QSPI_BOOT)
#  define BOOTCOMMAND "run flashboot"
#elif defined(CONFIG_SD_BOOT)
#  ifdef CONFIG_XEN_SUPPORT
#    define BOOTCOMMAND XEN_BOOTCMD
#  else
#    define BOOTCOMMAND \
	"mmc dev ${mmcdev}; " \
	"if mmc rescan; " \
	"then " \
		"if run loadimage; "\
		"then " \
			"run mmcboot; " \
		"else " \
			"run netboot; " \
		"fi; " \
	"else " \
		"run netboot;" \
	"fi"
#  endif
#endif

#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif
#define CONFIG_BOOTCOMMAND \
	EXTRA_BOOTCOMMAND \
	BOOTCOMMAND

/* Limit mtest to first DDR bank if no arguments are given */
#define CONFIG_SYS_MEMTEST_START	(PHYS_SDRAM_1)
#define CONFIG_SYS_MEMTEST_END		(PHYS_SDRAM_1 + \
					 PHYS_SDRAM_1_SIZE)

/*Kernel image load address */
#define CONFIG_LOADADDR			CONFIG_SYS_LOAD_ADDR
#define CONFIG_SYS_HZ			1000

#define CONFIG_SYS_SDRAM_BASE		PHYS_SDRAM_1

#ifdef CONFIG_SYS_I2C_MXC
#  define I2C_QUIRK_REG
#endif

#define CONFIG_BOOTP_BOOTFILESIZE

#define CONFIG_SYS_PCI_64BIT

#define CONFIG_SYS_LDSCRIPT		"arch/arm/cpu/armv8/u-boot.lds"

#define S32CC_SRAM_BASE			0x34000000

#if defined(CONFIG_SPI_FLASH) && defined(CONFIG_FSL_QSPI)
#  define CONFIG_SYS_FSL_QSPI_AHB
#  ifdef FSL_QSPI_FLASH_SIZE
#    undef FSL_QSPI_FLASH_SIZE
#  endif
#  define FSL_QSPI_FLASH_SIZE		(SZ_64M)
#endif

#endif
