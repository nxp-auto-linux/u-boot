// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2022 NXP
 */
#include <common.h>
#include <adc.h>
#include <env.h>
#include <dm/uclass.h>

#define SARADC0_CH5			5
#define SARADC0_TOLERANCE	200

static const struct {
	const char *rev;
	const char *desc;
	u32 lower;
	u32 upper;
} rdb_revisions[] = {
	{ /* 0V */
		.rev = "",
		.desc = "RDB",
		.lower = 0,
		.upper = 400,
	},
	{ /* 0.8v */
		.rev = "C",
		.desc = "RDB2/GLDBOX Revision C",
		.lower = 1820 - SARADC0_TOLERANCE,
		.upper = 1820 + SARADC0_TOLERANCE,
	},
	{ /* 1.0v */
		.rev = "D",
		.desc = "RDB2/GLDBOX Revision D",
		.lower = 2275 - SARADC0_TOLERANCE,
		.upper = 2275 + SARADC0_TOLERANCE,
	},
	{ /* 1.2v */
		.rev = "E",
		.desc = "RDB3",
		.lower = 2730 - SARADC0_TOLERANCE,
		.upper = 2730 + SARADC0_TOLERANCE,
	},
};

static int find_rdb_rev(uint32_t adc_value)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(rdb_revisions); i++) {
		if (rdb_revisions[i].lower <= adc_value &&
		    adc_value <= rdb_revisions[i].upper)
			return i;
	}

	return -EINVAL;
}

int board_late_init(void)
{
	int rev_idx;
	u32 adc_value;
	struct udevice *saradc0;
	int ret;

	ret = uclass_get_device_by_seq(UCLASS_ADC, 0, &saradc0);
	if (ret) {
		printf("%s: No SARADC0 (err = %d)\n", __func__, ret);
		return ret;
	}

	ret = adc_start_channel(saradc0, SARADC0_CH5);
	if (ret) {
		printf(":%s: Error on starting SARADC0 channel 5 (err = %d)\n",
		       __func__, ret);
		return ret;
	}

	ret = adc_channel_data(saradc0, SARADC0_CH5, &adc_value);
	if (ret) {
		printf(":%s: Error on reading value from SARADC0 channel 5 (err = %d)\n",
		       __func__, ret);
		return ret;
	}

	ret = adc_stop(saradc0);
	if (ret) {
		printf(":%s: Error on stopping SARADC0 channel 5 (err = %d)\n",
		       __func__, ret);
		return ret;
	}

	rev_idx = find_rdb_rev(adc_value);
	if (rev_idx >= 0) {
		env_set("board_rev", rdb_revisions[rev_idx].rev);
		switch (rdb_revisions[rev_idx].rev[0]) {
		case 'C':
			env_set("pfe1_phy_addr", "3");
			break;
		case 'D':
			env_set("pfe1_phy_addr", "8");
			break;
		case 'E':
			env_set("pfe1_phy_addr", "8");
			break;
		default:
			env_set("pfe1_phy_addr", NULL);
			break;
		}
		printf("Board revision:\t%s\n", rdb_revisions[rev_idx].desc);
		return 0;
	}

	env_set("board_rev", NULL);
	env_set("pfe1_phy_addr", NULL);
	printf("Board revision:\tRevision Unknown: (0x%x)\n", adc_value);
	return 0;
}
