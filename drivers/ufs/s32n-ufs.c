// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright 2024-2026 NXP
 */

#include <clk.h>
#include <config.h>
#include <errno.h>
#include <linux/types.h>
#include <linux/string.h>
#include <stdio.h>
#include <vsprintf.h>
#include <dm.h>
#include <scsi.h>
#include <ufs.h>
#include <linux/bitfield.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/time.h>
#include <dm/device_compat.h>

#include "ufs.h"
#include "ufshci-dwc.h"
#include "ufshcd-dwc.h"

/* MPHY register definitions */
#define MPHY_REG_LSB_MASK			0x00FF
#define MPHY_REG_MSB_MASK			0xFF00

/* MPHY specific register addresses */
#define RAWLANEN_DIG_PCS_XF_RX_OVRD_IN_0	0x3006
#define RAWLANEN_DIG_PCS_XF_RX_OVRD_IN_1	0x3106

/* Vendor specific attributes */
#define CBRATESEL				0x8114

/* TX FSM states */
#define TX_FSM_HIBERN8				1

/* CPort definitions */
#define CPORT_0					0

/* SCM Register Offsets. */
#define SCM_ONE_US_TICK			0x2CU
#define SCM_STATUS			0x30

/* SCM_ONE_US_TICK Register. */
#define SCM_ONE_US_TICK_MASK		GENMASK(8, 0)

/* SCM_MPHY_RAM_CONFIG_STATUS Register Fields. */
#define SCM_STATUS_PHY_RESET_MASK		GENMASK(0, 0)
#define SCM_STATUS_SRAM_BYPASS_MASK		GENMASK(1, 1)
#define SCM_STATUS_SRAM_EXT_LD_DONE_MASK	GENMASK(2, 2)
#define SCM_STATUS_SRAM_INIT_DONE_MASK		GENMASK(16, 16)

#define SCM_STATUS_PHY_RESET(val)		\
	FIELD_PREP(SCM_STATUS_PHY_RESET_MASK, val)

#define SCM_STATUS_SRAM_BYPASS(val)		\
	FIELD_PREP(SCM_STATUS_SRAM_BYPASS_MASK, val)

/* Clock validation limits. */
#define MAX_VALID_ONE_US_TICK		SCM_ONE_US_TICK_MASK

#define HBRN8_POLL_TOUT_MS     1000

/**
 * struct s32n_ufs - S32N79 UFS host controller data
 * @hba: UFS host controller instance
 * @reg_scm: SCM register base address
 * @mphy_boot_mode: MPHY boot mode configuration
 */
struct s32n_ufs {
	struct ufs_hba *hba;
	void __iomem *reg_scm;
	struct clk core_clk;
	u8 mphy_boot_mode;
};

/**
 * enum mphy_boot_mode - MPHY boot mode options
 * @MPHY_BOOT_NONE: Skip MPHY initialization
 * @MPHY_BOOT_ROM: Running MPHY from internal FW ROM
 * @MPHY_BOOT_COPY_SRAM: Running MPHY from a (modified) FW ROM copy in external SRAM
 */
enum mphy_boot_mode {
	MPHY_BOOT_NONE = 0,
	MPHY_BOOT_ROM = 1,
	MPHY_BOOT_COPY_SRAM = 2,
};

static unsigned long ufs_s32n_calculate_us_tick(unsigned long clk_rate)
{
	return clk_rate / USEC_PER_SEC;
}

static int ufs_s32n_configure_clocks(struct ufs_hba *hba)
{
	struct s32n_ufs *ufs = dev_get_priv(hba->dev);
	struct udevice *dev = hba->dev;
	unsigned long clk_rate = 0;
	unsigned long one_us_tick;

	clk_rate = clk_get_rate(&ufs->core_clk);
	if (IS_ERR_VALUE(clk_rate)) {
		dev_err(dev, "Failed to get valid clock rate.\n");
		return -EINVAL;
	}

	one_us_tick = ufs_s32n_calculate_us_tick(clk_rate);
	if (one_us_tick == 0 || one_us_tick > MAX_VALID_ONE_US_TICK) {
		dev_err(dev, "Invalid one_us_tick value: %lu (clk_rate: %lu Hz).\n",
			one_us_tick, clk_rate);
		return -EINVAL;
	}

	dev_dbg(dev, "Core clock rate: %lu Hz, one_us_tick = %lu.\n",
		clk_rate, one_us_tick);

	/* Configure the micro-second tick rate generator based on core
	 * clock rate.
	 */
	writel(one_us_tick, ufs->reg_scm + SCM_ONE_US_TICK);
	ufshcd_dwc_program_clk_div(hba, one_us_tick);

	return 0;
}

static int ufs_s32n_phy_read_cr(struct ufs_hba *hba, u16 addr, u16 *value)
{
	unsigned int uic_arg3 = 0;
	int ret = 0;

	/* CR LSB parallel address - 0x8116. */
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBCREGADDRLSB, 0), addr & 0x00FF);
	/* CR MSB parallel address - 0x8117. */
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBCREGADDRMSB, 0), addr >> 8);
	/* CR parallel control value: 1- write, 0 - read - 0x811c. */
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBCREGRDWRSEL, 0), 0x0);
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VS_MPHYCFGUPDT, 0), 0x1);
	/* CR LSB parallel read value - 0x811a. */
	ret |= ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(CBCREGRDLSB, 0), &uic_arg3);
	*value = (u16)(uic_arg3 & MPHY_REG_LSB_MASK);
	/* CR MSB parallel read value - 0x811b. */
	ret |= ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(CBCREGRDMSB, 0), &uic_arg3);
	*value |= (u16)((uic_arg3 << 8) & MPHY_REG_MSB_MASK);

	return ret;
}

static int ufs_s32n_phy_write_cr(struct ufs_hba *hba, u16 addr, u16 value)
{
	int ret = 0;

	/* CR LSB parallel address - 0x8116. */
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBCREGADDRLSB, 0), addr & 0x00FF);
	/* CR MSB parallel address - 0x8117. */
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBCREGADDRMSB, 0), addr >> 8);
	/* CR LSB parallel write value - 0x8118. */
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBCREGWRLSB, 0), value & 0x00FF);
	/* CR MSB parallel write value - 0x8119. */
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBCREGWRMSB, 0), value >> 8);
	/* CR parallel control value: 1- write, 0 - read - 0x811c. */
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBCREGRDWRSEL, 0), 0x1);
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VS_MPHYCFGUPDT, 0), 0x1);

	return ret;
}

static int ufs_s32n_phy_sram_ext_loading(struct ufs_hba *hba)
{
	int ret = 0;
	u16 reg;

	ret |= ufs_s32n_phy_read_cr(hba, 0xC002, &reg);
	ret |= ufs_s32n_phy_write_cr(hba, 0xC002, reg);

	return ret;
}

static int ufs_s32n_phy_configure(struct ufs_hba *hba)
{
	int ret = 0;

	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBCRCTRL, 0), 0x1);
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VS_MPHYCFGUPDT, 0), 0x1);

	return ret;
}

static int ufs_s32n_hibern8(struct ufs_hba *hba, unsigned int num_lanes)
{
	bool success = false;
	unsigned long start;
	u32 tx_fsm_val = 0;
	unsigned int i;
	int err = 0;

	if (!num_lanes)
		return -EINVAL;

	start = get_timer(0);

	do {
		success = true;
		err = 0;

		for (i = 0; i < num_lanes; i++) {
			err = ufshcd_dme_get(hba,
					     UIC_ARG_MIB_SEL(TX_FSM_STATE, i),
					     &tx_fsm_val);
			if (err)
				goto out;
			if (tx_fsm_val != TX_FSM_HIBERN8) {
				success = false;
				break;
			}
		}

		if (success)
			goto out;

		/* sleep for max. 200us */
		udelay(200);
	} while (get_timer(start) < HBRN8_POLL_TOUT_MS);

out:
	if (err) {
		dev_err(hba->dev, "%s: unable to get TX_FSM_STATE, err %d\n",
			__func__, err);
	} else if (!success) {
		err = -1;
		dev_err(hba->dev, "%s: invalid TX_FSM_STATE\n", __func__);
	}

	return err;
}

static int ufs_s32n_phy_initial_calib(struct ufs_hba *hba)
{
	int ret = 0;

	/* Clock Control */
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBREFCLKCTRL2, 0), 0x80);
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VS_MPHYCFGUPDT, 0), 0x1);

	ret |= ufs_s32n_phy_write_cr(hba, 0x401C, 0x6);
	ret |= ufs_s32n_phy_write_cr(hba, 0x411C, 0x6);
	ret |= ufs_s32n_phy_write_cr(hba, 0x10B0, 0x1);
	ret |= ufs_s32n_phy_write_cr(hba, 0x11B0, 0x1);
	ret |= ufs_s32n_phy_write_cr(hba, 0x10AF, 0x8E);
	ret |= ufs_s32n_phy_write_cr(hba, 0x11AF, 0x91);
	ret |= ufs_s32n_phy_write_cr(hba, 0x10B1, 0x1);
	ret |= ufs_s32n_phy_write_cr(hba, 0x11B1, 0x1);
	ret |= ufs_s32n_phy_write_cr(hba, 0x10B8, 0x1);
	ret |= ufs_s32n_phy_write_cr(hba, 0x11B8, 0x1);
	ret |= ufs_s32n_phy_write_cr(hba, 0x10B0, 0x0);
	ret |= ufs_s32n_phy_write_cr(hba, 0x11B0, 0x0);
	ret |= ufs_s32n_phy_write_cr(hba, 0x10B0, 0x1);
	ret |= ufs_s32n_phy_write_cr(hba, 0x11B0, 0x1);
	ret |= ufs_s32n_phy_write_cr(hba, 0x10AF, 0x71);
	ret |= ufs_s32n_phy_write_cr(hba, 0x11AF, 0x7F);
	ret |= ufs_s32n_phy_write_cr(hba, 0x10B1, 0x2);
	ret |= ufs_s32n_phy_write_cr(hba, 0x11B1, 0x2);
	ret |= ufs_s32n_phy_write_cr(hba, 0x10B8, 0x1);
	ret |= ufs_s32n_phy_write_cr(hba, 0x11B8, 0x1);
	ret |= ufs_s32n_phy_write_cr(hba, 0x10B0, 0x0);
	ret |= ufs_s32n_phy_write_cr(hba, 0x11B0, 0x0);
	ret |= ufs_s32n_phy_write_cr(hba, 0x404D, 0x100);
	ret |= ufs_s32n_phy_write_cr(hba, 0x414D, 0x100);

	return ret;
}

static int ufs_s32n_link_startup_pre_change(struct ufs_hba *hba)
{
	struct s32n_ufs *ufs = dev_get_priv(hba->dev);
	struct udevice *dev = hba->dev;
	u32 reg;
	int ret = 0;

	/* There are three supported MPHY boot options implemented:
	 * - Running MPHY from internal FW ROM:
	 *  MPHY_RAM_CONFIG_STATUS.SRAM_BYPASS = 1
	 *  MPHY_RAM_CONFIG_STATUS.SRAM_EXT_LD_DONE = 0
	 * - Running MPHY from a (modified) FW ROM copy in external SRAM:
	 *  MPHY_RAM_CONFIG_STATUS.SRAM_BYPASS = 0
	 *  MPHY_RAM_CONFIG_STATUS.SRAM_EXT_LD_DONE = 0 (-> 1 after external loading is done).
	 * - MPHY initialization skipped used when running in a simulated
	 * environment or when another component is doing it.
	 */
	if (ufs->mphy_boot_mode == MPHY_BOOT_NONE) {
		dev_info(dev, "Skipping UFS MPHY init.\n");
		return 0;
	}

	ret = ufs_s32n_configure_clocks(hba);
	if (ret)
		return ret;

	/* Reset SCM.MPHY_RAM_CONFIG_STATUS to default value; keep MPHY in reset. */
	writel(SCM_STATUS_PHY_RESET(1), ufs->reg_scm + SCM_STATUS);
	if (ufs->mphy_boot_mode == MPHY_BOOT_ROM) {
		/* ROM Mode. */
		writel(SCM_STATUS_PHY_RESET(1) | SCM_STATUS_SRAM_BYPASS(1),
		       ufs->reg_scm + SCM_STATUS);
	}

	ret = ufs_s32n_phy_configure(hba);
	if (ret) {
		dev_err(dev, "UFS MPHY configuration failed!\n");
		return ret;
	}

	/* Release PHY_RESET. */
	writel(readl(ufs->reg_scm + SCM_STATUS) & ~SCM_STATUS_PHY_RESET(1),
	       ufs->reg_scm + SCM_STATUS);

	/* Wait until SRAM_INIT_DONE = 1.
	 * In case of MPHY FW running from SRAM max can be as high as 15ms@100MHz
	 * reference clock (FW is copied from MPHY ROM to SRAM).
	 */
	ret = readl_poll_timeout(ufs->reg_scm + SCM_STATUS, reg,
				 reg & SCM_STATUS_SRAM_INIT_DONE_MASK, 1000);
	if (ret) {
		dev_err(dev, "UFS MPHY init not done!\n");
		return ret;
	}

	if (ufs->mphy_boot_mode == MPHY_BOOT_COPY_SRAM) {
		/* SRAM External Loading phase - MPHY control registers can be
		 * overwritten using CREG interface.
		 */
		ret |= ufs_s32n_phy_sram_ext_loading(hba);
		if (ret) {
			dev_err(dev, "SRAM external loading failed!\n");
			return ret;
		}

		/* Set MPHY_RAM_CONFIG_STATUS.SRAM_EXT_LD_DONE. */
		writel(readl(ufs->reg_scm + SCM_STATUS) |
		       SCM_STATUS_SRAM_EXT_LD_DONE_MASK,
		       ufs->reg_scm + SCM_STATUS);
	}

	ret = ufs_s32n_phy_initial_calib(hba);
	if (ret) {
		dev_err(dev, "UFS MPHY failed initial calibration!\n");
		return ret;
	}

	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VS_MPHYDISABLE, 0), 0x0);
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VS_MPHYCFGUPDT, 0), 0x1);

	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(N_DEVICEID, 0), 0x0);

	if (ret)
		return ret;

	return ufs_s32n_hibern8(hba, 2);
}

static int ufs_s32n_link_startup_post_change(struct ufs_hba *hba)
{
	struct s32n_ufs *ufs = dev_get_priv(hba->dev);
	unsigned int data = 0;
	int ret = 0;

	/* If MPHY initialization is not handled by TF-A,
	 * invoke the generic DW power mode update procedure.
	 */
	if (ufs->mphy_boot_mode == MPHY_BOOT_NONE)
		return 0;

	/* Set Connection State to IDLE (it allows CPort Attributes to be set). */
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(T_CONNECTIONSTATE, CPORT_0), CPORT_IDLE);

	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(T_CPORTFLAGS, CPORT_0), CPORT_DEF_FLAGS);
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(T_CONNECTIONSTATE, CPORT_0), CPORT_CONNECTED);
	if (ret)
		return ret;

	ret = ufshcd_dme_get(hba, UIC_ARG_MIB_SEL(T_CONNECTIONSTATE, CPORT_0), &data);
	if (ret != 0 || data != CPORT_CONNECTED)
		return ret;

	/* Post startup link calibration. */
	ret |= ufs_s32n_phy_write_cr(hba, RAWLANEN_DIG_PCS_XF_RX_OVRD_IN_0, 0xc);
	ret |= ufs_s32n_phy_write_cr(hba, RAWLANEN_DIG_PCS_XF_RX_OVRD_IN_1, 0xc);
	ret |= ufs_s32n_phy_write_cr(hba, RAWLANEN_DIG_PCS_XF_RX_OVRD_IN_0, 0x8);
	ret |= ufs_s32n_phy_write_cr(hba, RAWLANEN_DIG_PCS_XF_RX_OVRD_IN_1, 0x8);
	ret |= ufs_s32n_phy_write_cr(hba, RAWLANEN_DIG_PCS_XF_RX_OVRD_IN_0, 0x0);
	ret |= ufs_s32n_phy_write_cr(hba, RAWLANEN_DIG_PCS_XF_RX_OVRD_IN_1, 0x0);

	/* Performing MPHY configuration for rate change:
	 * CB rate selection: 0 - rate A, 1 - rate B;
	 */
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(CBRATESEL, 0), 0x1);
	ret |= ufshcd_dme_set(hba, UIC_ARG_MIB_SEL(VS_MPHYCFGUPDT, 0), 0x1);

	/* End of Gear4 calibration */
	return ret;
}

static int ufs_s32n_link_startup_notify(struct ufs_hba *hba,
					enum ufs_notify_change_status status)
{
	int err = 0;

	if (status == PRE_CHANGE) {
		err = ufs_s32n_link_startup_pre_change(hba);
		if (err) {
			dev_err(hba->dev, "MPHY setup failed (%d).\n", err);
			goto out;
		}
	} else { /* POST_CHANGE */
		err = ufshcd_dwc_link_is_up(hba);
		if (err) {
			dev_err(hba->dev, "Link is not up.\n");
			goto out;
		}

		err = ufs_s32n_link_startup_post_change(hba);
		if (err)
			dev_err(hba->dev, "Connection setup failed (%d).\n",
				err);
	}

out:
	return err;
}

static int ufs_s32n_init(struct ufs_hba *hba)
{
	struct s32n_ufs *ufs = dev_get_priv(hba->dev);
	struct udevice *dev = hba->dev;
	const char *mphy_boot_mode;
	fdt_size_t size = 0;
	fdt_addr_t addr;
	int ret;

	ufs->hba = hba;
	ufs->mphy_boot_mode = MPHY_BOOT_NONE;

	mphy_boot_mode = dev_read_string(dev, "nxp,mphy-boot-mode");
	if (!mphy_boot_mode) {
		dev_warn(dev,
			 "nxp,mphy-boot-mode property not found. UFS MPHY init will be skipped.\n");
		return 0;
	}

	if (!strcmp(mphy_boot_mode, "rom")) {
		ufs->mphy_boot_mode = MPHY_BOOT_ROM;
	} else if (!strcmp(mphy_boot_mode, "copy-sram")) {
		ufs->mphy_boot_mode = MPHY_BOOT_COPY_SRAM;
	} else {
		dev_warn(dev,
			 "Unrecognized nxp,mphy-boot-mode property. UFS MPHY init will be skipped.\n");
		return 0;
	}

	addr = dev_read_addr_size_name(dev, "scm", &size);
	if (addr == FDT_ADDR_T_NONE) {
		dev_err(dev, "Can't get SCM base addresses(ret = %lld)!\n", addr);
		return -EINVAL;
	}

	ufs->reg_scm = devm_ioremap(dev, addr, size);
	if (!ufs->reg_scm) {
		dev_err(dev, "ioremap failed for SCM region.\n");
		return -ENOMEM;
	}

	ret = clk_get_by_name(dev, "core_clk", &ufs->core_clk);
	if (ret) {
		dev_err(dev, "Failed to get core clock. %d\n", ret);
		return ret;
	}

	return 0;
}

struct s32n79_match_data {
	struct ufs_hba_ops *ops;
	unsigned long max_bytes_per_req;
};

static struct ufs_hba_ops s32n_hba_ops = {
	.init = ufs_s32n_init,
	.link_startup_notify = ufs_s32n_link_startup_notify,
};

static const struct s32n79_match_data ufs_s32n79_data = {
	.ops = &s32n_hba_ops,
	.max_bytes_per_req = UFS_MAX_BYTES,
};

static const struct s32n79_match_data ufs_s32n79_virt_data = {
	.ops = &s32n_hba_ops,
	.max_bytes_per_req = (120 * 256 * 1024),
};

static int s32n_ufs_pltfm_probe(struct udevice *dev)
{
	struct s32n79_match_data *data;
	struct scsi_plat *scsi_plat;
	struct udevice *scsi_dev;
	int err;

	data = (struct s32n79_match_data *)(uintptr_t)dev_get_driver_data(dev);
	if (!data)
		return -ENODATA;

	err = ufshcd_probe(dev, data->ops);
	if (err) {
		dev_err(dev, "ufshcd_probe() failed %d\n", err);
		return err;
	}

	if (data->max_bytes_per_req) {
		device_find_first_child(dev, &scsi_dev);
		if (!scsi_dev)
			return -ENODEV;

		scsi_plat = dev_get_uclass_plat(scsi_dev);
		scsi_plat->max_bytes_per_req = data->max_bytes_per_req;
	}

	return 0;
}

static const struct udevice_id s32n_ufs_pltfm_ids[] = {
	{
		.compatible = "nxp,s32n79-ufshc",
		.data = (ulong)&ufs_s32n79_data,
	},
	{
		.compatible = "nxp,s32n79-ufshc-virt",
		.data = (ulong)&ufs_s32n79_virt_data,
	},
	{},
};

U_BOOT_DRIVER(s32n_ufs_pltfm) = {
	.name		= "ufs-s32n",
	.id		= UCLASS_UFS,
	.of_match	= s32n_ufs_pltfm_ids,
	.priv_auto	= sizeof(struct s32n_ufs),
	.probe		= s32n_ufs_pltfm_probe,
};
