// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2023 NXP
 * S32CC PCIe End Point driver
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <generic-phy.h>
#include <hwconfig.h>
#include <malloc.h>
#include <pci.h>
#include <pci_ep.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <dm/uclass.h>
#include <linux/io.h>
#include <linux/iopoll.h>
#include <linux/sizes.h>
#include <linux/sizes.h>
#include <s32-cc/serdes_hwconfig.h>

#include "pci-s32cc-regs.h"
#include "pci_s32cc.h"

#define PCI_DEVICE_ID_S32CC	0x4002

static const struct pci_epc_features s32cc_pcie_epc_features = {
	.reserved_bar = BIT(BAR_1) | BIT(BAR_5),
	.bar_fixed_64bit = BIT(BAR_0),
	.bar_fixed_size[BAR_0] = SZ_2M,
	.bar_fixed_size[BAR_2] = SZ_1M,
	.bar_fixed_size[BAR_3] = SZ_64K,
	.bar_fixed_size[BAR_4] = 256,
};

static int s32cc_pcie_ep_write_header(struct udevice *dev, uint fn,
				      struct pci_ep_header *hdr)
{
	struct s32cc_pcie_ep *s32cc_ep = dev_get_priv(dev);
	struct dw_pcie *pcie = &s32cc_ep->common.pcie;
	u32 class;

	/* S32CC only supports one function, as it is not SR-IOV */
	if (fn != 0) {
		dev_err(dev, "Only EP Function 0 supported\n");
		return -EOPNOTSUPP;
	}

	class = (PCI_BASE_CLASS_PROCESSOR << 24) |
		(0x80 /* other */ << 16);
	dw_pcie_writel_dbi(pcie, PCI_CLASS_REVISION, class);

	if (hdr) {
		dw_pcie_writeb_dbi(pcie, PCI_CLASS_PROG,
				   hdr->progif_code);
		dw_pcie_writew_dbi(pcie, PCI_CLASS_DEVICE,
				   hdr->subclass_code |
				   hdr->baseclass_code << 8);
		dw_pcie_writeb_dbi(pcie, PCI_CACHE_LINE_SIZE, hdr->cache_line_size);
		dw_pcie_writew_dbi(pcie, PCI_SUBSYSTEM_ID, hdr->subsys_id);
		dw_pcie_writeb_dbi(pcie, PCI_INTERRUPT_PIN, hdr->interrupt_pin);
	}

	s32cc_pcie_set_device_id(&s32cc_ep->common);

	return 0;
}

static int s32cc_pcie_ep_set_bar(struct udevice *dev, uint fn, struct pci_bar *ep_bar)
{
	struct s32cc_pcie_ep *s32cc_ep = dev_get_priv(dev);
	struct s32cc_pcie *s32cc_pp = &s32cc_ep->common;
	struct dw_pcie *pci = &s32cc_pp->pcie;
	enum pci_barno bar = ep_bar->barno;
	size_t size = ep_bar->size;
	int flags = ep_bar->flags;
	enum dw_pcie_as_type as_type;
	u32 reg;
	int ret = 0;

	/* S32CC only supports one function, as it is not SR-IOV */
	if (fn != 0) {
		dev_err(dev, "Only EP Function 0 supported\n");
		return -EOPNOTSUPP;
	}

	if (!ep_bar->size || !ep_bar->phys_addr) {
		dev_info(dev, "func:%d: Skipping BAR%d\n", fn, ep_bar->barno);
		return 0;
	}

	reg = PCI_BASE_ADDRESS_0 + (4 * bar);

	if (!(flags & PCI_BASE_ADDRESS_SPACE))
		as_type = DW_PCIE_AS_MEM;
	else
		as_type = DW_PCIE_AS_IO;

	dev_info(dev, "func:%d: Configuring BAR%d: size=%d bytes, addr=0x%llx, %s, %s, %s\n",
		 fn, bar, (int)size, ep_bar->phys_addr,
		 (flags & PCI_BASE_ADDRESS_SPACE_IO ? "IO" : "MEM"),
		 (flags & PCI_BASE_ADDRESS_MEM_TYPE_64 ? "64bit" : "32bit"),
		 (flags & PCI_BASE_ADDRESS_MEM_PREFETCH ? "PREF" : "Non-PREF"));

	dw_pcie_dbi_ro_wr_en(pci);

	dw_pcie_writel_dbi2(pci, reg, lower_32_bits(size - 1));
	dw_pcie_writel_dbi(pci, reg, flags);

	if (flags & PCI_BASE_ADDRESS_MEM_TYPE_64) {
		dw_pcie_writel_dbi2(pci, reg + 4, upper_32_bits(size - 1));
		dw_pcie_writel_dbi(pci, reg + 4, 0);
	}

	dw_pcie_dbi_ro_wr_dis(pci);

	/* Use next free region index.
	 * The region index must be reset outside this function.
	 */
	ret = dw_pcie_prog_inbound_atu_unroll(pci, fn, s32cc_pp->atu_in_num++, ep_bar->barno,
					      ep_bar->phys_addr, as_type);

	if (ret < 0) {
		dev_err(pci->dev, "Failed to program IB window\n");
		return ret;
	}

	return 0;
}

static const struct pci_epc_features *s32cc_pcie_ep_get_features(void)
{
	return &s32cc_pcie_epc_features;
}

int s32cc_pcie_ep_setup_inbound(struct s32cc_pcie_ep *s32cc_ep)
{
	struct s32cc_pcie *s32cc_pp = &s32cc_ep->common;
	struct dw_pcie *pci = &s32cc_pp->pcie;
	struct udevice *dev = pci->dev;
	enum pci_barno bar_num = BAR_0, idx;
	const struct pci_epc_features *epc_features;
	dma_addr_t next_phys_addr = 0;
	int next_bar;
	int ret = 0;

	s32cc_pp->atu_out_num = 0;
	s32cc_pp->atu_in_num = 0;

	epc_features = s32cc_pcie_ep_get_features();
	if (!epc_features) {
		dev_err(dev, "Invalid S32CC EP controller features\n");
		return -ENODEV;
	}

	next_phys_addr = (phys_addr_t)s32cc_ep->shared_base;

	/* Setup BARs and inbound regions, up to BAR5 inclusivelly */
	for (idx = bar_num; idx <= BAR_5; idx += next_bar) {
		struct pci_bar *epf_bar = &s32cc_ep->ep_bars[idx];

		epf_bar->flags = 0;
		epf_bar->size = 0;

		if (epc_features->bar_fixed_64bit & (1 << idx)) {
			epf_bar->flags &= ~PCI_BASE_ADDRESS_MEM_TYPE_MASK;
			epf_bar->flags |= PCI_BASE_ADDRESS_MEM_TYPE_64;
		}
		if (epc_features->bar_fixed_size[idx])
			epf_bar->size = epc_features->bar_fixed_size[idx];

		epf_bar->barno = idx;
		next_bar = (epf_bar->flags & PCI_BASE_ADDRESS_MEM_TYPE_64) ? 2 : 1;

		if (!!(epc_features->reserved_bar & (1 << idx)) ||
		    !epf_bar->size) {
			epf_bar->size = 0;
			continue;
		}

		/* shift BAR physical address if it interferes with
		 * previous BAR
		 */
		if (next_phys_addr > epf_bar->phys_addr)
			epf_bar->phys_addr = next_phys_addr;
		ret = s32cc_pcie_ep_set_bar(dev, 0, epf_bar);
		if (ret)
			dev_err(dev, "Unable to init BAR%d\n", idx);

		next_phys_addr = epf_bar->phys_addr + epf_bar->size;
	}

	return ret;
}

/* s32cc_pcie_dt_init_ep - Function intended to initialize platform
 * data from the (live) device tree.
 * Note that it is called before the probe function.
 */
int s32cc_pcie_dt_init_ep(struct udevice *dev)
{
	struct s32cc_pcie_ep *s32cc_ep = dev_get_priv(dev);
	struct s32cc_pcie *s32cc_pp = &s32cc_ep->common;
	struct dw_pcie *pcie = &s32cc_pp->pcie;
	struct ofnode_phandle_args phandle = {.node = ofnode_null()};
	int ret;

	s32cc_pp->mode = DW_PCIE_UNKNOWN_TYPE;
	pcie->dev = dev;

	ret = s32cc_pcie_dt_init_common(s32cc_pp);
	if (ret)
		return ret;

	/* The EP needs dbi, dbi2, physical addr_space from the device tree.
	 * Optionally, if we want to auto-configure BARs, we need a valid
	 * shared-mem phandle and auto-config-bars property defined (for iATU).
	 * dbi, dbi2 are read in s32cc_pcie_dt_init_common.
	 * The others we read below.
	 */

	s32cc_ep->auto_config_bars = false;

	ret = dev_read_phandle_with_args(dev, "shared-mem",
					 "reg", 0, 0, &phandle);
	if (!ret && ofnode_valid(phandle.node)) {
		s32cc_ep->shared_base = (void *)ofnode_get_addr_size(phandle.node,
								     "reg",
								      &s32cc_ep->shared_size);
	} else {
		dev_warn(dev, "Can't find shared-mem phandle node\n");
		s32cc_ep->shared_base = (void *)FDT_ADDR_T_NONE;
	}

	if ((phys_addr_t)s32cc_ep->shared_base != FDT_ADDR_T_NONE) {
		/* Shared mem address is valid; it makes sense to auto-configure BARs */
		s32cc_ep->auto_config_bars = dev_read_bool(dev, "auto-config-bars");
	} else {
		dev_warn(dev, "Invalid Shared Mem address\n");
		s32cc_ep->shared_base = NULL;
		s32cc_ep->shared_size = 0;
	}

	s32cc_ep->addr_space_base = (void *)dev_read_addr_size_name(dev, "addr_space",
							 &s32cc_ep->addr_space_size);
	if ((fdt_addr_t)s32cc_ep->addr_space_base == FDT_ADDR_T_NONE) {
		dev_warn(dev, "PCIe%d: resource 'addr_space' not found\n",
			 s32cc_pp->id);
		s32cc_ep->addr_space_base = NULL;
		s32cc_ep->addr_space_size = 0;
	} else {
		dev_dbg(dev, "Address Space base: 0x%lx\n", (uintptr_t)s32cc_ep->addr_space_base);
	}

	return 0;
}

static int s32cc_pcie_config_ep(struct s32cc_pcie_ep *s32cc_ep)
{
	struct s32cc_pcie *s32cc_pp = &s32cc_ep->common;
	struct dw_pcie *pcie = &s32cc_pp->pcie;
	struct udevice *dev = pcie->dev;
	int ret = 0;

	s32cc_pcie_ep_write_header(dev, 0, NULL);

	ret = s32cc_pcie_init_controller(s32cc_pp);
	if (ret)
		return ret;

	if (s32cc_ep->auto_config_bars) {
		int result = -1;
		int bar = 0;

		if (s32cc_ep->shared_base && s32cc_ep->shared_size > 0) {
			result = s32cc_pcie_ep_setup_inbound(s32cc_ep);
			if (!result) {
				/* Verify BARs fit the reserved region */
				struct pci_bar *epf_bar, *s32cc_ep_bars =
					s32cc_ep->ep_bars;
				u32 sum = 0;

				/* ep_bars was checked in
				 * s32cc_pcie_ep_setup_inbound
				 */
				for (; bar <= BAR_5; bar++) {
					epf_bar = &s32cc_ep_bars[bar];
					sum += epf_bar->size;
				}
				if (sum > s32cc_ep->shared_size) {
					dev_warn(dev, "EP BARs too large\n");
					result = -1;
				}
			}

			s32cc_pcie_dump_atu(s32cc_pp);
		}
		if (result)
			dev_warn(dev, "Failed to configure EP BARs\n");
	}

	return ret;
}

extern struct dw_pcie_ops s32cc_dw_pcie_ops;

static int s32cc_pcie_probe_ep(struct udevice *dev)
{
	struct s32cc_pcie_ep *s32cc_ep = dev_get_priv(dev);
	struct s32cc_pcie *s32cc_pp = &s32cc_ep->common;
	struct dw_pcie *pcie = &s32cc_pp->pcie;
	int ret = 0;

	ret = s32cc_check_serdes(dev);
	if (ret)
		return ret;

	pcie->first_busno = dev_seq(dev);
	pcie->ops = &s32cc_dw_pcie_ops;
	s32cc_pp->atu_out_num = 0;
	s32cc_pp->atu_in_num = 0;

	s32cc_pp->mode = DW_PCIE_EP_TYPE;

	ret = s32cc_pcie_config_ep(s32cc_ep);
	if (ret) {
		dev_err(dev, "Failed to set PCIe EP settings\n");
		s32cc_pp->mode = DW_PCIE_UNKNOWN_TYPE;
	}

	return ret;
}

static struct pci_ep_ops s32cc_pcie_ep_ops = {
	.write_header = s32cc_pcie_ep_write_header,
	.set_bar = s32cc_pcie_ep_set_bar,
};

static const struct udevice_id s32cc_pcie_ids[] = {
	{ .compatible = PCIE_COMPATIBLE_EP },
	{ }
};

U_BOOT_DRIVER(pci_s32cc_ep) = {
	.name = "pci_s32cc_ep",
	.id = UCLASS_PCI_EP,
	.of_match = s32cc_pcie_ids,
	.ops = &s32cc_pcie_ep_ops,
	.of_to_plat = s32cc_pcie_dt_init_ep,
	.probe = s32cc_pcie_probe_ep,
	.priv_auto = sizeof(struct s32cc_pcie_ep),
	.flags = DM_FLAG_SEQ_PARENT_ALIAS,
};
