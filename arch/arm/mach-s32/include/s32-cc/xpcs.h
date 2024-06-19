/* SPDX-License-Identifier: GPL-2.0 */
/**
 * Copyright 2022-2024 NXP
 */
#ifndef S32CC_XPCS_H
#define S32CC_XPCS_H

#include <generic-phy.h>
#include <linux/types.h>

struct s32cc_xpcs;

enum pcie_xpcs_mode {
	NOT_SHARED,
	PCIE_XPCS_1G,
	PCIE_XPCS_2G5,
};

struct phylink_link_state {
	int speed;
	int duplex;
	int pause;
	u32 advertising;
	unsigned int link:1;
	unsigned int an_enabled:1;
	unsigned int an_complete:1;
};

struct s32cc_xpcs_ops {
	int (*init)(struct s32cc_xpcs **xpcs, struct udevice *dev,
		    unsigned char id, void __iomem *base, bool ext_clk,
		    unsigned long rate, enum pcie_xpcs_mode pcie_shared);
	int (*power_on)(struct s32cc_xpcs *xpcs);
	int (*config)(struct s32cc_xpcs *xpcs,
		      const struct phylink_link_state *state);
	int (*vreset)(struct s32cc_xpcs *xpcs);
	int (*wait_vreset)(struct s32cc_xpcs *xpcs);
	int (*init_plls)(struct s32cc_xpcs *xpcs);
	int (*reset_rx)(struct s32cc_xpcs *xpcs);
	bool (*has_valid_rx)(struct s32cc_xpcs *xpcs);
	void (*pre_reset_pcie_2g5)(struct s32cc_xpcs *xpcs);
	int (*post_reset_pcie_2g5)(struct s32cc_xpcs *xpcs);

	/* These functions are planned to be used directly
	 * by phylink in newer kernels (starting from 5.10).
	 */
	int (*xpcs_config)(struct s32cc_xpcs *xpcs,
			   const struct phylink_link_state *state);
	int (*xpcs_get_state)(struct s32cc_xpcs *xpcs,
			      struct phylink_link_state *state);
	int (*get_id)(struct s32cc_xpcs *xpcs);
};

const struct s32cc_xpcs_ops *s32cc_xpcs_get_ops(void);

/**
 * s32cc_phy2xpcs() - Get XPCS instance associated with a PHY
 *
 * @phy: A generic PHY obtained from s32cc SerDes driver.
 *
 * The return value will be the XPCS instance associated with the
 * passed SerDes PHY.
 */
struct s32cc_xpcs *s32cc_phy2xpcs(struct phy *phy);

#endif
