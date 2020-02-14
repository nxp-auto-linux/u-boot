// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */
#include "fsl_qspi.h"
#include <linux/mtd/spi-nor.h>
#include <spi-mem.h>

typedef int (*s32gen1_exec_op_t)(struct fsl_qspi_priv *,
				 const struct spi_mem_op *);

static int s32gen1_adjust_op_size(struct spi_slave *slave,
				  struct spi_mem_op *op)
{
	return 0;
}

static int s32gen1_readid(struct fsl_qspi_priv *qspi,
			  const struct spi_mem_op *op)
{
	qspi_op_rdid(qspi, op->data.buf.in, op->data.nbytes);
	return 0;
}

static int s32gen1_read_extaddr(struct fsl_qspi_priv *qspi,
				const struct spi_mem_op *op)
{
	u8 bar = qspi->bar_addr / SZ_16M;

	*(u8 *)op->data.buf.in = bar;
	return 0;
}

static int s32gen1_write_extaddr(struct fsl_qspi_priv *qspi,
				 const struct spi_mem_op *op)
{
	u8 bar = *(u8 *)op->data.buf.out;

	qspi->bar_addr = bar * SZ_16M;
	return 0;
}

static int s32gen1_write_enable(struct fsl_qspi_priv *qspi,
				const struct spi_mem_op *op)
{
	return 0;
}

static int s32gen1_write_disable(struct fsl_qspi_priv *qspi,
				 const struct spi_mem_op *op)
{
	return 0;
}

static void update_address(struct fsl_qspi_priv *qspi,
			   const struct spi_mem_op *op)
{
	u32 address;

	address = swab32(*(u32 *)op->data.buf.out);

	if (op->data.nbytes == 3)
		address >>= 8;

	qspi->sf_addr = qspi->bar_addr + address;
}

static int s32gen1_erase_sector(struct fsl_qspi_priv *qspi,
				const struct spi_mem_op *op)
{
	update_address(qspi, op);
	qspi->cur_seqid = SPINOR_OP_SE;
	qspi_op_erase(qspi);
	qspi_ahb_invalid(qspi);
	return 0;
}

static int s32gen1_read_fast(struct fsl_qspi_priv *qspi,
			     const struct spi_mem_op *op)
{
	qspi->sf_addr = op->addr.val;
	qspi_ahb_read(qspi, op->data.buf.in,  op->data.nbytes);
	qspi_ahb_invalid(qspi);

	return 0;
}

static int s32gen1_write_page(struct fsl_qspi_priv *qspi,
			      const struct spi_mem_op *op)
{
	qspi->sf_addr = op->addr.val;
	qspi_op_write(qspi, (u8 *)op->data.buf.out,  op->data.nbytes);
	qspi_ahb_invalid(qspi);
	return 0;
}

static int s32gen1_read_status(struct fsl_qspi_priv *qspi,
			       const struct spi_mem_op *op)
{
	qspi_op_rdsr(qspi, op->data.buf.in, op->data.nbytes);
	return 0;
}

static s32gen1_exec_op_t s32gen1_ops[] = {
	[SPINOR_OP_RDID] = s32gen1_readid,
	[SPINOR_OP_RDEAR] = s32gen1_read_extaddr,
	[SPINOR_OP_WREAR] = s32gen1_write_extaddr,
	[SPINOR_OP_WREN] = s32gen1_write_enable,
	[SPINOR_OP_WRDI] = s32gen1_write_disable,
	[SPINOR_OP_SE] = s32gen1_erase_sector,
	[SPINOR_OP_RDSR] = s32gen1_read_status,
	[SPINOR_OP_READ_FAST] = s32gen1_read_fast,
	[SPINOR_OP_PP] = s32gen1_write_page,
};

static int s32gen1_exec_op(struct spi_slave *slave,
			   const struct spi_mem_op *op)
{
	struct fsl_qspi_priv *priv;
	struct udevice *bus;

	bus = slave->dev->parent;
	priv = dev_get_priv(bus);

	s32gen1_ops[op->cmd.opcode](priv, op);
	return 0;
}

static bool s32gen1_supports_op(struct spi_slave *slave,
				const struct spi_mem_op *op)
{
	if (op->cmd.opcode >= ARRAY_SIZE(s32gen1_ops))
		return false;
	if (s32gen1_ops[op->cmd.opcode])
		return true;

	return false;
}

struct spi_controller_mem_ops s32gen1_mem_ops = {
	.adjust_op_size = s32gen1_adjust_op_size,
	.supports_op = s32gen1_supports_op,
	.exec_op = s32gen1_exec_op,
};

