/*
 * (C) Copyright 2017 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef S32V_OCOTP_H
#define S32V_OCOTP_H

#include <common.h>
#include <asm/arch/cse.h>

#define OCOTP_REG(OFF) ((u32 *)(OCOTP_BASE_ADDR + (OFF)))

#define OCOTP_ECC_FUSE0_OFFSET   0x730

#define OCOTP_CTRL_REG           OCOTP_REG(0x00)
#define OCOTP_TIMING_REG         OCOTP_REG(0x10)
#define OCOTP_DATA_REG           OCOTP_REG(0x20)
#define OCOTP_READ_CTRL_REG      OCOTP_REG(0x30)
#define OCOTP_READ_FUSE_DATA_REG OCOTP_REG(0x40)
#define OCOTP_LOCK_REG           OCOTP_REG(0x400)
#define OCOTP_SEC0_REG           OCOTP_REG(0xc00)
#define OCOTP_SEC1_REG           OCOTP_REG(0xc10)
#define OCOTP_DEC0_REG           OCOTP_REG(0xc20)
#define OCOTP_DEC1_REG           OCOTP_REG(0xc30)

#define OCOTP_CTRL_BUSY_BIT           BIT(8)
#define OCOTP_CTRL_ERROR_BIT          BIT(9)
#define OCOTP_CTRL_RELOAD_SHADOWS_BIT BIT(10)
#define OCOTP_READ_CTRL_READ_FUSE_BIT BIT(0)
#define OCOTP_CTRL_WR_UNLOCK_MASK     (0xffffUL << 16)
#define OCOTP_CTRL_WR_UNLOCK_CODE     (0x3e77UL << 16)

#define OCOTP_STROBE_READ_TIME  31
#define OCOTP_RELAX_TIME        2
#define OCOTP_STROBE_PROG_TIME  1600

#define BANKWORD2ADDR(BANK, WORD)   ((BANK)*8+(WORD))
#define ADDR2SHADOW(ADDR)           ((ADDR)*16+0x400)
#define BANKWORD2SHADOW(BANK, WORD) (ADDR2SHADOW(BANKWORD2ADDR(BANK, WORD)))
#define REDUNDANTADDR(ADDR)         ((ADDR)+0x40)

#define MAC0ADDR BANKWORD2ADDR(4, 2)
#define MAC1ADDR BANKWORD2ADDR(4, 3)

#define OCOTP_MAC0_REG OCOTP_REG(ADDR2SHADOW(MAC0ADDR))
#define OCOTP_MAC1_REG OCOTP_REG(ADDR2SHADOW(MAC1ADDR))

int s32v_ocotp_read_fuse(const u8 addr, u32 *const fuse_word);
int s32v_ocotp_write_fuse(const u8 addr, const u32 fuse_word);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* S32V_OCOTP_H */
