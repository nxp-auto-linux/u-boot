/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2022-2024 NXP
 */
#ifndef __S32CC_H__
#define __S32CC_H__

#include <configs/s32.h>

/* memory mapped external flash */
#define CONFIG_SYS_FLASH_BASE		0x0UL
#define CONFIG_SYS_FLASH_SIZE		(SZ_512M)

#define PHYS_SDRAM_1			0x80000000UL
#define PHYS_SDRAM_1_SIZE		(SZ_2G)
#define PHYS_SDRAM_2			0x880000000UL
#define PHYS_SDRAM_2_SIZE		(SZ_2G)

#define S32CC_SRAM_BASE			0x34000000

#ifndef CONFIG_SYS_BAUDRATE_TABLE
#define CONFIG_SYS_BAUDRATE_TABLE    { 9600, 19200, 38400, 57600, 115200, \
				       921600, 1000000, 1500000, 2000000 }
#endif

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

#define MMC_PART_FAT			2
#define MMC_PART_EXT			3
#define MMC_ROOT			"/dev/mmcblk0p3"

#define NFSRAMFS_ADDR			"-"
#define NFSRAMFS_TFTP_CMD		""

#if defined(CONFIG_CMD_NET)
#define S32CC_IPADDR			"10.0.0.100"
#define S32CC_NETMASK			"255.255.255.0"
#define S32CC_SERVERIP			"10.0.0.1"
#else
#define S32CC_IPADDR			""
#define S32CC_NETMASK			""
#define S32CC_SERVERIP			""
#endif

#define CONFIG_HWCONFIG

#if CONFIG_CONS_INDEX == 1
#  define S32_LINFLEX_MODULE 0
#elif CONFIG_CONS_INDEX == 2
#  define S32_LINFLEX_MODULE 1
#elif CONFIG_CONS_INDEX == 3
#  define S32_LINFLEX_MODULE 2
#elif CONFIG_CONS_INDEX == 4
#  define S32_LINFLEX_MODULE 3
#elif CONFIG_CONS_INDEX == 5
#  define S32_LINFLEX_MODULE 4
#elif CONFIG_CONS_INDEX == 6
#  define S32_LINFLEX_MODULE 5
#elif CONFIG_CONS_INDEX == 7
#  define S32_LINFLEX_MODULE 6
#else
#  error "The CONFIG_CONS_INDEX can be in the range 1(serial0)-7(serial6)"
#endif

#if defined(CONFIG_S32CC_HWCONFIG)
#  define SERDES_EXTRA_ENV_SETTINGS "hwconfig=" CONFIG_S32CC_HWCONFIG "\0"
#else
#  define SERDES_EXTRA_ENV_SETTINGS ""
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

#if defined(CONFIG_LINUX_FDT_HS400_FIXUP) && \
	!defined(CONFIG_FIT_SIGNATURE)
#  define FDT_FIXUP \
	"run fdt_enable_hs400;"
#else
#  define FDT_FIXUP ";"
#endif

#define S32CC_ENV_SETTINGS \
	S32_ENV_SETTINGS \
	"console=ttyLF" __stringify(S32_LINFLEX_MODULE) "\0" \
	"fdt_enable_hs400=" \
		"fdt addr ${fdt_addr}; " \
		"fdt rm /soc/mmc no-1-8-v; " \
		"fdt resize; \0" \
	"flashboot=echo Booting from flash...; " \
		"run flashbootargs;"\
		"mtd read Kernel ${loadaddr};"\
		"mtd read DTB ${fdt_addr};"\
		"mtd read Rootfs ${ramdisk_addr};"\
		"${boot_mtd} ${loadaddr} ${ramdisk_addr} ${fdt_addr};\0" \
	"flashbootargs=setenv bootargs console=${console},${baudrate}" \
		" root=/dev/ram rw" LINUX_EARLY_CONSOLE \
		LINUX_LOG_DISABLE EXTRA_BOOT_ARGS ";"\
		"setenv flashsize " __stringify(FSL_QSPI_FLASH_SIZE) ";\0" \
	"ipaddr=" S32CC_IPADDR "\0"\
	"loadtftpfdt=tftp ${fdt_addr} ${fdt_file};\0" \
	"loadtftpimage=tftp ${loadaddr} ${image};\0" \
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
	"script=boot.scr\0" \
	"serverip=" S32CC_SERVERIP "\0" \
	SERDES_EXTRA_ENV_SETTINGS \
	XEN_EXTRA_ENV_SETTINGS \

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
#    if defined(CONFIG_FIT_SIGNATURE)
#        define PRECONFIG_BOOTCOMMAND \
		"mtd read Kernel ${loadaddr};" \
		"mtd read Rootfs ${ramdisk_addr};" \
		"setenv boot_mtd bootm;" \
		"setenv flashboot ${boot_mtd} ${loadaddr} ${ramdisk_addr} ${loadaddr}; " \
		"run flashbootargs; "
#    else
#        define PRECONFIG_BOOTCOMMAND
#    endif
#    define CONFIG_BOOTCOMMAND \
	PRECONFIG_BOOTCOMMAND \
	"run flashboot;"
#  elif defined(CONFIG_SD_BOOT)
#    if defined(CONFIG_XEN_SUPPORT)
#      define CONFIG_BOOTCOMMAND XEN_BOOTCMD
#    else
#     if defined(CONFIG_FIT_SIGNATURE)
#       define PRECONFIG_BOOTCOMMAND \
		"setenv image kernel.itb; " \
		"setenv boot_mtd bootm; " \
		"setenv mmcboot ${boot_mtd} ${loadaddr}; " \
		"run mmcargs; "
#      else
#        define PRECONFIG_BOOTCOMMAND
#      endif
#      define CONFIG_BOOTCOMMAND \
	PRECONFIG_BOOTCOMMAND \
	"mmc dev ${mmcdev}; " \
	"if mmc rescan; " \
	"then " \
		"if run loadimage; "\
		"then " \
			"run mmcboot; " \
		"fi; " \
	"fi"
#    endif
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

#endif
