// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2025 NXP
 */

#include <common.h>
#include <dm.h>
#include <misc.h>
#include <s32n79-scmi-misc.h>
#include <soc.h>
#include <dm/device_compat.h>
#include <dm/uclass.h>
#include <linux/bitfield.h>

#define FIRST_PROD_LETTER_MASK		GENMASK(23, 16)
#define SECOND_PROD_LETTER_MASK		GENMASK(15, 8)
#define THIRD_PROD_LETTER_MASK		GENMASK(7, 0)

#define SCMI_MISC_NAME		"protocol@86"

struct soc_s32n_priv {
	s32 status;
	u32 prod_status_and_brand;
	u32 family;
	u32 series;
	u32 arm_cores_and_sram;
	u32 device_variant;
	u32 device_options;
	u32 security;
	u32 arm_core_speeds;
	u32 fab_and_mask_rev;
	u32 temp_range;
	u32 package_code;
	u32 shipping_method;
};

static int soc_s32n_get_machine(struct udevice *dev,
				char *buf, int size)
{
	struct soc_s32n_priv *priv = dev_get_priv(dev);
	int ret;

	if (!priv)
		return -ENODEV;

	ret = snprintf(buf, size, "%c%c%c", priv->series,
		       priv->arm_cores_and_sram, priv->device_options);
	if (ret >= size || ret < 0)
		return -ENOSPC;

	return 0;
}

static int soc_s32n_get_family(struct udevice *dev,
			       char *buf, int size)
{
	struct soc_s32n_priv *priv = dev_get_priv(dev);
	u32 prod_letters[3];
	int ret;

	if (!priv)
		return -ENODEV;

	prod_letters[0] = FIELD_GET(FIRST_PROD_LETTER_MASK,
				    priv->prod_status_and_brand);
	prod_letters[1] = FIELD_GET(SECOND_PROD_LETTER_MASK,
				    priv->prod_status_and_brand);
	prod_letters[2] = FIELD_GET(THIRD_PROD_LETTER_MASK,
				    priv->prod_status_and_brand);

	ret = snprintf(buf, size, "NXP %c%c%c%c", prod_letters[0],
		       prod_letters[1], prod_letters[2], priv->family);
	if (ret >= size || ret < 0)
		return -ENOSPC;

	return 0;
}

static const struct soc_ops soc_s32n_ops = {
	.get_machine = soc_s32n_get_machine,
	.get_family = soc_s32n_get_family,
};

static int soc_s32n_probe(struct udevice *dev)
{
	struct soc_s32n_priv *priv = dev_get_priv(dev);
	struct udevice *misc_dev = NULL;
	int ret;

	if (!priv)
		return -EINVAL;

	ret = uclass_get_device_by_name(UCLASS_MISC,
					SCMI_MISC_NAME, &misc_dev);
	if (ret) {
		dev_err(dev, "MISC SCMI device cannot be found, ret: %d\n", ret);
		return ret;
	}

	ret = misc_read(misc_dev, S32N79_SOC_INFO_OFFSET,
			priv, S32N79_SOC_INFO_SIZE);
	if (ret < 0) {
		dev_err(dev, "Failed to read SoC Misc, err: %d", ret);
		return ret;
	}

	return 0;
}

static const struct udevice_id soc_s32n_ids[] = {
	{ .compatible = "nxp,s32n79-soc" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(soc_s32n_drv) = {
	.name = "soc_s32n",
	.id = UCLASS_SOC,
	.ops = &soc_s32n_ops,
	.of_match = soc_s32n_ids,
	.probe = soc_s32n_probe,
	.priv_auto = sizeof(struct soc_s32n_priv),
	.flags = DM_FLAG_PRE_RELOC,
};
