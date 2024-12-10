/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Freescale DSPI Module Defines
 *
 * Copyright (C) 2004-2007, 2015 Freescale Semiconductor, Inc.
 * Copyright 2025 NXP
 * TsiChung Liew (Tsi-Chung.Liew@freescale.com)
 * Chao Fu (B44548@freesacle.com)
 * Haikun Wang (B53464@freescale.com)
 */

#ifndef _FSL_DSPI_H_
#define _FSL_DSPI_H_

/* DMA Serial Peripheral Interface (DSPI) */
struct dspi {
	u32 mcr;	/* 0x00 */
	u32 resv0;	/* 0x04 */
	u32 tcr;	/* 0x08 */
	union {
		struct {
			u32 ctar[8];	/* 0x0C - 0x28 */
			u32 sr;		/* 0x2C */
			u32 irsr;	/* 0x30 */
		} v1;
		struct {
			u32 ctar[6];	/* 0x0C - 0x24 */
			u32 resv1[2];	/* 0x24 - 0x2C */
			u32 rser;	/* 0x2C - 0x30 */
			u32 pushr_cmd;	/* 0x30 */
		} v2;
	} s1;

	u32 tfr;	/* 0x34 - PUSHR */
	u32 rfr;	/* 0x38 - POPR */

	union {
		struct {
			u32 tfdr[16];	/* 0x3C */
			u32 rfdr[16];	/* 0x7C */
		} v1;
		struct {
			u32 fcr;	/* 0x3C - 0x40 FIFO Control (FCR) */
			u32 resv2[4];   /* 0x40 - 0x50 */
			u32 rtis;	/* 0x50 - 0x54 Reset to Initial State (RTIS) */
			u32 resv3[11];  /* 0x54 - 0x80 */
			u32 tfdr[16];	/* 0x80 - 0xC0 */
			u32 rfdr[16];	/* 0xC0 - 0x100 */
			u32 resv4[32];  /* 0x100 - 0x180 */
			u32 sr;		/* 0x180 - 184 */
			u32 fsr;	/* 0x184 - FIFO Status (FIFO_SR)*/
		} v2;
	} s2;
};

/* Module configuration */
#define DSPI_MCR_MSTR			BIT(31)
#define DSPI_MCR_CSCK			BIT(30)
#define DSPI_MCR_DCONF(x)		(((x) & GENMASK(1, 0)) << 28)
#define DSPI_MCR_FRZ			BIT(27)
#define DSPI_MCR_MTFE			BIT(26)
#define DSPI_MCR_PCSSE			BIT(25)
#define DSPI_MCR_ROOE			BIT(24)
#define DSPI_MCR_PCSIS(x)		(1 << (16 + (x)))
#define DSPI_MCR_PCSIS_MASK		GENMASK(23, 16)
#define DSPI_V2_MCR_PCSIS_MASK		GENMASK(20, 16)
#define DSPI_MCR_CSIS7			BIT(23)
#define DSPI_MCR_CSIS6			BIT(22)
#define DSPI_MCR_CSIS5			BIT(21)
#define DSPI_MCR_CSIS4			BIT(20)
#define DSPI_MCR_CSIS3			BIT(19)
#define DSPI_MCR_CSIS2			BIT(18)
#define DSPI_MCR_CSIS1			BIT(17)
#define DSPI_MCR_CSIS0			BIT(16)
#define DSPI_MCR_DOZE			BIT(15)
#define DSPI_MCR_MDIS			BIT(14)
#define DSPI_MCR_DTXF			BIT(13)
#define DSPI_MCR_DRXF			BIT(12)
#define DSPI_MCR_CTXF			BIT(11)
#define DSPI_MCR_CRXF			BIT(10)
#define DSPI_MCR_SMPL_PT(x)		(((x) & GENMASK(2, 0)) << 8)

#define DSPI_MCR_FCPCS			BIT(2)
#define DSPI_MCR_PES			BIT(1)
#define DSPI_MCR_HALT			BIT(0)

/* Transfer count */
#define DSPI_TCR_SPI_TCNT(x)		(((x) & GENMASK(15, 0)) << 16)

/* Clock and transfer attributes */
#define DSPI_CTAR(x)			(0x0c + (((x) & GENMASK(2, 0)) * 4))
#define DSPI_CTAR_DBR			BIT(31)

#define DSPI_V2_PUSHR_CMD_FMSZ(x)	(((x) & GENMASK(15, 0)))
#define DSPI_CTAR_TRSZ(x)		(((x) & GENMASK(3, 0)) << 27)

#define DSPI_CTAR_CPOL			BIT(26)
#define DSPI_CTAR_CPHA			BIT(25)
#define DSPI_CTAR_LSBFE			BIT(24)
#define DSPI_CTAR_PCSSCK(x)		(((x) << 22) & GENMASK(23, 22))
#define DSPI_CTAR_PCSSCK_7CLK		0x00A00000
#define DSPI_CTAR_PCSSCK_5CLK		BIT(23)
#define DSPI_CTAR_PCSSCK_3CLK		BIT(22)
#define DSPI_CTAR_PCSSCK_1CLK		0x00000000
#define DSPI_CTAR_PASC(x)		(((x) << 20) & GENMASK(21, 20))
#define DSPI_CTAR_PASC_7CLK		GENMASK(21, 20)
#define DSPI_CTAR_PASC_5CLK		BIT(21)
#define DSPI_CTAR_PASC_3CLK		BIT(20)
#define DSPI_CTAR_PASC_1CLK		0x00000000
#define DSPI_CTAR_PDT(x)		(((x) << 18) & GENMASK(19, 18))
#define DSPI_CTAR_PDT_7CLK		0x000A0000
#define DSPI_CTAR_PDT_5CLK		BIT(19)
#define DSPI_CTAR_PDT_3CLK		BIT(18)
#define DSPI_CTAR_PDT_1CLK		0x00000000
#define DSPI_CTAR_PBR(x)		(((x) << 16) & GENMASK(17, 16))
#define DSPI_CTAR_PBR_7CLK		GENMASK(17, 16)
#define DSPI_CTAR_PBR_5CLK		BIT(17)
#define DSPI_CTAR_PBR_3CLK		BIT(16)
#define DSPI_CTAR_PBR_1CLK		0x00000000
#define DSPI_CTAR_CSSCK(x)		(((x) << 12) & GENMASK(15, 12))
#define DSPI_CTAR_ASC(x)		(((x) << 8) & GENMASK(11, 8))
#define DSPI_CTAR_DT(x)			(((x) << 4) & GENMASK(7, 4))
#define DSPI_CTAR_BR(x)			((x) & GENMASK(3, 0))
#define DSPI_CTAR_SCALE_BITS		GENMASK(3, 0)

/* Status */
#define DSPI_SR_TCF			BIT(31)
#define DSPI_SR_TXRXS			BIT(30)
#define DSPI_SR_EOQF			BIT(28)
#define DSPI_SR_TFUF			BIT(27)
#define DSPI_SR_TFFF			BIT(25)
#define DSPI_SR_RFOF			BIT(19)
#define DSPI_SR_RFDF			BIT(17)

#define DSPI_SR_TXPTR(x)		(((x) & GENMASK(11, 8)) >> 8)

#define DSPI_V2_FIFO_SR_TXCTR(x)	(((x) & GENMASK(17, 13)) >> 13)
#define DSPI_V2_FIFO_SR_RXCTR(x)	(((x) & GENMASK(26, 22)) >> 22)

#define DSPI_SR_TXCTR(x)		(((x) & GENMASK(15, 12)) >> 12)
#define DSPI_SR_RXCTR(x)		(((x) & GENMASK(7, 4)) >> 4)

#define DSPI_SR_RXPTR(x)		((x) & GENMASK(3, 0))

/* DMA/interrupt request selct and enable */
#define DSPI_IRSR_TCFE			BIT(31)
#define DSPI_IRSR_EOQFE			BIT(28)
#define DSPI_IRSR_TFUFE			BIT(27)
#define DSPI_IRSR_TFFFE			BIT(25)
#define DSPI_IRSR_TFFFS			BIT(24)
#define DSPI_IRSR_RFOFE			BIT(19)
#define DSPI_IRSR_RFDFE			BIT(17)
#define DSPI_IRSR_RFDFS			BIT(16)

/* Transfer control - 32-bit access */
#define DSPI_TFR_PCS(x)			(((1 << (x)) & GENMASK(5, 0)) << 16)
#define DSPI_TFR_CTCNT			BIT(26)
#define DSPI_V2_TFR_PCS(x)		(((1 << (x)) & GENMASK(4, 0)) << 16)
#define DSPI_V2_PUSHR_CTCNT		BIT(26)

#define DSPI_TFR_CONT			BIT(31)
#define DSPI_TFR_CTAS(x)		(((x) & GENMASK(2, 0)) << 28)
#define DSPI_TFR_EOQ			BIT(27)
#define DSPI_TFR_CS7			BIT(23)
#define DSPI_TFR_CS6			BIT(22)
#define DSPI_TFR_CS5			BIT(21)
#define DSPI_TFR_CS4			BIT(20)
#define DSPI_TFR_CS3			BIT(19)
#define DSPI_TFR_CS2			BIT(18)
#define DSPI_TFR_CS1			BIT(17)
#define DSPI_TFR_CS0			BIT(16)

#define DSPI_V2_TFR_TXDATA(x)		((x) & 0xFFFFFFFF)
#define DSPI_V2_RFR_RXDATA(x)		((x) & 0xFFFFFFFF)
#define DSPI_V2_TFDR_TXDATA(x)		((x) & 0xFFFFFFFF)
#define DSPI_V2_RFDR_RXDATA(x)		((x) & 0xFFFFFFFF)

/* Transfer Fifo */
#define DSPI_TFR_TXDATA(x)		((x) & GENMASK(15, 0))
/* Bit definitions and macros for DRFR */
#define DSPI_RFR_RXDATA(x)		((x) & GENMASK(15, 0))

/* Bit definitions and macros for DTFDR group */
#define DSPI_TFDR_TXDATA(x)		((x) & GENMASK(15, 0))
#define DSPI_TFDR_TXCMD(x)		(((x) & GENMASK(15, 0)) << 16)

/* Bit definitions and macros for DRFDR group */
#define DSPI_RFDR_RXDATA(x)		((x) & GENMASK(15, 0))


#endif				/* _FSL_DSPI_H_ */
