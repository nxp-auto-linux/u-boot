// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2021 NXP
 */
#include "fsl_qspi.h"
#include <cpu_func.h>
#include <asm/io.h>
#include <asm/arch/siul.h>
#include <linux/mtd/spi-nor.h>
#include <spi-mem.h>
#include <inttypes.h>

#define LUT_INVALID_INDEX -1
#define LUT_STOP_CMD 0x0
#define MAX_OPCODE 0xff

#define MAX_LUTS 80
#define LUTS_PER_CONFIG 5
#define MAX_LUTS_CONFIGS (MAX_LUTS / LUTS_PER_CONFIG)

/* JESD216D.01 */
#define SPINOR_OP_RDCR2		0x71
#define SPINOR_OP_WRCR2		0x72

#define QSPI_CFG2_OPI_MASK		(0x3)
#define QSPI_CFG2_STR_OPI_ENABLED	BIT(0)
#define QSPI_CFG2_DTR_OPI_ENABLED	BIT(1)

struct lut_config {
	bool enabled;
	u32 conf[MAX_LUTS_CONFIGS];
	u8 fill;
	u8 index;
};

struct qspi_op {
	const struct spi_mem_op *op;
	u8 *cfg;
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

static u8 luts_next_config;
static struct lut_config lut_configs[MAX_OPCODE];

#ifdef CONFIG_SPI_FLASH_MACRONIX
/* JESD216D.01 operations used for DTR OPI switch */
static struct spi_mem_op rdcr2_sdr_op =
SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_RDCR2, 1),
	   SPI_MEM_OP_ADDR(0x4, 0x0, 1),
	   SPI_MEM_OP_NO_DUMMY,
	   SPI_MEM_OP_DATA_IN(1, NULL, 1));

static struct spi_mem_op wren_sdr_op =
SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_WREN, 1),
	   SPI_MEM_OP_NO_ADDR,
	   SPI_MEM_OP_NO_DUMMY,
	   SPI_MEM_OP_DATA_IN(0, NULL, 1));

static struct spi_mem_op rdsr_sdr_op =
SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_RDSR, 1),
	   SPI_MEM_OP_NO_ADDR,
	   SPI_MEM_OP_NO_DUMMY,
	   SPI_MEM_OP_DATA_IN(1, NULL, 1));

static struct spi_mem_op wrcr2_sdr_op =
SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_WRCR2, 1),
	   SPI_MEM_OP_ADDR(0x4, 0x0, 1),
	   SPI_MEM_OP_NO_DUMMY,
	   SPI_MEM_OP_DATA_OUT(1, NULL, 1));

/* JESD216D.01 operations used for soft reset */
static struct spi_mem_op rsten_ddr_op =
SPI_MEM_OP(SPI_MEM_OP_CMD(0x66, 8),
	   SPI_MEM_OP_NO_ADDR,
	   SPI_MEM_OP_NO_DUMMY,
	   SPI_MEM_OP_NO_DATA);

static struct spi_mem_op rst_ddr_op =
SPI_MEM_OP(SPI_MEM_OP_CMD(0x99, 8),
	   SPI_MEM_OP_NO_ADDR,
	   SPI_MEM_OP_NO_DUMMY,
	   SPI_MEM_OP_NO_DATA);
#endif /* CONFIG_SPI_FLASH_MACRONIX */

#ifdef DEBUG
static void dump_op(const struct spi_mem_op *op)
{
	printf("\tcmd.opcode = 0x%" PRIx8 "\n", op->cmd.opcode);
	printf("\tcmd.buswidth = 0x%" PRIx8 "\n", op->cmd.buswidth);
	printf("\taddr.nbytes = 0x%" PRIx8 "\n", op->addr.nbytes);
	printf("\taddr.buswidth = 0x%" PRIx8 "\n", op->addr.buswidth);
	printf("\taddr.val = 0x%" PRIx64 "\n", op->addr.val);
	printf("\tdummy.nbytes = 0x%" PRIx8 "\n", op->dummy.nbytes);
	printf("\tdummy.buswidth = 0x%" PRIx8 "\n", op->dummy.buswidth);
	printf("\tdata.nbytes = 0x%x\n", op->data.nbytes);
	printf("\tdata.dir = %s\n", op->data.dir == SPI_MEM_DATA_OUT ?
	       "OUT" : "IN");
}
#endif

static int s32gen1_adjust_op_size(struct spi_slave *slave,
				  struct spi_mem_op *op)
{
	return 0;
}

static u32 clear_fifos(struct fsl_qspi_priv *priv)
{
	struct fsl_qspi_regs *regs = priv->regs;
	u32 mcr_reg;

	mcr_reg = qspi_read32(priv->flags, &regs->mcr);

	/* Clear TX & RX fifos */
	qspi_write32(priv->flags, &regs->mcr,
		     mcr_reg |
		     QSPI_MCR_CLR_RXF_MASK | QSPI_MCR_CLR_TXF_MASK |
		     QSPI_MCR_RESERVED_MASK | QSPI_MCR_END_CFD_LE);
	return mcr_reg;
}

static int qspi_write_reg(struct fsl_qspi_priv *priv,
			  const struct spi_mem_op *op, u8 lut_cfg)
{
	struct fsl_qspi_regs *regs = priv->regs;
	u32 mcr_reg, i, words = 0;
	u32 tbsr, trctr, trbfl;
	const u32 *txbuf = op->data.buf.out;
	u32 len = op->data.nbytes;

	mcr_reg = clear_fifos(priv);

	/* Controller isn't busy */
	while (qspi_read32(priv->flags, &regs->sr) & QSPI_SR_BUSY_MASK)
		;

	/* TX buffer is empty */
	while (QSPI_TBSR_TRBFL(qspi_read32(priv->flags, &regs->tbsr)))
		;

	if (op->addr.nbytes) {
		/* Set address */
		qspi_write32(priv->flags, &regs->sfar, op->addr.val);
	}

	if (op->data.nbytes) {
		words = len / 4;

		if (len % 4)
			words++;

		for (i = 0; i < words; i++) {
			qspi_write32(priv->flags, &regs->tbdr, *txbuf);
			txbuf++;
		}

		qspi_write32(priv->flags, &regs->ipcr,
			     (lut_cfg << QSPI_IPCR_SEQID_SHIFT) | len);

		while (qspi_read32(priv->flags, &regs->sr) &
		       QSPI_SR_BUSY_MASK)
			;

		/* Wait until all bytes are transmitted */
		do {
			tbsr = qspi_read32(priv->flags, &regs->tbsr);
			trctr = QSPI_TBSR_TRCTR(tbsr);
			trbfl = QSPI_TBSR_TRBFL(tbsr);
		} while ((trctr != words) || trbfl);
	} else {
		qspi_write32(priv->flags, &regs->ipcr,
			     (lut_cfg << QSPI_IPCR_SEQID_SHIFT));
	}

	while (qspi_read32(priv->flags, &regs->sr) & QSPI_SR_BUSY_MASK)
		;

	qspi_write32(priv->flags, &regs->mcr, mcr_reg);
	return 0;
}

static int qspi_read_reg(struct fsl_qspi_priv *priv,
			 const struct spi_mem_op *op, u8 lut_cfg)
{
	struct fsl_qspi_regs *regs = priv->regs;
	u32 mcr_reg, rbsr_reg, data, size = 0;
	int i;
	u32 len = op->data.nbytes;
	u32 *rxbuf = op->data.buf.in;

	mcr_reg = clear_fifos(priv);

	/* Controller isn't busy */
	while (qspi_read32(priv->flags, &regs->sr) & QSPI_SR_BUSY_MASK)
		;

	/* Clear all flags */
	qspi_write32(priv->flags, &regs->fr, QSPI_FR_ALL_FLAGS_MASK);

	/* Read using IP registers */
	qspi_write32(priv->flags, &regs->rbct, QSPI_RBCT_RXBRD_USEIPS);

	if (op->addr.nbytes)
		qspi_write32(priv->flags, &regs->sfar, op->addr.val);

	qspi_write32(priv->flags, &regs->ipcr,
		     (lut_cfg << QSPI_IPCR_SEQID_SHIFT) | len);

	while (qspi_read32(priv->flags, &regs->sr) & QSPI_SR_BUSY_MASK)
		;

	i = 0;
	while (len > 0) {
		rbsr_reg = qspi_read32(priv->flags, &regs->rbsr);
		if (rbsr_reg & QSPI_RBSR_RDBFL_MASK) {
			data = qspi_read32(priv->flags, &regs->rbdr[i]);
			size = (len < 4) ? len : 4;
			memcpy(rxbuf, &data, size);
			len -= size;
			rxbuf++;
			i++;
		} else {
			break;
		}
	}

	qspi_write32(priv->flags, &regs->mcr, mcr_reg);
	return 0;
}

static u8 busw_to_pads(u8 buswidth, int *status)
{
	*status = 0;
	switch (buswidth) {
	case 1:
		return 0x0;
	case 2:
		return 0x1;
	case 4:
		return 0x2;
	case 8:
		return 0x3;
	}

	*status = -1;
	printf("Error: Unknown buswidth: 0x%0x\n", buswidth);
	return 0x3;
}

static void append_lut(struct lut_config *config, u16 lut)
{
	u32 conf_index = config->fill / 2;
	u32 mask, shift;

	if (config->fill % 2) {
		mask = GENMASK(31, 16);
		shift = 16;
	} else {
		mask = GENMASK(15, 0);
		shift = 0;
	}

	config->conf[conf_index] &= ~mask;
	config->conf[conf_index] |= (lut << shift);

	config->fill++;
}

static bool fill_qspi_cmd(struct fsl_qspi_priv *priv,
			  const struct spi_mem_op *op,
			  struct lut_config *lut_conf)
{
	u16 lut;
	int status;
	u8 opcode = op->cmd.opcode;
	u8 lut_cmd;

	switch (op->cmd.buswidth) {
		/* SPI */
	case 1:
		lut_cmd = LUT_CMD;
		break;
		/* OPI */
	case 8:
		if (priv->ddr_mode)
			lut_cmd = LUT_CMD_DDR;
		else
			lut_cmd = LUT_CMD;
		break;
	default:
		return false;
	};

	lut = OPRND0(opcode) | PAD0(busw_to_pads(op->cmd.buswidth, &status)) |
	    INSTR0(lut_cmd);
	if (status)
		return false;

	append_lut(lut_conf, lut);

	/* Octal command */
	if (op->cmd.buswidth == 8) {
		lut = OPRND0(~opcode & 0xFFU) |
		    PAD0(busw_to_pads(op->cmd.buswidth, &status)) |
		    INSTR0(lut_cmd);
		if (status)
			return false;

		append_lut(lut_conf, lut);
	}

	return true;
}

static bool fill_qspi_addr(struct fsl_qspi_priv *priv,
			   const struct spi_mem_op *op,
			   struct lut_config *lut_conf)
{
	u16 lut;
	int status;
	u8 lut_addr;

	lut_addr = LUT_ADDR;
	switch (op->addr.buswidth) {
		/* No address */
	case 0:
		return true;
		/* SPI */
	case 1:
		break;
		/* OPI */
	case 8:
		if (op->memop && priv->ddr_mode)
			lut_addr = LUT_ADDR_DDR;
		break;
	default:
		return false;
	};

	if (op->addr.nbytes) {
		lut = OPRND0(op->addr.nbytes * 8) |
		    PAD0(busw_to_pads(op->addr.buswidth, &status)) |
		    INSTR0(lut_addr);
		if (status)
			return false;

		append_lut(lut_conf, lut);
	}

	return true;
}

static bool fill_qspi_data(struct fsl_qspi_priv *priv,
			   const struct spi_mem_op *op,
			   struct lut_config *lut_conf)
{
	u16 lut;
	int status;
	u8 lut_read, lut_write;

	if (!op->data.nbytes)
		return true;

	lut_read = LUT_READ;
	lut_write = LUT_WRITE;
	switch (op->data.buswidth) {
		/* SPI */
	case 1:
		break;
		/* OPI */
	case 8:
		if (op->memop && priv->ddr_mode) {
			lut_read = LUT_READ_DDR;
			lut_write = LUT_WRITE_DDR;
		}
		break;
	default:
		return false;
	};

	if (op->data.dir == SPI_MEM_DATA_IN)
		lut = INSTR0(lut_read);
	else
		lut = INSTR0(lut_write);

	/* HW limitation for memory write operation */
	if (op->data.dir == SPI_MEM_DATA_OUT && op->memop) {
		lut |= OPRND0(0);
	} else {
		if (op->data.nbytes > RX_BUFFER_SIZE)
			lut |= OPRND0(RX_BUFFER_SIZE);
		else
			lut |= OPRND0(op->data.nbytes);
	}

	if (op->data.buswidth)
		lut |= PAD0(busw_to_pads(op->data.buswidth, &status));
	else
		lut |= PAD0(busw_to_pads(op->cmd.buswidth, &status));

	if (status)
		return false;

	append_lut(lut_conf, lut);

	return true;
}

static bool add_op_to_lutdb(struct fsl_qspi_priv *priv,
			    const struct spi_mem_op *op, u8 *index)
{
	u8 opcode = op->cmd.opcode;
	struct lut_config *lut_conf;
	u16 lut;
	int status;

	lut_conf = &lut_configs[opcode];
	lut_conf->fill = 0;

	if (!fill_qspi_cmd(priv, op, lut_conf))
		return false;

	if (!fill_qspi_addr(priv, op, lut_conf))
		return false;

	if (op->dummy.nbytes) {
		lut = OPRND0(op->dummy.nbytes * 8 / op->dummy.buswidth) |
		    INSTR0(LUT_DUMMY) |
		    PAD0(busw_to_pads(op->dummy.buswidth, &status));
		if (status)
			return false;

		append_lut(lut_conf, lut);
	}

	if (!fill_qspi_data(priv, op, lut_conf))
		return false;

	append_lut(lut_conf, LUT_STOP_CMD);
	append_lut(lut_conf, LUT_STOP_CMD);

	if (!lut_conf->index) {
		lut_conf->index = luts_next_config;
		luts_next_config++;
	}
	*index = lut_conf->index;

	return true;
}

static u32 *get_lut_seq_start(struct fsl_qspi_regs *regs, u32 index)
{
	return &regs->lut[index * LUTS_PER_CONFIG];
}

static void set_lut(struct fsl_qspi_priv *priv, u8 index, u8 opcode)
{
	struct fsl_qspi_regs *regs = priv->regs;
	u32 *iterb, *iter;
	u32 *lutaddr;

	iter = &lut_configs[opcode].conf[0];
	iterb = iter;

	lutaddr = get_lut_seq_start(regs, index);

	/* Unlock the LUT */
	qspi_write32(priv->flags, &regs->lutkey, LUT_KEY_VALUE);
	qspi_write32(priv->flags, &regs->lckcr, QSPI_LCKCR_UNLOCK);

	while (((*iter & GENMASK(15, 0)) != LUT_STOP_CMD) &&
	       (iter - iterb < sizeof(lut_configs[opcode].conf))) {
		qspi_write32(priv->flags, lutaddr, *iter);
		debug("lut: *(0x%p) = 0x%" PRIx32 "\n", lutaddr, *iter);
		iter++;
		lutaddr++;
	}
	qspi_write32(priv->flags, lutaddr, LUT_STOP_CMD);
	debug("lut: *(0x%p) = 0x%" PRIx32 "\n", lutaddr, LUT_STOP_CMD);

	/* Lock the LUT */
	qspi_write32(priv->flags, &regs->lutkey, LUT_KEY_VALUE);
	qspi_write32(priv->flags, &regs->lckcr, QSPI_LCKCR_LOCK);
}

static bool enable_op(struct fsl_qspi_priv *priv, const struct spi_mem_op *op)
{
	u8 lut_index;
	u8 opcode = op->cmd.opcode;
#ifdef DEBUG
	dump_op(op);
#endif

	if (luts_next_config >= MAX_LUTS_CONFIGS) {
		printf("Error: Too many LUT configurations\n");
		return false;
	}

	if (!lut_configs[opcode].enabled) {
		if (!add_op_to_lutdb(priv, op, &lut_index)) {
			printf("Error: Failed to add LUT configuration\n");
			return false;
		}

		set_lut(priv, lut_index, opcode);
		lut_configs[opcode].enabled = true;
	}

	return true;
}

#ifdef CONFIG_SPI_FLASH_MACRONIX
static bool enable_operators(struct fsl_qspi_priv *priv,
			     struct qspi_op *ops, size_t n_ops)
{
	bool res;
	size_t i;
	const struct spi_mem_op *op;
	u8 *cfg;

	for (i = 0; i < n_ops; i++) {
		op = ops[i].op;
		cfg = ops[i].cfg;

		/* In case it's already enabled */
		lut_configs[op->cmd.opcode].enabled = false;
		res = enable_op(priv, op);
		*cfg = lut_configs[op->cmd.opcode].index;

		if (!res || !lut_configs[op->cmd.opcode].enabled) {
			printf("Error: Failed to register operator 0x%x\n",
			       op->cmd.opcode);
			return false;
		}
	}

	return true;
}

static void disable_operators(struct qspi_op *ops, size_t n_ops)
{
	size_t i;
	const struct spi_mem_op *op;

	for (i = 0; i < n_ops; i++) {
		op = ops[i].op;

		lut_configs[op->cmd.opcode].enabled = false;
	}
}

static int memory_enable_ddr(struct fsl_qspi_priv *priv)
{
	struct fsl_qspi_regs *regs = priv->regs;
	u8 wren_cfg, rdcr2_cfg, rdsr_cfg, wrcr2_cfg;
	u8 cfg2_reg = 0x0;
	u8 status = 0;
	u32 mcr2;

	rdcr2_sdr_op.data.buf.out = &cfg2_reg;
	rdsr_sdr_op.data.buf.out = &status;
	wrcr2_sdr_op.data.buf.in = &cfg2_reg;

	struct qspi_op ops[] = {
		{
		 .op = &rdcr2_sdr_op,
		 .cfg = &rdcr2_cfg,
		 },
		{
		 .op = &wren_sdr_op,
		 .cfg = &wren_cfg,
		 },
		{
		 .op = &rdsr_sdr_op,
		 .cfg = &rdsr_cfg,
		 },
		{
		 .op = &wrcr2_sdr_op,
		 .cfg = &wrcr2_cfg,
		 },
	};

	while (qspi_read32(priv->flags, &regs->sr) & QSPI_SR_BUSY_MASK)
		;

	if (!enable_operators(priv, ops, ARRAY_SIZE(ops)))
		return -1;

	mcr2 = qspi_read32(priv->flags, &regs->mcr);

	/* Enable the module */
	qspi_write32(priv->flags, &regs->mcr, mcr2 & ~QSPI_MCR_MDIS_MASK);

	if (qspi_read_reg(priv, &rdcr2_sdr_op, rdcr2_cfg))
		return -1;

	cfg2_reg &= ~QSPI_CFG2_OPI_MASK;
	cfg2_reg |= QSPI_CFG2_DTR_OPI_ENABLED;

	/* Enable write */
	if (qspi_write_reg(priv, &wren_sdr_op, wren_cfg))
		return -1;

	/* Wait write enabled */
	while (!(status & FLASH_STATUS_WEL)) {
		if (qspi_read_reg(priv, &rdsr_sdr_op, rdsr_cfg))
			return -1;
	}

	if (qspi_write_reg(priv, &wrcr2_sdr_op, wrcr2_cfg))
		return -1;

	qspi_write32(priv->flags, &regs->mcr, mcr2);

	disable_operators(ops, ARRAY_SIZE(ops));
	udelay(400);

	return 0;
}
#endif

static void dllcra_bypass(struct fsl_qspi_priv *priv, u32 dllmask)
{
	u32 dllcra;
	struct fsl_qspi_regs *regs = priv->regs;

	dllmask &= QSPI_DLLCR_MASK;

	dllcra = QSPI_DLLCR_SLV_EN | QSPI_DLLCR_SLV_BYPASS_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	dllcra |= dllmask;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	dllcra |= QSPI_DLLCR_SLV_BYPASS_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	dllcra |= QSPI_DLLCR_SLV_UPD_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	while (!(qspi_read32(priv->flags, &regs->dllsr) &
		 QSPI_DLLSR_DLLA_LOCK_MASK))
		;

	dllcra &= ~QSPI_DLLCR_SLV_UPD_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);
}

static void dllcra_manual(struct fsl_qspi_priv *priv, u32 dllmask)
{
	u32 dllcra;
	struct fsl_qspi_regs *regs = priv->regs;

	dllmask &= QSPI_DLLCR_MASK;

	dllcra = QSPI_DLLCR_SLV_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	dllcra |= dllmask;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	dllcra |= QSPI_DLLCR_DLLEN_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	while (!(qspi_read32(priv->flags, &regs->dllsr) &
		 QSPI_DLLSR_DLLA_LOCK_MASK))
		;

	dllcra &= ~QSPI_DLLCR_SLV_AUTO_UPDT_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	dllcra &= ~QSPI_DLLCR_SLV_BYPASS_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	dllcra = qspi_read32(priv->flags, &regs->dllcra);
	dllcra |= QSPI_DLLCR_SLV_UPD_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	while (!(qspi_read32(priv->flags, &regs->dllsr) &
		 QSPI_DLLSR_SLVA_LOCK_MASK))
		;
}

static void dllcra_auto(struct fsl_qspi_priv *priv, u32 dllmask)
{
	u32 dllcra;
	struct fsl_qspi_regs *regs = priv->regs;

	dllmask &= QSPI_DLLCR_MASK;

	dllcra = QSPI_DLLCR_SLV_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	dllcra |= dllmask;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	dllcra |= QSPI_DLLCR_SLV_AUTO_UPDT_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	dllcra |= QSPI_DLLCR_DLLEN_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	while (!(qspi_read32(priv->flags, &regs->dllsr) &
		 QSPI_DLLSR_DLLA_LOCK_MASK))
		;

	dllcra = qspi_read32(priv->flags, &regs->dllcra);
	dllcra &= ~QSPI_DLLCR_SLV_BYPASS_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);

	dllcra |= QSPI_DLLCR_SLV_UPD_EN;
	qspi_write32(priv->flags, &regs->dllcra, dllcra);
	while (!(qspi_read32(priv->flags, &regs->dllsr) &
		 QSPI_DLLSR_SLVA_LOCK_MASK))
		;
}

static int program_dllcra(struct fsl_qspi_priv *priv, u32 dllcra)
{
	u32 bypass = (dllcra >> QSPI_DLLCR_SLV_BYPASS_SHIFT) & BIT(0);
	u32 slven = (dllcra >> QSPI_DLLCR_SLV_EN_SHIFT) & BIT(0);
	u32 autoupd = (dllcra >> QSPI_DLLCR_SLV_AUTO_UPDT_SHIFT) & BIT(0);

	/* Bypass mode */
	if (slven && bypass && !autoupd) {
		dllcra_bypass(priv, dllcra);
		return 0;
	}

	/* Manual mode */
	if (slven && !bypass && !autoupd) {
		dllcra_manual(priv, dllcra);
		return 0;
	}

	/* Auto update mode */
	if (slven && !bypass && autoupd) {
		dllcra_auto(priv, dllcra);
		return 0;
	}

	printf("Error: Failed to detect a correct mode for dllcr: 0x%"
	       PRIx32 "\n", dllcra);

	return -1;
}

static struct qspi_config ddr_config = {
	.mcr = QSPI_MCR_END_CFD_MASK |
	    QSPI_MCR_DQS_EN |
	    QSPI_MCR_DDR_EN_MASK |
	    QSPI_MCR_ISD2FA_EN |
	    QSPI_MCR_ISD3FA_EN |
	    QSPI_MCR_ISD2FB_EN |
	    QSPI_MCR_ISD3FB_EN |
	    QSPI_MCR_DQS_EXTERNAL,
	.flshcr = QSPI_FLSHCR_TCSS(3) |
	    QSPI_FLSHCR_TCHS(3) |
	    QSPI_FLSHCR_TDH(1),
	.dllcr = QSPI_DLLCR_SLV_EN |
	    QSPI_DLLCR_SLV_AUTO_UPDT_EN |
	    QSPI_DLLCR_DLLRES_N(8) |
	    QSPI_DLLCR_DLL_REFCNTR_N(2) |
	    QSPI_DLLCR_FREQEN_EN |
	    QSPI_DLLCR_DLLEN_EN,
	.sfacr = QSPI_SFACR_BSWAP_EN,
	.smpr = QSPI_SMPR_DLLFSMPFA_NTH(4) |
		QSPI_SMPR_DLLFSMPFB_NTH(4),
	.dlcr = QSPI_DLCR_RESERVED_MASK |
	    QSPI_DLCR_DLP_SEL_FA(1) |
	    QSPI_DLCR_DLP_SEL_FB(1),
	.flash1_size = 0x20000000,
	.flash2_size = 0x20000000,
	.dlpr = QSPI_DLPR_RESET_VALUE,
};

static int enable_ddr(struct fsl_qspi_priv *priv)
{
	struct fsl_qspi_regs *regs = priv->regs;
	u32 mcr;
	int ret;

	if (priv->ddr_mode)
		return 0;

#ifdef CONFIG_SPI_FLASH_MACRONIX
	if (memory_enable_ddr(priv)) {
		printf("Error: Failed to enable OPI DDR mode\n");
		return -1;
	}
#endif

	while (qspi_read32(priv->flags, &regs->sr) & QSPI_SR_BUSY_MASK)
		;

	/* Disable the module */
	mcr = qspi_read32(priv->flags, &regs->mcr);
	mcr |= QSPI_MCR_MDIS_MASK;
	qspi_write32(priv->flags, &regs->mcr, mcr);

	mcr |= ddr_config.mcr;
	qspi_write32(priv->flags, &regs->mcr, mcr);

	qspi_write32(priv->flags, &regs->flshcr, ddr_config.flshcr);
	qspi_write32(priv->flags, &regs->sfacr, ddr_config.sfacr);
	qspi_write32(priv->flags, &regs->smpr, ddr_config.smpr);
	qspi_write32(priv->flags, &regs->dlcr, ddr_config.dlcr);

	qspi_write32(priv->flags, &regs->dlpr, ddr_config.dlpr);
	qspi_write32(priv->flags, &regs->rbct, 0x0);

	/* Init AHB interface - 1024 bytes */
	qspi_write32(priv->flags, &regs->buf3cr, QSPI_BUF3CR_ALLMST_MASK |
		     (0x80 << QSPI_BUF3CR_ADATSZ_SHIFT));

	/* We only use the buffer3 */
	qspi_write32(priv->flags, &regs->buf0ind, 0);
	qspi_write32(priv->flags, &regs->buf1ind, 0);
	qspi_write32(priv->flags, &regs->buf2ind, 0);

	qspi_write32(priv->flags, &regs->sfa1ad, ddr_config.flash1_size);
	qspi_write32(priv->flags, &regs->sfa2ad, ddr_config.flash2_size);
	qspi_write32(priv->flags, &regs->sfb1ad, ddr_config.flash1_size);
	qspi_write32(priv->flags, &regs->sfb2ad, ddr_config.flash2_size);

	/* Enable the module */
	mcr = qspi_read32(priv->flags, &regs->mcr);
	mcr &= ~QSPI_MCR_MDIS_MASK;
	qspi_write32(priv->flags, &regs->mcr, mcr);

#if defined(CONFIG_TARGET_S32G274AEVB) || defined(CONFIG_TARGET_S32G274ARDB)
	if (is_s32gen1_soc_rev1())
		ddr_config.dllcr &= ~QSPI_DLLCR_FREQEN_EN;
#endif
#if defined(CONFIG_TARGET_S32R45EVB)
	ddr_config.dllcr &= ~QSPI_DLLCR_FREQEN_EN;
#endif

	ret = program_dllcra(priv, ddr_config.dllcr);
	if (ret) {
		printf("Error: Failed to apply dllcra settings\n");
		return ret;
	}

	priv->ddr_mode = true;
	priv->num_pads = 8;

	return 0;
}

#ifdef CONFIG_SPI_FLASH_MACRONIX
static int memory_reset(struct fsl_qspi_priv *priv)
{
	struct fsl_qspi_regs *regs = priv->regs;
	u8 rsten_cfg, rst_cfg;
	u32 mcr2;

	struct qspi_op ops[] = {
		{
		 .op = &rsten_ddr_op,
		 .cfg = &rsten_cfg,
		 },
		{
		 .op = &rst_ddr_op,
		 .cfg = &rst_cfg,
		 },
	};

	rsten_ddr_op.cmd.buswidth = priv->num_pads;
	rst_ddr_op.cmd.buswidth = priv->num_pads;

	mcr2 = qspi_read32(priv->flags, &regs->mcr);
	qspi_write32(priv->flags, &regs->mcr, mcr2 & ~QSPI_MCR_MDIS_MASK);

	if (!enable_operators(priv, ops, ARRAY_SIZE(ops)))
		return -1;

	if (qspi_write_reg(priv, &rsten_ddr_op, rsten_cfg))
		return -1;

	if (qspi_write_reg(priv, &rst_ddr_op, rst_cfg))
		return -1;

	/* Reset recovery time after a read operation */
	udelay(40);
	disable_operators(ops, ARRAY_SIZE(ops));

	return 0;
}

void reset_bootrom_settings(struct fsl_qspi_priv *priv)
{
	struct fsl_qspi_regs *regs = priv->regs;
	u32 bfgencr, lutid;
	u32 lut;
	u32 instr0;
	u32 *lutaddr;

	/* Read the configuration left by BootROM */

	bfgencr = qspi_read32(priv->flags, &regs->bfgencr);
	lutid = (bfgencr & QSPI_BFGENCR_SEQID_MASK) >> QSPI_BFGENCR_SEQID_SHIFT;

	lutaddr = get_lut_seq_start(regs, lutid);
	lut = qspi_read32(priv->flags, lutaddr);

	/* Not configured */
	if (!lut)
		return;

	priv->num_pads = (1 << LUT2PAD0(lut));
	instr0 = LUT2INSTR0(lut);

	if (instr0 == LUT_CMD_DDR)
		priv->ddr_mode = true;
	else
		priv->ddr_mode = false;

	memory_reset(priv);
}
#endif

int enable_spi(struct fsl_qspi_priv *priv, bool force)
{
	struct fsl_qspi_regs *regs = priv->regs;
	u32 mcr;

	if (!priv->ddr_mode && !force)
		return 0;

#ifdef CONFIG_SPI_FLASH_MACRONIX
	if (priv->ddr_mode) {
		if (memory_reset(priv))
			return -1;
	}
#endif

	/* Controller isn't busy */
	while (qspi_read32(priv->flags, &regs->sr) & QSPI_SR_BUSY_MASK)
		;

	/* Disable the module */
	mcr = qspi_read32(priv->flags, &regs->mcr);
	mcr |= QSPI_MCR_MDIS_MASK;
	qspi_write32(priv->flags, &regs->mcr, mcr);

	mcr = QSPI_MCR_DQS_EN |
	    QSPI_MCR_MDIS_MASK |
	    QSPI_MCR_ISD2FA_EN |
	    QSPI_MCR_ISD3FA_EN |
	    QSPI_MCR_ISD2FB_EN | QSPI_MCR_ISD3FB_EN | QSPI_MCR_END_CFD_MASK;

	qspi_write32(priv->flags, &regs->mcr, mcr);

	mcr |= QSPI_MCR_DQS_PAD_LOOPBACK;
	qspi_write32(priv->flags, &regs->mcr, mcr);

	qspi_write32(priv->flags, &regs->flshcr,
		     QSPI_FLSHCR_TDH(0) |
		     QSPI_FLSHCR_TCHS(3) | QSPI_FLSHCR_TCSS(3));

	qspi_write32(priv->flags, &regs->smpr, QSPI_SMPR_DLLFSMPFA_NTH(0) |
		     QSPI_SMPR_FSPHS_MASK);

	qspi_write32(priv->flags, &priv->regs->sfacr, 0x0);

	mcr = qspi_read32(priv->flags, &regs->mcr);
	mcr &= ~QSPI_MCR_DLPEN_MASK;

	mcr &= ~QSPI_MCR_MDIS_MASK;
	qspi_write32(priv->flags, &regs->mcr, mcr);

	program_dllcra(priv, QSPI_DLLCR_SLV_BYPASS_EN | QSPI_DLLCR_SLV_EN);

	priv->ddr_mode = false;
	priv->num_pads = 1;

	return 0;
}

static int qspi_read_mem(struct fsl_qspi_priv *priv,
			 const struct spi_mem_op *op, u8 lut_cfg)
{
	unsigned long us_before, us_after, us;
	struct fsl_qspi_regs *regs = priv->regs;
	u32 mcr_reg;

	while (qspi_read32(priv->flags, &regs->sr) & QSPI_SR_BUSY_MASK)
		;
	mcr_reg = clear_fifos(priv);
	qspi_write32(priv->flags, &regs->bfgencr,
		     lut_cfg << QSPI_BFGENCR_SEQID_SHIFT);

	invalidate_dcache_range(op->addr.val, op->addr.val + op->data.nbytes);

	/* Read out the data directly from the AHB buffer. */
	us_before = timer_get_boot_us();
	memcpy_fromio(op->data.buf.in, (void *)(uintptr_t)op->addr.val,
		      op->data.nbytes);
	us_after = timer_get_boot_us();
	us = us_after - us_before;
	printf("%u bytes read in %lu us", op->data.nbytes, us);
	if (us > 0) {
		puts(" (");
		print_size((double)op->data.nbytes / us * 1000000, "/s");
		puts(")");
	}
	puts("\n");

	qspi_write32(priv->flags, &regs->mcr, mcr_reg);
	return 0;
}

static void qspi_invalidate_ahb(struct fsl_qspi_priv *priv)
{
	struct fsl_qspi_regs *regs = priv->regs;
	u32 mcr;
	u32 reset_mask = QSPI_MCR_SWRSTHD_MASK | QSPI_MCR_SWRSTSD_MASK;

	mcr = qspi_read32(priv->flags, &regs->mcr);

	mcr &= ~QSPI_MCR_MDIS_MASK;
	qspi_write32(priv->flags, &regs->mcr, mcr);

	mcr = qspi_read32(priv->flags, &regs->mcr);
	mcr |= reset_mask;
	qspi_write32(priv->flags, &regs->mcr, mcr);

	mcr = qspi_read32(priv->flags, &regs->mcr) | QSPI_MCR_MDIS_MASK;
	qspi_write32(priv->flags, &regs->mcr, mcr);

	mcr &= ~(reset_mask);
	qspi_write32(priv->flags, &regs->mcr, mcr);

	mcr = qspi_read32(priv->flags, &regs->mcr);
	mcr &= ~QSPI_MCR_MDIS_MASK;
	qspi_write32(priv->flags, &regs->mcr, mcr);
}

static int s32gen1_exec_op(struct spi_slave *slave, const struct spi_mem_op *op)
{
	struct fsl_qspi_priv *priv;
	struct udevice *bus;
	u8 lut_cfg = lut_configs[op->cmd.opcode].index;
	bool enabled = lut_configs[op->cmd.opcode].enabled;
	int ret;

	if (!enabled) {
		printf("ERROR: Configuration for operator 0x%x isn't enabled\n",
		       op->cmd.opcode);
		return -1;
	}

	if (lut_cfg == LUT_INVALID_INDEX) {
		printf("ERROR: Invalid configuration index for operator 0x%x\n",
		       op->cmd.opcode);
		return -1;
	}

	bus = slave->dev->parent;
	priv = dev_get_priv(bus);

	/* Register and memory write */
	if (op->data.dir == SPI_MEM_DATA_OUT) {
		priv->flags &= ~QSPI_FLAG_PREV_READ_MEM;
		return qspi_write_reg(priv, op, lut_cfg);
	}

	/* Memory operation */
	if (op->memop) {
		if (!(priv->flags & QSPI_FLAG_PREV_READ_MEM))
			qspi_invalidate_ahb(priv);

		priv->flags |= QSPI_FLAG_PREV_READ_MEM;
		ret = qspi_read_mem(priv, op, lut_cfg);
		/*
		 * On S32R45EVB platform, the Macronix Flash memory
		 * does not have the 'RESET_B' (functional reset) signal wired,
		 * but only POR (power on reset).
		 * Therefore, in order to prevent an improper state on the
		 * Macronix Flash after any functional reset, we enter SPI MODE
		 * after any DTR-OPI read operation.
		 */
#if defined(CONFIG_TARGET_S32R45EVB)
		enable_spi(priv, true);
#endif
		return ret;
	}

	priv->flags &= ~QSPI_FLAG_PREV_READ_MEM;

	/* Read Register */
	return qspi_read_reg(priv, op, lut_cfg);
}

static bool s32gen1_supports_op(struct spi_slave *slave,
				const struct spi_mem_op *op)
{
	struct udevice *bus;
	struct fsl_qspi_priv *priv;

	bus = slave->dev->parent;
	priv = dev_get_priv(bus);

	/* Enable DTR for 8D-8D-8D mode only */
	if (op->cmd.buswidth == 8 && op->addr.buswidth == 8 &&
	    op->dummy.buswidth == 8 && op->data.buswidth == 8) {
		if (enable_ddr(priv))
			return -1;
	} else {
		if (enable_spi(priv, false))
			return -1;
	}

	return enable_op(priv, op);
}

struct spi_controller_mem_ops s32gen1_mem_ops = {
	.adjust_op_size = s32gen1_adjust_op_size,
	.supports_op = s32gen1_supports_op,
	.exec_op = s32gen1_exec_op,
};
