/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __ASM_ARCH_CSE_H
#define __ASM_ARCH_CSE_H

#define CSE_KEYID_AVK			(0x100UL)
#define CSE_CMD_VERIFY_MAC		(0x6UL)
#define CSE_CMD_CANCEL			(0x11UL)
#define CSE_CMD_SECURE_BOOT		(0xDUL)
#define CSE_CMD_INIT_CSE		(0x15UL)
#define CSE_CMD_INIT_RNG		(0x0AUL)

#define CSE_SR_BSY				(0x1UL)
#define CSE_SR_BOK				(0x10UL)

#define CSE_BASE_ADDR	(0x40001000UL)
#define CSE_SR			(CSE_BASE_ADDR + 0x04UL)
#define CSE_ECR			(CSE_BASE_ADDR + 0x0CUL)
#define CSE_CMD			(CSE_BASE_ADDR + 0x20UL)
#define CSE_P1			(CSE_BASE_ADDR + 0x24UL)
#define CSE_P2			(CSE_BASE_ADDR + 0x28UL)
#define CSE_P3			(CSE_BASE_ADDR + 0x2CUL)
#define CSE_P4			(CSE_BASE_ADDR + 0x30UL)
#define CSE_P5			(CSE_BASE_ADDR + 0x34UL)
#define CSE_KIA0		(CSE_BASE_ADDR + 0x50UL)
#define CSE_KIA1		(CSE_BASE_ADDR + 0x54UL)

#if defined CONFIG_SECURE_BOOT
int cse_auth(void);
#elif defined CONFIG_CSE3
int cse_init(void);
#endif

#endif /* __ASM_ARCH_CSE_H */

