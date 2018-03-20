/*
 * (C) Copyright 2017 MicroSys Electronics GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "s32v_ocotp.h"
#include <asm/io.h>
#include <asm/errno.h>

#define OCOTP_TIMEOUT 100000

static void clear_mask(u32 *reg, const u32 mask)
{
	u32 val = readl(reg);
	val &= ~mask;
	writel(val, reg);
}

static int s32v_ocotp_wait_busy_clear(void)
{
	uint32_t reg = 0;

	do {
		udelay(10);
		reg = readl(OCOTP_CTRL_REG);
		reg &= OCOTP_CTRL_BUSY_BIT;
	} while (reg);

	return 0;
}

static inline void s32v_ocotp_clear_error(void)
{
	clear_mask(OCOTP_CTRL_REG, OCOTP_CTRL_ERROR_BIT);
}

static void s32v_ocotp_set_addr(u8 addr)
{
	/* set address of fuse to read: */
	u32 reg = readl(OCOTP_CTRL_REG);
	reg &= ~0xff;
	reg |= (u32) addr;
	writel(reg, OCOTP_CTRL_REG);
}

static int s32v_ocotp_reload_shadows(void)
{
	if (s32v_ocotp_wait_busy_clear() != 0)
		return -EINVAL;

	u32 reg = readl(OCOTP_CTRL_REG);
	reg |= OCOTP_CTRL_RELOAD_SHADOWS_BIT;
	s32v_ocotp_wait_busy_clear();
	return 0;
}

static int s32v_ocotp_set_timing(void)
{
	u32 reg = readl(OCOTP_TIMING_REG);
	reg &= ~0x3fffff;
	reg |= (OCOTP_STROBE_READ_TIME << 16) | (OCOTP_RELAX_TIME << 12) \
			| (OCOTP_STROBE_PROG_TIME);

	writel(reg, OCOTP_TIMING_REG);
	return 0;
}

static int s32v_ocotp_prepare(void)
{
	udelay(20);

	s32v_ocotp_set_timing();

	/* wait until controller is ready: */
	if (s32v_ocotp_wait_busy_clear() != 0)
		return -EINVAL;

	/* clear error flag: */
	s32v_ocotp_clear_error();

	return 0;
}

int s32v_ocotp_read_fuse(const u8 addr, u32 *const fuse_word)
{
	if (s32v_ocotp_prepare() != 0)
		return -EIO;

	/* set address of fuse to read: */
	s32v_ocotp_set_addr(addr);

	/* initiate read to OTP: */
	u32 reg = readl(OCOTP_READ_CTRL_REG);
	reg |= OCOTP_READ_CTRL_READ_FUSE_BIT;
	writel(reg, OCOTP_READ_CTRL_REG);

	/* check for error first: */
	reg = readl(OCOTP_CTRL_REG);
	if (reg & OCOTP_CTRL_ERROR_BIT)
		/* read from read-protected fuse */
		return -EINVAL;

	/* wait until controller is ready: */
	if (s32v_ocotp_wait_busy_clear() != 0)
		return -EIO;

	*fuse_word = readl(OCOTP_READ_FUSE_DATA_REG);

	return 0;
}

static u8 s32v_ocotp_calc_ecc7(const u32 fuse_word)
{
	static const u32 OCOTP_ECC_G[7] = {
		0xC14840FF,
		0x2124FF90,
		0x6CFF0808,
		0xFF01A444,
		0x16F092A6,
		0x101F7161,
		0x8A820F1B
	};

	int i, j;
	u32 ecc = 0;
	u32 res;
	u8 ecc7 = 0;

	for (i = 0; i < 7; i++) {
		ecc = fuse_word & OCOTP_ECC_G[i];
		res = (ecc & BIT(0)) ^ ((ecc & BIT(1)) >> 1);

		for (j = 2; j < 32; j++)
			res = res ^ ((ecc & BIT(j)) >> j);

		ecc7 += res << i;
	}

	return ecc7 & 0x7f;
}

static int s32v_ocotp_burn_fuse(const u8 addr, const u32 fuse_word)
{
	if (s32v_ocotp_prepare() != 0)
		return -EIO;

	/* set address of fuse to write: */
	s32v_ocotp_set_addr(addr);

	/* unlock fuse write: */
	u32 reg = readl(OCOTP_CTRL_REG);
	reg &= ~OCOTP_CTRL_WR_UNLOCK_MASK;
	reg |= OCOTP_CTRL_WR_UNLOCK_CODE;
	writel(reg, OCOTP_CTRL_REG);

	/* write the data: */
	writel(fuse_word, OCOTP_DATA_REG);

	/* wait until controller is ready: */
	if (s32v_ocotp_wait_busy_clear() != 0) {
		s32v_ocotp_set_addr(0);
		return -EIO;
	}

	int i;
	for (i = 0; i < 0xfff; i++)
		__asm__("nop");

	s32v_ocotp_set_addr(0);

	return 0;
}

int s32v_ocotp_write_fuse(const u8 addr, const u32 fuse_word)
{
	if (addr >= BANKWORD2ADDR(6, 3))
		return -EINVAL;

	/* nothing to program */
	if (fuse_word == 0)
		return 0;

	const u32 shadow = ADDR2SHADOW(addr);

	const u32 shdw_fuse = readl(OCOTP_REG(shadow));
	u32 fuse = shdw_fuse;

	/*
	 * Check if operation is possible:
	 */

	/* all bits already set in fuse */
	u32 tmp = fuse & fuse_word;
	/* all bits that should be added */
	tmp = ~tmp & fuse_word;
	/* no bit to add */
	if (tmp == 0)
		return -EIO;

	fuse |= tmp;
	writel(fuse, OCOTP_REG(shadow));

	u8 ecc7 = s32v_ocotp_calc_ecc7(fuse);

	u32 ecc_fuse = (addr / 4) * 16 + OCOTP_ECC_FUSE0_OFFSET;
	const u32 ecc_byte = addr % 4;

	/*
	 * Check ECC:
	 */
	const u32 shdw_ecc = readl(OCOTP_REG(ecc_fuse));
	u32 ecc = shdw_ecc;

	/* bit 7 enables ECC for the word */
	ecc7 |= BIT(7);
	ecc &= ~(0xffUL << (ecc_byte*8));
	ecc |= ((u32)ecc7 << (ecc_byte*8));
	writel(ecc, OCOTP_REG(ecc_fuse));

	const u32 sec1 = readl(OCOTP_SEC1_REG);
	const u32 dec1 = readl(OCOTP_DEC1_REG);
	const u32 sec0 = readl(OCOTP_SEC0_REG);
	const u32 dec0 = readl(OCOTP_DEC0_REG);

	/*
	 * Now check for ECC single-bit and double-bit error:
	 */
	u32 bit_no = addr;
	if (bit_no <= 31) {
		if (sec0 & BIT(bit_no)) {
			puts("Error: single-bit error indicated!\n");
			writel(shdw_fuse, OCOTP_REG(shadow));
			writel(shdw_ecc, OCOTP_REG(ecc_fuse));
			s32v_ocotp_reload_shadows();
			return -EIO;
		}
		if (dec0 & BIT(bit_no)) {
			writel(shdw_fuse, OCOTP_REG(shadow));
			writel(shdw_ecc, OCOTP_REG(ecc_fuse));
			s32v_ocotp_reload_shadows();
			puts("Error: double-bit error indicated!\n");
			return -EIO;
		}
	} else {
		bit_no -= 32;
		if (sec1 & BIT(bit_no)) {
			writel(shdw_fuse, OCOTP_REG(shadow));
			writel(shdw_ecc, OCOTP_REG(ecc_fuse));
			s32v_ocotp_reload_shadows();
			puts("Error: single-bit error indicated!\n");
			return -EIO;
		}
		if (dec1 & BIT(bit_no)) {
			writel(shdw_fuse, OCOTP_REG(shadow));
			writel(shdw_ecc, OCOTP_REG(ecc_fuse));
			s32v_ocotp_reload_shadows();
			puts("Error: double-bit error indicated!\n");
			return -EIO;
		}
	}

	/* Burn the fuse: */
	if (s32v_ocotp_burn_fuse(addr, tmp) != 0) {
		writel(shdw_fuse, OCOTP_REG(shadow));
		writel(shdw_ecc, OCOTP_REG(ecc_fuse));
		s32v_ocotp_reload_shadows();
		printf("Error: cannot burn fuse at 0x%02x!\n", addr);
		return -EIO;
	}

	/* Burn the redundant fuse: */
	if (s32v_ocotp_burn_fuse(REDUNDANTADDR(addr), tmp) != 0) {
		writel(shdw_fuse, OCOTP_REG(shadow));
		writel(shdw_ecc, OCOTP_REG(ecc_fuse));
		s32v_ocotp_reload_shadows();
		printf("Error: cannot burn redundant fuse at 0x%02x!\n"
			, REDUNDANTADDR(addr));
		return -EIO;
	}

	/*
	 * ECC fuse block starts at bank 6,3 == 0x33
	 */

	/* Burn the ECC into its fuse: */
	ecc_fuse = (addr / 4) + BANKWORD2ADDR(6, 3);
	ecc &= (0xffUL << (ecc_byte*8));
	if (s32v_ocotp_burn_fuse(ecc_fuse, ecc) != 0) {
		writel(shdw_fuse, OCOTP_REG(shadow));
		writel(shdw_ecc, OCOTP_REG(ecc_fuse));
		s32v_ocotp_reload_shadows();
		printf("Error: cannot burn ECC fuse at 0x%02x!\n", ecc_fuse);
		return -EIO;
	}

	/* TODO: set lock on fuse */

	return 0;
}
