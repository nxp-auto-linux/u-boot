// SPDX-License-Identifier: GPL-2.0+

/*
 * Freescale QuadSPI driver.
 *
 * Copyright (C) 2013 Freescale Semiconductor, Inc.
 * Copyright (C) 2018 Bootlin
 * Copyright (C) 2018 exceet electronics GmbH
 * Copyright (C) 2018 Kontron Electronics GmbH
 * Copyright 2019-2020,2022-2024 NXP
 *
 * This driver is a ported version of Linux Freescale QSPI driver taken from
 * v5.5-rc1 tag having following information.
 *
 * Transition to SPI MEM interface:
 * Authors:
 *     Boris Brezillon <bbrezillon@kernel.org>
 *     Frieder Schrempf <frieder.schrempf@kontron.de>
 *     Yogesh Gaur <yogeshnarayan.gaur@nxp.com>
 *     Suresh Gupta <suresh.gupta@nxp.com>
 *
 * Based on the original fsl-quadspi.c spi-nor driver.
 * Transition to spi-mem in spi-fsl-qspi.c
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <inttypes.h>
#include <log.h>
#include <spi.h>
#include <spi-mem.h>
#include <asm/cache.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/libfdt.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>
#include <linux/err.h>
#include <asm/armv8/mmu.h>
#include <linux/mtd/spi-nor.h>

/*
 * The driver only uses one single LUT entry, that is updated on
 * each call of exec_op(). Index 0 is preset at boot with a basic
 * read operation, so let's use the last entry (15).
 */
#define	SEQID_LUT			15
#define	SEQID_LUT_AHB		14

/* Registers used by the driver */
#define QUADSPI_MCR			0x00
#define QUADSPI_MCR_DQS_EXTERNAL	(0x3 << 24)
#define QUADSPI_MCR_DQS_INT_PAD		(0x1 << 24)
#define QUADSPI_MCR_RESERVED_MASK	GENMASK(19, 16)
#define QUADSPI_MCR_ISD3FB		BIT(19)
#define QUADSPI_MCR_ISD2FB		BIT(18)
#define QUADSPI_MCR_ISD3FA		BIT(17)
#define QUADSPI_MCR_ISD2FA		BIT(16)
#define QUADSPI_MCR_MDIS_MASK		BIT(14)
#define QUADSPI_MCR_CLR_TXF_MASK	BIT(11)
#define QUADSPI_MCR_CLR_RXF_MASK	BIT(10)
#define QUADSPI_MCR_DQS_EN		BIT(6)
#define QUADSPI_MCR_DDR_EN_MASK		BIT(7)
#define QUADSPI_MCR_END_CFG_MASK	GENMASK(3, 2)
#define QUADSPI_MCR_SWRSTHD_MASK	BIT(1)
#define QUADSPI_MCR_SWRSTSD_MASK	BIT(0)

#define QUADSPI_IPCR			0x08
#define QUADSPI_IPCR_SEQID(x)		((x) << 24)
#define QUADSPI_FLSHCR			0x0c
#define QUADSPI_FLSHCR_TCSS_MASK	GENMASK(3, 0)
#define QUADSPI_FLSHCR_TCSS(N)		((N) << 0)
#define QUADSPI_FLSHCR_TCSH_MASK	GENMASK(11, 8)
#define QUADSPI_FLSHCR_TCHS(N)		((N) << 8)
#define QUADSPI_FLSHCR_TDH_MASK		GENMASK(17, 16)
#define QUADSPI_FLSHCR_TDH(N)		((N) << 16)

#define QUADSPI_BUF3CR			0x1c
#define QUADSPI_BUF3CR_ALLMST_MASK	BIT(31)
#define QUADSPI_BUF3CR_ADATSZ_SHIFT	8
#define QUADSPI_BUF3CR_ADATSZ(x)	((x) << QUADSPI_BUF3CR_ADATSZ_SHIFT)
#define QUADSPI_BUF3CR_ADATSZ_MASK	GENMASK(15, 8)

#define QUADSPI_BFGENCR			0x20
#define QUADSPI_BFGENCR_SEQID(x)	((x) << 12)

#define QUADSPI_BUF0IND			0x30
#define QUADSPI_BUF1IND			0x34
#define QUADSPI_BUF2IND			0x38

#define QUADSPI_DLLCRA			0x60
#define QUADSPI_DLLCR_MASK		0x7FFFFFF0UL
#define QUADSPI_DLLCR_DLLEN_EN		BIT(31)
#define QUADSPI_DLLCR_FREQEN_EN		BIT(30)
#define QUADSPI_DLLCR_DLL_REFCNTR_N(N)	((N) << 24)
#define QUADSPI_DLLCR_DLLRES_N(N)	((N) << 20)
#define QUADSPI_DLLCR_SLV_DLY_OFFSET_N(N)	((N) << 12)
#define QUADSPI_DLLCR_SLV_DLY_COARSE_N(N)	((N) << 8)
#define QUADSPI_DLLCR_SLV_AUTO_UPDT_SHIFT	3
#define QUADSPI_DLLCR_SLV_AUTO_UPDT_EN	BIT(QUADSPI_DLLCR_SLV_AUTO_UPDT_SHIFT)
#define QUADSPI_DLLCR_SLV_EN_SHIFT	2
#define QUADSPI_DLLCR_SLV_EN		BIT(QUADSPI_DLLCR_SLV_EN_SHIFT)
#define QUADSPI_DLLCR_SLV_BYPASS_SHIFT	1
#define QUADSPI_DLLCR_SLV_BYPASS_EN	BIT(QUADSPI_DLLCR_SLV_BYPASS_SHIFT)
#define QUADSPI_DLLCR_SLV_UPD_EN	BIT(0)

#define QUADSPI_SFAR			0x100

#define QUADSPI_SFACR			0x104
#define QUADSPI_SFACR_BSWAP		BIT(17)

#define QUADSPI_SMPR			0x108
#define QUADSPI_SMPR_DLLFSMPFB_MASK	GENMASK(30, 28)
#define QUADSPI_SMPR_DLLFSMPFB_NTH(N)	((N) << 28)
#define QUADSPI_SMPR_DLLFSMPFA_MASK	GENMASK(26, 24)
#define QUADSPI_SMPR_DLLFSMPFA_NTH(N)	((N) << 24)
#define QUADSPI_SMPR_DDRSMP_MASK	GENMASK(18, 16)
#define QUADSPI_SMPR_FSDLY_MASK		BIT(6)
#define QUADSPI_SMPR_FSPHS_MASK		BIT(5)
#define QUADSPI_SMPR_HSENA_MASK		BIT(0)

#define QUADSPI_RBCT			0x110
#define QUADSPI_RBCT_WMRK_MASK		GENMASK(4, 0)
#define QUADSPI_RBCT_RXBRD_USEIPS	BIT(8)

#define QUADSPI_DLLSR			0x12c
#define QUADSPI_DLLSR_SLVA_LOCK		BIT(14)
#define QUADSPI_DLLSR_DLLA_LOCK		BIT(15)

#define QUADSPI_TBDR			0x154

#define QUADSPI_SR			0x15c
#define QUADSPI_SR_BUSY			BIT(0)
#define QUADSPI_SR_IP_ACC_MASK		BIT(1)
#define QUADSPI_SR_AHB_ACC_MASK		BIT(2)

#define QUADSPI_FR			0x160
#define QUADSPI_FR_TFF_MASK		BIT(0)

#define QUADSPI_RSER			0x164
#define QUADSPI_RSER_TFIE		BIT(0)

#define QUADSPI_SPTRCLR			0x16c
#define QUADSPI_SPTRCLR_IPPTRC		BIT(8)
#define QUADSPI_SPTRCLR_BFPTRC		BIT(0)

#define QUADSPI_SFA1AD			0x180
#define QUADSPI_SFA2AD			0x184
#define QUADSPI_SFB1AD			0x188
#define QUADSPI_SFB2AD			0x18c
#define QUADSPI_RBDR(x)			(0x200 + ((x) * 4))

#define QUADSPI_DLCR			0x130
#define QUADSPI_DLCR_RESERVED_MASK	((0xff << 0) | (0xff << 16))
#define QUADSPI_DLCR_DLP_SEL_FB(N)	((N) << 30)
#define QUADSPI_DLCR_DLP_SEL_FA(N)	((N) << 14)

#define QUADSPI_TBSR			0x150
#define QUADSPI_TBSR_TRCTR(TBSR)	((TBSR) >> 16)
#define QUADSPI_TBSR_TRBFL(TBSR)	((TBSR) & 0xFF)
#define QUADSPI_TBSR_TRBFL_MASK		0xFFU

#define QUADSPI_DLPR			0x190
#define QUADSPI_DLPR_RESET_VALUE	0xaa553443

#define QUADSPI_LUTKEY			0x300
#define QUADSPI_LUTKEY_VALUE		0x5AF05AF0

#define QUADSPI_LCKCR			0x304
#define QUADSPI_LCKER_LOCK		BIT(0)
#define QUADSPI_LCKER_UNLOCK		BIT(1)

#define QUADSPI_LUT_BASE		0x310

#ifdef CONFIG_NXP_S32CC
#define LUTS_PER_CONFIG			5
#else
#define LUTS_PER_CONFIG			4
#endif

#define QUADSPI_LUT_OFFSET		(SEQID_LUT * 4 * LUTS_PER_CONFIG)
#define QUADSPI_LUT_REG(idx) \
	(QUADSPI_LUT_BASE + QUADSPI_LUT_OFFSET + (idx) * 4)

#define QUADSPI_AHB_LUT_OFFSET		(SEQID_LUT_AHB * 4 * LUTS_PER_CONFIG)
#define QUADSPI_AHB_LUT_REG(idx) \
	(QUADSPI_LUT_BASE + QUADSPI_AHB_LUT_OFFSET + (idx) * 4)

#define LUT_SIZE_BITS		16

/* Instruction set for the LUT register */
#define LUT_STOP		0
#define LUT_CMD			1
#define LUT_ADDR		2
#define LUT_DUMMY		3
#define LUT_MODE		4
#define LUT_MODE2		5
#define LUT_MODE4		6
#define LUT_FSL_READ		7
#define LUT_FSL_WRITE		8
#define LUT_JMP_ON_CS		9
#define LUT_ADDR_DDR		10
#define LUT_MODE_DDR		11
#define LUT_MODE2_DDR		12
#define LUT_MODE4_DDR		13
#define LUT_FSL_READ_DDR	14
#define LUT_FSL_WRITE_DDR	15
#define LUT_DATA_LEARN		16
#define LUT_CMD_DDR		17

/*
 * The PAD definitions for LUT register.
 *
 * The pad stands for the number of IO lines [0:3].
 * For example, the quad read needs four IO lines,
 * so you should use LUT_PAD(4).
 */
#define LUT_PAD(x) (fls(x) - 1)

/*
 * Macro for constructing the LUT entries with the following
 * register layout:
 *
 *  ---------------------------------------------------
 *  | INSTR1 | PAD1 | OPRND1 | INSTR0 | PAD0 | OPRND0 |
 *  ---------------------------------------------------
 */
#define LUT_DEF(idx, ins, pad, opr)					\
	((((ins) << 10) | ((pad) << 8) | ((opr) & 0xFFU)) << (((idx) % 2) * 16))

/* Controller needs driver to swap endianness */
#define QUADSPI_QUIRK_SWAP_ENDIAN	BIT(0)

/* Controller needs 4x internal clock */
#define QUADSPI_QUIRK_4X_INT_CLK	BIT(1)

/*
 * TKT253890, the controller needs the driver to fill the txfifo with
 * 16 bytes at least to trigger a data transfer, even though the extra
 * data won't be transferred.
 */
#define QUADSPI_QUIRK_TKT253890		BIT(2)

/* TKT245618, the controller cannot wake up from wait mode */
#define QUADSPI_QUIRK_TKT245618		BIT(3)

/*
 * Controller adds QSPI_AMBA_BASE (base address of the mapped memory)
 * internally. No need to add it when setting SFXXAD and SFAR registers
 */
#define QUADSPI_QUIRK_BASE_INTERNAL	BIT(4)

/*
 * Controller uses TDH bits in register QUADSPI_FLSHCR.
 * They need to be set in accordance with the DDR/SDR mode.
 */
#define QUADSPI_QUIRK_USE_TDH_SETTING	BIT(5)

/*
 * Controller only has Two CS on flash A, no flash B port
 */
#define QUADSPI_QUIRK_SINGLE_BUS		BIT(6)

/*
 * Controller has Delay Locked Loop logic
 */
#define QUADSPI_QUIRK_HAS_DLL			BIT(7)

/*
 * Has octal support
 */
#define QUADSPI_QUIRK_OCTAL_SUPPORT		BIT(8)

/*
 * Allow to read the entire AHB space at once
 */
#define QUADSPI_QUIRK_READ_ENTIRE_AHB		BIT(9)

/*
 * Selects delay chain for low frequency of operation
 */
#define QUADSPI_QUIRK_LOW_FREQ_DELAY_CHAIN	BIT(10)

/*
 * Selects internal pad loopback for SDR operations
 */
#define QUADSPI_QUIRK_INT_LOOPBACK_SDR		BIT(11)

struct fsl_qspi_devtype_data {
	unsigned int rxfifo;
	unsigned int txfifo;
	unsigned int ahb_buf_size;
	unsigned int quirks;
	bool little_endian;
};

static const struct fsl_qspi_devtype_data vybrid_data = {
	.rxfifo = SZ_128,
	.txfifo = SZ_64,
	.ahb_buf_size = SZ_1K,
	.quirks = QUADSPI_QUIRK_SWAP_ENDIAN,
	.little_endian = true,
};

static const struct fsl_qspi_devtype_data imx6sx_data = {
	.rxfifo = SZ_128,
	.txfifo = SZ_512,
	.ahb_buf_size = SZ_1K,
	.quirks = QUADSPI_QUIRK_4X_INT_CLK | QUADSPI_QUIRK_TKT245618,
	.little_endian = true,
};

static const struct fsl_qspi_devtype_data imx7d_data = {
	.rxfifo = SZ_128,
	.txfifo = SZ_512,
	.ahb_buf_size = SZ_1K,
	.quirks = QUADSPI_QUIRK_TKT253890 | QUADSPI_QUIRK_4X_INT_CLK |
		  QUADSPI_QUIRK_USE_TDH_SETTING,
	.little_endian = true,
};

static const struct fsl_qspi_devtype_data imx6ul_data = {
	.rxfifo = SZ_128,
	.txfifo = SZ_512,
	.ahb_buf_size = SZ_1K,
	.quirks = QUADSPI_QUIRK_TKT253890 | QUADSPI_QUIRK_4X_INT_CLK |
		  QUADSPI_QUIRK_USE_TDH_SETTING,
	.little_endian = true,
};

static const struct fsl_qspi_devtype_data imx7ulp_data = {
	.rxfifo = SZ_64,
	.txfifo = SZ_64,
	.ahb_buf_size = SZ_128,
	.quirks = QUADSPI_QUIRK_TKT253890 | QUADSPI_QUIRK_4X_INT_CLK |
		  QUADSPI_QUIRK_USE_TDH_SETTING | QUADSPI_QUIRK_SINGLE_BUS,
	.little_endian = true,
};

static const struct fsl_qspi_devtype_data ls1021a_data = {
	.rxfifo = SZ_128,
	.txfifo = SZ_64,
	.ahb_buf_size = SZ_1K,
	.quirks = 0,
	.little_endian = false,
};

static const struct fsl_qspi_devtype_data ls2080a_data = {
	.rxfifo = SZ_128,
	.txfifo = SZ_64,
	.ahb_buf_size = SZ_1K,
	.quirks = QUADSPI_QUIRK_TKT253890 | QUADSPI_QUIRK_BASE_INTERNAL,
	.little_endian = true,
};

static const struct fsl_qspi_devtype_data s32g_data = {
	.rxfifo = SZ_128,
	.txfifo = SZ_256,
	.ahb_buf_size = SZ_1K,
	.quirks = QUADSPI_QUIRK_USE_TDH_SETTING | QUADSPI_QUIRK_HAS_DLL |
	    QUADSPI_QUIRK_OCTAL_SUPPORT | QUADSPI_QUIRK_READ_ENTIRE_AHB |
	    QUADSPI_QUIRK_INT_LOOPBACK_SDR,
	.little_endian = true,
};

static const struct fsl_qspi_devtype_data s32g3_data = {
	.rxfifo = SZ_128,
	.txfifo = SZ_256,
	.ahb_buf_size = SZ_1K,
	.quirks = QUADSPI_QUIRK_USE_TDH_SETTING | QUADSPI_QUIRK_HAS_DLL |
	    QUADSPI_QUIRK_OCTAL_SUPPORT | QUADSPI_QUIRK_READ_ENTIRE_AHB |
	    QUADSPI_QUIRK_INT_LOOPBACK_SDR,
	.little_endian = true,
};

static const struct fsl_qspi_devtype_data s32r45_data = {
	.rxfifo = SZ_128,
	.txfifo = SZ_256,
	.ahb_buf_size = SZ_1K,
	.quirks = QUADSPI_QUIRK_USE_TDH_SETTING | QUADSPI_QUIRK_HAS_DLL |
	    QUADSPI_QUIRK_OCTAL_SUPPORT | QUADSPI_QUIRK_READ_ENTIRE_AHB |
	    QUADSPI_QUIRK_LOW_FREQ_DELAY_CHAIN | QUADSPI_QUIRK_INT_LOOPBACK_SDR,
	.little_endian = true,
};

struct qspi_config {
	u32 mcr;
	u32 flshcr;
	u32 dllcr;
	u32 sfacr;
	u32 smpr;
	u32 dlcr;
	u32 flash1_size;
	u32 flash2_size;
	u32 dlpr;
};

static const struct qspi_config octal_ddr_conf = {
	.mcr = QUADSPI_MCR_END_CFG_MASK |
	    QUADSPI_MCR_DQS_EN |
	    QUADSPI_MCR_DDR_EN_MASK |
	    QUADSPI_MCR_ISD2FA |
	    QUADSPI_MCR_ISD3FA |
	    QUADSPI_MCR_ISD2FB |
	    QUADSPI_MCR_ISD3FB |
	    QUADSPI_MCR_DQS_EXTERNAL,
	.flshcr = QUADSPI_FLSHCR_TCSS(3) |
	    QUADSPI_FLSHCR_TCHS(3) |
	    QUADSPI_FLSHCR_TDH(1),
	.dllcr = QUADSPI_DLLCR_SLV_EN |
	    QUADSPI_DLLCR_SLV_AUTO_UPDT_EN |
	    QUADSPI_DLLCR_DLLRES_N(8) |
	    QUADSPI_DLLCR_DLL_REFCNTR_N(2) |
	    QUADSPI_DLLCR_FREQEN_EN |
	    QUADSPI_DLLCR_DLLEN_EN,
	.sfacr = QUADSPI_SFACR_BSWAP,
	.smpr = QUADSPI_SMPR_DLLFSMPFA_NTH(4) |
		QUADSPI_SMPR_DLLFSMPFB_NTH(4),
	.dlcr = QUADSPI_DLCR_RESERVED_MASK |
	    QUADSPI_DLCR_DLP_SEL_FA(1) |
	    QUADSPI_DLCR_DLP_SEL_FB(1),
	.flash1_size = 0x20000000,
	.flash2_size = 0x20000000,
	.dlpr = QUADSPI_DLPR_RESET_VALUE,
};

struct fsl_qspi {
	struct udevice *dev;
	void __iomem *iobase;
	void __iomem *ahb_addr;
	u32 memmap_phy;
	u32 memmap_size;
	const struct fsl_qspi_devtype_data *devtype_data;
	int selected;
	enum spi_nor_protocol proto;
};

static inline int needs_swap_endian(struct fsl_qspi *q)
{
	return q->devtype_data->quirks & QUADSPI_QUIRK_SWAP_ENDIAN;
}

static inline int needs_4x_clock(struct fsl_qspi *q)
{
	return q->devtype_data->quirks & QUADSPI_QUIRK_4X_INT_CLK;
}

static inline int needs_fill_txfifo(struct fsl_qspi *q)
{
	return q->devtype_data->quirks & QUADSPI_QUIRK_TKT253890;
}

static inline int needs_wakeup_wait_mode(struct fsl_qspi *q)
{
	return q->devtype_data->quirks & QUADSPI_QUIRK_TKT245618;
}

static inline int needs_amba_base_offset(struct fsl_qspi *q)
{
	return !(q->devtype_data->quirks & QUADSPI_QUIRK_BASE_INTERNAL);
}

static inline int needs_tdh_setting(struct fsl_qspi *q)
{
	return q->devtype_data->quirks & QUADSPI_QUIRK_USE_TDH_SETTING;
}

static inline int needs_single_bus(struct fsl_qspi *q)
{
	return q->devtype_data->quirks & QUADSPI_QUIRK_SINGLE_BUS;
}

static inline int has_dll(struct fsl_qspi *q)
{
	return q->devtype_data->quirks & QUADSPI_QUIRK_HAS_DLL;
}

static inline int has_octal_support(struct fsl_qspi *q)
{
	return q->devtype_data->quirks & QUADSPI_QUIRK_OCTAL_SUPPORT;
}

static inline int can_read_entire_ahb(struct fsl_qspi *q)
{
	return q->devtype_data->quirks & QUADSPI_QUIRK_READ_ENTIRE_AHB;
}

static inline int low_freq_chain(struct fsl_qspi *q)
{
	return q->devtype_data->quirks & QUADSPI_QUIRK_LOW_FREQ_DELAY_CHAIN;
}

static inline int sdr_internal_loopback(struct fsl_qspi *q)
{
	return q->devtype_data->quirks & QUADSPI_QUIRK_INT_LOOPBACK_SDR;
}

static inline bool is_s32g3_qspi(struct fsl_qspi *q)
{
	return q->devtype_data == &s32g3_data;
}

/*
 * An IC bug makes it necessary to rearrange the 32-bit data.
 * Later chips, such as IMX6SLX, have fixed this bug.
 */
static inline u32 fsl_qspi_endian_xchg(struct fsl_qspi *q, u32 a)
{
	return needs_swap_endian(q) ? __swab32(a) : a;
}

/*
 * R/W functions for big- or little-endian registers:
 * The QSPI controller's endianness is independent of
 * the CPU core's endianness. So far, although the CPU
 * core is little-endian the QSPI controller can use
 * big-endian or little-endian.
 */
static void qspi_writel(struct fsl_qspi *q, u32 val, void __iomem *addr)
{
	if (q->devtype_data->little_endian)
		out_le32(addr, val);
	else
		out_be32(addr, val);
}

static u32 qspi_readl(struct fsl_qspi *q, void __iomem *addr)
{
	if (q->devtype_data->little_endian)
		return in_le32(addr);

	return in_be32(addr);
}

static int fsl_qspi_check_buswidth(struct fsl_qspi *q, u8 width)
{
	switch (width) {
	case 1:
	case 2:
	case 4:
		return 0;
	case 8:
		if (has_octal_support(q))
			return 0;
	}

	return -ENOTSUPP;
}

static void enable_ahb_buf_cache(struct fsl_qspi *q)
{
	u64 mm_region_attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
		PTE_BLOCK_AP_RO | PTE_BLOCK_AP_EL0_RO |
		PTE_TYPE_VALID;

	mmu_change_region_attr(q->memmap_phy, q->memmap_size,
			       mm_region_attrs);
}

static void disable_ahb_buf_cache(struct fsl_qspi *q)
{
	u64 mm_region_attrs = PTE_BLOCK_NON_SHARE |
		PTE_TYPE_FAULT;

	mmu_change_region_attr(q->memmap_phy, q->memmap_size,
			       mm_region_attrs);
}

static bool fsl_qspi_supports_op(struct spi_slave *slave,
				 const struct spi_mem_op *op)
{
	struct fsl_qspi *q = dev_get_priv(slave->dev->parent);
	int ret;

	ret = fsl_qspi_check_buswidth(q, op->cmd.buswidth);

	if (op->addr.nbytes)
		ret |= fsl_qspi_check_buswidth(q, op->addr.buswidth);

	if (op->dummy.nbytes)
		ret |= fsl_qspi_check_buswidth(q, op->dummy.buswidth);

	if (op->data.nbytes)
		ret |= fsl_qspi_check_buswidth(q, op->data.buswidth);

	if (ret)
		return false;

	/*
	 * The number of instructions needed for the op, needs
	 * to fit into a single LUT entry.
	 */
	if (op->addr.nbytes +
	   (op->dummy.nbytes ? 1 : 0) +
	   (op->data.nbytes ? 1 : 0) > 6)
		return false;

	/* Max 64 dummy clock cycles supported */
	if (op->dummy.nbytes &&
	    (op->dummy.nbytes * 8 / op->dummy.buswidth > 64))
		return false;

	/* Max data length, check controller limits and alignment */
	if (op->data.dir == SPI_MEM_DATA_IN &&
	    (op->data.nbytes > q->devtype_data->ahb_buf_size ||
	     (op->data.nbytes > q->devtype_data->rxfifo - 4 &&
	      !IS_ALIGNED(op->data.nbytes, 8)))) {
		if (!can_read_entire_ahb(q) &&
		    op->data.nbytes > q->devtype_data->ahb_buf_size)
			return false;
	}

	if (op->data.dir == SPI_MEM_DATA_OUT &&
	    op->data.nbytes > q->devtype_data->txfifo)
		return false;

	if (op->cmd.dtr && op->addr.dtr && op->dummy.dtr && op->data.dtr)
		ret = spi_mem_dtr_supports_op(slave, op);
	else
		ret = spi_mem_default_supports_op(slave, op);

	return ret;
}

static void fsl_qspi_prepare_lut(struct fsl_qspi *q,
				 const struct spi_mem_op *op)
{
	u8 lut_cmd, lut_addr, lut_data, data_size;
	void __iomem *base = q->iobase;
	u32 lutval[4] = {};
	int lutidx = 1, i;

	if (op->cmd.dtr)
		lut_cmd = LUT_CMD_DDR;
	else
		lut_cmd = LUT_CMD;

	lutval[0] |= LUT_DEF(0, lut_cmd, LUT_PAD(op->cmd.buswidth),
			     op->cmd.opcode);

	if (op->cmd.buswidth == 8) {
		lutval[lutidx / 2] <<= LUT_SIZE_BITS;
		lutval[lutidx / 2] |= LUT_DEF(0, lut_cmd,
					      LUT_PAD(op->cmd.buswidth),
					      op->cmd.opcode >> 8);
		lutidx++;
	}

	if (op->addr.dtr)
		lut_addr = LUT_ADDR_DDR;
	else
		lut_addr = LUT_ADDR;

	if (IS_ENABLED(CONFIG_FSL_QSPI_AHB_FULL_MAP)) {
		if (op->addr.nbytes) {
			lutval[lutidx / 2] |= LUT_DEF(lutidx, lut_addr,
						      LUT_PAD(op->addr.buswidth),
						      (op->addr.nbytes == 4) ? 0x20 : 0x18);
			lutidx++;
		}
	} else {
		/*
		 * For some unknown reason, using LUT_ADDR doesn't work in some
		 * cases (at least with only one byte long addresses), so
		 * let's use LUT_MODE to write the address bytes one by one
		 */
		for (i = 0; i < op->addr.nbytes; i++) {
			u8 addrbyte = op->addr.val >> (8 * (op->addr.nbytes - i - 1));

			lutval[lutidx / 2] |= LUT_DEF(lutidx, LUT_MODE,
						      LUT_PAD(op->addr.buswidth),
						      addrbyte);
			lutidx++;
		}
	}

	if (op->dummy.nbytes) {
		lutval[lutidx / 2] |= LUT_DEF(lutidx, LUT_DUMMY,
					      LUT_PAD(op->dummy.buswidth),
					      op->dummy.nbytes * 8 /
					      op->dummy.buswidth);
		lutidx++;
	}

	if (op->data.nbytes) {
		if (op->data.dir == SPI_MEM_DATA_IN) {
			if (op->addr.dtr)
				lut_data = LUT_FSL_READ_DDR;
			else
				lut_data = LUT_FSL_READ;
		} else {
			if (op->addr.dtr)
				lut_data = LUT_FSL_WRITE_DDR;
			else
				lut_data = LUT_FSL_WRITE;
		}

		/* HW limitation for memory write operation */
		if (op->data.dir == SPI_MEM_DATA_OUT && op->data.dtr) {
			data_size = 0;
		} else {
			if (op->data.nbytes > q->devtype_data->rxfifo)
				data_size = q->devtype_data->rxfifo;
			else
				data_size = op->data.nbytes;
		}

		lutval[lutidx / 2] |= LUT_DEF(lutidx,
					      lut_data,
					      LUT_PAD(op->data.buswidth),
					      data_size);
		lutidx++;
	}

	lutval[lutidx / 2] |= LUT_DEF(lutidx, LUT_STOP, 0, 0);

	/* unlock LUT */
	qspi_writel(q, QUADSPI_LUTKEY_VALUE, q->iobase + QUADSPI_LUTKEY);
	qspi_writel(q, QUADSPI_LCKER_UNLOCK, q->iobase + QUADSPI_LCKCR);

	dev_dbg(q->dev, "CMD[%x] lutval[0:%x \t 1:%x \t 2:%x \t 3:%x]\n",
		op->cmd.opcode, lutval[0], lutval[1], lutval[2], lutval[3]);

	/* fill LUT */
	for (i = 0; i < ARRAY_SIZE(lutval); i++)
		qspi_writel(q, lutval[i], base + QUADSPI_LUT_REG(i));

	if (IS_ENABLED(CONFIG_FSL_QSPI_AHB_FULL_MAP)) {
		if (op->data.nbytes && op->data.dir == SPI_MEM_DATA_IN &&
		    op->addr.nbytes) {
			for (i = 0; i < ARRAY_SIZE(lutval); i++)
				qspi_writel(q, lutval[i], base + QUADSPI_AHB_LUT_REG(i));
		}
	}

	/* lock LUT */
	qspi_writel(q, QUADSPI_LUTKEY_VALUE, q->iobase + QUADSPI_LUTKEY);
	qspi_writel(q, QUADSPI_LCKER_LOCK, q->iobase + QUADSPI_LCKCR);
}

/*
 * If we have changed the content of the flash by writing or erasing, or if we
 * read from flash with a different offset into the page buffer, we need to
 * invalidate the AHB buffer. If we do not do so, we may read out the wrong
 * data. The spec tells us reset the AHB domain and Serial Flash domain at
 * the same time.
 */
static void fsl_qspi_invalidate(struct fsl_qspi *q)
{
	u32 mcr;
	u32 reset_mask = QUADSPI_MCR_SWRSTHD_MASK | QUADSPI_MCR_SWRSTSD_MASK;

	mcr = qspi_readl(q, q->iobase + QUADSPI_MCR);

	mcr &= ~QUADSPI_MCR_MDIS_MASK;
	qspi_writel(q, mcr, q->iobase + QUADSPI_MCR);

	mcr = qspi_readl(q, q->iobase + QUADSPI_MCR);
	mcr |= reset_mask;
	qspi_writel(q, mcr, q->iobase + QUADSPI_MCR);

	mcr = qspi_readl(q, q->iobase + QUADSPI_MCR) | QUADSPI_MCR_MDIS_MASK;
	qspi_writel(q, mcr, q->iobase + QUADSPI_MCR);

	mcr &= ~(reset_mask);
	qspi_writel(q, mcr, q->iobase + QUADSPI_MCR);

	mcr = qspi_readl(q, q->iobase + QUADSPI_MCR);
	mcr &= ~QUADSPI_MCR_MDIS_MASK;
	qspi_writel(q, mcr, q->iobase + QUADSPI_MCR);
}

static void fsl_qspi_select_mem(struct fsl_qspi *q, struct spi_slave *slave)
{
	struct dm_spi_slave_plat *plat =
		dev_get_parent_plat(slave->dev);

	if (q->selected == plat->cs)
		return;

	q->selected = plat->cs;
	fsl_qspi_invalidate(q);
}

static u32 fsl_qspi_memsize_per_cs(struct fsl_qspi *q)
{
	if (IS_ENABLED(CONFIG_FSL_QSPI_AHB_FULL_MAP)) {
		if (needs_single_bus(q))
			return q->memmap_size / 2;
		else
			return q->memmap_size / 4;
	} else {
		return ALIGN(q->devtype_data->ahb_buf_size, 0x400);
	}
}

static void fsl_qspi_read_ahb(struct fsl_qspi *q, const struct spi_mem_op *op)
{
	void __iomem *ahb_read_addr = q->ahb_addr;

	if (IS_ENABLED(CONFIG_FSL_QSPI_AHB_FULL_MAP)) {
		if (op->addr.nbytes)
			ahb_read_addr += op->addr.val;
	}

	enable_ahb_buf_cache(q);
	memcpy_fromio(op->data.buf.in,
		      ahb_read_addr + q->selected * fsl_qspi_memsize_per_cs(q),
		      op->data.nbytes);
	disable_ahb_buf_cache(q);
}

static int fsl_qspi_default_setup(struct fsl_qspi *q);
static int fsl_qspi_readl_poll_tout(struct fsl_qspi *q, void __iomem *base,
				    u32 mask, u32 delay_us, u32 timeout_us);

static int fsl_qspi_fill_txfifo(struct fsl_qspi *q,
				const struct spi_mem_op *op)
{
	void __iomem *base = q->iobase;
	int i, err;
	u32 val;

	/* TX buffer is empty */
	err = fsl_qspi_readl_poll_tout(q, base + QUADSPI_TBSR,
				       QUADSPI_TBSR_TRBFL_MASK,
				       10, 1000);
	if (err)
		return err;

	for (i = 0; i < ALIGN_DOWN(op->data.nbytes, 4); i += 4) {
		memcpy(&val, op->data.buf.out + i, 4);
		val = fsl_qspi_endian_xchg(q, val);
		qspi_writel(q, val, base + QUADSPI_TBDR);
	}

	if (i < op->data.nbytes) {
		memcpy(&val, op->data.buf.out + i, op->data.nbytes - i);
		val = fsl_qspi_endian_xchg(q, val);
		qspi_writel(q, val, base + QUADSPI_TBDR);
	}

	if (needs_fill_txfifo(q)) {
		for (i = op->data.nbytes; i < 16; i += 4)
			qspi_writel(q, 0, base + QUADSPI_TBDR);
	}

	return 0;
}

static void fsl_qspi_read_rxfifo(struct fsl_qspi *q,
				 const struct spi_mem_op *op)
{
	void __iomem *base = q->iobase;
	int i;
	u8 *buf = op->data.buf.in;
	u32 val;

	for (i = 0; i < ALIGN_DOWN(op->data.nbytes, 4); i += 4) {
		val = qspi_readl(q, base + QUADSPI_RBDR(i / 4));
		val = fsl_qspi_endian_xchg(q, val);
		memcpy(buf + i, &val, 4);
	}

	if (i < op->data.nbytes) {
		val = qspi_readl(q, base + QUADSPI_RBDR(i / 4));
		val = fsl_qspi_endian_xchg(q, val);
		memcpy(buf + i, &val, op->data.nbytes - i);
	}
}

static int fsl_qspi_readl_poll_tout(struct fsl_qspi *q, void __iomem *base,
				    u32 mask, u32 delay_us, u32 timeout_us)
{
	u32 reg;

	if (!q->devtype_data->little_endian)
		mask = (u32)cpu_to_be32(mask);

	return readl_poll_timeout(base, reg, !(reg & mask), timeout_us);
}

static int fsl_qspi_do_op(struct fsl_qspi *q, const struct spi_mem_op *op)
{
	void __iomem *base = q->iobase;
	int err = 0;
	u32 tbsr, trctr, trbfl, words;

	/*
	 * Always start the sequence at the same index since we update
	 * the LUT at each exec_op() call. And also specify the DATA
	 * length, since it's has not been specified in the LUT.
	 */
	qspi_writel(q, op->data.nbytes | QUADSPI_IPCR_SEQID(SEQID_LUT),
		    base + QUADSPI_IPCR);

	if (op->data.nbytes && op->data.dir == SPI_MEM_DATA_OUT) {
		words = op->data.nbytes / 4;
		if (op->data.nbytes % 4)
			words++;

		/* Wait until all bytes are transmitted */
		do {
			tbsr = qspi_readl(q, base + QUADSPI_TBSR);
			trctr = QUADSPI_TBSR_TRCTR(tbsr);
			trbfl = QUADSPI_TBSR_TRBFL(tbsr);
		} while ((trctr != words) || trbfl);
	}

	/* wait for the controller being ready */
	err = fsl_qspi_readl_poll_tout(q, base + QUADSPI_SR,
				       (QUADSPI_SR_IP_ACC_MASK |
					QUADSPI_SR_AHB_ACC_MASK |
					QUADSPI_SR_BUSY),
					10, 1000);

	if (!err && op->data.nbytes && op->data.dir == SPI_MEM_DATA_IN)
		fsl_qspi_read_rxfifo(q, op);

	return err;
}

static void dllcra_bypass(struct fsl_qspi *q, u32 dllmask)
{
	void __iomem *base = q->iobase;
	u32 dllcra;

	dllmask &= QUADSPI_DLLCR_MASK;

	dllcra = QUADSPI_DLLCR_SLV_EN | QUADSPI_DLLCR_SLV_BYPASS_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	dllcra |= dllmask;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	dllcra |= QUADSPI_DLLCR_SLV_BYPASS_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	dllcra |= QUADSPI_DLLCR_SLV_UPD_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	while (!(qspi_readl(q, base + QUADSPI_DLLSR) &
		 QUADSPI_DLLSR_SLVA_LOCK))
		;

	dllcra &= ~QUADSPI_DLLCR_SLV_UPD_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);
}

static void dllcra_manual(struct fsl_qspi *q, u32 dllmask)
{
	void __iomem *base = q->iobase;
	u32 dllcra;

	dllmask &= QUADSPI_DLLCR_MASK;

	dllcra = QUADSPI_DLLCR_SLV_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	dllcra |= dllmask;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	dllcra |= QUADSPI_DLLCR_DLLEN_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	while (!(qspi_readl(q, base + QUADSPI_DLLSR) &
		 QUADSPI_DLLSR_DLLA_LOCK))
		;

	dllcra &= ~QUADSPI_DLLCR_SLV_AUTO_UPDT_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	dllcra &= ~QUADSPI_DLLCR_SLV_BYPASS_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	dllcra = qspi_readl(q, base + QUADSPI_DLLCRA);
	dllcra |= QUADSPI_DLLCR_SLV_UPD_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	while (!(qspi_readl(q, base + QUADSPI_DLLSR) &
		 QUADSPI_DLLSR_SLVA_LOCK))
		;
}

static void dllcra_auto(struct fsl_qspi *q, u32 dllmask)
{
	void __iomem *base = q->iobase;
	u32 dllcra;

	dllmask &= QUADSPI_DLLCR_MASK;

	dllcra = QUADSPI_DLLCR_SLV_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	dllcra |= dllmask;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	dllcra |= QUADSPI_DLLCR_SLV_AUTO_UPDT_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	dllcra |= QUADSPI_DLLCR_DLLEN_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	while (!(qspi_readl(q, base + QUADSPI_DLLSR) &
		 QUADSPI_DLLSR_DLLA_LOCK))
		;

	dllcra = qspi_readl(q, base + QUADSPI_DLLCRA);
	dllcra &= ~QUADSPI_DLLCR_SLV_BYPASS_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);

	dllcra |= QUADSPI_DLLCR_SLV_UPD_EN;
	qspi_writel(q, dllcra, base + QUADSPI_DLLCRA);
	while (!(qspi_readl(q, base + QUADSPI_DLLSR) &
		 QUADSPI_DLLSR_SLVA_LOCK))
		;
}

static int program_dllcra(struct fsl_qspi *q, u32 dllcra)
{
	u32 bypass = (dllcra >> QUADSPI_DLLCR_SLV_BYPASS_SHIFT) & BIT(0);
	u32 slven = (dllcra >> QUADSPI_DLLCR_SLV_EN_SHIFT) & BIT(0);
	u32 autoupd = (dllcra >> QUADSPI_DLLCR_SLV_AUTO_UPDT_SHIFT) & BIT(0);

	/* Bypass mode */
	if (slven && bypass && !autoupd) {
		dllcra_bypass(q, dllcra);
		return 0;
	}

	/* Manual mode */
	if (slven && !bypass && !autoupd) {
		dllcra_manual(q, dllcra);
		return 0;
	}

	/* Auto update mode */
	if (slven && !bypass && autoupd) {
		dllcra_auto(q, dllcra);
		return 0;
	}

	printf("Error: Failed to detect a correct mode for dllcr: 0x%"
	       PRIx32 "\n", dllcra);

	return -1;
}

static int enable_octal_ddr(struct fsl_qspi *q)
{
	void __iomem *base = q->iobase;
	u32 mcr, dllcr, smpr;
	int ret;

	if (q->proto == SNOR_PROTO_8_8_8_DTR)
		return 0;

	while (qspi_readl(q, base + QUADSPI_SR) & QUADSPI_SR_BUSY)
		;

	/* Disable the module */
	mcr = qspi_readl(q, base + QUADSPI_MCR);
	mcr |= QUADSPI_MCR_MDIS_MASK;
	qspi_writel(q, mcr, base + QUADSPI_MCR);

	mcr |= octal_ddr_conf.mcr;
	qspi_writel(q, mcr, base + QUADSPI_MCR);

	smpr = octal_ddr_conf.smpr;
	if (is_s32g3_qspi(q))
		smpr = QUADSPI_SMPR_DLLFSMPFA_NTH(3) |
			QUADSPI_SMPR_DLLFSMPFB_NTH(3);

	qspi_writel(q, octal_ddr_conf.flshcr, base + QUADSPI_FLSHCR);
	qspi_writel(q, octal_ddr_conf.sfacr, base + QUADSPI_SFACR);
	qspi_writel(q, smpr, base + QUADSPI_SMPR);
	qspi_writel(q, octal_ddr_conf.dlcr, base + QUADSPI_DLCR);

	qspi_writel(q, octal_ddr_conf.dlpr, base + QUADSPI_DLPR);

	/* Init AHB interface - 1024 bytes */
	qspi_writel(q, QUADSPI_BUF3CR_ALLMST_MASK |
		    (q->devtype_data->rxfifo << QUADSPI_BUF3CR_ADATSZ_SHIFT),
		    base + QUADSPI_BUF3CR);

	qspi_writel(q, octal_ddr_conf.flash1_size, base + QUADSPI_SFA1AD);
	qspi_writel(q, octal_ddr_conf.flash2_size, base + QUADSPI_SFA2AD);
	qspi_writel(q, octal_ddr_conf.flash1_size, base + QUADSPI_SFB1AD);
	qspi_writel(q, octal_ddr_conf.flash2_size, base + QUADSPI_SFB2AD);

	/* Enable the module */
	mcr = qspi_readl(q, base + QUADSPI_MCR);
	mcr &= ~QUADSPI_MCR_MDIS_MASK;
	qspi_writel(q, mcr, base + QUADSPI_MCR);

	dllcr = octal_ddr_conf.dllcr;
	if (low_freq_chain(q))
		dllcr &= ~QUADSPI_DLLCR_FREQEN_EN;

	ret = program_dllcra(q, dllcr);
	if (ret) {
		printf("Error: Failed to apply dllcra settings\n");
		return ret;
	}

	q->proto = SNOR_PROTO_8_8_8_DTR;

	return 0;
}

static bool is_octal_dtr_op(const struct spi_mem_op *op)
{
	/* Command */
	if (op->cmd.buswidth == 8 && op->cmd.dtr &&
	    op->data.dir == SPI_MEM_NO_DATA)
		return true;

	if (op->cmd.buswidth == 8 && op->cmd.dtr &&
	    op->addr.buswidth == 8 && op->addr.dtr &&
	    op->dummy.buswidth == 8 && op->dummy.dtr)
		return true;

	return false;
}

static bool is_spi_op(const struct spi_mem_op *op)
{
	if (op->cmd.buswidth == 1 && !op->cmd.dtr)
		return true;

	if (op->cmd.buswidth == 1 && !op->cmd.dtr &&
	    op->addr.buswidth == 1 && !op->addr.dtr &&
	    op->dummy.buswidth == 1 && !op->dummy.dtr)
		return true;

	return false;
}

#ifdef DEBUG
static void dump_op(const struct spi_mem_op *op)
{
	printf("\tcmd.opcode = 0x%" PRIx8 "\n", op->cmd.opcode);
	printf("\tcmd.buswidth = 0x%" PRIx8 "\n", op->cmd.buswidth);
	printf("\tcmd.dtr = 0x%" PRIx8 "\n", op->cmd.dtr);
	printf("\taddr.nbytes = 0x%" PRIx8 "\n", op->addr.nbytes);
	printf("\taddr.buswidth = 0x%" PRIx8 "\n", op->addr.buswidth);
	printf("\taddr.val = 0x%" PRIx64 "\n", op->addr.val);
	printf("\taddr.dtr = 0x%" PRIx8 "\n", op->addr.dtr);
	printf("\tdummy.nbytes = 0x%" PRIx8 "\n", op->dummy.nbytes);
	printf("\tdummy.buswidth = 0x%" PRIx8 "\n", op->dummy.buswidth);
	printf("\tdata.nbytes = 0x%x\n", op->data.nbytes);
	printf("\tdata.dir = %s\n", op->data.dir == SPI_MEM_DATA_OUT ?
	       "OUT" : "IN");
	printf("\tdata.dtr = 0x%" PRIx8 "\n", op->data.dtr);
}
#endif

static int fsl_qspi_exec_op(struct spi_slave *slave,
			    const struct spi_mem_op *op)
{
	struct fsl_qspi *q = dev_get_priv(slave->dev->parent);
	void __iomem *base = q->iobase;
	u32 addr_offset = 0;
	int err = 0;

	/* wait for the controller being ready */
	fsl_qspi_readl_poll_tout(q, base + QUADSPI_SR, (QUADSPI_SR_IP_ACC_MASK |
				 QUADSPI_SR_AHB_ACC_MASK | QUADSPI_SR_BUSY),
				 10, 1000);

	fsl_qspi_select_mem(q, slave);

	if (needs_amba_base_offset(q))
		addr_offset = q->memmap_phy;

	if (IS_ENABLED(CONFIG_FSL_QSPI_AHB_FULL_MAP)) {
		if (op->addr.nbytes)
			addr_offset += op->addr.val;
	}

	qspi_writel(q,
		    q->selected * fsl_qspi_memsize_per_cs(q) + addr_offset,
		    base + QUADSPI_SFAR);

	qspi_writel(q, qspi_readl(q, base + QUADSPI_MCR) |
		    QUADSPI_MCR_CLR_RXF_MASK | QUADSPI_MCR_CLR_TXF_MASK,
		    base + QUADSPI_MCR);

	qspi_writel(q, QUADSPI_SPTRCLR_BFPTRC | QUADSPI_SPTRCLR_IPPTRC,
		    base + QUADSPI_SPTRCLR);

#ifdef DEBUG
	dump_op(op);
#endif
	fsl_qspi_prepare_lut(q, op);

	if (is_octal_dtr_op(op))
		enable_octal_ddr(q);

	if (is_spi_op(op))
		fsl_qspi_default_setup(q);

	/*
	 * If we have large chunks of data, we read them through the AHB bus
	 * by accessing the mapped memory. In all other cases we use
	 * IP commands to access the flash.
	 */
	if (op->data.nbytes > (q->devtype_data->rxfifo - 4) &&
	    op->data.dir == SPI_MEM_DATA_IN) {
		qspi_writel(q, QUADSPI_RBCT_WMRK_MASK |
			    0, base + QUADSPI_RBCT);
		fsl_qspi_read_ahb(q, op);
	} else {
		qspi_writel(q, QUADSPI_RBCT_WMRK_MASK |
			    QUADSPI_RBCT_RXBRD_USEIPS, base + QUADSPI_RBCT);

		if (op->data.nbytes && op->data.dir == SPI_MEM_DATA_OUT)
			err = fsl_qspi_fill_txfifo(q, op);

		if (!err)
			err = fsl_qspi_do_op(q, op);
	}

	/* Invalidate the data in the AHB buffer. */
	fsl_qspi_invalidate(q);

	return err;
}

static int fsl_qspi_adjust_op_size(struct spi_slave *slave,
				   struct spi_mem_op *op)
{
	struct fsl_qspi *q = dev_get_priv(slave->dev->parent);

	if (op->data.dir == SPI_MEM_DATA_OUT) {
		if (op->data.nbytes > q->devtype_data->txfifo)
			op->data.nbytes = q->devtype_data->txfifo;
	} else {
		if (op->data.nbytes > q->devtype_data->ahb_buf_size) {
			if (!can_read_entire_ahb(q))
				op->data.nbytes = q->devtype_data->ahb_buf_size;
		} else if (op->data.nbytes > (q->devtype_data->rxfifo - 4)) {
			op->data.nbytes = ALIGN_DOWN(op->data.nbytes, 8);
		}
	}

	return 0;
}

static int fsl_qspi_default_setup(struct fsl_qspi *q)
{
	void __iomem *base = q->iobase;
	u32 reg, addr_offset = 0, memsize_cs;
	int ret = 0;

	if (q->proto == SNOR_PROTO_1_1_1)
		return 0;

	/* Reset the module */
	qspi_writel(q, QUADSPI_MCR_SWRSTSD_MASK | QUADSPI_MCR_SWRSTHD_MASK,
		    base + QUADSPI_MCR);
	udelay(1);

	/* Disable the module */
	qspi_writel(q, QUADSPI_MCR_MDIS_MASK | QUADSPI_MCR_RESERVED_MASK,
		    base + QUADSPI_MCR);

	/*
	 * Previous boot stages (BootROM, bootloader) might have used DDR
	 * mode and did not clear the TDH bits. As we currently use SDR mode
	 * only, clear the TDH bits if necessary.
	 */
	if (needs_tdh_setting(q))
		qspi_writel(q, qspi_readl(q, base + QUADSPI_FLSHCR) &
			    ~QUADSPI_FLSHCR_TDH_MASK,
			    base + QUADSPI_FLSHCR);

	reg = qspi_readl(q, base + QUADSPI_SMPR);
	reg &= ~(QUADSPI_SMPR_FSDLY_MASK
		 | QUADSPI_SMPR_FSPHS_MASK
		 | QUADSPI_SMPR_HSENA_MASK
		 | QUADSPI_SMPR_DDRSMP_MASK);

	if (has_dll(q)) {
		reg &= ~(QUADSPI_SMPR_DLLFSMPFA_MASK |
			 QUADSPI_SMPR_DLLFSMPFB_MASK);
		reg |= QUADSPI_SMPR_FSPHS_MASK;
	}

	qspi_writel(q, reg, base + QUADSPI_SMPR);

	/* We only use the buffer3 for AHB read */
	qspi_writel(q, 0, base + QUADSPI_BUF0IND);
	qspi_writel(q, 0, base + QUADSPI_BUF1IND);
	qspi_writel(q, 0, base + QUADSPI_BUF2IND);

	if (IS_ENABLED(CONFIG_FSL_QSPI_AHB_FULL_MAP))
		qspi_writel(q, QUADSPI_BFGENCR_SEQID(SEQID_LUT_AHB),
			    q->iobase + QUADSPI_BFGENCR);
	else
		qspi_writel(q, QUADSPI_BFGENCR_SEQID(SEQID_LUT),
			    q->iobase + QUADSPI_BFGENCR);

	qspi_writel(q, QUADSPI_RBCT_WMRK_MASK, base + QUADSPI_RBCT);
	qspi_writel(q, QUADSPI_BUF3CR_ALLMST_MASK |
		    QUADSPI_BUF3CR_ADATSZ(q->devtype_data->ahb_buf_size / 8),
		    base + QUADSPI_BUF3CR);

	if (needs_amba_base_offset(q))
		addr_offset = q->memmap_phy;

	/*
	 * In HW there can be a maximum of four chips on two buses with
	 * two chip selects on each bus. We use four chip selects in SW
	 * to differentiate between the four chips.
	 * We use ahb_buf_size for each chip and set SFA1AD, SFA2AD, SFB1AD,
	 * SFB2AD accordingly.
	 */
	memsize_cs = fsl_qspi_memsize_per_cs(q);
	qspi_writel(q, memsize_cs + addr_offset,
		    base + QUADSPI_SFA1AD);
	qspi_writel(q, memsize_cs * 2 + addr_offset,
		    base + QUADSPI_SFA2AD);
	if (!needs_single_bus(q)) {
		qspi_writel(q, memsize_cs * 3 + addr_offset,
			    base + QUADSPI_SFB1AD);
		qspi_writel(q, memsize_cs * 4 + addr_offset,
			    base + QUADSPI_SFB2AD);
	}

	q->selected = -1;

	/* Enable the module */
	reg = QUADSPI_MCR_RESERVED_MASK | QUADSPI_MCR_END_CFG_MASK;

	if (sdr_internal_loopback(q))
		reg |= QUADSPI_MCR_DQS_INT_PAD;

	if (has_dll(q)) {
		reg |= QUADSPI_MCR_DQS_EN;
		qspi_writel(q, 0, base + QUADSPI_SFACR);
	}

	qspi_writel(q, reg, base + QUADSPI_MCR);

	if (has_dll(q))
		ret = program_dllcra(q, QUADSPI_DLLCR_SLV_BYPASS_EN |
				     QUADSPI_DLLCR_SLV_EN);

	if (!ret)
		q->proto = SNOR_PROTO_1_1_1;

	return ret;
}

static __maybe_unused int fsl_qspi_clk_en(struct udevice *bus)
{
	struct clk clk_qspi, clk_qspi_en;
	int ret;

	ret = clk_get_by_name(bus, "qspi_en", &clk_qspi_en);
	if (ret)
		return ret;

	ret = clk_get_by_name(bus, "qspi", &clk_qspi);
	if (ret)
		return ret;

	ret = clk_enable(&clk_qspi_en);
	if (ret)
		return ret;

	ret = clk_enable(&clk_qspi);
	if (ret)
		clk_disable(&clk_qspi_en);

	return ret;
}

static const struct spi_controller_mem_ops fsl_qspi_mem_ops = {
	.adjust_op_size = fsl_qspi_adjust_op_size,
	.supports_op = fsl_qspi_supports_op,
	.exec_op = fsl_qspi_exec_op,
};

static int fsl_qspi_probe(struct udevice *bus)
{
	struct dm_spi_bus *dm_bus = dev_get_uclass_priv(bus);
	struct fsl_qspi *q = dev_get_priv(bus);
	fdt_addr_t mem_base = 0U;
	fdt_addr_t mem_size = 0U;
	int ret;

	q->dev = bus;
	q->devtype_data = (struct fsl_qspi_devtype_data *)
			   dev_get_driver_data(bus);

	ret = fsl_qspi_clk_en(bus);
	if (ret) {
		dev_err(bus, "Failed to enable qspi clk(ret = %d)!\n", ret);
		return ret;
	}

	/* find the resources */
	mem_base = dev_read_addr_size_name(bus, "QuadSPI", &mem_size);
	if (mem_base == FDT_ADDR_T_NONE) {
		dev_err(bus, "Can't get regs base addresses(ret = %d)!\n", ret);
		return -ENOMEM;
	}

	q->iobase = devm_ioremap(dev, mem_base, mem_size);

	mem_base = dev_read_addr_size_name(bus, "QuadSPI-memory", &mem_size);
	if (mem_base == FDT_ADDR_T_NONE) {
		dev_err(bus, "Can't get AMBA base addresses(ret = %d)!\n", ret);
		return -ENOMEM;
	}

	q->ahb_addr = devm_ioremap(dev, mem_base, mem_size);
	q->memmap_phy = mem_base;
	q->memmap_size = mem_size;

	dm_bus->max_hz = dev_read_u32_default(bus, "spi-max-frequency",
					      66000000);

	fsl_qspi_default_setup(q);

	return 0;
}

static int fsl_qspi_xfer(struct udevice *dev, unsigned int bitlen,
			 const void *dout, void *din, unsigned long flags)
{
	return 0;
}

static int fsl_qspi_claim_bus(struct udevice *dev)
{
	return 0;
}

static int fsl_qspi_release_bus(struct udevice *dev)
{
	return 0;
}

static int fsl_qspi_set_speed(struct udevice *bus, uint speed)
{
	return 0;
}

static int fsl_qspi_set_mode(struct udevice *bus, uint mode)
{
	return 0;
}

static const struct dm_spi_ops fsl_qspi_ops = {
	.claim_bus	= fsl_qspi_claim_bus,
	.release_bus	= fsl_qspi_release_bus,
	.xfer		= fsl_qspi_xfer,
	.set_speed	= fsl_qspi_set_speed,
	.set_mode	= fsl_qspi_set_mode,
	.mem_ops	= &fsl_qspi_mem_ops,
};

static const struct udevice_id fsl_qspi_ids[] = {
	{ .compatible = "fsl,vf610-qspi", .data = (ulong)&vybrid_data, },
	{ .compatible = "fsl,imx6sx-qspi", .data = (ulong)&imx6sx_data, },
	{ .compatible = "fsl,imx6ul-qspi", .data = (ulong)&imx6ul_data, },
	{ .compatible = "fsl,imx7d-qspi", .data = (ulong)&imx7d_data, },
	{ .compatible = "fsl,imx7ulp-qspi", .data = (ulong)&imx7ulp_data, },
	{ .compatible = "fsl,ls1021a-qspi", .data = (ulong)&ls1021a_data, },
	{ .compatible = "fsl,ls1088a-qspi", .data = (ulong)&ls2080a_data, },
	{ .compatible = "fsl,ls2080a-qspi", .data = (ulong)&ls2080a_data, },
	{ .compatible = "nxp,s32g-qspi", .data = (ulong)&s32g_data },
	{ .compatible = "nxp,s32g3-qspi", .data = (ulong)&s32g3_data },
	{ .compatible = "nxp,s32r45-qspi", .data = (ulong)&s32r45_data },
	{ }
};

U_BOOT_DRIVER(fsl_qspi) = {
	.name	= "fsl_qspi",
	.id	= UCLASS_SPI,
	.of_match = fsl_qspi_ids,
	.ops	= &fsl_qspi_ops,
	.priv_auto	= sizeof(struct fsl_qspi),
	.probe	= fsl_qspi_probe,
};
