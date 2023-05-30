// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright 2018-2023 NXP
 */
#include <common.h>
#include <adc.h>
#include <env.h>
#include <dm/uclass.h>
#include <s32-cc/nvmem.h>
#include <dt-bindings/nvmem/s32cc-siul2-nvmem.h>

#define SARADC0_CH5		5
#define SARADC0_TOLERANCE	200

/**
 * enum board_type - RDB board type
 * @RDB: unknown RDB board
 * @RDB2: S32G-VNP-RDB2
 * @RDB3: S32G-VNP-RDB3
 */
enum board_type {
	RDB = 0,
	RDB2 = 2,
	RDB3 = 3,
};

struct rdb_revisions {
	const char *rev;
	const char *desc;
	u32 lower;
	u32 upper;
};

static const struct rdb_revisions rdb2_revisions[] = {
	{ /* 0V */
		.rev = "",
		.desc = "RDB2",
		.lower = 0,
		.upper = 400,
	}, { /* 0.8V */
		.rev = "C",
		.desc = "RDB2/GLDBOX Revision C",
		.lower = 1820 - SARADC0_TOLERANCE,
		.upper = 1820 + SARADC0_TOLERANCE,
	}, { /* 1.0V */
		.rev = "D",
		.desc = "RDB2/GLDBOX Revision D",
		.lower = 2275 - SARADC0_TOLERANCE,
		.upper = 2275 + SARADC0_TOLERANCE,
	}, { /* 1.2V */
		.rev = "E",
		.desc = "RDB2/GLDBOX Revision E",
		.lower = 2730 - SARADC0_TOLERANCE,
		.upper = 2730 + SARADC0_TOLERANCE,
	},
};

static const struct rdb_revisions rdb3_revisions[] = {
	{ /* 0V */
		.rev = "",
		.desc = "RDB3",
		.lower = 0,
		.upper = 400,
	}, { /* 1.2V */
		.rev = "E",
		.desc = "RDB3 Revision E",
		.lower = 2730 - SARADC0_TOLERANCE,
		.upper = 2730 + SARADC0_TOLERANCE,
	}, { /* 1.4V */
		.rev = "F",
		.desc = "RDB3 Revision F",
		.lower = 3185 - SARADC0_TOLERANCE,
		.upper = 3185 + SARADC0_TOLERANCE,
	},
};

static int get_board_type(enum board_type *board)
{
	int ret;
	const char *dev_name = "siul2_0_nvram";
	struct udevice *siul20_nvmem = NULL;
	u32 part_number = 0;
	struct nvmem_cell cell;

	if (!board)
		return -EINVAL;

	ret = uclass_get_device_by_name(UCLASS_MISC, dev_name, &siul20_nvmem);
	if (ret)
		goto nvmem_err;

	ret = nvmem_cell_get_by_offset(siul20_nvmem, S32CC_SOC_PART_NO, &cell);
	if (ret)
		goto nvmem_err;

	ret = nvmem_cell_read(&cell, &part_number, sizeof(part_number));
	if (ret)
		goto nvmem_err;

	/* extract SoC family, i.e. S32G2/S32G3 */
	while (part_number / 10)
		part_number /= 10;

	switch (part_number) {
	case 2:
		*board = RDB2;
		break;
	case 3:
		*board = RDB3;
		break;
	default:
		*board = RDB;
		printf(":%s: Invalid part number\n", __func__);
		break;
	}

	return 0;
nvmem_err:
	printf(":%s: Failed to read SIUL2 NVMEM (err = %d)\n", __func__, ret);
	return ret;
}

static int get_adc_value(u32 *adc_value)
{
	struct udevice *saradc0;
	int ret;

	if (!adc_value)
		return -EINVAL;

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

	ret = adc_channel_data(saradc0, SARADC0_CH5, adc_value);
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

	return 0;
}

static struct rdb_revisions *find_rdb_rev(enum board_type board, u32 adc_value)
{
	const struct rdb_revisions *array;
	size_t array_size;
	size_t i;

	switch (board) {
	case RDB2:
		array = rdb2_revisions;
		array_size = ARRAY_SIZE(rdb2_revisions);
		break;
	case RDB3:
		array = rdb3_revisions;
		array_size = ARRAY_SIZE(rdb3_revisions);
		break;
	default:
		return NULL;
	}

	for (i = 0; i < array_size; i++)
		if (array[i].lower <= adc_value && adc_value <= array[i].upper)
			return (struct rdb_revisions *)&array[i];

	return NULL;
}

int board_late_init(void)
{
	enum board_type board;
	u32 adc_value = 0;
	struct rdb_revisions *rdb;
	int ret;

	printf("Board revision:\t");

	ret = get_board_type(&board);
	if (ret)
		goto find_err;

	ret = get_adc_value(&adc_value);
	if (ret)
		goto find_err;

	rdb = find_rdb_rev(board, adc_value);
	if (!rdb)
		goto find_err;

	env_set("board_rev", rdb->rev);
	switch (rdb->rev[0]) {
	case 'C':
		env_set("pfe1_phy_addr", "3");
		break;
	case 'D':
	case 'E':
	case 'F':
		env_set("pfe1_phy_addr", "8");
		break;
	default:
		env_set("pfe1_phy_addr", NULL);
		break;
	}
	printf("%s\n", rdb->desc);

	return 0;

find_err:
	env_set("board_rev", NULL);
	env_set("pfe1_phy_addr", NULL);
	printf("Revision Unknown: (0x%x)\n", adc_value);

	return 0;
}
