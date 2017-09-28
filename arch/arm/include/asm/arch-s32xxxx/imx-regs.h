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
#include <asm/arch/s32v234-regs.h>
#elif defined(CONFIG_S32V244)
#include <asm/arch/s32xxxx-gen1-regs.h>
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

/* MSCM interrupt router */
#define MSCM_IRSPRC_CPn_EN		3
#define MSCM_IRSPRC_NUM			176
#define MSCM_CPXTYPE_RYPZ_MASK		0xFF
#define MSCM_CPXTYPE_RYPZ_OFFSET	0
#define MSCM_CPXTYPE_PERS_MASK		0xFFFFFF00
#define MSCM_CPXTYPE_PERS_OFFSET	8
#define MSCM_CPXTYPE_PERS_A53		0x413533
#define MSCM_CPXTYPE_PERS_CM4		0x434d34


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

/* MSCM Interrupt Router */
struct mscm_ir {
	u32 cpxtype;		/* Processor x Type Register			*/
	u32 cpxnum;			/* Processor x Number Register			*/
	u32 cpxmaster;		/* Processor x Master Number Register	*/
	u32 cpxcount;		/* Processor x Count Register			*/
	u32 cpxcfg0;		/* Processor x Configuration 0 Register */
	u32 cpxcfg1;		/* Processor x Configuration 1 Register */
	u32 cpxcfg2;		/* Processor x Configuration 2 Register */
	u32 cpxcfg3;		/* Processor x Configuration 3 Register */
	u32 cp0type;		/* Processor 0 Type Register			*/
	u32 cp0num;			/* Processor 0 Number Register			*/
	u32 cp0master;		/* Processor 0 Master Number Register	*/
	u32 cp0count;		/* Processor 0 Count Register			*/
	u32 cp0cfg0;		/* Processor 0 Configuration 0 Register	*/
	u32 cp0cfg1;		/* Processor 0 Configuration 1 Register	*/
	u32 cp0cfg2;		/* Processor 0 Configuration 2 Register	*/
	u32 cp0cfg3;		/* Processor 0 Configuration 3 Register	*/
	u32 cp1type;		/* Processor 1 Type Register			*/
	u32 cp1num;			/* Processor 1 Number Register			*/
	u32 cp1master;		/* Processor 1 Master Number Register	*/
	u32 cp1count;		/* Processor 1 Count Register			*/
	u32 cp1cfg0;		/* Processor 1 Configuration 0 Register	*/
	u32 cp1cfg1;		/* Processor 1 Configuration 1 Register	*/
	u32 cp1cfg2;		/* Processor 1 Configuration 2 Register	*/
	u32 cp1cfg3;		/* Processor 1 Configuration 3 Register	*/
	u32 reserved_0x060[232];
	u32 ocmdr0;			/* On-Chip Memory Descriptor Register	*/
	u32 reserved_0x404[2];
	u32 ocmdr3;			/* On-Chip Memory Descriptor Register	*/
	u32 reserved_0x410[28];
	u32 tcmdr[4];		/* Generic Tightly Coupled Memory Descriptor Register	*/
	u32 reserved_0x490[28];
	u32 cpce0;			/* Core Parity Checking Enable Register 0				*/
	u32 reserved_0x504[191];
	u32 ircp0ir;		/* Interrupt Router CP0 Interrupt Register				*/
	u32 ircp1ir;		/* Interrupt Router CP1 Interrupt Register				*/
	u32 reserved_0x808[6];
	u32 ircpgir;		/* Interrupt Router CPU Generate Interrupt Register		*/
	u32 reserved_0x824[23];
	u16 irsprc[176];	/* Interrupt Router Shared Peripheral Routing Control Register	*/
	u32 reserved_0x9e0[136];
	u32 iahbbe0;		/* Gasket Burst Enable Register							*/
	u32 reserved_0xc04[63];
	u32 ipcge;			/* Interconnect Parity Checking Global Enable Register	*/
	u32 reserved_0xd04[3];
	u32 ipce[4];		/* Interconnect Parity Checking Enable Register			*/
	u32 reserved_0xd20[8];
	u32 ipcgie;			/* Interconnect Parity Checking Global Injection Enable Register	*/
	u32 reserved_0xd44[3];
	u32 ipcie[4];		/* Interconnect Parity Checking Injection Enable Register	*/
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
