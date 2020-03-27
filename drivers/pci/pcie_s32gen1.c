// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020 NXP
 * S32Gen1 PCIe driver
 */

#include <common.h>
#include <pci.h>
#include <asm/io.h>
#include <errno.h>
#include <malloc.h>
#include <dm.h>
#include <asm/arch/clock.h>
#include <linux/sizes.h>
#include <dm/device-internal.h>
#include <asm/arch-s32/siul.h>
#include <hwconfig.h>

/* CFG1 is used in linux when finding devices on the bus.
 * It is actually the upper half of the config space
 * (defined as "config" in device tree).
 * TBD when it is actually used (one EP per RC does not require it) */
#define PCIE_USE_CFG1

#include "pcie_s32gen1.h"

#define PCIE_DEFAULT_INTERNAL_CLK 0

#define PCIE_OVERCONFIG_BUS

/* Enable this if we want RC to be able to send config commands to EP */
#define PCIE_ENABLE_EP_CFG_FROM_RC

#define PCIE_EP_RC_MODE(ep_mode) ((ep_mode) ? "Endpoint" : "Root Complex")

#define PCIE_ALIGNMENT 2

#define PCIE_TABLE_HEADER \
"BusDevFun           VendorId   DeviceId   Device Class       Sub-Class\n" \
"______________________________________________________________________\n"


#ifdef CONFIG_TARGET_S32G274AEVB
/* First SOC revision with functional PCIe: rev 1.0.1, which means
 * major 0, minor 0, subminor 1
 */
#define PCIE_MIN_SOC_REV_SUPPORTED 0x1
#endif

DECLARE_GLOBAL_DATA_PTR;

LIST_HEAD(s32_pcie_list);

static void s32_get_link_status(struct s32_pcie *, u32 *, u32 *, bool);

static void s32_pcie_show_link_err_status(struct s32_pcie *pcie)
{
	printf("LINK_DBG_1: 0x%08x, LINK_DBG_2: 0x%08x ",
			in_le32(pcie->dbi + SS_PE0_LINK_DBG_1),
			in_le32(pcie->dbi + SS_PE0_LINK_DBG_2));
	printf("(expected 0x%08x)\n",
			SERDES_LINKUP_EXPECT);
	printf("DEBUG_R0: 0x%08x, DEBUG_R1: 0x%08x\n",
			in_le32(PCIE_PL_DEBUG0(pcie->dbi)),
			in_le32(PCIE_PL_DEBUG1(pcie->dbi)));
}

/* From the S32G RM:
 *
 * Power on and link training
 * 1  Finish the functional reset of the chip.
 * 2  Finish the reset of the SerDes subsystem.
 * 3  If your application requires programming of the CDM registers, program
 * these registers.
 * 4  If you want to change to the Gen2 speed rate (5.0 Gbps) after linkup,
 * write a 1 to GEN2_CTRL[DIRECT_SPEED_CHANGE].
 * 5  Write a 1 to PE0_GEN_CTRL_3[LTSSM_EN] to start link training.
 * 6  Wait until the following fields in PE0_LINK_DBG_2 are both 1:
 *    - RDLH_LINK_UP
 *    - SMLH_LINK_UP
 */

static bool s32_pcie_wait_link_up(struct s32_pcie *pcie)
{
	int count = PCIE_LINK_UP_COUNT;

	bool ready = (wait_read32((void *)(pcie->dbi + SS_PE0_LINK_DBG_2),
			SERDES_LINKUP_EXPECT, SERDES_LINKUP_MASK, count) == 0);

	if (ready) {
		u32 speed, width;

		s32_get_link_status(pcie, &width, &speed, true);
		printf("PCIe%d: Link established at X%d, Gen%d\n",
				pcie->id, width, speed);
		pcie->linkwidth = width;
		pcie->linkspeed = speed;
	}

	return ready;
}

#ifdef PCIE_OVERCONFIG_BUS
static void s32_pcie_cfg0_set_busdev(struct s32_pcie *pcie, u32 busdev)
{
	W32(PCIE_IATU_LWR_TARGET_ADDR_OUTBOUND_0(pcie->dbi), busdev);
}

#ifdef PCIE_USE_CFG1
static void s32_pcie_cfg1_set_busdev(struct s32_pcie *pcie, u32 busdev)
{
	W32(PCIE_IATU_LWR_TARGET_ADDR_OUTBOUND_0(pcie->dbi) + 0x200, busdev);
}
#endif
#endif

void s32_pcie_atu_outbound_set(struct s32_pcie *pcie, uint32_t region_no,
	uint64_t s_addr, uint32_t s_addr_lim, uint64_t d_addr,
	uint32_t ctrl_1, uint32_t shift_mode)
{
	if (region_no < PCIE_ATU_NR_REGIONS) {
		BCLR32((PCIE_IATU_REGION_CTRL_2_OUTBOUND_0(pcie->dbi) +
				(0x200 * region_no)), PCIE_REGION_EN);
		W32((PCIE_IATU_LWR_BASE_ADDR_OUTBOUND_0(pcie->dbi) +
				(0x200 * region_no)), (uint32_t)s_addr);
		W32((PCIE_IATU_UPPER_BASE_ADDR_OUTBOUND_0(pcie->dbi) +
				(0x200 * region_no)), (s_addr >> 32));
		W32((PCIE_IATU_LIMIT_ADDR_OUTBOUND_0(pcie->dbi) +
				(0x200 * region_no)), s_addr_lim);
		W32((PCIE_IATU_LWR_TARGET_ADDR_OUTBOUND_0(pcie->dbi) +
				(0x200 * region_no)), (uint32_t)d_addr);
		W32((PCIE_IATU_UPPER_TARGET_ADDR_OUTBOUND_0(pcie->dbi) +
				(0x200 * region_no)), (d_addr >> 32));
		W32((PCIE_IATU_REGION_CTRL_1_OUTBOUND_0(pcie->dbi) +
				(0x200 * region_no)), ctrl_1);
		RMW32((PCIE_IATU_REGION_CTRL_2_OUTBOUND_0(pcie->dbi) +
				(0x200 * region_no)),
			PCIE_REGION_EN_VALUE(1) |
			PCIE_CFG_SHIFT_MODE_VALUE(shift_mode),
			PCIE_REGION_EN | PCIE_CFG_SHIFT_MODE);
	} else
		printf("Invalid iATU OUT region %d\n", region_no);
}

/* Use bar match mode and MEM type as default */
static void s32_pcie_atu_inbound_set_bar(struct s32_pcie *pcie,
	uint32_t region_no, uint32_t bar, uint64_t phys, uint32_t ctrl_1)
{
	debug("%s: iATU%d: BAR%d; addr=%p\n", __func__, region_no, bar,
			(void *)phys);
	if (region_no < PCIE_ATU_NR_REGIONS) {
		BCLR32((PCIE_IATU_REGION_CTRL_2_INBOUND_0(pcie->dbi) +
				(0x200 * region_no)), PCIE_REGION_EN);
		W32(PCIE_IATU_LWR_TARGET_ADDR_INBOUND_0(pcie->dbi) +
				(0x200 * region_no), (uint32_t)phys);
		W32(PCIE_IATU_UPPER_TARGET_ADDR_INBOUND_0(pcie->dbi) +
				(0x200 * region_no), phys >> 32);
		W32(PCIE_IATU_REGION_CTRL_1_INBOUND_0(pcie->dbi) +
				(0x200 * region_no), ctrl_1);
		RMW32(PCIE_IATU_REGION_CTRL_2_INBOUND_0(pcie->dbi) +
				(0x200 * region_no),
			PCIE_REGION_EN_VALUE(1) | PCIE_MATCH_MODE_VALUE(1) |
			PCIE_BAR_NUM_VALUE(bar),
			PCIE_REGION_EN | PCIE_MATCH_MODE | PCIE_BAR_NUM);
	} else {
		printf("Invalid iATU IN region %d\n", region_no);
	}
}

#ifdef DEBUG
static void s32_pcie_dump_atu(struct s32_pcie *pcie)
{
	int i;

	for (i = 0; i < pcie->atu_out_num; i++) {
		debug("OUT iATU%d:\n", i);
		debug("\tLOWER PHYS 0x%08x\n",
		    in_le32(PCIE_IATU_LWR_BASE_ADDR_OUTBOUND_0(pcie->dbi) +
					(0x200 * i)));
		debug("\tUPPER PHYS 0x%08x\n",
		    in_le32(PCIE_IATU_UPPER_BASE_ADDR_OUTBOUND_0(pcie->dbi) +
					(0x200 * i)));
		debug("\tLOWER BUS  0x%08x\n",
		    in_le32(PCIE_IATU_LWR_TARGET_ADDR_OUTBOUND_0(pcie->dbi) +
					(0x200 * i)));
		debug("\tUPPER BUS  0x%08x\n",
		    in_le32(PCIE_IATU_UPPER_TARGET_ADDR_OUTBOUND_0(pcie->dbi) +
					(0x200 * i)));
		debug("\tLIMIT      0x%08x\n",
		    in_le32(PCIE_IATU_LIMIT_ADDR_OUTBOUND_0(pcie->dbi) +
					(0x200 * i)));
		debug("\tCR1        0x%08x\n",
		    in_le32(PCIE_IATU_REGION_CTRL_1_OUTBOUND_0(pcie->dbi) +
					(0x200 * i)));
		debug("\tCR2        0x%08x\n",
		    in_le32(PCIE_IATU_REGION_CTRL_2_OUTBOUND_0(pcie->dbi) +
					(0x200 * i)));
	}

	for (i = 0; i < pcie->atu_in_num; i++) {
		debug("IN iATU%d:\n", i);
		debug("\tLOWER PHYS 0x%08x\n",
		    in_le32(PCIE_IATU_LWR_BASE_ADDR_INBOUND_0(pcie->dbi) +
					(0x200 * i)));
		debug("\tUPPER PHYS 0x%08x\n",
		    in_le32(PCIE_IATU_UPPER_BASE_ADDR_INBOUND_0(pcie->dbi) +
					(0x200 * i)));
		debug("\tLOWER BUS  0x%08x\n",
		    in_le32(PCIE_IATU_LWR_TARGET_ADDR_INBOUND_0(pcie->dbi) +
					(0x200 * i)));
		debug("\tUPPER BUS  0x%08x\n",
		    in_le32(PCIE_IATU_UPPER_TARGET_ADDR_INBOUND_0(pcie->dbi) +
					(0x200 * i)));
		debug("\tLIMIT      0x%08x\n",
		    in_le32(PCIE_IATU_LIMIT_ADDR_INBOUND_0(pcie->dbi) +
					(0x200 * i)));
		debug("\tCR1        0x%08x\n",
		    in_le32(PCIE_IATU_REGION_CTRL_1_INBOUND_0(pcie->dbi) +
					(0x200 * i)));
		debug("\tCR2        0x%08x\n",
		    in_le32(PCIE_IATU_REGION_CTRL_2_INBOUND_0(pcie->dbi) +
					(0x200 * i)));
	}
}
#endif

static void s32_pcie_rc_setup_atu(struct s32_pcie *pcie)
{
	uint64_t cfg_start = pcie->cfg_res.start;
#ifndef PCIE_USE_CFG1
	uint64_t cfg_size = fdt_resource_size(&pcie->cfg_res);
#else
	uint64_t cfg_size = fdt_resource_size(&pcie->cfg_res) / 2;
#endif
	uint64_t cfg_limit = cfg_start + cfg_size;

	pcie->atu_out_num = 0;
	pcie->atu_in_num = 0;

	debug("%s: Create outbound windows\n", __func__);

	/* ATU 0 : OUTBOUND : CFG0 */
	s32_pcie_atu_outbound_set(pcie, pcie->atu_out_num++,
			cfg_start, cfg_limit,
			(uint64_t)0x0, PCIE_ATU_TYPE_CFG0, 0);

#ifdef PCIE_USE_CFG1
	/* ATU 1 : OUTBOUND : CFG1 */
	s32_pcie_atu_outbound_set(pcie, pcie->atu_out_num++,
			cfg_limit, cfg_limit + cfg_size,
			(uint64_t)0x0, PCIE_ATU_TYPE_CFG1, 0);
#endif

	/* TODO: create regions returned by pci_get_regions()
	 */

#ifdef DEBUG
	s32_pcie_dump_atu(pcie);
#endif
}

/* Return 0 if the address is valid, -errno if not valid */
static int s32_pcie_addr_valid(struct s32_pcie *pcie, pci_dev_t bdf)
{
	struct udevice *bus = pcie->bus;

	if (PCI_BUS(bdf) < bus->seq) {
		debug_wr("%s: (0x%x, %d): seq too low\n", __func__,
				PCI_BUS(bdf), bus->seq);
		return -EINVAL;
	}

	if (PCI_BUS(bdf) <= (bus->seq + 1) && (PCI_DEV(bdf) > 0)) {
		debug_wr("%s: (0x%x, 0x%x, %d): inval dev/seq\n",
				__func__, PCI_BUS(bdf), PCI_DEV(bdf), bus->seq);
		return -EINVAL;
	}

	return 0;
}

int s32_pcie_conf_address(struct udevice *bus, pci_dev_t bdf,
			 uint offset, void **paddress)
{
	struct s32_pcie *pcie = dev_get_priv(bus);
#ifdef PCIE_OVERCONFIG_BUS
	u32 busdev = PCIE_ATU_BUS(PCI_BUS(bdf) - bus->seq) |
				 PCIE_ATU_DEV(PCI_DEV(bdf)) |
				 PCIE_ATU_FUNC(PCI_FUNC(bdf));
#endif

	if (s32_pcie_addr_valid(pcie, bdf))
		return -EINVAL;

	if (PCI_BUS(bdf) == bus->seq) {
		*paddress = pcie->dbi + offset;
		debug_wr("%s: cfg addr: %p\n", __func__, *paddress);
		return 0;
	}

	if (PCI_BUS(bdf) == bus->seq + 1) {
#ifdef PCIE_OVERCONFIG_BUS
		debug_wr("%s: cfg0_set_busdev 0x%x\n", __func__, busdev);
		s32_pcie_cfg0_set_busdev(pcie, busdev);
#endif
		*paddress = pcie->cfg0 + offset;
		debug_wr("%s: cfg0 addr: %p\n", __func__, *paddress);
	} else {
#ifdef PCIE_USE_CFG1
#ifdef PCIE_OVERCONFIG_BUS
		debug_wr("%s: cfg1_set_busdev %d\n", __func__, busdev);
		s32_pcie_cfg1_set_busdev(pcie, busdev);
#endif
		*paddress = pcie->cfg1 + offset;
		debug_wr("%s: cfg1 addr: %p\n", __func__, *paddress);
#else
		debug_wr("%s: Unsupported bus sequence %d\n", __func__,
				PCI_BUS(bdf));
		return -EINVAL;
#endif
	}
	return 0;
}

static int s32_pcie_read_config(struct udevice *bus, pci_dev_t bdf,
			       uint offset, ulong *valuep,
			       enum pci_size_t size)
{
	return pci_generic_mmap_read_config(bus, s32_pcie_conf_address,
					    bdf, offset, valuep, size);
}

static int s32_pcie_write_config(struct udevice *bus, pci_dev_t bdf,
				uint offset, ulong value,
				enum pci_size_t size)
{
	return pci_generic_mmap_write_config(bus, s32_pcie_conf_address,
					     bdf, offset, value, size);
}

/* Clear multi-function bit */
static void s32_pcie_clear_multifunction(struct s32_pcie *pcie)
{
	writeb(PCI_HEADER_TYPE_BRIDGE, pcie->dbi + PCI_HEADER_TYPE);
}

/* Fix class value */
static int s32_pcie_fix_class(struct s32_pcie *pcie)
{
	debug("%s: Set the correct PCI class (Bridge)\n", __func__);
	W16(pcie->dbi + PCI_CLASS_DEVICE, PCI_CLASS_BRIDGE_PCI);
	__iowmb();

	/* check back if class was set */
	if (in_le16(pcie->dbi + PCI_CLASS_DEVICE) != PCI_CLASS_BRIDGE_PCI) {
		printf("WARNING: Cannot set PCIe%d class type\n", pcie->id);
		return -EPERM;
	}

	return 0;
}

/* Drop MSG TLP except for Vendor MSG */
static void s32_pcie_drop_msg_tlp(struct s32_pcie *pcie)
{
	u32 val;

	val = in_le32(PCIE_SYMBOL_TIMER_FILTER_1(pcie->dbi));
	val &= 0xDFFFFFFF;
	W32(PCIE_SYMBOL_TIMER_FILTER_1(pcie->dbi), val);
}

static int s32_pcie_setup_ctrl(struct s32_pcie *pcie)
{
	bool ret = 0;

	/* Enable writing dbi registers */
	BSET32(PCIE_PORT_LOGIC_MISC_CONTROL_1(pcie->dbi),
			PCIE_DBI_RO_WR_EN);

	s32_pcie_rc_setup_atu(pcie);

	ret = s32_pcie_fix_class(pcie);

	s32_pcie_clear_multifunction(pcie);
	s32_pcie_drop_msg_tlp(pcie);

	BCLR32(PCIE_PORT_LOGIC_MISC_CONTROL_1(pcie->dbi),
			PCIE_DBI_RO_WR_EN);

	return ret;
}

static void s32_pcie_ep_set_bar(char __iomem *dbi_base, int baroffset,
		int enable, uint32_t size, uint32_t init)
{
	uint32_t mask = (enable) ? ((size - 1) & ~1) : 0;

	W32(dbi_base + PCIE_CS2_OFFSET + baroffset, enable);
	W32(dbi_base + PCIE_CS2_OFFSET + baroffset, enable | mask);
	W32(dbi_base + baroffset, init);
}

static void s32_pcie_ep_setup_bars(struct s32_pcie *pcie)
{

	/* Preconfigure the BAR registers, so that the RC can
	 * enumerate us properly and assign address spaces.
	 * Mask registers are W only!
	 */
	s32_pcie_ep_set_bar(pcie->dbi, PCI_BASE_ADDRESS_0,
			     PCIE_EP_BAR0_EN_DIS,
			     PCIE_EP_BAR0_SIZE,
			     PCIE_EP_BAR0_INIT);
	s32_pcie_ep_set_bar(pcie->dbi, PCI_BASE_ADDRESS_1,
			     PCIE_EP_BAR1_EN_DIS,
			     PCIE_EP_BAR1_SIZE,
			     PCIE_EP_BAR1_INIT);
	s32_pcie_ep_set_bar(pcie->dbi, PCI_BASE_ADDRESS_2,
			     PCIE_EP_BAR2_EN_DIS,
			     PCIE_EP_BAR2_SIZE,
			     PCIE_EP_BAR2_INIT);
	s32_pcie_ep_set_bar(pcie->dbi, PCI_BASE_ADDRESS_3,
			     PCIE_EP_BAR3_EN_DIS,
			     PCIE_EP_BAR3_SIZE,
			     PCIE_EP_BAR3_INIT);
	s32_pcie_ep_set_bar(pcie->dbi, PCI_BASE_ADDRESS_4,
			     PCIE_EP_BAR4_EN_DIS,
			     PCIE_EP_BAR4_SIZE,
			     PCIE_EP_BAR4_INIT);
	s32_pcie_ep_set_bar(pcie->dbi, PCI_ROM_ADDRESS,
			     PCIE_EP_ROM_EN_DIS,
			     PCIE_EP_ROM_SIZE,
			     PCIE_EP_ROM_INIT);
}

static void s32_pcie_ep_setup_atu(struct s32_pcie *pcie)
{
	u64 phys = CONFIG_SYS_PCI_EP_MEMORY_BASE;

	pcie->atu_out_num = 0;
	pcie->atu_in_num = 0;

	debug("%s: Create inbound windows\n", __func__);

	/* ATU 0 : INBOUND : map BAR0 */
	if (PCIE_EP_BAR0_EN_DIS) {
		s32_pcie_atu_inbound_set_bar(pcie, pcie->atu_in_num++, 0, phys,
				PCIE_ATU_TYPE_MEM);
		phys += PCIE_EP_BAR0_SIZE;
	}
	/* ATU 1 : INBOUND : map BAR1 */
	if (PCIE_EP_BAR1_EN_DIS) {
		s32_pcie_atu_inbound_set_bar(pcie, pcie->atu_in_num++, 1, phys,
				PCIE_ATU_TYPE_MEM);
		phys += PCIE_EP_BAR1_SIZE;
	}
	/* ATU 2 : INBOUND : map BAR2 */
	if (PCIE_EP_BAR2_EN_DIS) {
		s32_pcie_atu_inbound_set_bar(pcie, pcie->atu_in_num++, 2, phys,
				PCIE_ATU_TYPE_MEM);
		phys += PCIE_EP_BAR2_SIZE;
	}
	/* ATU 3 : INBOUND : map BAR3 */
	if (PCIE_EP_BAR3_EN_DIS) {
		s32_pcie_atu_inbound_set_bar(pcie, pcie->atu_in_num++, 3, phys,
				PCIE_ATU_TYPE_MEM);
		phys += PCIE_EP_BAR3_SIZE;
	}
	/* ATU 4 : INBOUND : map BAR4 */
	if (PCIE_EP_BAR4_EN_DIS) {
		s32_pcie_atu_inbound_set_bar(pcie, pcie->atu_in_num++, 4, phys,
				PCIE_ATU_TYPE_MEM);
		phys += PCIE_EP_BAR4_SIZE;
	}

	debug("%s: Create outbound windows\n", __func__);

	/* ATU 0 : OUTBOUND : map MEM */
	s32_pcie_atu_outbound_set(pcie, pcie->atu_out_num++,
				 pcie->cfg_res.start,
				 0, 0,
				 PCIE_ATU_TYPE_MEM, 0);

#ifdef DEBUG
	s32_pcie_dump_atu(pcie);
#endif
}


/* Delay incoming configuration requests, to allow initialization to complete,
 * by enabling/disabling Configuration Request Retry Status (CRS).
 */
static void s32_serdes_delay_cfg(struct s32_pcie *pcie, bool enable)
{
#ifdef PCIE_ENABLE_EP_CFG_FROM_RC
	if (enable)
		BSET32(pcie->dbi + SS_PE0_GEN_CTRL_3, CRS_EN);
	else
		BCLR32(pcie->dbi + SS_PE0_GEN_CTRL_3, CRS_EN);
#endif
}

static int s32_pcie_setup_ep(struct s32_pcie *pcie)
{
	uint32_t class;
	int ret = 0;

	/* Enable writing dbi registers */
	BSET32(PCIE_PORT_LOGIC_MISC_CONTROL_1(pcie->dbi), PCIE_DBI_RO_WR_EN);

	s32_serdes_delay_cfg(pcie, true);

	/* Set the CLASS_REV of EP CFG header to something that
	 * makes sense for this SoC by itself. For a product,
	 * the class setting should be board/product specific,
	 * so we'd technically need a CONFIG_PCIE_CLASS as part
	 * of the board configuration.
	 */
	class = (PCI_BASE_CLASS_PROCESSOR << 24) |
		(0x80 /* other */ << 16);
	W32(pcie->dbi + PCI_CLASS_REVISION, class);
	__iowmb();

	/* check back if class was set */
	if (in_le32(pcie->dbi + PCI_CLASS_REVISION) != class) {
		printf("WARNING: Cannot set PCIe%d class type\n", pcie->id);
		ret = -EPERM;
	}

	s32_pcie_ep_setup_bars(pcie);
	s32_pcie_ep_setup_atu(pcie);

	s32_serdes_delay_cfg(pcie, false);

	/* Enable writing dbi registers */
	BCLR32(PCIE_PORT_LOGIC_MISC_CONTROL_1((pcie->dbi)), PCIE_DBI_RO_WR_EN);

	return ret;
}

void s32_pcie_change_mstr_ace_domain(void __iomem *dbi, uint32_t ardomain,
		uint32_t awdomain)
{
	BSET32(PCIE_PORT_LOGIC_COHERENCY_CONTROL_3(dbi),
	       PCIE_CFG_MSTR_ARDOMAIN_MODE_VALUE(3) |
		   PCIE_CFG_MSTR_AWDOMAIN_MODE_VALUE(3));
	RMW32(PCIE_PORT_LOGIC_COHERENCY_CONTROL_3(dbi),
			PCIE_CFG_MSTR_ARDOMAIN_VALUE_VALUE(ardomain) |
			PCIE_CFG_MSTR_AWDOMAIN_VALUE_VALUE(awdomain),
			PCIE_CFG_MSTR_ARDOMAIN_VALUE |
			PCIE_CFG_MSTR_AWDOMAIN_VALUE);
}

void s32_pcie_change_mstr_ace_cache(void __iomem *dbi, uint32_t arcache,
		uint32_t awcache)
{
	BSET32(PCIE_PORT_LOGIC_COHERENCY_CONTROL_3(dbi),
	       PCIE_CFG_MSTR_ARCACHE_MODE_VALUE(0xF) |
		   PCIE_CFG_MSTR_AWCACHE_MODE_VALUE(0xF));
	RMW32(PCIE_PORT_LOGIC_COHERENCY_CONTROL_3(dbi),
	      PCIE_CFG_MSTR_ARCACHE_VALUE_VALUE(arcache) |
		  PCIE_CFG_MSTR_AWCACHE_VALUE_VALUE(awcache),
		  PCIE_CFG_MSTR_ARCACHE_VALUE|PCIE_CFG_MSTR_AWCACHE_VALUE);
}

bool s32_pcie_init(void __iomem *dbi, int id, bool rc_mode,
		enum serdes_link_width linkwidth)
{
	debug("Configure pcie%d as %s\n", id, PCIE_EP_RC_MODE(!rc_mode));

	/* Set device type */
	if (rc_mode)
		W32(dbi + SS_PE0_GEN_CTRL_1, DEVICE_TYPE_VALUE(PCIE_RC));
	else
		W32(dbi + SS_PE0_GEN_CTRL_1, DEVICE_TYPE_VALUE(PCIE_EP));

	/* Enable writing dbi registers */
	BSET32(PCIE_PORT_LOGIC_MISC_CONTROL_1(dbi),
			PCIE_DBI_RO_WR_EN);

	/* Set link width */
	RMW32(PCIE_PORT_LOGIC_GEN2_CTRL(dbi),
			PCIE_NUM_OF_LANES_VALUE(linkwidth),
			PCIE_NUM_OF_LANES);
	if (linkwidth == X1) {
		debug("Set link X1\n");
		RMW32(PCIE_PORT_LOGIC_PORT_LINK_CTRL(dbi),
				PCIE_LINK_CAPABLE_VALUE(1),
				PCIE_LINK_CAPABLE);
	} else {
		debug("Set link X2\n");
		RMW32(PCIE_PORT_LOGIC_PORT_LINK_CTRL(dbi),
				PCIE_LINK_CAPABLE_VALUE(3),
				PCIE_LINK_CAPABLE);
	}

	/* Enable direct speed change */
	BSET32(PCIE_PORT_LOGIC_GEN2_CTRL(dbi),
			PCIE_DIRECT_SPEED_CHANGE);

	RMW32(PCIE_PORT_LOGIC_GEN3_EQ_CONTROL(dbi),
		PCIE_GEN3_EQ_FB_MODE_VALUE(1) |
		PCIE_GEN3_EQ_PSET_REQ_VEC_VALUE(0x84),
		PCIE_GEN3_EQ_FB_MODE | PCIE_GEN3_EQ_PSET_REQ_VEC);

	/* Set domain to 0 and cache to 3 */
	s32_pcie_change_mstr_ace_cache(dbi, 3, 3);
	s32_pcie_change_mstr_ace_domain(dbi, 0, 0);

	if (rc_mode) {
		/* Set link number to 0 */
		BCLR32(PCIE_PORT_LOGIC_PORT_FORCE(dbi), PCIE_LINK_NUM);
		/* Set max payload supported */
		RMW32(PCIE_CAP_DEVICE_CONTROL_DEVICE_STATUS(dbi),
			PCIE_CAP_MAX_PAYLOAD_SIZE_CS_VALUE(1) |
			PCIE_CAP_MAX_READ_REQ_SIZE_VALUE(1),
			PCIE_CAP_MAX_PAYLOAD_SIZE_CS |
			PCIE_CAP_MAX_READ_REQ_SIZE);
		/* Enable IO/Mem/Bus master */
		W32(PCIE_CTRL_TYPE1_STATUS_COMMAND_REG(dbi),
				PCIE_IO_EN | PCIE_MSE | PCIE_BME);
	}

	BCLR32(PCIE_PORT_LOGIC_MISC_CONTROL_1(dbi),
			PCIE_DBI_RO_WR_EN);

	return true;
}

static int s32_pcie_get_hw_mode_ep(struct s32_pcie *pcie)
{
	u8 header_type;

	header_type = readb(pcie->dbi + PCI_HEADER_TYPE);
	return (header_type & 0x7f) == PCI_HEADER_TYPE_NORMAL;
}

static int s32_pcie_get_config_from_device_tree(struct s32_pcie *pcie)
{
	const void *fdt = gd->fdt_blob;
	struct udevice *dev = pcie->bus;
	int node = dev_of_offset(dev);
	int ret = 0;

	debug("dt node: %d\n", node);

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "dbi", &pcie->dbi_res);
	if (ret) {
		printf("s32-pcie: resource \"dbi\" not found\n");
		return ret;
	}

	pcie->dbi = map_physmem(pcie->dbi_res.start,
				fdt_resource_size(&pcie->dbi_res),
				MAP_NOCACHE);

	debug("%s: dbi: 0x%p (0x%p)\n", __func__, (void *)pcie->dbi_res.start,
			pcie->dbi);

	ret = fdt_get_named_resource(fdt, node, "reg", "reg-names",
				     "config", &pcie->cfg_res);
	if (ret) {
		printf("%s: resource \"config\" not found\n", dev->name);
		return ret;
	}

	pcie->cfg0 = map_physmem(pcie->cfg_res.start,
				 fdt_resource_size(&pcie->cfg_res),
				 MAP_NOCACHE);
#ifdef PCIE_USE_CFG1
	pcie->cfg1 = pcie->cfg0 + fdt_resource_size(&pcie->cfg_res) / 2;
#endif

	debug("%s: cfg: 0x%p (0x%p)\n", __func__, (void *)pcie->cfg_res.start,
			pcie->cfg0);

	pcie->id = fdtdec_get_int(fdt, node, "device_id", -1);

	/* get supported speed (Gen1/Gen2/Gen3) from device tree */
	pcie->linkspeed = fdtdec_get_int(fdt, node, "link-speed", GEN1);

	return ret;
}

#ifdef PCIE_LINK_RECONFIG_ON_ERR
void s32_pcie_switch_speed(struct s32_pcie *pcie)
{
	debug("Switching to GEN%d\n", pcie->linkspeed);

	/* Set link speed */
	RMW32(PCIE_CAP_LINK_CONTROL2_LINK_STATUS2_REG(pcie->dbi),
			PCIE_CAP_TARGET_LINK_SPEED_VALUE(pcie->linkspeed),
			PCIE_CAP_TARGET_LINK_SPEED);

	if (pcie->linkspeed > GEN1) {
		BCLR32(PCIE_PORT_LOGIC_GEN2_CTRL(pcie->dbi),
				PCIE_DIRECT_SPEED_CHANGE);
		BSET32(PCIE_PORT_LOGIC_GEN2_CTRL(pcie->dbi),
				PCIE_DIRECT_SPEED_CHANGE);
	}
}
#endif

static void s32_get_link_status(struct s32_pcie *pcie,
		u32 *width, u32 *speed, bool verbose)
{
	u16 link_sta;

#ifdef DEBUG
	/* PCIE_CAP_LINK_CAP register contains the maximum link width and speed.
	 * For the field SUPPORT_LINK_SPEED_VECTOR,
	 * The bit definitions within this field are:
	 * Bit 0: 2.5 GT/s (Gen1)
	 * Bit 1: 5.0 GT/s (Gen2)
	 * Bit 2: 8.0 GT/s (Gen3)
	 * Bits 3-6: Reserved
	 */
	if (verbose) {
		u16 link_cap = readw(PCIE_CAP_LINK_CAP(pcie->dbi));

		debug("PCIe%d: max X%d Gen%d\n", pcie->id,
				PCIE_BIT_VALUE(link_cap, PCIE_MAX_LINK_WIDTH),
				PCIE_BIT_VALUE(link_cap, PCIE_MAX_LINK_SPEED));
	}
#endif

	link_sta = readw(PCIE_LINK_STATUS(pcie->dbi));
	/* update link width based on negotiated link status */
	*width = PCIE_BIT_VALUE(link_sta, PCIE_LINK_WIDTH);
	/* For link speed, LINK_SPEED value specifies a bit location in
	 * LINK_CAPABILITIES_2[SUPPORT_LINK_SPEED_VECTOR] that corresponds
	 * to the current link speed.
	 * 0001b - SUPPORT_LINK_SPEED_VECTOR bit 0 (Gen1)
	 * 0010b - SUPPORT_LINK_SPEED_VECTOR bit 1 (Gen2)
	 * 0011b - SUPPORT_LINK_SPEED_VECTOR bit 2 (Gen3)
	 */
	*speed = PCIE_BIT_VALUE(link_sta, PCIE_LINK_SPEED);
	if (verbose)
		debug("PCIe%d: current X%d Gen%d\n", pcie->id, *width, *speed);
}

static bool is_s32gen1_pcie_ltssm_enabled(struct s32_pcie *pcie)
{
	return (in_le32(pcie->dbi + SS_PE0_GEN_CTRL_3) & LTSSM_EN);
}

static int s32_pcie_probe(struct udevice *dev)
{
	struct s32_pcie *pcie = dev_get_priv(dev);
	struct uclass *uc = dev->uclass;
	int ret = 0;
	bool ltssm_en = false;

#ifdef PCIE_MIN_SOC_REV_SUPPORTED
	uint32_t raw_rev = 0;

	/* construct a revision number based on major, minor and subminor,
	 * each part using one hex digit
	 */
	raw_rev = (get_siul2_midr1_major() << 8) |
		(get_siul2_midr1_minor() << 4) |
		(get_siul2_midr2_subminor());

	if (raw_rev < PCIE_MIN_SOC_REV_SUPPORTED) {
		printf("PCIe not supported\n");
		return -ENXIO;
	}
#endif

	debug("%s: probing %s\n", __func__, dev->name);
	if (!pcie) {
		printf("s32-pcie: invalid internal data\n");
		return -EINVAL;
	}

	pcie->bus = dev;

	list_add(&pcie->list, &s32_pcie_list);

	ret = s32_pcie_get_config_from_device_tree(pcie);
	if (ret)
		return ret;

	ltssm_en = is_s32gen1_pcie_ltssm_enabled(pcie);
	if (!ltssm_en) {
		printf("Not configuring pcie%d, PHY not configured\n",
			pcie->id);
		pcie->enabled = false;
		return -ENXIO;
	}

	if (s32_pcie_wait_link_up(pcie)) {
		printf("PCIe%d: Link up! X%d, Gen%d\n", pcie->id,
				pcie->linkwidth, pcie->linkspeed);
	} else {
		printf("PCIe%d: Failed to get link up\n", pcie->id);
		s32_pcie_show_link_err_status(pcie);
		return false;
	}

	pcie->enabled = true;

	/* TODO: If link is not established at >GEN1, then attempt setting
	 * speed manually
	 */

	if (pcie->ep_mode != s32_pcie_get_hw_mode_ep(pcie)) {
		printf("WARNING: Unable to configure PCIe%d as %s\n",
			pcie->id, PCIE_EP_RC_MODE(pcie->ep_mode));
		return -EPERM;
	}

	/* apply other custom settings (bars, iATU etc.) */
	if (!pcie->ep_mode)
		ret = s32_pcie_setup_ctrl(pcie);
	else
		ret = s32_pcie_setup_ep(pcie);

	if (ret)
		return ret;

	if (uc) {
		struct uclass_driver *uc_drv = uc->uc_drv;

		/* for EP mode, skip postprobing functions since
		 * it corrupts configuration
		 */
		if (pcie->ep_mode && uc_drv && (uc_drv->post_probe))
			uc_drv->post_probe = NULL;
	}

	return 0;
}

static void show_pci_devices(struct udevice *bus, struct udevice *dev,
		int depth, int last_flag)
{
	int i, is_last;
	struct udevice *child;
	struct pci_child_platdata *pplat;

	for (i = depth; i >= 0; i--) {
		is_last = (last_flag >> i) & 1;
		if (i) {
			if (is_last)
				printf("    ");
			else
				printf("|   ");
		} else {
			if (is_last)
				printf("`-- ");
			else
				printf("|-- ");
		}
	}

	pplat = dev_get_parent_platdata(dev);
	printf("%02x:%02x.%02x", bus->seq,
	       PCI_DEV(pplat->devfn), PCI_FUNC(pplat->devfn));

	for (i = (PCIE_ALIGNMENT - depth); i > 0; i--)
		printf("    ");
	pci_header_show_brief(dev);

	list_for_each_entry(child, &dev->child_head, sibling_node) {
		is_last = list_is_last(&child->sibling_node, &dev->child_head);
		show_pci_devices(dev, child, depth + 1,
			(last_flag << 1) | is_last);
	}
}

int pci_get_depth(struct udevice *dev)
{
	if (!dev)
		return 0;

	return (1 + pci_get_depth(dev->parent));
}

void show_pcie_devices(void)
{
	struct udevice *bus;
	bool show_header = true;

	for (uclass_find_first_device(UCLASS_PCI, &bus);
		     bus;
		     uclass_find_next_device(&bus)) {
		struct udevice *dev;
		struct s32_pcie *pcie = dev_get_priv(bus);

		if (pcie && pcie->enabled) {
			if (show_header) {
				printf(PCIE_TABLE_HEADER);
				show_header = false;
			}
			printf("%s %s\n", bus->name,
				PCIE_EP_RC_MODE(pcie->ep_mode));
		}
		for (device_find_first_child(bus, &dev);
			    dev;
			    device_find_next_child(&dev)) {
			int depth = pci_get_depth(dev);
			int is_last = list_is_last(&dev->sibling_node,
					&bus->child_head);
			if (dev->seq < 0)
				continue;
			show_pci_devices(bus, dev, depth - 3, is_last);
		}
	}
}

static const struct dm_pci_ops s32_pcie_ops = {
	.read_config	= s32_pcie_read_config,
	.write_config	= s32_pcie_write_config,
};

static const struct udevice_id s32_pcie_ids[] = {
	{ .compatible = "fsl,s32gen1-pcie" },
	{ }
};

U_BOOT_DRIVER(pci_s32gen1) = {
	.name = "pci_s32gen1",
	.id = UCLASS_PCI,
	.of_match = s32_pcie_ids,
	.ops = &s32_pcie_ops,
	.probe	= s32_pcie_probe,
	.priv_auto_alloc_size = sizeof(struct s32_pcie),
};
