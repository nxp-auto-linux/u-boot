/*
 * Copyright 2009-2012 Freescale Semiconductor, Inc.
 *
 *
 * Description:
 * MPC5125 Nand driver.
 * Port to m54418twr/Vybrid board.
 *
 * Based on original driver mpc5121_nfc.c.
 *
 * This is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <common.h>
#include <malloc.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/compat.h>

#include <mtd/fsl_nfc.h>

#include <nand.h>
#include <errno.h>
#include <asm/io.h>
#include <asm/processor.h>

#ifdef CONFIG_COLDFIRE
#include <asm/immap.h>
#endif

#define	DRV_NAME		"fsl_nfc"
#define	DRV_VERSION		"0.5"

/* Timeouts */
#define NFC_RESET_TIMEOUT	1000	/* 1 ms */
#define NFC_TIMEOUT		5000	/* 1/10 s */
#define ECC_SRAM_ADDR		(0x840 >> 3)
#define ECC_STATUS_MASK	0x80
#define ECC_ERR_COUNT	0x3F

/*#define CONFIG_MTD_NAND_FSL_NFC_SWECC*/

#ifdef CONFIG_MTD_NAND_FSL_NFC_SWECC
static int hardware_ecc;
#else
static int hardware_ecc = 1;
#endif

struct fsl_nfc_prv {
	struct mtd_info		mtd;
	struct nand_chip	chip;
	int			irq;
	void __iomem		*regs;
	struct clk		*clk;
	uint			column;
	int			spareonly;
	u8			*testbuf;
	int			pg_boot;
	int			page;
};

int fsl_nfc_chip;

static int get_status;
static int get_id;

static u8 bbt_pattern[] = {'B', 'b', 't', '0' };
static u8 mirror_pattern[] = {'1', 't', 'b', 'B' };

static struct nand_bbt_descr bbt_main_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
		   NAND_BBT_2BIT | NAND_BBT_VERSION,
	.offs =	11,
	.len = 4,
	.veroffs = 15,
	.maxblocks = 4,
	.pattern = bbt_pattern,
};

static struct nand_bbt_descr bbt_mirror_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
		   NAND_BBT_2BIT | NAND_BBT_VERSION,
	.offs =	11,
	.len = 4,
	.veroffs = 15,
	.maxblocks = 4,
	.pattern = mirror_pattern,
};

static struct nand_ecclayout fsl_nfc_ecc45 = {
	.eccbytes = 45,
	.eccpos = {19, 20, 21, 22, 23,
		   24, 25, 26, 27, 28, 29, 30, 31,
		   32, 33, 34, 35, 36, 37, 38, 39,
		   40, 41, 42, 43, 44, 45, 46, 47,
		   48, 49, 50, 51, 52, 53, 54, 55,
		   56, 57, 58, 59, 60, 61, 62, 63},
	.oobfree = {
		{.offset = 8,
		.length = 11} }
};

static inline u32 nfc_read(struct mtd_info *mtd, uint reg)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_nfc_prv *prv = chip->priv;
#ifdef CONFIG_COLDFIRE
	return in_be32(prv->regs + reg);
#else
	return in_le32(prv->regs + reg);
#endif
}

/* Write NFC register */
static inline void nfc_write(struct mtd_info *mtd, uint reg, u32 val)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_nfc_prv *prv = chip->priv;
#ifdef CONFIG_COLDFIRE
	out_be32(prv->regs + reg, val);
#else
	out_le32(prv->regs + reg, val);
#endif
}

/* Set bits in NFC register */
static inline void nfc_set(struct mtd_info *mtd, uint reg, u32 bits)
{
	nfc_write(mtd, reg, nfc_read(mtd, reg) | bits);
}

/* Clear bits in NFC register */
static inline void nfc_clear(struct mtd_info *mtd, uint reg, u32 bits)
{
	nfc_write(mtd, reg, nfc_read(mtd, reg) & ~bits);
}

static inline void
nfc_set_field(struct mtd_info *mtd, u32 reg, u32 mask, u32 shift, u32 val)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_nfc_prv *prv = chip->priv;
#ifdef CONFIG_COLDFIRE
	out_be32(prv->regs + reg,
			(in_be32(prv->regs + reg) & (~mask))
			| val << shift);
#else
	out_le32(prv->regs + reg,
			(in_le32(prv->regs + reg) & (~mask))
			| val << shift);
#endif
}

static inline int
nfc_get_field(struct mtd_info *mtd, u32 reg, u32 field_mask)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_nfc_prv *prv = chip->priv;
#ifdef CONFIG_COLDFIRE
	return in_be32(prv->regs + reg) & field_mask;
#else
	return in_le32(prv->regs + reg) & field_mask;
#endif
}

static inline u8 nfc_check_status(struct mtd_info *mtd)
{
	u8 fls_status = 0;
	fls_status = nfc_get_field(mtd, NFC_FLASH_STATUS2, STATUS_BYTE1_MASK);
	return fls_status;
}

/* clear cmd_done and cmd_idle falg for the coming command */
static void fsl_nfc_clear(struct mtd_info *mtd)
{
	nfc_write(mtd, NFC_IRQ_STATUS, 1 << CMD_DONE_CLEAR_SHIFT);
	nfc_write(mtd, NFC_IRQ_STATUS, 1 << IDLE_CLEAR_SHIFT);
}

/* Wait for operation complete */
static void fsl_nfc_done(struct mtd_info *mtd)
{
	uint start = 0;

	nfc_set_field(mtd, NFC_FLASH_CMD2, START_MASK,
			START_SHIFT, 1);

	start = get_timer(0);

	while (!nfc_get_field(mtd, NFC_IRQ_STATUS, IDLE_IRQ_MASK)) {
		if (get_timer(start) > NFC_TIMEOUT)
			printf("Timeout while waiting for BUSY.\n");
	}
	fsl_nfc_clear(mtd);
}

static u8 fsl_nfc_get_id(struct mtd_info *mtd, int col)
{
	u32 flash_id1 = 0;
	u8 *pid;

	flash_id1 = nfc_read(mtd, NFC_FLASH_STATUS1);
	pid = (u8 *)&flash_id1;
#ifdef CONFIG_COLDFIRE
	return *(pid + col);
#else
	return *(pid + 3 - col);
#endif
}

static inline u8 fsl_nfc_get_status(struct mtd_info *mtd)
{
	u32 flash_status = 0;
	u8 *pstatus;

	flash_status = nfc_read(mtd, NFC_FLASH_STATUS2);
	pstatus = (u8 *)&flash_status;
#ifdef CONFIG_COLDFIRE
	return *(pstatus + 3);
#else
	return *(pstatus);
#endif
}

/* Invoke command cycle */
static inline void
fsl_nfc_send_cmd(struct mtd_info *mtd, u32 cmd_byte1,
		u32 cmd_byte2, u32 cmd_code)
{
	fsl_nfc_clear(mtd);
	nfc_set_field(mtd, NFC_FLASH_CMD2, CMD_BYTE1_MASK,
			CMD_BYTE1_SHIFT, cmd_byte1);

	nfc_set_field(mtd, NFC_FLASH_CMD1, CMD_BYTE2_MASK,
			CMD_BYTE2_SHIFT, cmd_byte2);

	nfc_set_field(mtd, NFC_FLASH_CMD2, BUFNO_MASK,
			BUFNO_SHIFT, 0);

	nfc_set_field(mtd, NFC_FLASH_CMD2, CMD_CODE_MASK,
			CMD_CODE_SHIFT, cmd_code);

	if (cmd_code == RANDOM_OUT_CMD_CODE)
		nfc_set_field(mtd, NFC_FLASH_CMD2, BUFNO_MASK,
			BUFNO_SHIFT, 1);
}

/* Receive ID and status from NAND flash */
static inline void
fsl_nfc_send_one_byte(struct mtd_info *mtd, u32 cmd_byte1, u32 cmd_code)
{
	fsl_nfc_clear(mtd);

	nfc_set_field(mtd, NFC_FLASH_CMD2, CMD_BYTE1_MASK,
			CMD_BYTE1_SHIFT, cmd_byte1);

	nfc_set_field(mtd, NFC_FLASH_CMD2, BUFNO_MASK,
			BUFNO_SHIFT, 0);

	nfc_set_field(mtd, NFC_FLASH_CMD2, CMD_CODE_MASK,
			CMD_CODE_SHIFT, cmd_code);
}


/* Do address cycle(s) */
static void
fsl_nfc_addr_cycle(struct mtd_info *mtd, int column, int page)
{

	if (column != -1) {
		nfc_set_field(mtd, NFC_COL_ADDR,
				COL_ADDR_MASK,
				COL_ADDR_SHIFT, column);
	}

	if (page != -1) {
		nfc_set_field(mtd, NFC_ROW_ADDR,
				ROW_ADDR_MASK,
				ROW_ADDR_SHIFT, page);
	}

	/* DMA Disable */
	nfc_clear(mtd, NFC_FLASH_CONFIG, CONFIG_DMA_REQ_MASK);

	/* PAGE_CNT = 1 */
	nfc_set_field(mtd, NFC_FLASH_CONFIG, CONFIG_PAGE_CNT_MASK,
			CONFIG_PAGE_CNT_SHIFT, 0x1);
}


/* Control chips select signal on the board */
static void
nfc_select_chip(struct mtd_info *mtd, int chip)
{
#ifdef CONFIG_COLDFIRE
	volatile gpio_t *gpio = (gpio_t *) MMAP_GPIO;

	if (chip < 0) {
		gpio->par_fbctl &= (GPIO_PAR_FBCTL_ALE_MASK &
				   GPIO_PAR_FBCTL_TA_MASK);
		gpio->par_fbctl |= GPIO_PAR_FBCTL_ALE_FB_TS |
				   GPIO_PAR_FBCTL_TA_TA;
		gpio->par_be =
		    GPIO_PAR_BE_BE3_BE3 | GPIO_PAR_BE_BE2_BE2 |
		    GPIO_PAR_BE_BE1_BE1 | GPIO_PAR_BE_BE0_BE0;
		gpio->par_cs &= ~GPIO_PAR_CS_CS1_NFC_CE;
		gpio->par_cs = GPIO_PAR_CS_CS0_CS0;
		return;
	}

	gpio->par_fbctl &= (GPIO_PAR_FBCTL_ALE_MASK & GPIO_PAR_FBCTL_TA_MASK);
	gpio->par_fbctl |= GPIO_PAR_FBCTL_ALE_FB_ALE |
			   GPIO_PAR_FBCTL_TA_NFC_RB;
	gpio->par_be =
	    GPIO_PAR_BE_BE3_FB_A1 | GPIO_PAR_BE_BE2_FB_A0 |
	    GPIO_PAR_BE_BE1_BE1 | GPIO_PAR_BE_BE0_BE0;
	gpio->par_cs &= (GPIO_PAR_BE_BE3_MASK & GPIO_PAR_BE_BE2_MASK);
	gpio->par_cs = GPIO_PAR_CS_CS1_NFC_CE;
#endif
}

void board_nand_select_device(struct nand_chip *nand, int chip)
{
	fsl_nfc_chip = chip;
}

/* Read NAND Ready/Busy signal */
static int
fsl_nfc_dev_ready(struct mtd_info *mtd)
{
	/*
	 * NFC handles ready/busy signal internally. Therefore, this function
	 * always returns status as ready.
	 */
	return 1;
}

/* Write command to NAND flash */
static void
fsl_nfc_command(struct mtd_info *mtd, unsigned command,
					int column, int page)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_nfc_prv *prv = chip->priv;

	prv->column = (column >= 0) ? column : 0;
	prv->spareonly = 0;
	get_id = 0;
	get_status = 0;

	if (page != -1)
		prv->page = page;

	if (!prv->pg_boot) {

		if (hardware_ecc)
			nfc_set_field(mtd, NFC_FLASH_CONFIG,
				CONFIG_ECC_MODE_MASK,
				CONFIG_ECC_MODE_SHIFT, ECC_45_BYTE);
		else
			/* set ECC BY_PASS */
			nfc_set_field(mtd, NFC_FLASH_CONFIG,
				CONFIG_ECC_MODE_MASK,
				CONFIG_ECC_MODE_SHIFT, ECC_BYPASS);

		if (!(page%0x40))
			nfc_set_field(mtd, NFC_FLASH_CONFIG,
				CONFIG_ECC_MODE_MASK,
				CONFIG_ECC_MODE_SHIFT, ECC_BYPASS);
	}

	switch (command) {
	case NAND_CMD_PAGEPROG:
		if (!(prv->page%0x40) && !prv->pg_boot)
			nfc_set_field(mtd, NFC_FLASH_CONFIG,
				CONFIG_ECC_MODE_MASK,
				CONFIG_ECC_MODE_SHIFT, ECC_BYPASS);

		fsl_nfc_send_cmd(mtd,
				PROGRAM_PAGE_CMD_BYTE1,
				PROGRAM_PAGE_CMD_BYTE2,
				PROGRAM_PAGE_CMD_CODE);
		break;
	/*
	 * NFC does not support sub-page reads and writes,
	 * so emulate them using full page transfers.
	 */
	case NAND_CMD_READ0:
		column = 0;
		goto read0;
		break;

	case NAND_CMD_READ1:
		prv->column += 256;
		command = NAND_CMD_READ0;
		column = 0;
		goto read0;
		break;

	case NAND_CMD_READOOB:
		prv->spareonly = 1;
		command = NAND_CMD_READ0;
		column = 0;
read0:
		fsl_nfc_send_cmd(mtd,
				PAGE_READ_CMD_BYTE1,
				PAGE_READ_CMD_BYTE2,
				READ_PAGE_CMD_CODE);
		break;

	case NAND_CMD_SEQIN:
		fsl_nfc_command(mtd, NAND_CMD_READ0, column, page);
		column = 0;
		break;

	case NAND_CMD_ERASE1:
		fsl_nfc_send_cmd(mtd,
				ERASE_CMD_BYTE1,
				ERASE_CMD_BYTE2,
				ERASE_CMD_CODE);
		break;
	case NAND_CMD_ERASE2:
		return;
	case NAND_CMD_READID:
		get_id = 1;
		fsl_nfc_send_one_byte(mtd, command, READ_ID_CMD_CODE);
		break;
	case NAND_CMD_STATUS:
		get_status = 1;
		fsl_nfc_send_one_byte(mtd, command, STATUS_READ_CMD_CODE);
		break;
	case NAND_CMD_RNDOUT:
		fsl_nfc_send_cmd(mtd,
				RANDOM_OUT_CMD_BYTE1,
				RANDOM_OUT_CMD_BYTE2,
				RANDOM_OUT_CMD_CODE);
		break;
	case NAND_CMD_RESET:
		fsl_nfc_send_one_byte(mtd, command, RESET_CMD_CODE);
		break;
	default:
		return;
	}

	fsl_nfc_addr_cycle(mtd, column, page);

	fsl_nfc_done(mtd);
}

/* Copy data from/to NFC spare buffers. */
static void
fsl_nfc_copy_spare(struct mtd_info *mtd, uint offset,
			u8 *buffer, uint size, int wr)
{
	struct nand_chip *nand = mtd->priv;
	struct fsl_nfc_prv *prv = nand->priv;
	uint o, s, sbsize, blksize;

	/*
	 * NAND spare area is available through NFC spare buffers.
	 * The NFC divides spare area into (page_size / 512) chunks.
	 * Each chunk is placed into separate spare memory area, using
	 * first (spare_size / num_of_chunks) bytes of the buffer.
	 *
	 * For NAND device in which the spare area is not divided fully
	 * by the number of chunks, number of used bytes in each spare
	 * buffer is rounded down to the nearest even number of bytes,
	 * and all remaining bytes are added to the last used spare area.
	 *
	 * For more information read section 26.6.10 of MPC5121e
	 * Microcontroller Reference Manual, Rev. 3.
	 */

	/* Calculate number of valid bytes in each spare buffer */
/*	sbsize = (mtd->oobsize / (mtd->writesize / 512)) & ~1;*/
	sbsize = (mtd->oobsize / (mtd->writesize / 2048)) & ~1;


	while (size) {
		/* Calculate spare buffer number */
		s = offset / sbsize;
		if (s > NFC_SPARE_BUFFERS - 1)
			s = NFC_SPARE_BUFFERS - 1;

		/*
		 * Calculate offset to requested data block in selected spare
		 * buffer and its size.
		 */
		o = offset - (s * sbsize);
		blksize = min(sbsize - o, size);

		if (wr)
			memcpy(prv->regs + NFC_SPARE_AREA(s) + o,
							buffer, blksize);
		else {
			memcpy(buffer,
				prv->regs + NFC_SPARE_AREA(s) + o, blksize);
		}

		buffer += blksize;
		offset += blksize;
		size -= blksize;
	};
}

/* Copy data from/to NFC main and spare buffers */
static void
fsl_nfc_buf_copy(struct mtd_info *mtd, u_char *buf, int len, int wr)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_nfc_prv *prv = chip->priv;
	uint c = prv->column;
	uint l;

	/* Handle spare area access */
	if (prv->spareonly || c >= mtd->writesize) {
		/* Calculate offset from beginning of spare area */
		if (c >= mtd->writesize)
			c -= mtd->writesize;

		prv->column += len;
		fsl_nfc_copy_spare(mtd, c, buf, len, wr);
		return;
	}

	/*
	 * Handle main area access - limit copy length to prevent
	 * crossing main/spare boundary.
	 */
	l = min((uint)len, mtd->writesize - c);
	prv->column += l;

	if (wr)
		memcpy(prv->regs + NFC_MAIN_AREA(0) + c, buf, l);
	else {
		if (get_status) {
			get_status = 0;
			*buf = fsl_nfc_get_status(mtd);
		} else if (l == 1 && c <= 3 && get_id) {
			*buf = fsl_nfc_get_id(mtd, c);
		} else
			memcpy(buf, prv->regs + NFC_MAIN_AREA(0) + c, l);
	}

	/* Handle crossing main/spare boundary */
	if (l != len) {
		buf += l;
		len -= l;
		fsl_nfc_buf_copy(mtd, buf, len, wr);
	}
}

/* Read data from NFC buffers */
static void
fsl_nfc_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	fsl_nfc_buf_copy(mtd, buf, len, 0);
}

/* Write data to NFC buffers */
static void
fsl_nfc_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	fsl_nfc_buf_copy(mtd, (u_char *)buf, len, 1);
}

/* Compare buffer with NAND flash */
static int
fsl_nfc_verify_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	u_char tmp[256];
	uint bsize;

	while (len) {
		bsize = min(len, 256);
		fsl_nfc_read_buf(mtd, tmp, bsize);

		if (memcmp(buf, tmp, bsize))
			return 1;

		buf += bsize;
		len -= bsize;
	}

	return 0;
}

/* Read byte from NFC buffers */
static u8
fsl_nfc_read_byte(struct mtd_info *mtd)
{
	u8 tmp;
	fsl_nfc_read_buf(mtd, &tmp, sizeof(tmp));
	return tmp;
}

/* Read word from NFC buffers */
static u16
fsl_nfc_read_word(struct mtd_info *mtd)
{
	u16 tmp;
	fsl_nfc_read_buf(mtd, (u_char *)&tmp, sizeof(tmp));
	return tmp;
}

static void
copy_from_to_spare(struct mtd_info *mtd, void *pbuf, int len, int wr)
{
	struct nand_chip *chip = mtd->priv;
	struct fsl_nfc_prv *prv = chip->priv;
	int i = 0, copy_count, copy_size;

	copy_count = mtd->writesize / 2048;
	/*
	 * Each spare area has 16 bytes for 512, 2K and normal 4K nand.
	 * For 4K nand with large 218 byte spare size, the size is 26 bytes for
	 * the first 7 buffers and 36 for the last.
	 */
	copy_size = 64;
	/*
	 * Each spare area has 16 bytes for 512, 2K and normal 4K nand.
	 * For 4K nand with large 218 byte spare size, the size is 26
	 * bytes for the first 7 buffers and 36 for the last.
	 */
	for (i = 0; i < copy_count - 1 && len > 0; i++) {
		if (wr)
			memcpy(prv->regs + NFC_SPARE_AREA(i),
					pbuf, MIN(len, copy_size));
		else
			memcpy(pbuf, prv->regs + NFC_SPARE_AREA(i),
					MIN(len, copy_size));
		pbuf += copy_size;
		len -= copy_size;
	}
	if (len > 0) {
		if (wr)
			memcpy(prv->regs + NFC_SPARE_AREA(i),
				pbuf, len);
		else
			memcpy(pbuf,
				prv->regs + NFC_SPARE_AREA(i), len);
	}
}


static int fsl_nfc_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
				int page)
{
	fsl_nfc_command(mtd, NAND_CMD_READ0, 0, page);
	copy_from_to_spare(mtd, chip->oob_poi, mtd->oobsize, 0);
	return 0;
}

static int fsl_nfc_write_oob(struct mtd_info *mtd, struct nand_chip *chip,
					int page)
{
	fsl_nfc_command(mtd, NAND_CMD_READ0, 0, page);

	fsl_nfc_command(mtd, NAND_CMD_SEQIN, 0, page);

	/* copy the oob data */
	copy_from_to_spare(mtd, chip->oob_poi, mtd->oobsize, 1);

	fsl_nfc_command(mtd, NAND_CMD_PAGEPROG, 0, page);

	return 0;
}

static int fsl_nfc_read_page(struct mtd_info *mtd, struct nand_chip *chip,
					uint8_t *buf, int oob_required, int page)
{
	struct fsl_nfc_prv *prv = chip->priv;
	/*fsl_nfc_check_ecc_status(mtd);*/

	memcpy((void *)buf, prv->regs + NFC_MAIN_AREA(0),
			mtd->writesize);
	copy_from_to_spare(mtd, chip->oob_poi, mtd->oobsize, 0);
	return 0;
}

static int fsl_nfc_write_page(struct mtd_info *mtd,
		struct nand_chip *chip, const uint8_t *buf, int oob_required)
{
	struct fsl_nfc_prv *prv = chip->priv;

	memcpy(prv->regs + NFC_MAIN_AREA(0), buf, mtd->writesize);
	copy_from_to_spare(mtd, chip->oob_poi, mtd->oobsize, 1);
	return 0;
}

static void fsl_nfc_enable_hwecc(struct mtd_info *mtd, int mode)
{
	return;
}

int board_nand_init(struct nand_chip *chip)
{
	struct fsl_nfc_prv *prv;
	struct mtd_info *mtd;
	uint chips_no = 0;
	u8 *testbuf = NULL;

	if (chip->IO_ADDR_R == NULL)
		return -1;

	prv = malloc(sizeof(*prv));
	if (!prv) {
		printf(KERN_ERR DRV_NAME ": Memory exhausted!\n");
		return -ENOMEM;
	}

	mtd = &nand_info[chips_no++];
	mtd->priv = chip;
	chip->priv = prv;

	prv->regs = (void __iomem *)chip->IO_ADDR_R;
	prv->testbuf = testbuf;
	prv->pg_boot = 0;

	mtd->writesize = 2048;
	mtd->oobsize = 64;
	chip->dev_ready = fsl_nfc_dev_ready;
	chip->cmdfunc = fsl_nfc_command;
	chip->read_byte = fsl_nfc_read_byte;
	chip->read_word = fsl_nfc_read_word;
	chip->read_buf = fsl_nfc_read_buf;
	chip->write_buf = fsl_nfc_write_buf;
	chip->verify_buf = fsl_nfc_verify_buf;
	chip->options = NAND_BUSWIDTH_16 | NAND_CACHEPRG;
        chip->bbt_options = NAND_BBT_USE_FLASH;
	chip->select_chip = nfc_select_chip;

	if (hardware_ecc) {
		chip->ecc.read_page = fsl_nfc_read_page;
		chip->ecc.write_page = fsl_nfc_write_page;
		chip->ecc.read_oob = fsl_nfc_read_oob;
		chip->ecc.write_oob = fsl_nfc_write_oob;
		chip->ecc.layout = &fsl_nfc_ecc45;

		/* propagate ecc.layout to mtd_info */
		mtd->ecclayout = chip->ecc.layout;
		chip->ecc.calculate = NULL;
		chip->ecc.hwctl = fsl_nfc_enable_hwecc;
		chip->ecc.correct = NULL;
		chip->ecc.mode = NAND_ECC_HW;
		/* RS-ECC is applied for both MAIN+SPARE not MAIN alone */
		chip->ecc.steps = 1;
		chip->ecc.bytes = 45;
		chip->ecc.size = 0x800;
		chip->ecc.strength = 4;

		nfc_set_field(mtd, NFC_FLASH_CONFIG,
				CONFIG_ECC_MODE_MASK,
				CONFIG_ECC_MODE_SHIFT, ECC_45_BYTE);
		/* set ECC_STATUS write position */
		nfc_set_field(mtd, NFC_FLASH_CONFIG,
				CONFIG_ECC_SRAM_ADDR_MASK,
				CONFIG_ECC_SRAM_ADDR_SHIFT, ECC_SRAM_ADDR);
		/* enable ECC_STATUS results write */
		nfc_set_field(mtd, NFC_FLASH_CONFIG,
				CONFIG_ECC_SRAM_REQ_MASK,
				CONFIG_ECC_SRAM_REQ_SHIFT, 1);
	} else {
		chip->ecc.mode = NAND_ECC_SOFT;
		/* set ECC BY_PASS */

		nfc_set_field(mtd, NFC_FLASH_CONFIG,
				CONFIG_ECC_MODE_MASK,
				CONFIG_ECC_MODE_SHIFT, ECC_BYPASS);
	}
	chip->bbt_td = &bbt_main_descr;
	chip->bbt_md = &bbt_mirror_descr;
	bbt_main_descr.pattern = bbt_pattern;
	bbt_mirror_descr.pattern = mirror_pattern;

	/* SET SECTOR SIZE */
	nfc_write(mtd, NFC_SECTOR_SIZE, (PAGE_2K | PAGE_64) + 1);

	nfc_set_field(mtd, NFC_FLASH_CONFIG,
			CONFIG_ADDR_AUTO_INCR_MASK,
			CONFIG_ADDR_AUTO_INCR_SHIFT, 0);

	nfc_set_field(mtd, NFC_FLASH_CONFIG,
			CONFIG_BUFNO_AUTO_INCR_MASK,
			CONFIG_BUFNO_AUTO_INCR_SHIFT, 0);

	nfc_set_field(mtd, NFC_FLASH_CONFIG,
			CONFIG_16BIT_MASK,
			CONFIG_16BIT_SHIFT, 1);

	/* SET FAST_FLASH = 1 */
	nfc_set_field(mtd, NFC_FLASH_CONFIG,
			CONFIG_BOOT_MODE_MASK,
			CONFIG_BOOT_MODE_SHIFT, 0);

	return 0;
}

int do_nand_boot_update(cmd_tbl_t *cmdtp, int flag,
		int argc, char * const argv[])
{
	ulong mem_addr;
	size_t data_size;
	int j;
	struct mtd_info *mtd;
	u_char *addr;
	u32 saved_cfg;
	struct nand_chip *chip;
	struct fsl_nfc_prv *prv;
	nand_info_t *nand;

	if (nand_curr_device < 0 ||
		nand_curr_device >= CONFIG_SYS_MAX_NAND_DEVICE ||
		!nand_info[nand_curr_device].name) {
		puts("\nno devices available\n");
		return 1;
	}


	mtd =  &nand_info[nand_curr_device];
	nand = &nand_info[nand_curr_device];
	chip = mtd->priv;
	prv = chip->priv;

	prv->pg_boot = 1;

	if (argc < 3) {
		cmd_usage(cmdtp);
		return -1;
	}

	strict_strtoul(argv[1], 16, &mem_addr);
	strict_strtoul(argv[2], 16, &data_size);

	saved_cfg = nfc_read(mtd, NFC_FLASH_CONFIG);

	nfc_write(mtd, NFC_FLASH_CONFIG, 0x000ea671);
	memcpy((void *)CONFIG_SYS_NAND_BASE, (unsigned char *)mem_addr, 0xf80);

	nfc_write(mtd, NFC_FLASH_CMD1, 0x10000000);
	nfc_write(mtd, NFC_FLASH_CONFIG, 0x000ea631);
	nfc_write(mtd, NFC_SECTOR_SIZE, 0x0000420);
	nfc_write(mtd, NFC_FLASH_COMMAND_REPEAT, 0x0);

	/*program the first 4 pages.*/
	nfc_select_chip(mtd, 0);
	for (j = 0; j < 4; j++) {
		nfc_write(mtd, NFC_ROW_ADDR, 0x11000000 + j);
		nfc_write(mtd, NFC_COL_ADDR, 0x00);

		nfc_write(mtd, NFC_FLASH_CMD2, 0x807e0000);
		fsl_nfc_done(mtd);
		nfc_write(mtd, NFC_FLASH_CMD2, 0x8001c000 + j*2);
		fsl_nfc_done(mtd);

	}
	nfc_select_chip(mtd, -1);

	/*program the other part*/

	fsl_nfc_clear(mtd);

	nfc_write(mtd, NFC_FLASH_CONFIG, 0x000e0681);

	nfc_write(mtd, NFC_SECTOR_SIZE, (PAGE_2K | PAGE_64) + 1);

	nfc_write(mtd, NFC_FLASH_CMD2, 0x007ee000);
	nfc_write(mtd, NFC_ROW_ADDR, 0x11000000);

	addr = (u_char *)mem_addr + 0xf80;

	data_size = ((data_size - 0xf80) + 0x800) & ~(0x800 - 1) ;


	if (nand_write_skip_bad(nand, 0x2000, &data_size, NULL, nand->size, addr, 0)) {
		printf("write nand boot error!\n");
		return -1;
	}

	prv->pg_boot = 0;

	nfc_write(mtd, NFC_FLASH_CONFIG, saved_cfg);

	return 0;
}

U_BOOT_CMD(nb_update, 4, 1, do_nand_boot_update,
	   "Nand boot update  program",
	   "mem_addr size");
