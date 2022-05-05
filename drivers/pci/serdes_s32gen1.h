/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2019-2022 NXP
 * S32Gen1 PCIe driver
 */

#ifndef SERDES_S32GEN1_H
#define SERDES_S32GEN1_H

#include <dm.h>
#include <pci.h>
#include <asm/io.h>

#include "serdes_regs.h"
#include "serdes_s32gen1_io.h"
#include "ss_pcie_regs.h"

#define PCIE_LINK_UP_COUNT 100
#define PCIE_MPLL_LOCK_COUNT 10
#define PCIE_RESET_COUNT 50
#define DELAY_QUANTUM 1000

#define LTSSM_STATE_L0		0x11 /* L0 state */

#define SERDES_LINKUP_MASK	(SMLH_LINK_UP | RDLH_LINK_UP | \
		SMLH_LTSSM_STATE)
#define SERDES_LINKUP_EXPECT	(SMLH_LINK_UP | RDLH_LINK_UP | \
		BUILD_MASK_VALUE(SMLH_LTSSM_STATE, LTSSM_STATE_L0))

/* Configuration Request Retry Status (CRS) Enable. Active high. */
/* Defer incoming configuration requests. */
#define CRS_EN					0x2
/* LTSSM Enable. Active high. Set it low to hold the LTSSM in Detect state. */
#define LTSSM_EN				0x1


/* Register SS_RW_REG_0 */

#define CLKEN_MASK	0x1
#define CLKEN_OFF	23

#define RST_CTRL		0x3010
#define WARM_RST		0x2
#define COLD_RST		0x1

/* RGM peripheral reset registers */
#define RGM_PRST(MC_RGM, per)		(UPTR(MC_RGM) + 0x40 + \
					((per) * 0x8))
#define RGM_PSTAT(rgm, per)		(UPTR(rgm) + 0x140 + \
					 ((per) * 0x8))
#define RGM_PERIPH_RST(num)		BIT(num)

#define PRST_PCIE_0_SERDES		4
#define PRST_PCIE_0_FUNC		5
#define PRST_PCIE_1_SERDES		16
#define PRST_PCIE_1_FUNC		17

#define MC_RGM_BASE_ADDR		(0x40078000)

enum serdes_link_width {
	X1 = 0x1,
	X2 = 0x2
};

struct s32_serdes {
	struct list_head list;
	struct udevice *bus;
	struct fdt_resource dbi_res;
	void __iomem *dbi;
	enum serdes_mode ss_mode;

	int id;
	enum serdes_dev_type devtype;
	enum serdes_xpcs_mode xpcs_mode;
	enum serdes_clock clktype;
	enum serdes_clock_fmhz fmhz;
	enum serdes_phy_mode phy_mode;
	enum serdes_link_width linkwidth;
};

int s32_serdes_set_mode(void __iomem *dbi, int id, enum serdes_mode mode);
enum serdes_mode s32_get_serdes_mode_from_target(void __iomem *dbi, int id);

int rgm_issue_reset(u32 pid);
int rgm_release_reset(u32 pid);

void s32_serdes_disable_ltssm(void __iomem *dbi);
void s32_serdes_enable_ltssm(void __iomem *dbi);
bool s32_pcie_wait_link_up(void __iomem *dbi);
bool s32_pcie_set_link_width(void __iomem *dbi,
		int id, enum serdes_link_width linkwidth);
bool s32_pcie_init(void __iomem *dbi, int id, bool rc_mode,
		enum serdes_link_width linkwidth);
void show_pcie_devices(void);
int initr_pci(void);

int s32_eth_xpcs_init(void __iomem *serdes_base, int id,
		      enum serdes_mode ss_mode,
		      enum serdes_xpcs_mode xpcs_mode,
		      enum serdes_clock clktype,
		      enum serdes_clock_fmhz fmhz);

#endif /* PCIE_S32GEN1_H */
