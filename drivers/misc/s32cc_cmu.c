// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2021-2023 NXP
 */

#include <common.h>
#include <clk.h>
#include <command.h>
#include <dm.h>
#include <inttypes.h>
#include <misc.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/iopoll.h>
#include <linux/math64.h>

#include "s32cc_cmu.h"

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

#define CMU_FC_WAIT				100		/* us */
#define CMU_FC_TIMEOUT			100000	/* us */

#define CMU_FM_GCR(BASE)		((BASE) + 0x0)
#define CMU_FM_GCR_FME			BIT(0)
#define CMU_FM_RCCR(BASE)		((BASE) + 0x4)
#define CMU_FM_SR(BASE)			((BASE) + 0x8)
#define CMU_FM_SR_MET_CNT(SR)		(((SR) >> 8) & 0xFFFFFFU)
#define CMU_FM_SR_RS			BIT(4)
#define CMU_FM_SR_STATE(SR)		(((SR) >> 2) & 0x3)
#define CMU_FM_SR_FMTO			BIT(1)
#define CMU_FM_SR_FMC			BIT(0)

#define MAX_PERIPH_FREQ			(2000000000ULL)
#define KHZ_10				(10000ULL)
#define MHZ_1				(1000000ULL)

#define MAX_DEPTH 20

struct s32cc_cmu_data {
	struct cmu *cmu_blocks;
	int n_blocks;
};

struct s32cc_cmu {
	void __iomem *base_addr;
	struct s32cc_cmu_data *data;
	struct clk cmu_clk_module;
	struct clk cmu_clk_reg;
};

struct cmu_params {
	u64 ref_cnt;
	u32 href;
	u32 lref;
	u32 udelay;
};

struct cmu_fm_params {
	u64 ref_cnt;
};

struct freq_interval {
	u64 min;
	u64 max;
};

enum fc_result {
	MATCH = 0x0,
	LOWER = 0x1,
	HIGHER = 0x2,
	TIMEOUT = 0x3,
};

struct cmu_result {
	int id;
	char mon_clk_name[16];
	char ref_clk_name[8];
	struct freq_interval expected;
	struct freq_interval measured;
};

static struct cmu s32g2_cmu_blocks[] = {
	FIRC_PERIPH_CMU_FC(0, FXOSC, FXOSC_FREQ),

	/* +-5% drift due to silicon and temperature factors  */
	FXOSC_PERIPH_CMU_FM_RANGE(1, FIRC, FIRC_MIN_FREQ, FIRC_MAX_FREQ),
	FXOSC_PERIPH_CMU_FM(2, SIRC, SIRC_FREQ),
	FXOSC_PERIPH_CMU_FM(3, FTM_0_REF, 40000000),
	FXOSC_PERIPH_CMU_FM(4, FTM_1_REF, 40000000),

	FIRC_PERIPH_CMU_FC(5, XBAR_DIV3, 133333333),
	FIRC_PERIPH_CMU_FC(6, XBAR_M7_0, 400000000),

	FXOSC_PERIPH_CMU_FC(7, XBAR_DIV3, 133333333),

	FIRC_PERIPH_CMU_FC(8, XBAR_M7_1, 400000000),
	FIRC_PERIPH_CMU_FC(9, XBAR_M7_2, 400000000),
	FIRC_PERIPH_CMU_FC(10, PER, 80000000),

	FXOSC_PERIPH_CMU_FC_RANGE(11, SERDES_REF, 100000000, 125000000),
	FXOSC_PERIPH_CMU_FC(12, FLEXRAY_PE, 40000000),
	FXOSC_PERIPH_CMU_FC(13, CAN_PE, 80000000),
	FXOSC_PERIPH_CMU_FC_RANGE(14, GMAC_0_TX, 2500000, 125000000),
	FXOSC_PERIPH_CMU_FC_RANGE(15, GMAC_TS, 5000000, 200000000),
	FXOSC_PERIPH_CMU_FC(16, LIN, 125000000),
	FXOSC_PERIPH_CMU_FC_RANGE(17, QSPI_1X, 0, 200000000),
	FXOSC_PERIPH_CMU_FC(18, SDHC, 400000000),

	FIRC_PERIPH_CMU_FC(20, DDR, 666666666),

	FXOSC_PERIPH_CMU_FC_RANGE(21, GMAC_0_RX, 2500000, 125000000),
	FXOSC_PERIPH_CMU_FC(22, SPI, 100000000),

	FXOSC_PERIPH_CMU_FC(27, A53_CORE, 1000000000),

	FIRC_PERIPH_CMU_FC(28, A53_CORE, 1000000000),

	FXOSC_PERIPH_CMU_FC(39, PFE_SYS, 300000000),
	FXOSC_PERIPH_CMU_FC_RANGE(46, PFE_MAC_0_TX, 2500000, 312500000),
	FXOSC_PERIPH_CMU_FC_RANGE(47, PFE_MAC_0_RX, 2500000, 312500000),
	FXOSC_PERIPH_CMU_FC_RANGE(48, PFE_MAC_1_TX, 2500000, 125000000),
	FXOSC_PERIPH_CMU_FC_RANGE(49, PFE_MAC_1_RX, 2500000, 125000000),
	FXOSC_PERIPH_CMU_FC_RANGE(50, PFE_MAC_2_TX, 2500000, 125000000),
	FXOSC_PERIPH_CMU_FC_RANGE(51, PFE_MAC_2_RX, 2500000, 125000000),
};

static struct cmu s32g3_cmu_blocks[] = {
	FIRC_PERIPH_CMU_FC(0, FXOSC, FXOSC_FREQ),

	/* +-5% drift due to silicon and temperature factors  */
	FXOSC_PERIPH_CMU_FM_RANGE(1, FIRC, FIRC_MIN_FREQ, FIRC_MAX_FREQ),
	FXOSC_PERIPH_CMU_FM(2, SIRC, SIRC_FREQ),
	FXOSC_PERIPH_CMU_FM(3, FTM_0_REF, 40000000),
	FXOSC_PERIPH_CMU_FM(4, FTM_1_REF, 40000000),

	FIRC_PERIPH_CMU_FC(5, XBAR_DIV3, 133333333),
	FIRC_PERIPH_CMU_FC(6, XBAR_M7_0, 400000000),

	FXOSC_PERIPH_CMU_FC(7, XBAR_DIV3, 133333333),

	FIRC_PERIPH_CMU_FC(8, XBAR_M7_1, 400000000),
	FIRC_PERIPH_CMU_FC(9, XBAR_M7_2, 400000000),
	FIRC_PERIPH_CMU_FC(24, XBAR_M7_3, 400000000),
	FIRC_PERIPH_CMU_FC(10, PER, 80000000),

	FXOSC_PERIPH_CMU_FC_RANGE(11, SERDES_REF, 100000000, 125000000),
	FXOSC_PERIPH_CMU_FC(12, FLEXRAY_PE, 40000000),
	FXOSC_PERIPH_CMU_FC(13, CAN_PE, 80000000),
	FXOSC_PERIPH_CMU_FC_RANGE(14, GMAC_0_TX, 2500000, 125000000),
	FXOSC_PERIPH_CMU_FC_RANGE(15, GMAC_TS, 5000000, 200000000),
	FXOSC_PERIPH_CMU_FC(16, LIN, 125000000),
	FXOSC_PERIPH_CMU_FC_RANGE(17, QSPI_1X, 0, 200000000),
	FXOSC_PERIPH_CMU_FC(18, SDHC, 400000000),

	FIRC_PERIPH_CMU_FC(20, DDR, 800000000),

	FXOSC_PERIPH_CMU_FC_RANGE(21, GMAC_0_RX, 2500000, 125000000),
	FXOSC_PERIPH_CMU_FC(22, SPI, 100000000),
	FXOSC_PERIPH_CMU_FC(27, A53_CORE, 1300000000),

	FIRC_PERIPH_CMU_FC(28, A53_CORE, 1300000000),

	FXOSC_PERIPH_CMU_FC(39, PFE_SYS, 300000000),
	FXOSC_PERIPH_CMU_FC_RANGE(46, PFE_MAC_0_TX, 2500000, 312500000),
	FXOSC_PERIPH_CMU_FC_RANGE(47, PFE_MAC_0_RX, 2500000, 312500000),
	FXOSC_PERIPH_CMU_FC_RANGE(48, PFE_MAC_1_TX, 2500000, 312500000),
	FXOSC_PERIPH_CMU_FC_RANGE(49, PFE_MAC_1_RX, 2500000, 312500000),
	FXOSC_PERIPH_CMU_FC_RANGE(50, PFE_MAC_2_TX, 2500000, 125000000),
	FXOSC_PERIPH_CMU_FC_RANGE(51, PFE_MAC_2_RX, 2500000, 125000000),
};

static struct cmu s32r45_cmu_blocks[] = {
	FIRC_PERIPH_CMU_FC(0, FXOSC, FXOSC_FREQ),

	/* +-5% drift due to silicon and temperature factors  */
	FXOSC_PERIPH_CMU_FM_RANGE(1, FIRC, FIRC_MIN_FREQ, FIRC_MAX_FREQ),
	FXOSC_PERIPH_CMU_FM(2, SIRC, SIRC_FREQ),
	FXOSC_PERIPH_CMU_FM(3, FTM_0_REF, 40000000),
	FXOSC_PERIPH_CMU_FM(4, FTM_1_REF, 40000000),

	FIRC_PERIPH_CMU_FC(5, XBAR_DIV3, 133333333),
	FIRC_PERIPH_CMU_FC(6, XBAR_M7_0, 400000000),

	FXOSC_PERIPH_CMU_FC(7, XBAR_DIV3, 133333333),

	FIRC_PERIPH_CMU_FC(8, XBAR_M7_1, 400000000),
	FIRC_PERIPH_CMU_FC(9, XBAR_M7_2, 400000000),
	FIRC_PERIPH_CMU_FC(10, PER, 80000000),

	FXOSC_PERIPH_CMU_FC_RANGE(11, SERDES_REF, 100000000, 125000000),
	FXOSC_PERIPH_CMU_FC(12, FLEXRAY_PE, 40000000),
	FXOSC_PERIPH_CMU_FC(13, CAN_PE, 80000000),
	FXOSC_PERIPH_CMU_FC(14, GMAC_0_TX, 125000000),
	FXOSC_PERIPH_CMU_FC(15, GMAC_TS, 200000000),
	FXOSC_PERIPH_CMU_FC(16, LIN, 125000000),
	FXOSC_PERIPH_CMU_FC_RANGE(17, QSPI_1X, 0, 100000000),
	FXOSC_PERIPH_CMU_FC(18, SDHC, 400000000),

	FIRC_PERIPH_CMU_FC(20, DDR, 800000000),

	FXOSC_PERIPH_CMU_FC_RANGE(21, GMAC_0_RX, 2500000, 125000000),
	FXOSC_PERIPH_CMU_FC(22, SPI, 100000000),
	FXOSC_PERIPH_CMU_FC(27, A53_CORE, 800000000),

	FIRC_PERIPH_CMU_FC(28, A53_CORE, 800000000),

	FXOSC_PERIPH_CMU_FC(38, SPT, 600000000),
	FXOSC_PERIPH_CMU_FC(39, LAX0, 400000000),
	FXOSC_PERIPH_CMU_FC(40, LAX1, 400000000),
	FXOSC_PERIPH_CMU_FC(46, GMAC_1_TX, 125000000),
	FXOSC_PERIPH_CMU_FC(51, GMAC_1_RX, 125000000),
	FXOSC_PERIPH_CMU_FC(52, MIPICSI2_0, 400000000),
	FXOSC_PERIPH_CMU_FC(53, MIPICSI2_0, 400000000),
	FXOSC_PERIPH_CMU_FC(54, SERDES_REF, 125000000),
};

static struct s32cc_cmu_data s32g2_cmu_data = {
	.cmu_blocks = s32g2_cmu_blocks,
	.n_blocks   = ARRAY_SIZE(s32g2_cmu_blocks),
};

static struct s32cc_cmu_data s32g3_cmu_data = {
	.cmu_blocks = s32g3_cmu_blocks,
	.n_blocks   = ARRAY_SIZE(s32g3_cmu_blocks),
};

static struct s32cc_cmu_data s32r45_cmu_data = {
	.cmu_blocks = s32r45_cmu_blocks,
	.n_blocks   = ARRAY_SIZE(s32r45_cmu_blocks),
};

static u64 get_min_freq(u64 clk, u64 variation)
{
	return clk - div_u64(clk, 1000) * variation;
}

static u64 get_max_freq(u64 clk, u64 variation)
{
	return clk + div_u64(clk, 1000) * variation;
}

static int calc_cmu_ref_cnt(u64 ref_clk, u64 mon_clk,
			    u64 *ref_cnt)
{
	u64 max_iter, d_cnt;
	u32 remainder;

	d_cnt = div_u64_rem(ref_clk, mon_clk, &remainder);
	if (remainder > 0)
		d_cnt++;

	if ((u32)(d_cnt) > CMU_FC_RCCR_REF_CNT_MAX) {
		pr_err("REF_CNT (0x%" PRIx32 ") > max value\n", (u32)(d_cnt));
		return -EINVAL;
	}

	*ref_cnt = d_cnt;
	/* Maximum accuracy */
	max_iter = div_u64(CMU_FC_RCCR_REF_CNT_MAX, *ref_cnt);
	*ref_cnt *= max_iter;

	/* Overflow */
	if ((u64)*ref_cnt < d_cnt) {
		pr_err("REF_CNT overflow\n");
		return -EINVAL;
	}

	return 0;
}

static int get_fm_params(u64 ref_clk, u64 mon_clk,
			 struct cmu_fm_params *conf)
{
	return calc_cmu_ref_cnt(ref_clk, mon_clk, &conf->ref_cnt);
}

static int get_fc_params(u64 ref_clk, u64 mon_clk,
			 u64 ref_var, u64 mon_var,
			 struct cmu_params *conf)
{
	u64 min_ref = get_min_freq(ref_clk, ref_var);
	u64 max_ref = get_max_freq(ref_clk, ref_var);
	u64 min_mon = get_min_freq(mon_clk, mon_var);
	u64 max_mon = get_max_freq(mon_clk, mon_var);
	u64 href, lref;
	u32 remainder;
	int ret;

	ret = calc_cmu_ref_cnt(ref_clk, mon_clk, &conf->ref_cnt);
	if (ret)
		return ret;

	conf->udelay = div_u64_rem(conf->ref_cnt * MHZ_1, ref_clk,
				   &remainder);
	if (remainder >= div_u64(ref_clk, 2))
		conf->udelay++;

	href = div_u64(conf->ref_cnt * max_mon, min_ref);
	if ((u64)(href) > CMU_FC_TCR_FREF_MAX) {
		pr_err("HREF (0x%" PRIx64 ") > max value\n", (u64)(href));
		return -EINVAL;
	}

	conf->href = href;
	/* Overflow */
	if ((u64)conf->href < (u64)href) {
		pr_err("HREF overflow\n");
		return -EINVAL;
	}

	lref = div_u64(conf->ref_cnt * min_mon, max_ref);
	if ((u64)(lref) > CMU_FC_TCR_FREF_MAX) {
		pr_err("LREF (0x%" PRIx64 ") > max value\n", (u64)(lref));
		return -EINVAL;
	}

	conf->lref = lref;
	/* Overflow */
	if ((u64)conf->href < (u64)href) {
		pr_err("LREF overflow\n");
		return -EINVAL;
	}

	return 0;
}

static enum fc_result fc_check_frequency(uintptr_t addr,
					 struct cmu_params *params)
{
	u32 fc_sr_val;

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

	if (read_poll_timeout(readl, CMU_FC_SR(addr), fc_sr_val,
			      (fc_sr_val & CMU_FC_SR_RS),
				  CMU_FC_WAIT, CMU_FC_TIMEOUT))
		return (enum fc_result)(TIMEOUT);

	udelay(params->udelay * 3);

	/* Disable the module */
	writel(0x0, CMU_FC_GCR(addr));

	return (enum fc_result)(readl(CMU_FC_SR(addr)) & CMU_FC_SR_FLAGS);
}

static int _get_fc_mon_freq(struct cmu *s32cc_cmu, uintptr_t addr,
			    struct freq_interval *freq_int, u32 depth)
{
	struct cmu_params params;
	enum fc_result res;
	u64 mon_freq;
	int ret;

	if (depth == 0)
		return 0;

	mon_freq = div_u64(freq_int->min + freq_int->max, 2);

	/* Assume 0 if the frequency is lower than 10KHz */
	if (mon_freq < KHZ_10)
		return 0;

	ret = get_fc_params(s32cc_cmu->ref_freq, mon_freq,
			    s32cc_cmu->ref_var, s32cc_cmu->mon_var,
			    &params);
	if (ret) {
		pr_err("Failed to determine CMU_FC parameters for clock: %s\n",
		       s32cc_cmu->mon_name);
		return ret;
	}

	depth--;

	res = fc_check_frequency(addr, &params);
	switch (res) {
	case HIGHER:
		freq_int->min = mon_freq;
		return _get_fc_mon_freq(s32cc_cmu, addr, freq_int, depth);
	case LOWER:
		freq_int->max = mon_freq;
		return _get_fc_mon_freq(s32cc_cmu, addr, freq_int, depth);
	case TIMEOUT:
		pr_err("Timeout while measuring the frequency of %s\n",
		       s32cc_cmu->mon_name);
		return -ETIMEDOUT;
	default:
		freq_int->min = mon_freq;
		freq_int->max = mon_freq;
		return 0;
	}
}

static int get_fc_mon_freq(struct cmu *s32cc_cmu, uintptr_t addr,
			   struct freq_interval *freq_int)
{
	freq_int->min = 0;
	freq_int->max = MAX_PERIPH_FREQ;
	return _get_fc_mon_freq(s32cc_cmu, addr, freq_int, MAX_DEPTH);
}

static u64 get_max_exp_freq(struct cmu *s32cc_cmu)
{
	if (s32cc_cmu->has_exp_range)
		return s32cc_cmu->exp_range.max;

	return s32cc_cmu->exp_freq;
}

static int get_fm_mon_freq(struct cmu *s32cc_cmu, uintptr_t addr,
			   u64 *mon_freq)
{
	struct cmu_fm_params cmu_fm;
	u32 met_cnt, sr;
	int ret;

	ret = get_fm_params(s32cc_cmu->ref_freq, get_max_exp_freq(s32cc_cmu),
			    &cmu_fm);
	if (ret)
		return ret;

	/* Disable the module*/
	writel(0x0, CMU_FM_GCR(addr));

	/* Clear flags */
	writel(CMU_FM_SR_FMTO | CMU_FM_SR_FMC, CMU_FM_SR(addr));

	/* Sampling period */
	writel(cmu_fm.ref_cnt, CMU_FM_RCCR(addr));
	/* Start the measurement */
	writel(CMU_FM_GCR_FME, CMU_FM_GCR(addr));

	while (readl(CMU_FM_SR(addr)) &
	       (CMU_FM_SR_FMTO | CMU_FM_SR_FMC))
		;

	do {
		sr = readl(CMU_FM_SR(addr));
		if (sr & CMU_FM_SR_FMTO) {
			pr_err("Timeout while measuring the frequency of %s\n",
			       s32cc_cmu->mon_name);
			return -ETIMEDOUT;
		}
	} while (!CMU_FM_SR_MET_CNT(sr));

	/* Disable the module*/
	writel(0x0, CMU_FM_GCR(addr));

	met_cnt = CMU_FM_SR_MET_CNT(sr);

	*mon_freq = met_cnt * div_u64(s32cc_cmu->ref_freq, cmu_fm.ref_cnt);

	return 0;
}

static int s32cc_cmu_read(struct udevice *dev, int offset,
			  void *buf, int size)
{
	struct s32cc_cmu *priv = dev_get_priv(dev);
	struct s32cc_cmu_data *data = priv->data;
	struct cmu_result *result = buf;
	struct freq_interval freq_int;
	struct cmu *s32cc_cmu;
	u64 mon_freq = 0;
	uintptr_t addr;

	debug("%s(dev=%p)\n", __func__, dev);

	if (!size || offset >= data->n_blocks)
		return 0;

	if (size != sizeof(*result))
		return -EINVAL;

	s32cc_cmu = &data->cmu_blocks[offset];
	addr = (uintptr_t)priv->base_addr + s32cc_cmu->offset;

	if (s32cc_cmu->fc) {
		get_fc_mon_freq(s32cc_cmu, addr, &freq_int);
	} else {
		get_fm_mon_freq(s32cc_cmu, addr, &mon_freq);
		freq_int.min = mon_freq;
		freq_int.max = mon_freq;
	}

	strlcpy(result->mon_clk_name, s32cc_cmu->mon_name,
		sizeof(result->mon_clk_name));
	strlcpy(result->ref_clk_name, s32cc_cmu->ref_name,
		sizeof(result->ref_clk_name));

	result->id = s32cc_cmu->offset >> 5;

	if (s32cc_cmu->has_exp_range) {
		result->expected.min = s32cc_cmu->exp_range.min;
		result->expected.max = s32cc_cmu->exp_range.max;
	} else {
		result->expected.min = s32cc_cmu->exp_freq;
		result->expected.max = s32cc_cmu->exp_freq;
	}

	result->measured = freq_int;

	return size;
}

static int s32cc_cmu_probe(struct udevice *dev)
{
	struct s32cc_cmu *priv = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	priv->base_addr = (void __iomem *)dev_read_addr_ptr(dev);
	if (!priv->base_addr)
		return -EINVAL;

	priv->data = (struct s32cc_cmu_data *)dev_get_driver_data(dev);

	ret = clk_get_by_index(dev, 0, &priv->cmu_clk_module);
	if (ret)
		return ret;

	ret = clk_enable(&priv->cmu_clk_module);
	if (ret)
		return ret;

	ret = clk_get_by_index(dev, 1, &priv->cmu_clk_reg);
	if (ret)
		return ret;

	return clk_enable(&priv->cmu_clk_reg);
}

static struct misc_ops s32cc_cmu_ops = {
	.read = s32cc_cmu_read,
};

static const struct udevice_id s32cc_cmu_ids[] = {
	{ .compatible = "nxp,s32g2-cmu", .data = (ulong)&s32g2_cmu_data },
	{ .compatible = "nxp,s32g3-cmu", .data = (ulong)&s32g3_cmu_data },
	{ .compatible = "nxp,s32r45-cmu", .data = (ulong)&s32r45_cmu_data },
	{ }
};

U_BOOT_DRIVER(s32_cmu) = {
	.name		= "s32_cmu",
	.id		= UCLASS_MISC,
	.of_match	= s32cc_cmu_ids,
	.probe		= s32cc_cmu_probe,
	.ops		= &s32cc_cmu_ops,
	.priv_auto	= sizeof(struct s32cc_cmu),
};

static void print_u64_mhz(u64 val, int space)
{
	u32 dec_part = 0;
	u32 int_part = 0;
	u32 digit = 0;
	char buff[20];
	u32 mul = 1;
	int i;

	val /= 1000;

	for (i = 0; i < 3; i++) {
		digit = val % 10;
		val /= 10;
		dec_part += mul * digit;
		mul *= 10;
	}

	digit = 0;
	mul = 1;
	do {
		digit = val % 10;
		val /= 10;
		int_part += mul * digit;
		mul *= 10;
	} while (val);

	space -= 8;
	if (space >= 0 && space < sizeof(buff)) {
		memset(buff, ' ', space);
		buff[space] = '\0';
	} else {
		buff[0] = '\0';
	}

	printf("%s%4u.%03u", buff, int_part, dec_part);
}

static void print_freq_interval(struct freq_interval *freq_int)
{
	if (freq_int->min != freq_int->max) {
		print_u64_mhz(freq_int->min, 8);
		printf(" - ");
		print_u64_mhz(freq_int->max, 8);
	} else {
		print_u64_mhz(freq_int->min, 19);
	}
}

static int do_verifclk(struct cmd_tbl *cmdtp, int flag, int argc,
		       char * const argv[])
{
	struct cmu_result result;
	struct udevice *cmu;
	int i = 0;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MISC,
					  DM_DRIVER_GET(s32_cmu), &cmu);
	if (ret)
		return ret;

	puts(" CMU | Monitored    | Reference | Expected            |");
	puts(" Verified\n");
	puts(" ID  | clock        | clock     | range (MHz)         |");
	puts(" range (MHz)\n");
	puts("-----|--------------|-----------|---------------------|");
	puts("--------------------\n");

	while (misc_read(cmu, i++, &result, sizeof(result)) > 0) {
		printf("%5d|", result.id);
		printf(" %12s | ", result.mon_clk_name);
		printf("%9s | ", result.ref_clk_name);
		print_freq_interval(&result.expected);
		puts(" | ");
		print_freq_interval(&result.measured);
		putc('\n');
	}

	return 0;
}

U_BOOT_CMD(verifclk, CONFIG_SYS_MAXARGS, 1, do_verifclk,
	   "Verifies clocks frequencies using CMU module",
	   NULL
);
