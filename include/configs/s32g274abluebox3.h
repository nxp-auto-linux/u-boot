/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright 2022 NXP
 */
#ifndef __S32G274ABLUEBOX3_H__
#define __S32G274ABLUEBOX3_H__

#include <configs/s32g2.h>

#define EXTRA_BOOTCOMMAND		PFE_INIT_CMD
#define EXTRA_BOOT_ARGS			PFE_EXTRA_BOOT_ARGS
#define FDT_FILE			"s32g274a-bluebox3.dtb"

#ifdef CONFIG_FSL_PFENG
#  define PFENG_MODE			"enable,sgmii,none,none"
#  define PFENG_EMAC			"0"
#endif

#ifdef NFSRAMFS_ADDR
#  undef NFSRAMFS_ADDR
#endif
#define NFSRAMFS_ADDR			"${ramdisk_addr}"
#ifdef NFSRAMFS_TFTP_CMD
#  undef NFSRAMFS_TFTP_CMD
#endif
#define NFSRAMFS_TFTP_CMD		"run loadtftpramdisk; "

#endif
