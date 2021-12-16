// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2021 NXP
 */
#include <asm/io.h>
#include <cmu.h>
#include <command.h>
#include <common.h>
#include <inttypes.h>
#include <linux/kernel.h>

/* Clocks variations in percentages */
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

static u32 get_ndigits(double val, u32 precision)
{
	u32 digits = 0;
	u32 val32 = (u32)val;

	digits = u32_get_digits(val32);

	/* Dot */
	digits++;

	/* Fractional part */
	precision /= 10;
	while (precision) {
		digits++;
		precision /= 10;
	}

	return digits;
}

static void print_double(double val, u32 space, u32 precision)
{
	size_t i;
	size_t ndigits;
	u32 val32 = (u32)val;
	u32 frac = (u32)((val - (double)val32) * (double)precision);

	ndigits = get_ndigits(val, precision);
	if (space > ndigits) {
		for (i = 0; i < space - ndigits; i++)
			printf(" ");
	}

	if (precision == 1000)
		printf("%" PRIu32 ".%03" PRIu32, val32, frac);
	else if (precision == 100)
		printf("%" PRIu32 ".%02" PRIu32, val32, frac);
	else
		printf("%" PRIu32 ".%01" PRIu32, val32, frac);
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

static double get_max_exp_freq(struct cmu *inst)
{
	if (inst->has_exp_range)
		return inst->exp_range.max;

	return inst->exp_freq;
}

static int get_fm_mon_freq(struct cmu *inst, double *mon_freq)
{
	struct cmu_fm_params cmu_fm;
	u32 met_cnt, sr;

	if (get_fm_params(inst->ref_freq, get_max_exp_freq(inst), &cmu_fm))
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

static void print_expected_freq(struct cmu *inst)
{
	if (inst->has_exp_range) {
		print_double(inst->exp_range.min, 5, 10);
		puts(" - ");
		print_double(inst->exp_range.max, 5, 10);
		return;
	}

	print_double(inst->exp_freq, 12, 1);
}

static int do_verify_clocks(cmd_tbl_t *cmdtp, int flag, int argc,
			    char * const argv[])
{
	struct freq_interval freq_int;
	struct cmu *inst;
	size_t i;
	double max_var;
	double mon_freq = 0;

	puts(" CMU       | Monitored    | Reference | Expected      |");
	puts(" Verified\n");
	puts(" Address   | clock        | clock     | range (MHz)   |");
	puts(" range (MHz)\n");
	puts("-----------|--------------|-----------|---------------|");
	puts("--------------------\n");

	for (i = 0; i < get_cmu_blocks_number(); i++) {
		inst = get_cmu_block(i);
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
		printf(" %12s | ", inst->mon_name);
		printf("%9s | ", inst->ref_name);
		print_expected_freq(inst);
		printf(" | ");
		if (inst->fc) {
			print_double(get_min_freq(freq_int.min, max_var),
				     8, 1000);
			printf(" - ");
			print_double(get_max_freq(freq_int.max, max_var),
				     8, 1000);
		} else {
			print_double(mon_freq, 19, 1000);
		}
		printf("\n");
	}

	return 0;
}

U_BOOT_CMD(verifclk, CONFIG_SYS_MAXARGS, 1, do_verify_clocks,
	   "Verifies clocks frequencies using CMU module",
	   NULL
);
