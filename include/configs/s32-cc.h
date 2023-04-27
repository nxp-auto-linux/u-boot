/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022-2023 NXP
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

#define CONFIG_HWCONFIG

#if defined(CONFIG_S32CC_HWCONFIG)
#  define SERDES_EXTRA_ENV_SETTINGS "hwconfig=" CONFIG_S32CC_HWCONFIG "\0"
#else
#  define SERDES_EXTRA_ENV_SETTINGS ""
#endif

#define S32CC_ENV_SETTINGS \
	BOOTENV \
	"boot_mtd=booti\0" \
	"console=ttyLF0\0" \
	"fdt_addr=" __stringify(S32CC_FDT_ADDR) "\0" \
	"fdt_enable_hs400es=" \
		"fdt addr ${fdt_addr}; " \
		"fdt rm /soc/mmc no-1-8-v; " \
		"fdt resize; \0" \
	"fdt_file=" FDT_FILE "\0" \
	"fdt_high=" __stringify(S32CC_FDT_HIGH_ADDR) "\0" \
	"fdt_override=;\0" \
	"flashboot=echo Booting from flash...; " \
		"run flashbootargs;"\
		"mtd read Kernel ${loadaddr};"\
		"mtd read DTB ${fdt_addr};"\
		"mtd read Rootfs ${ramdisk_addr};"\
		"${boot_mtd} ${loadaddr} ${ramdisk_addr} ${fdt_addr};\0" \
	"flashbootargs=setenv bootargs console=${console},${baudrate}" \
		" root=/dev/ram rw earlycon " EXTRA_BOOT_ARGS ";"\
		"setenv flashsize " __stringify(FSL_QSPI_FLASH_SIZE) ";\0" \
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
	SERDES_EXTRA_ENV_SETTINGS

#if defined(CONFIG_TARGET_TYPE_S32CC_EMULATOR)
#  define BOOTCOMMAND "${boot_mtd} ${loadaddr} - ${fdt_addr}"
#elif defined(CONFIG_QSPI_BOOT)
#  define BOOTCOMMAND "run flashboot"
#elif defined(CONFIG_SD_BOOT)
#  define BOOTCOMMAND \
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
#endif

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
