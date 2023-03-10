// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2022-2023 NXP
 * S32CC SAR-ADC driver
 */

#include <common.h>
#include <adc.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#define SARADC_MCR	0x0
#define SARADC_MSR	0x4
#define SARADC_ISR	0x10
#define SARADC_CTR0	0x94
#define SARADC_NCMR0	0xA4
#define SARADC_PCDR(x)	(0x100 + (x) * 4)

#define SARADC_MCR_PWDN			BIT(0)
#define SARADC_MCR_ADCLKSE		BIT(8)
#define SARADC_MCR_TSAMP_MASK	GENMASK(10, 9)
#define SARADC_MCR_AVGEN		BIT(13)
#define SARADC_MCR_CALSTART		BIT(14)
#define SARADC_MCR_NSTART		BIT(24)
#define SARADC_MCR_SCAN_MODE	BIT(29)
#define SARADC_MCR_WLSIDE		BIT(30)
#define SARADC_MCR_OWREN		BIT(31)

#define SARADC_MSR_CALBUSY		BIT(29)
#define SARADC_MSR_CALFAIL		BIT(30)

#define SARADC_ISR_ECH			BIT(0)

#define SARADC_CTR0_INPSAMP(x)	(x)

#define SARADC_PCDR_VALID		BIT(19)
#define SARADC_PCDR_CDATA(x)	((x) & 0xfff)

#define SARADC_NSEC_PER_SEC		1000000000
#define SARADC_WAIT				2000   /* us */
#define SARADC_TIMEOUT_VALUE	100000 /* us */
#define RDB_CHECK_REV_CH		5

struct s32_saradc_data {
	int num_bits;
	int num_channels;
};

static const struct s32_saradc_data saradc_data = {
	.num_bits = 12,
	.num_channels = 8,
};

struct s32_saradc_priv {
	int active_channel;
	const struct s32_saradc_data *data;
	ulong base;
	ulong clk_rate;
};

static void s32_saradc_powerdown(ulong base)
{
	u32 tmp;

	tmp = readl(base + SARADC_MCR) | SARADC_MCR_PWDN;
	writel(tmp, base + SARADC_MCR);
}

static void s32_saradc_powerup(ulong base)
{
	u32 tmp;

	tmp = readl(base + SARADC_MCR) & ~SARADC_MCR_PWDN;
	writel(tmp, base + SARADC_MCR);
}

static int s32_saradc_calibration(ulong base,
				  struct adc_uclass_plat *pdata)
{
	u32 tmp;

	s32_saradc_powerdown(base);

	/* Configure clock = bus_clock / 2 */
	tmp = readl(base + SARADC_MCR) & ~SARADC_MCR_ADCLKSE;
	writel(tmp, base + SARADC_MCR);

	s32_saradc_powerup(base);

	tmp = readl(base + SARADC_MCR);
	tmp |= SARADC_MCR_AVGEN;
	tmp &= ~SARADC_MCR_TSAMP_MASK;
	tmp |= SARADC_MCR_CALSTART;
	writel(tmp, base + SARADC_MCR);

	if (read_poll_timeout(readl, base + SARADC_MSR, tmp,
			      !(tmp & SARADC_MSR_CALBUSY), SARADC_WAIT,
			      pdata->data_timeout_us))
		return -EINVAL;

	if (tmp & SARADC_MSR_CALFAIL)
		return -EINVAL;

	return 0;
}

static int s32_adc_channel_data(struct udevice *dev, int channel,
				unsigned int *data)
{
	u32 tmp;
	struct s32_saradc_priv *priv = dev_get_priv(dev);
	struct adc_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);

	if (!priv)
		return -EINVAL;

	if (channel != priv->active_channel) {
		printf("Requested channel: %d is not active!\n", channel);
		return -EINVAL;
	}

	if (!(readl(priv->base + SARADC_ISR) & SARADC_ISR_ECH))
		return -EBUSY;

	/* clear status */
	writel(SARADC_ISR_ECH, priv->base + SARADC_ISR);

	tmp = readl(priv->base + SARADC_PCDR(channel));
	if (!(tmp & SARADC_PCDR_VALID))
		return -EBUSY;

	/* Read value */
	*data = SARADC_PCDR_CDATA(tmp);
	*data &= uc_pdata->data_mask;

	return 0;
}

static int s32_adc_start_channel(struct udevice *dev, int channel)
{
	struct s32_saradc_priv *priv = dev_get_priv(dev);
	u32 tmp;

	if (!priv)
		return -EINVAL;

	if (channel < 0 || channel >= priv->data->num_channels) {
		printf("Requested channel: %d is invalid\n", channel);
		return -EINVAL;
	}

	s32_saradc_powerup(priv->base);

	writel(BIT(channel), priv->base + SARADC_NCMR0);

	/* Ensure there are at least three cycles between the
	 * configuration of NCMR and the setting of NSTART
	 */
	ndelay((SARADC_NSEC_PER_SEC / (priv->clk_rate >> 1)) * 3);

	tmp = readl(priv->base + SARADC_MCR);
	tmp |= SARADC_MCR_OWREN;
	tmp &= ~SARADC_MCR_WLSIDE;
	tmp &= ~SARADC_MCR_SCAN_MODE;

	tmp |= SARADC_MCR_NSTART;
	writel(tmp, priv->base + SARADC_MCR);

	priv->active_channel = channel;

	return 0;
}

static int s32_adc_stop(struct udevice *dev)
{
	struct s32_saradc_priv *priv = dev_get_priv(dev);

	if (!priv)
		return -EINVAL;

	s32_saradc_powerdown(priv->base);
	priv->active_channel = -1;

	return 0;
}

static int s32_saradc_probe(struct udevice *dev)
{
	struct s32_saradc_priv *priv = dev_get_priv(dev);
	struct adc_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);
	struct clk clk;
	u32 tmp;
	int ret;

	if (!priv)
		return -EINVAL;

	ret = clk_get_by_index(dev, 0, &clk);
	if (ret)
		return ret;

	ret = clk_enable(&clk);
	if (ret) {
		printf("Failed to enable saradc clock: %d\n", ret);
		return ret;
	}

	priv->clk_rate = clk_get_rate(&clk);
	if (!priv->clk_rate) {
		printf("Invalid clk rate: %lu\n", priv->clk_rate);
		return -EINVAL;
	}

	ret = s32_saradc_calibration(priv->base, uc_pdata);
	s32_saradc_powerdown(priv->base);
	if (ret) {
		printf("SARADC calibration failed\n");
		return ret;
	}

	tmp = readl(priv->base + SARADC_MCR) | SARADC_MCR_ADCLKSE;
	writel(tmp, priv->base + SARADC_MCR);

	writel(SARADC_CTR0_INPSAMP(0xFF), priv->base + SARADC_CTR0);

	priv->active_channel = -1;

	return 0;
}

static int s32_saradc_ofdata_to_plat(struct udevice *dev)
{
	struct adc_uclass_plat *uc_pdata = dev_get_uclass_plat(dev);
	struct s32_saradc_priv *priv = dev_get_priv(dev);

	if (!priv)
		return -EINVAL;

	priv->base = (ulong)dev_read_addr(dev);
	if (priv->base == (ulong)FDT_ADDR_T_NONE)
		return -EINVAL;

	priv->data = (struct s32_saradc_data *)dev_get_driver_data(dev);

	uc_pdata->data_mask = GENMASK(priv->data->num_bits - 1, 0);
	uc_pdata->data_format = ADC_DATA_FORMAT_BIN;
	uc_pdata->data_timeout_us = SARADC_TIMEOUT_VALUE;
	uc_pdata->channel_mask = GENMASK(priv->data->num_channels - 1, 0);

	return 0;
}

static const struct adc_ops s32_saradc_ops = {
	.start_channel = s32_adc_start_channel,
	.channel_data = s32_adc_channel_data,
	.stop = s32_adc_stop,
};

static const struct udevice_id s32_saradc_ids[] = {
	{ .compatible = "nxp,s32cc-adc", .data = (ulong)&saradc_data },
	{ }
};

U_BOOT_DRIVER(s32_saradc) = {
	.name = "s32-saradc",
	.id = UCLASS_ADC,
	.of_match = s32_saradc_ids,
	.ops = &s32_saradc_ops,
	.probe = s32_saradc_probe,
	.of_to_plat = s32_saradc_ofdata_to_plat,
	.priv_auto = sizeof(struct s32_saradc_priv),
};
