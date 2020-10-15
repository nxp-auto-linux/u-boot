// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 */
#include <asm/io.h>
#include <asm/arch/siul.h>
#include <command.h>
#include <common.h>
#include <inttypes.h>
#include <linux/kernel.h>

#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

/* Clocks variations in percentages */
#define FIRC_VARIATION			6.0f
#define FXOSC_VARIATION			0.5f
#define PERIPH_VARIATION		0.5f

#define FIRC_FREQ			((double)48)
#define FXOSC_FREQ			((double)40)
#define SIRC_FREQ			((double)0.032)
#define MAX_PERIPH_FREQ			((double)2000)

#define CMU_FC_GCR(BASE)		((BASE) + 0x0)
#define CMU_FC_GCR_FCE			BIT(0)
#define CMU_FC_RCCR(BASE)		((BASE) + 0x4)
#define CMU_FC_RCCR_REF_CNT_MAX		(0xFFFFU)
#define CMU_FC_HTCR(BASE)		((BASE) + 0x8)
#define CMU_FC_LTCR(BASE)		((BASE) + 0xC)
#define CMU_FC_TCR_FREF_MAX		(0xFFFFFFU)
#define CMU_FC_SR(BASE)			((BASE) + 0x10)
#define CMU_FC_SR_FLL			BIT(0)
#define CMU_FC_SR_HLL			BIT(1)
#define CMU_FC_SR_FLAGS			(CMU_FC_SR_FLL | CMU_FC_SR_HLL)
#define CMU_FC_SR_STATE(SR)		(((SR) >> 2) & 0x3)
#define CMU_FC_SR_RS			BIT(4)

#define CMU_FM_GCR(BASE)		((BASE) + 0x0)
#define CMU_FM_GCR_FME			BIT(0)
#define CMU_FM_RCCR(BASE)		((BASE) + 0x4)
#define CMU_FM_SR(BASE)			((BASE) + 0x8)
#define CMU_FM_SR_MET_CNT(SR)		(((SR) >> 8) & 0xFFFFFFU)
#define CMU_FM_SR_RS			BIT(4)
#define CMU_FM_SR_STATE(SR)		(((SR) >> 2) & 0x3)
#define CMU_FM_SR_FMTO			BIT(1)
#define CMU_FM_SR_FMC			BIT(0)

#define CMU(ID, REF_CLK, MON_CLK, REF_FRQ, MON_FRQ, REF_VAR, MON_VAR, FC) \
{\
	.addr = CMU_BASE_ADDR + 0x20 * (ID),\
	.ref_clk = (REF_CLK),\
	.mon_clk = (MON_CLK),\
	.ref_name = STR(REF_CLK),\
	.mon_name = STR(MON_CLK),\
	.ref_freq = (REF_FRQ),\
	.mon_freq = (MON_FRQ),\
	.ref_var = (REF_VAR),\
	.mon_var = (MON_VAR),\
	.fc = (FC),\
}

#define FXOSC_PERIPH_CMU_FC(ID, MON, MON_FRQ) \
	CMU(ID, FXOSC_CLK, MON, FXOSC_FREQ, MON_FRQ, \
			FXOSC_VARIATION, PERIPH_VARIATION, true)

#define FIRC_PERIPH_CMU_FC(ID, MON, MON_FRQ) \
	CMU(ID, FIRC_CLK, MON, FIRC_FREQ, MON_FRQ, \
			FIRC_VARIATION, PERIPH_VARIATION, true)

#define FXOSC_PERIPH_CMU_FM(ID, MON, MON_FRQ) \
	CMU(ID, FXOSC_CLK, MON, FXOSC_FREQ, MON_FRQ, \
			0, 0, false)

#define FIRC_PERIPH_CMU_FM(ID, MON, MON_FRQ) \
	CMU(ID, FIRC_CLK, MON, FIRC_FREQ, MON_FRQ, \
			0, 0, false)

#define MAX_DEPTH 20

struct cmu_params {
	u16 ref_cnt;
	u32 href;
	u32 lref;
	u32 udelay;
};

struct cmu_fm_params {
	u16 ref_cnt;
};

struct freq_interval {
	double min;
	double max;
};

enum fc_result {
	MATCH = 0x0,
	LOWER = 0x1,
	HIGHER = 0x2,
};

enum cmu_fc_clk {
	FIRC_CLK,
	FXOSC_CLK,
	SIRC_CLK,
	XBAR_CLK_M7_0,
	XBAR_CLK_M7_1,
	XBAR_CLK_M7_2,
	XBAR_DIV3_CLK,
	SERDES_REF_CLK,
	PER_CLK,
	CAN_PE_CLK,
	LIN_CLK,
	QSPI_1X_CLK,
	SDHC_CLK,
	DDR_CLK,
	SPI_CLK,
	A53_CORE_CLK,
	ACCEL3_CLK,
	ACCEL4_CLK_0,
	ACCEL4_CLK_1,
	MIPICSI2_0,
	MIPICSI2_2,
	GMAC_TS_CLK,
	GMAC_0_TX_CLK,
	GMAC_0_RX_CLK,
	GMAC_1_TX_CLK,
	GMAC_1_RX_CLK,
	PFE_SYS_CLK,
	PFE_MAC_0_TX_CLK,
	PFE_MAC_0_RX_CLK,
	PFE_MAC_1_TX_CLK,
	PFE_MAC_1_RX_CLK,
	PFE_MAC_2_TX_CLK,
	PFE_MAC_2_RX_CLK,
	FTM_0_REF_CLK,
	FTM_1_REF_CLK,
};

struct cmu {
	uintptr_t addr;
	enum cmu_fc_clk ref_clk;
	enum cmu_fc_clk mon_clk;
	const char *ref_name;
	const char *mon_name;
	double ref_freq;
	double mon_freq;
	double ref_var;
	double mon_var;
	bool fc;
};

#if defined(CONFIG_TARGET_S32R45EVB)
static struct cmu cmu_blocks[] = {
	FIRC_PERIPH_CMU_FC(0, FXOSC_CLK, FXOSC_FREQ),
	FXOSC_PERIPH_CMU_FM(1, FIRC_CLK, FIRC_FREQ),
	FXOSC_PERIPH_CMU_FM(2, SIRC_CLK, SIRC_FREQ),
	FXOSC_PERIPH_CMU_FM(3, FTM_0_REF_CLK, 40),
	FXOSC_PERIPH_CMU_FM(4, FTM_1_REF_CLK, 40),
	FIRC_PERIPH_CMU_FC(5, XBAR_DIV3_CLK, 133.33),
	FIRC_PERIPH_CMU_FC(6, XBAR_CLK_M7_0, 400),
	FXOSC_PERIPH_CMU_FC(7, XBAR_DIV3_CLK, 133.33),
	FIRC_PERIPH_CMU_FC(8, XBAR_CLK_M7_1, 400),
	FIRC_PERIPH_CMU_FC(9, XBAR_CLK_M7_2, 400),
	FIRC_PERIPH_CMU_FC(10, PER_CLK, 80),
	FXOSC_PERIPH_CMU_FC(11, SERDES_REF_CLK, 125),
	FXOSC_PERIPH_CMU_FC(13, CAN_PE_CLK, 80),
	FXOSC_PERIPH_CMU_FC(14, GMAC_0_TX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(15, GMAC_TS_CLK, 200),
	FXOSC_PERIPH_CMU_FC(16, LIN_CLK, 125),
	FXOSC_PERIPH_CMU_FC(17, QSPI_1X_CLK, 133.33),
	FXOSC_PERIPH_CMU_FC(18, SDHC_CLK, 200),
	FIRC_PERIPH_CMU_FC(20, DDR_CLK, 800),
	FXOSC_PERIPH_CMU_FC(21, GMAC_0_RX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(22, SPI_CLK, 100),
	FXOSC_PERIPH_CMU_FC(27, A53_CORE_CLK, 800),
	FIRC_PERIPH_CMU_FC(28, A53_CORE_CLK, 800),
	FXOSC_PERIPH_CMU_FC(38, ACCEL3_CLK, 600),
	FXOSC_PERIPH_CMU_FC(39, ACCEL4_CLK_0, 400),
	FXOSC_PERIPH_CMU_FC(40, ACCEL4_CLK_0, 400),
	FXOSC_PERIPH_CMU_FC(46, GMAC_1_TX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(51, GMAC_1_RX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(52, MIPICSI2_0, 400),
	FXOSC_PERIPH_CMU_FC(53, MIPICSI2_0, 400),
	FXOSC_PERIPH_CMU_FC(54, SERDES_REF_CLK, 125),
};
#elif defined(CONFIG_TARGET_S32G274AEVB) || \
	defined(CONFIG_TARGET_S32G274ARDB) || \
	defined(CONFIG_TARGET_S32G274ABLUEBOX3)
static struct cmu cmu_blocks[] = {
	FIRC_PERIPH_CMU_FC(0, FXOSC_CLK, FXOSC_FREQ),
	FXOSC_PERIPH_CMU_FM(1, FIRC_CLK, FIRC_FREQ),
	FXOSC_PERIPH_CMU_FM(2, SIRC_CLK, SIRC_FREQ),
	FXOSC_PERIPH_CMU_FM(3, FTM_0_REF_CLK, 40),
	FXOSC_PERIPH_CMU_FM(4, FTM_1_REF_CLK, 40),
	FIRC_PERIPH_CMU_FC(0, FXOSC_CLK, FXOSC_FREQ),
	FIRC_PERIPH_CMU_FC(5, XBAR_DIV3_CLK, 133.33),
	FIRC_PERIPH_CMU_FC(6, XBAR_CLK_M7_0, 400),
	FXOSC_PERIPH_CMU_FC(7, XBAR_DIV3_CLK, 133.33),
	FIRC_PERIPH_CMU_FC(8, XBAR_CLK_M7_1, 400),
	FIRC_PERIPH_CMU_FC(9, XBAR_CLK_M7_2, 400),
	FIRC_PERIPH_CMU_FC(10, PER_CLK, 80),
	FXOSC_PERIPH_CMU_FC(11, SERDES_REF_CLK, 125),
	FXOSC_PERIPH_CMU_FC(13, CAN_PE_CLK, 80),
	FXOSC_PERIPH_CMU_FC(14, GMAC_0_TX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(15, GMAC_TS_CLK, 200),
	FXOSC_PERIPH_CMU_FC(16, LIN_CLK, 125),
	FXOSC_PERIPH_CMU_FC(17, QSPI_1X_CLK, 200),
	FXOSC_PERIPH_CMU_FC(18, SDHC_CLK, 200),
	FIRC_PERIPH_CMU_FC(20, DDR_CLK, 666.66),
	FXOSC_PERIPH_CMU_FC(21, GMAC_0_RX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(22, SPI_CLK, 100),
	FXOSC_PERIPH_CMU_FC(27, A53_CORE_CLK, 1000),
	FIRC_PERIPH_CMU_FC(28, A53_CORE_CLK, 1000),
	FXOSC_PERIPH_CMU_FC(39, PFE_SYS_CLK, 300),
	FXOSC_PERIPH_CMU_FC(46, PFE_MAC_0_TX_CLK, 312.5),
	FXOSC_PERIPH_CMU_FC(47, PFE_MAC_0_RX_CLK, 312.5),
	FXOSC_PERIPH_CMU_FC(48, PFE_MAC_1_TX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(49, PFE_MAC_1_RX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(50, PFE_MAC_2_TX_CLK, 125),
	FXOSC_PERIPH_CMU_FC(51, PFE_MAC_2_RX_CLK, 125),

};
#else
#error CMU_FC cannot be enabled for this board
#endif

static double get_min_freq(double clk, double variation)
{
	return clk * ((double)1 - variation / 100);
}

static double get_max_freq(double clk, double variation)
{
	return clk * ((double)1 + variation / 100);
}

static u16 u16_ceiling(double val)
{
	u16 floor = (u16)val;

	if (val - (double)floor == (double)0)
		return floor;

	return floor + 1;
}

static u32 u32_round(double val)
{
	u32 floor = (u32)val;

	if (val - (double)floor < (double)0.5)
		return floor;

	return floor + 1;
}

static u32 u32_get_digits(u32 val)
{
	u32 digits = 0;

	if (!val)
		return 1;

	while (val) {
		digits++;
		val /= 10;
	}

	return digits;
}

static u32 get_ndigits(double val)
{
	u32 digits = 0;
	u32 val32 = (u32)val;

	digits = u32_get_digits(val32);

	/* Dot */
	digits++;

	/* Fractional part */
	digits += 3;

	return digits;
}

static void print_double(double val, u32 space)
{
	size_t i;
	u32 val32 = (u32)val;
	u32 frac = (u32)((val - (double)val32) * 1000.0);

	for (i = 0; i < space - get_ndigits(val); i++)
		printf(" ");

	printf("%" PRIu32 ".%03" PRIu32, val32, frac);
}

static int calc_cmu_ref_cnt(double ref_clk, double mon_clk,
			    u16 *ref_cnt)
{
	double d_cnt;
	u16 max_iter;

	d_cnt = 6.0 + 4.0 * ref_clk / mon_clk;
	if ((u32)(d_cnt) > CMU_FC_RCCR_REF_CNT_MAX) {
		pr_err("REF_CNT (0x%" PRIx32 ") > max value\n", (u32)(d_cnt));
		return -1;
	}

	*ref_cnt = u16_ceiling(d_cnt);
	/* Maximum accuracy */
	max_iter = CMU_FC_RCCR_REF_CNT_MAX / *ref_cnt;
	*ref_cnt *= max_iter;

	/* Overflow */
	if ((double)*ref_cnt < d_cnt) {
		pr_err("REF_CNT overflow\n");
		return -1;
	}

	return 0;
}

static int get_fm_params(double ref_clk, double mon_clk,
			 struct cmu_fm_params *conf)
{
	return calc_cmu_ref_cnt(ref_clk, mon_clk, &conf->ref_cnt);
}

static int get_fc_params(double ref_clk, double mon_clk,
			 double ref_var, double mon_var,
			 struct cmu_params *conf)
{
	double min_ref = get_min_freq(ref_clk, ref_var);
	double max_ref = get_max_freq(ref_clk, ref_var);
	double min_mon = get_min_freq(mon_clk, mon_var);
	double max_mon = get_max_freq(mon_clk, mon_var);
	double href, lref;

	if (calc_cmu_ref_cnt(ref_clk, mon_clk, &conf->ref_cnt))
		return -1;

	conf->udelay = u32_round((double)conf->ref_cnt / ref_clk);

	href = (max_mon / min_ref) * (double)conf->ref_cnt + 3;
	if ((u64)(href) > CMU_FC_TCR_FREF_MAX) {
		pr_err("HREF (0x%" PRIx64 ") > max value\n", (u64)(href));
		return -1;
	}

	conf->href = u32_round(href);
	/* Overflow */
	if ((u64)conf->href < (u64)href) {
		pr_err("HREF overflow\n");
		return -1;
	}

	lref = (min_mon / max_ref) * (double)conf->ref_cnt - 3;
	if ((u64)(lref) > CMU_FC_TCR_FREF_MAX) {
		pr_err("LREF (0x%" PRIx64 ") > max value\n", (u64)(lref));
		return -1;
	}

	conf->lref = u32_round(lref);
	/* Overflow */
	if ((u64)conf->href < (u64)href) {
		pr_err("LREF overflow\n");
		return -1;
	}

	return 0;
}

static enum fc_result fc_check_frequency(uintptr_t addr,
					 struct cmu_params *params)
{
	/* Disable the module */
	writel(0x0, CMU_FC_GCR(addr));

	/* Clear FLL & HLL flags */
	writel(CMU_FC_SR_FLL | CMU_FC_SR_HLL, CMU_FC_SR(addr));

	/* Write params */
	writel(params->ref_cnt, CMU_FC_RCCR(addr));
	writel(params->href, CMU_FC_HTCR(addr));
	writel(params->lref, CMU_FC_LTCR(addr));

	/* Start the frequency check */
	writel(CMU_FC_GCR_FCE, CMU_FC_GCR(addr));

	while (!(readl(CMU_FC_SR(addr)) & CMU_FC_SR_RS))
		;

	udelay(params->udelay * 3);

	/* Disable the module */
	writel(0x0, CMU_FC_GCR(addr));

	return readl(CMU_FC_SR(addr)) & CMU_FC_SR_FLAGS;
}

static int _get_fc_mon_freq(struct cmu *inst, struct freq_interval *freq_int,
			    u32 depth)
{
	int ret;
	double mon_freq;
	struct cmu_params params;
	enum fc_result res;

	if (depth == 0)
		return 0;

	mon_freq = (freq_int->min + freq_int->max) / 2;

	/* Assume 0 if the frequency is lower than 10KHz */
	if (mon_freq < 0.01)
		return 0;

	ret = get_fc_params(inst->ref_freq, mon_freq,
			    inst->ref_var, inst->mon_var,
			    &params);
	if (ret) {
		pr_err("Failed to determine CMU_FC parameters for clock: %s\n",
		       inst->mon_name);
		return ret;
	}

	depth--;

	res = fc_check_frequency(inst->addr, &params);
	switch (res) {
	case HIGHER:
		freq_int->min = mon_freq;
		freq_int->max = freq_int->max;
		return _get_fc_mon_freq(inst, freq_int, depth);
	case LOWER:
		freq_int->min = freq_int->min;
		freq_int->max = mon_freq;
		return _get_fc_mon_freq(inst, freq_int, depth);
	default:
		freq_int->min = mon_freq;
		freq_int->max = mon_freq;
		return 0;
	}

	return -1;
}

static int get_fc_mon_freq(struct cmu *inst,
			   struct freq_interval *freq_int)
{
	freq_int->min = 0;
	freq_int->max = MAX_PERIPH_FREQ;
	return _get_fc_mon_freq(inst, freq_int, MAX_DEPTH);
}

static int get_fm_mon_freq(struct cmu *inst, double *mon_freq)
{
	struct cmu_fm_params cmu_fm;
	u32 met_cnt, sr;

	if (get_fm_params(inst->ref_freq, inst->mon_freq, &cmu_fm))
		return -1;

	/* Disable the module*/
	writel(0x0, CMU_FM_GCR(inst->addr));

	/* Clear flags */
	writel(CMU_FM_SR_FMTO | CMU_FM_SR_FMC, CMU_FM_SR(inst->addr));

	/* Sampling period */
	writel(cmu_fm.ref_cnt, CMU_FM_RCCR(inst->addr));
	/* Start the measurement */
	writel(CMU_FM_GCR_FME, CMU_FM_GCR(inst->addr));

	while (readl(CMU_FM_SR(inst->addr)) & (CMU_FM_SR_FMTO | CMU_FM_SR_FMC))
		;

	do {
		sr = readl(CMU_FM_SR(inst->addr));
		if (sr & CMU_FM_SR_FMTO) {
			pr_err("Timeout while measuring the frequency of %s\n",
			       inst->mon_name);
			return -1;
		}
	} while (!CMU_FM_SR_MET_CNT(sr));

	/* Disable the module*/
	writel(0x0, CMU_FM_GCR(inst->addr));

	met_cnt = CMU_FM_SR_MET_CNT(sr);

	*mon_freq = (double)met_cnt * inst->ref_freq / cmu_fm.ref_cnt;

	return 0;
}

static int do_verify_clocks(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	struct freq_interval freq_int;
	struct cmu *inst;
	size_t i;
	double max_var;
	double mon_freq = 0;

	puts("    CMU    |     Monitored    | Reference | Expected ");
	puts("|  Verified interval \n");
	puts("  Address  |       clock      |   clock   |   (MHz)  ");
	puts("|       (MHz)       \n");
	puts("-----------|------------------|-----------|----------");
	puts("|--------------------\n");

#if defined(CONFIG_TARGET_S32G274AEVB) || defined(CONFIG_TARGET_S32G274ARDB)
	if (is_s32gen1_soc_rev1())
		cmu_blocks[17].mon_freq = 133.33;
#endif

	for (i = 0; i < ARRAY_SIZE(cmu_blocks); i++) {
		inst = &cmu_blocks[i];
		if (inst->fc) {
			if (get_fc_mon_freq(inst, &freq_int)) {
				pr_err("Failed to determine CMU_FC parameters "
				       "for clock: %s\n", inst->mon_name);
				continue;
			}

		} else {
			if (get_fm_mon_freq(inst, &mon_freq))
				continue;
		}

		printf("0x%" PRIxPTR " |", inst->addr);
		max_var = min(inst->mon_var, inst->ref_var);
		printf(" %16s | ", inst->mon_name);
		printf("%9s | ", inst->ref_name);
		print_double(inst->mon_freq, 8);
		printf(" | ");
		if (inst->fc) {
			print_double(get_min_freq(freq_int.min, max_var), 8);
			printf(" - ");
			print_double(get_max_freq(freq_int.max, max_var), 8);
		} else {
			print_double(mon_freq, 19);
		}
		printf("\n");
	}

	return 0;
}

U_BOOT_CMD(verifclk, CONFIG_SYS_MAXARGS, 1, do_verify_clocks,
	   "Verifies clocks frequencies using CMU module",
	   NULL
);
