/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022 NXP
 */
#ifndef __S32CC_H__
#define __S32CC_H__

#include <linux/sizes.h>
#include <generated/autoconf.h>

/* memory mapped external flash */
#define CONFIG_SYS_FLASH_BASE		0x0UL
#define CONFIG_SYS_FLASH_SIZE		(SZ_512M)

#define PHYS_SDRAM_1			0x80000000UL
#define PHYS_SDRAM_1_SIZE		(SZ_2G)
#define PHYS_SDRAM_2			0x880000000UL
#define PHYS_SDRAM_2_SIZE		(SZ_2G)

#define S32CC_SRAM_BASE			0x34000000

/**
 *
 * Before changing the device tree offset or size, please read
 * https://docs.kernel.org/arm64/booting.html#setup-the-device-tree
 * and doc/README.distro
 *
 * DDR images layout
 *
 * Name				Size	Address
 *
 * Image			46M	CONFIG_SYS_LOAD_ADDR
 * PXE				1M	CONFIG_SYS_LOAD_ADDR + 46M
 * boot.scr			1M	CONFIG_SYS_LOAD_ADDR + 47M
 * Linux DTB			2M	CONFIG_SYS_LOAD_ADDR + 48M
 * Reserved memory regions	206	CONFIG_SYS_LOAD_ADDR + 50M
 * Ramdisk			-	CONFIG_SYS_LOAD_ADDR + 256M
 */
#define S32CC_PXE_ADDR			0x82E00000
#define S32CC_BOOT_SCR_ADDR		0x82F00000
#define S32CC_FDT_ADDR			0x83000000
#define S32CC_RAMDISK_ADDR		0x90000000

/* Disable Ramdisk & FDT relocation*/
#define S32CC_INITRD_HIGH_ADDR		0xffffffffffffffff
#define S32CC_FDT_HIGH_ADDR		0xffffffffffffffff

#define CONFIG_SYS_INIT_SP_OFFSET	(SZ_16K)
#define CONFIG_SYS_INIT_SP_ADDR \
	(CONFIG_SYS_DATA_BASE + CONFIG_SYS_INIT_SP_OFFSET)

#define CONFIG_SYS_MMC_ENV_DEV		0
#define MMC_PART_FAT			1

/**
 * Note: The *_FLASH_ADDR and *_FLASH_MAXSIZE macros are used
 * with the 'setexpr' command. Therefore ensure none of them expand
 * into operations with more than two operands and avoid unnecessary
 * parantheses. Also these should be kept in sync with
 * 'conf/machine/include/s32*flashmap.inc'.
 *
 * QSPI flash map:
 *
 * Name		Size			Offset
 * FIP		~1.9M			0x0
 * QSPI env	64K(CONFIG_ENV_SIZE)	0x01e0000(CONFIG_ENV_OFFSET)
 * Image	14M			0x0e00000
 * Linux DTB	1M			0x0ff0000
 * Ramdisk	32M			0x10f0000
 */
#define KERNEL_FLASH_MAXSIZE		0x0e00000
#define FDT_FLASH_MAXSIZE		0x0100000
#define RAMDISK_FLASH_MAXSIZE		0x2000000
#define FIP_FLASH_ADDR			0x0000000
#define KERNEL_FLASH_ADDR		0x01f0000
#define FDT_FLASH_ADDR			0x0ff0000
#define RAMDISK_FLASH_ADDR		0x10f0000

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

#define S32CC_ENV_SETTINGS \
	BOOTENV \
	"boot_mtd=booti\0" \
	"console=ttyLF0\0" \
	"fdt_addr=" __stringify(S32CC_FDT_ADDR) "\0" \
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
		"setenv fip_flashaddr " __stringify(FIP_FLASH_ADDR) ";" \
		"setenv kernel_flashaddr " __stringify(KERNEL_FLASH_ADDR) ";" \
		"setenv flashsize " __stringify(FSL_QSPI_FLASH_SIZE) ";" \
		"setenv kernel_maxsize " __stringify(KERNEL_FLASH_MAXSIZE) ";" \
		"setenv fdt_flashaddr " __stringify(FDT_FLASH_ADDR) ";" \
		"setenv fdt_maxsize " __stringify(FDT_FLASH_MAXSIZE) ";" \
		"setenv ramdisk_flashaddr " \
				__stringify(RAMDISK_FLASH_ADDR) ";" \
		"setenv ramdisk_maxsize " \
				__stringify(RAMDISK_FLASH_MAXSIZE) ";\0" \
	"image=Image\0" \
	"initrd_high=" __stringify(S32CC_INITRD_HIGH_ADDR) "\0" \
	"loadfdt=fatload mmc ${mmcdev}:${mmcpart} ${fdt_addr} ${fdt_file}; " \
		 "run fdt_override;\0" \
	"loadimage=fatload mmc ${mmcdev}:${mmcpart} ${loadaddr} ${image}\0" \
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
	"ramdisk_addr=" __stringify(S32CC_RAMDISK_ADDR) "\0" \

#ifdef CONFIG_BOOTCOMMAND
#undef CONFIG_BOOTCOMMAND
#endif

#if defined(CONFIG_DISTRO_DEFAULTS)
#  define BOOT_TARGET_DEVICES(func) \
	func(MMC, mmc, CONFIG_SYS_MMC_ENV_DEV)
/*
 * Variables required by doc/README.distro
 */
#  define DISTRO_VARS \
	"fdt addr ${fdtcontroladdr};" \
	"fdt header get fdt_size totalsize;" \
	"cp.b ${fdtcontroladdr} ${fdt_addr} ${fdt_size};" \
	"setenv fdt_addr_r ${fdt_addr};" \
	"setenv ramdisk_addr_r " __stringify(S32CC_RAMDISK_ADDR) ";" \
	"setenv kernel_addr_r ${loadaddr};" \
	"setenv pxefile_addr_r " __stringify(S32CC_PXE_ADDR) ";" \
	"setenv scriptaddr " __stringify(S32CC_BOOT_SCR_ADDR) ";"
/*
 * Remove pinmuxing properties as SIUL2 driver isn't upstreamed yet
 */
#  define DISTRO_FIXUPS \
	"fdt addr ${fdt_addr_r};" \
	"fdt rm serial0 pinctrl-0;" \
	"fdt rm serial0 pinctrl-names;" \
	"fdt rm mmc0 pinctrl-0;" \
	"fdt rm mmc0 pinctrl-1;" \
	"fdt rm mmc0 pinctrl-2;" \
	"fdt rm mmc0 pinctrl-names;" \
	"fdt rm mmc0 mmc-ddr-1_8v;" \
	"fdt rm mmc0 clock-frequency;"
#  define CONFIG_BOOTCOMMAND \
	DISTRO_VARS \
	DISTRO_FIXUPS \
	"run distro_bootcmd"
#  include <config_distro_bootcmd.h>
#else
#  define BOOTENV
#  if defined(CONFIG_QSPI_BOOT)
#    define CONFIG_BOOTCOMMAND "run flashboot"
#  else
#    define CONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; " \
	"if mmc rescan; " \
	"then " \
		"if run loadimage; "\
		"then " \
			"run mmcboot; " \
		"fi; " \
	"fi"
#  endif
#endif

#ifdef CONFIG_SYS_I2C_MXC
#  define I2C_QUIRK_REG
#endif

#if defined(CONFIG_SPI_FLASH) && defined(CONFIG_FSL_QSPI)
#	ifdef FSL_QSPI_FLASH_SIZE
#		undef FSL_QSPI_FLASH_SIZE
#	endif
#	define FSL_QSPI_FLASH_SIZE	SZ_64M
#endif

#define CONFIG_SYS_CBSIZE		(SZ_512)
#endif
