/*
 * (C) Copyright 2015-2016 Freescale Semiconductor, Inc.
 * Copyright 2017 NXP
 *
 * SPDX-License-Idenfifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_CSE_H
#define __ASM_ARCH_CSE_H

#define CSE_KEYID_AVK		(0x100UL)
#define CSE_CMD_GENERATE_MAC	(0x5UL)
#define CSE_CMD_VERIFY_MAC	(0x6UL)
#define CSE_CMD_CANCEL		(0x11UL)
#define CSE_CMD_SECURE_BOOT	(0xDUL)
#define CSE_CMD_INIT_CSE	(0x15UL)
#define CSE_CMD_INIT_RNG	(0x0AUL)
#define CSE_CMD_OPEN_SEC_RAM	(0x19UL)

#define CSE_SR_BSY		(0x1UL)
#define CSE_SR_BOK		(0x10UL)

#define CSE_BASE_ADDR	(0x40001000UL)
#define CSE_SR		(CSE_BASE_ADDR + 0x04UL)
#define CSE_ECR		(CSE_BASE_ADDR + 0x0CUL)
#define CSE_CMD		(CSE_BASE_ADDR + 0x20UL)
#define CSE_P1		(CSE_BASE_ADDR + 0x24UL)
#define CSE_P2		(CSE_BASE_ADDR + 0x28UL)
#define CSE_P3		(CSE_BASE_ADDR + 0x2CUL)
#define CSE_P4		(CSE_BASE_ADDR + 0x30UL)
#define CSE_P5		(CSE_BASE_ADDR + 0x34UL)
#define CSE_KIA0	(CSE_BASE_ADDR + 0x50UL)
#define CSE_KIA1	(CSE_BASE_ADDR + 0x54UL)

#define MAC_LEN		16
#define CSE_SEQ_ERR	(0x02)

#define OCOTP_BASE_ADDR			(0x4005F000UL)
#define OCOTP_CFG3			(OCOTP_BASE_ADDR + 0x440UL)
#define OCOTP_CFG5			(OCOTP_BASE_ADDR + 0x460UL)
#define OCOTP_CFG5_SEC_BOOT_MODE	(0xC0UL)
#define OCOTP_CFG3_EXPORT_CONTROL	(0x1UL)

#define KRAM_ADDR	(0x7C01A000UL)

#if defined CONFIG_FSL_CSE3
int cse_init(void);
#endif /* CONFIG_FSL_CSE3 */

#endif /* __ASM_ARCH_CSE_H */

