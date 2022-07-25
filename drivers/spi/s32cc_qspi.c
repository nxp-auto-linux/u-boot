// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2022 NXP
 */
#include <cpu_func.h>
#include <inttypes.h>
#include <spi-mem.h>
#include <asm/io.h>
#include <linux/mtd/spi-nor.h>
#include "fsl_qspi.h"

#define LUT_INVALID_INDEX -1
#define LUT_STOP_CMD 0x00
#define MAX_OPCODE 0xff

#define MAX_LUTS 80
#define LUTS_PER_CONFIG 5
#define MAX_LUTS_CONFIGS (MAX_LUTS / LUTS_PER_CONFIG)


struct lut_config {
	bool enabled;
	u32 conf[MAX_LUTS_CONFIGS];
	u8 fill;
	u8 index;
};


static u8 luts_next_config;
static struct lut_config lut_configs[MAX_OPCODE];


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

static int s32cc_adjust_op_size(struct spi_slave *slave,
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

int s32cc_mem_exec_write_op(struct fsl_qspi_priv *priv,
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

int s32cc_mem_exec_read_op(struct fsl_qspi_priv *priv,
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

u32 *s32cc_get_lut_seq_start(struct fsl_qspi_regs *regs, u32 index)
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

	lutaddr = s32cc_get_lut_seq_start(regs, index);

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

bool s32cc_enable_operators(struct fsl_qspi_priv *priv, struct qspi_op *ops,
			    size_t n_ops)

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

void s32cc_disable_operators(struct qspi_op *ops, size_t n_ops)
{
	size_t i;
	const struct spi_mem_op *op;

	for (i = 0; i < n_ops; i++) {
		op = ops[i].op;

		lut_configs[op->cmd.opcode].enabled = false;
	}
}


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


static int enable_ddr(struct fsl_qspi_priv *priv)
{
	struct fsl_qspi_regs *regs = priv->regs;
	struct qspi_config ddr_config;
	u32 mcr;
	int ret;

	if (priv->ddr_mode)
		return 0;

#if defined(CONFIG_SPI_FLASH_MACRONIX) || \
	defined(CONFIG_SPI_FLASH_STMICRO)
	if (s32cc_mem_enable_ddr(priv, &ddr_config)) {
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

	if (is_s32g3_qspi(priv))
		ddr_config.smpr = QSPI_SMPR_DLLFSMPFA_NTH(3) |
			QSPI_SMPR_DLLFSMPFB_NTH(3);

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

int s32cc_enable_spi(struct fsl_qspi_priv *priv, bool force)
{
	struct fsl_qspi_regs *regs = priv->regs;
	u32 mcr;

	if (!priv->ddr_mode && !force)
		return 0;

#if defined(CONFIG_SPI_FLASH_MACRONIX) || \
	defined(CONFIG_SPI_FLASH_STMICRO)
	if (priv->ddr_mode) {
		if (s32cc_mem_reset(priv))
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

static inline void s32cc_qspi_print_read_speed(const struct spi_mem_op *op,
					       unsigned long us_before)
{
#ifdef DEBUG
	unsigned long us_after, us;

	us_after = timer_get_boot_us();
	us = us_after - us_before;
	printf("%u bytes read in %lu us", op->data.nbytes, us);
	if (us > 0) {
		puts(" (");
		print_size((double)op->data.nbytes / us * 1000000, "/s");
		puts(")");
	}
	puts("\n");
#endif
}

static inline unsigned long s32cc_get_initial_ts(void)
{
#ifdef DEBUG
	return timer_get_boot_us();
#else
	return 0;
#endif
}

static int qspi_read_mem(struct fsl_qspi_priv *priv,
			 const struct spi_mem_op *op, u8 lut_cfg)
{
	unsigned long us_before;
	struct fsl_qspi_regs *regs = priv->regs;
	u32 mcr_reg;

	while (qspi_read32(priv->flags, &regs->sr) & QSPI_SR_BUSY_MASK)
		;
	mcr_reg = clear_fifos(priv);
	qspi_write32(priv->flags, &regs->bfgencr,
		     lut_cfg << QSPI_BFGENCR_SEQID_SHIFT);

	invalidate_dcache_range(op->addr.val, op->addr.val + op->data.nbytes);

	/* Read out the data directly from the AHB buffer. */
	us_before = s32cc_get_initial_ts();
	memcpy_fromio(op->data.buf.in, (void *)(uintptr_t)op->addr.val,
		      op->data.nbytes);
	s32cc_qspi_print_read_speed(op, us_before);

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

static int s32cc_exec_op(struct spi_slave *slave, const struct spi_mem_op *op)
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
		return s32cc_mem_exec_write_op(priv, op, lut_cfg);
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
		s32cc_enable_spi(priv, true);
#endif
		return ret;
	}

	priv->flags &= ~QSPI_FLAG_PREV_READ_MEM;

	/* Read Register */
	return s32cc_mem_exec_read_op(priv, op, lut_cfg);
}

static bool s32cc_supports_op(struct spi_slave *slave,
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
		if (s32cc_enable_spi(priv, false))
			return -1;
	}

	return enable_op(priv, op);
}

struct spi_controller_mem_ops s32cc_mem_ops = {
	.adjust_op_size = s32cc_adjust_op_size,
	.supports_op = s32cc_supports_op,
	.exec_op = s32cc_exec_op,
};
