/*
 * (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
 * (C) Copyright 2017 NXP
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __ASM_ARCH_IMX_REGS_H__
#define __ASM_ARCH_IMX_REGS_H__

#define ARCH_MXC

#if defined(CONFIG_S32V234)
#include "s32v234/s32v234-regs.h"
#elif defined(CONFIG_S32_GEN1)
#include "s32-gen1/s32-gen1-regs.h"
#else
#error "Incomplete platform definition"
#endif

#define IS_ADDR_IN_IRAM(addr) \
	((addr) >= (IRAM_BASE_ADDR) && \
	(addr) <= (IRAM_BASE_ADDR) + (IRAM_SIZE))

#define CCI400_BASE_ADDR			(0x7E090000)

/* TODO Remove this after the IOMUX framework is implemented */
#define IOMUXC_BASE_ADDR SIUL2_BASE_ADDR

/* MUX mode and PAD ctrl are in one register */
#define CONFIG_IOMUX_SHARE_CONF_REG

#define FEC_QUIRK_ENET_MAC
#define I2C_QUIRK_REG

#if !(defined(__KERNEL_STRICT_NAMES) || defined(__ASSEMBLY__))
#include <asm/types.h>

/* Periodic Interrupt Timer (PIT) */
struct pit_reg {
	u32 mcr;
	u32 recv0[55];
	u32 ltmr64h;
	u32 ltmr64l;
	u32 recv1[6];
	u32 ldval0;
	u32 cval0;
	u32 tctrl0;
	u32 tflg0;
	u32 ldval1;
	u32 cval1;
	u32 tctrl1;
	u32 tflg1;
	u32 ldval2;
	u32 cval2;
	u32 tctrl2;
	u32 tflg2;
	u32 ldval3;
	u32 cval3;
	u32 tctrl3;
	u32 tflg3;
	u32 ldval4;
	u32 cval4;
	u32 tctrl4;
	u32 tflg4;
	u32 ldval5;
	u32 cval5;
	u32 tctrl5;
	u32 tflg5;
};

/* Watchdog Timer (WDOG) */
struct wdog_regs {
	u32 cr;
	u32 ir;
	u32 to;
	u32 wn;
	u32 sr;
	u32 co;
	u32 sk;
};

/* UART */
struct linflex_fsl {
	u32 lincr1;
	u32 linier;
	u32 linsr;
	u32 linesr;
	u32 uartcr;
	u32 uartsr;
	u32 lintcsr;
	u32 linocr;
	u32 lintocr;
	u32 linfbrr;
	u32 linibrr;
	u32 lincfr;
	u32 lincr2;
	u32 bidr;
	u32 bdrl;
	u32 bdrm;
	u32 ifer;
	u32 ifmi;
	u32 ifmr;
#ifdef CONFIG_LINFLEX_MASTER_SLAVE_MODE
	u32 ifcr0;
	u32 ifcr1;
	u32 ifcr2;
	u32 ifcr3;
	u32 ifcr4;
	u32 ifcr5;
	u32 ifcr6;
	u32 ifcr7;
	u32 ifcr8;
	u32 ifcr9;
	u32 ifcr10;
	u32 ifcr11;
	u32 ifcr12;
	u32 ifcr13;
	u32 ifcr14;
	u32 ifcr15;
#endif
	u32 gcr;
	u32 uartpto;
	u32 uartcto;
	u32 dmatxe;
	u32 dmarxe;
};

#define CCI400_CTRLORD_TERM_BARRIER	0x00000008
#define CCI400_CTRLORD_EN_BARRIER	0
#define CCI400_SHAORD_NON_SHAREABLE	0x00000002
#define CCI400_DVM_MESSAGE_REQ_EN	0x00000002
#define CCI400_SNOOP_REQ_EN		0x00000001
struct ccsr_cci400 {
	u32 ctrl_ord;			/* Control Override */
	u32 spec_ctrl;			/* Speculation Control */
	u32 secure_access;		/* Secure Access */
	u32 status;			/* Status */
	u32 impr_err;			/* Imprecise Error */
	u8 res_14[0x100 - 0x14];
	u32 pmcr;			/* Performance Monitor Control */
	u8 res_104[0xfd0 - 0x104];
	u32 pid[8];			/* Peripheral ID */
	u32 cid[4];			/* Component ID */
	struct {
		u32 snoop_ctrl;		/* Snoop Control */
		u32 sha_ord;		/* Shareable Override */
		u8 res_1008[0x1100 - 0x1008];
		u32 rc_qos_ord;		/* read channel QoS Value Override */
		u32 wc_qos_ord;		/* read channel QoS Value Override */
		u8 res_1108[0x110c - 0x1108];
		u32 qos_ctrl;		/* QoS Control */
		u32 max_ot;		/* Max OT */
		u8 res_1114[0x1130 - 0x1114];
		u32 target_lat;		/* Target Latency */
		u32 latency_regu;	/* Latency Regulation */
		u32 qos_range;		/* QoS Range */
		u8 res_113c[0x2000 - 0x113c];
	} slave[5];			/* Slave Interface */
	u8 res_6000[0x9004 - 0x6000];
	u32 cycle_counter;		/* Cycle counter */
	u32 count_ctrl;			/* Count Control */
	u32 overflow_status;		/* Overflow Flag Status */
	u8 res_9010[0xa000 - 0x9010];
	struct {
		u32 event_select;	/* Event Select */
		u32 event_count;	/* Event Count */
		u32 counter_ctrl;	/* Counter Control */
		u32 overflow_status;	/* Overflow Flag Status */
		u8 res_a010[0xb000 - 0xa010];
	} pcounter[4];			/* Performance Counter */
	u8 res_e004[0x10000 - 0xe004];
};
#endif	/* __ASSEMBLER__*/

#endif	/* __ASM_ARCH_IMX_REGS_H__ */
