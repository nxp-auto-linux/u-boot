// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021-2022 NXP
 */

#include "fsl_qspi.h"
#include <linux/mtd/spi-nor.h>
#include <spi-mem.h>

#define MT35XU512ABA_ID	0x2c5b1a
#define MX25UW51245G_ID 0xc2813a

#define SPI_NOR_MAX_ID_LEN        6
#define SPI_NOR_MIN_ID_LEN        3

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

static struct spi_mem_op read_id_op =
SPI_MEM_OP(SPI_MEM_OP_CMD(SPINOR_OP_RDID, 1),
	   SPI_MEM_OP_NO_ADDR,
	   SPI_MEM_OP_NO_DUMMY,
	   SPI_MEM_OP_DATA_IN(SPI_NOR_MAX_ID_LEN, NULL, 1));

int s32cc_mem_reset(struct fsl_qspi_priv *priv)
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

	if (!s32cc_enable_operators(priv, ops, ARRAY_SIZE(ops)))
		return -1;

	if (s32cc_mem_exec_write_op(priv, &rsten_ddr_op, rsten_cfg))
		return -1;

	if (s32cc_mem_exec_write_op(priv, &rst_ddr_op, rst_cfg))
		return -1;

	/* Reset recovery time after a read operation */
	udelay(40);
	s32cc_disable_operators(ops, ARRAY_SIZE(ops));

	return 0;
}

void s32cc_reset_bootrom_settings(struct fsl_qspi_priv *priv)
{
	struct fsl_qspi_regs *regs = priv->regs;
	u32 bfgencr, lutid;
	u32 lut;
	u32 instr0;
	u32 *lutaddr;

	/* Read the configuration left by BootROM */

	bfgencr = qspi_read32(priv->flags, &regs->bfgencr);
	lutid = (bfgencr & QSPI_BFGENCR_SEQID_MASK) >> QSPI_BFGENCR_SEQID_SHIFT;

	lutaddr = s32cc_get_lut_seq_start(regs, lutid);
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

	s32cc_mem_reset(priv);
}

int s32cc_mem_enable_ddr(struct fsl_qspi_priv *priv,
			 struct qspi_config *ddr_config)
{
		struct fsl_qspi_regs *regs = priv->regs;
	u8 id[SPI_NOR_MAX_ID_LEN];
	u8 read_id_cfg = 0x0;
	u64 jedec_id = 0;
	u32 mcr2;
	u8 byte;
	int i;

	read_id_op.data.buf.in = id;

	struct qspi_op ops[] = {
		{
		 .op = &read_id_op,
		 .cfg = &read_id_cfg,
		 },
	};

	while (qspi_read32(priv->flags, &regs->sr) & QSPI_SR_BUSY_MASK)
		;

	if (!s32cc_enable_operators(priv, ops, ARRAY_SIZE(ops)))
		return -1;

	mcr2 = qspi_read32(priv->flags, &regs->mcr);

	/* Enable the module */
	qspi_write32(priv->flags, &regs->mcr, mcr2 & ~QSPI_MCR_MDIS_MASK);

	if (s32cc_mem_exec_read_op(priv, &read_id_op, read_id_cfg))
		return -1;

	s32cc_disable_operators(ops, ARRAY_SIZE(ops));

	for (i = 0; i < SPI_NOR_MIN_ID_LEN; i++) {
		byte = (SPI_NOR_MIN_ID_LEN - 1) - i;
		jedec_id |= id[byte] << (8 * i);
	}

	switch (jedec_id) {
	case MX25UW51245G_ID:
		macronix_get_ddr_config(ddr_config);
		return macronix_mem_enable_ddr(priv);
	case MT35XU512ABA_ID:
		micron_get_ddr_config(ddr_config);
		return micron_mem_enable_ddr(priv);
	default:
		printf("Unknown memory found 0x%llx\n", jedec_id);
	}

	return -EINVAL;
}

