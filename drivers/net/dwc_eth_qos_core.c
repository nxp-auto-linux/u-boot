// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2016, NVIDIA CORPORATION.
 * Copyright 2019 NXP
 *
 * Portions based on U-Boot's rtl8169.c.
 */

/*
 * This driver supports the Synopsys Designware Ethernet QOS (Quality Of
 * Service) IP block. The IP supports multiple options for bus type, clocking/
 * reset structure, and feature list.
 *
 * The driver is written such that generic core logic is kept separate from
 * configuration-specific logic. Code that interacts with configuration-
 * specific resources is split out into separate functions to avoid polluting
 * common code. If/when this driver is enhanced to support multiple
 * configurations, the core code should be adapted to call all configuration-
 * specific functions through function pointers, with the definition of those
 * function pointers being supplied by struct udevice_id eqos_ids[]'s .data
 * field.
 *
 */

#include <common.h>
#include <clk.h>
#include <dm.h>
#include <errno.h>
#include <memalign.h>
#include <miiphy.h>
#include <net.h>
#include <netdev.h>
#include <phy.h>
#include <reset.h>
#include <wait_bit.h>
#if CONFIG_IS_ENABLED(DM_GPIO)
#include <asm/gpio.h>
#endif
#include <asm/io.h>

#include <dm/platform_data/dwc_eth_qos_dm.h>

#include "dwc_eth_qos.h"

/*
 * Warn if the cache-line size is larger than the descriptor size. In such
 * cases the driver will likely fail because the CPU needs to flush the cache
 * when requeuing RX buffers, therefore descriptors written by the hardware
 * may be discarded. Architectures with full IO coherence, such as x86, do not
 * experience this issue, and hence are excluded from this condition.
 *
 * This can be fixed by defining CONFIG_SYS_NONCACHED_MEMORY which will cause
 * the driver to allocate descriptors from a pool of non-cached memory.
 */
#if EQOS_DESCRIPTOR_SIZE < ARCH_DMA_MINALIGN
#if !defined(CONFIG_SYS_NONCACHED_MEMORY) && \
	!defined(CONFIG_SYS_DCACHE_OFF) && !defined(CONFIG_X86)
#warning Cache line size is larger than descriptor size
#endif
#endif

/*
 * TX and RX descriptors are 16 bytes. This causes problems with the cache
 * maintenance on CPUs where the cache-line size exceeds the size of these
 * descriptors. What will happen is that when the driver receives a packet
 * it will be immediately requeued for the hardware to reuse. The CPU will
 * therefore need to flush the cache-line containing the descriptor, which
 * will cause all other descriptors in the same cache-line to be flushed
 * along with it. If one of those descriptors had been written to by the
 * device those changes (and the associated packet) will be lost.
 *
 * To work around this, we make use of non-cached memory if available. If
 * descriptors are mapped uncached there's no need to manually flush them
 * or invalidate them.
 *
 * Note that this only applies to descriptors. The packet data buffers do
 * not have the same constraints since they are 1536 bytes large, so they
 * are unlikely to share cache-lines.
 */
static void *eqos_alloc_descs(unsigned int num)
{
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	return (void *)noncached_alloc(EQOS_DESCRIPTORS_SIZE,
				      EQOS_DESCRIPTOR_ALIGN);
#else
	return memalign(EQOS_DESCRIPTOR_ALIGN, EQOS_DESCRIPTORS_SIZE);
#endif
}

static void eqos_free_descs(void *descs)
{
#ifdef CONFIG_SYS_NONCACHED_MEMORY
	/* FIXME: noncached_alloc() has no opposite */
#else
	free(descs);
#endif
}

void eqos_inval_desc_generic(void *desc)
{
#ifndef CONFIG_SYS_NONCACHED_MEMORY
	unsigned long start = rounddown((unsigned long)desc, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)desc + EQOS_DESCRIPTOR_SIZE,
				    ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
#endif
}

void eqos_flush_desc_generic(void *desc)
{
#ifndef CONFIG_SYS_NONCACHED_MEMORY
	unsigned long start = rounddown((unsigned long)desc, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)desc + EQOS_DESCRIPTOR_SIZE,
				    ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
#endif
}

void eqos_inval_buffer_generic(void *buf, size_t size)
{
	unsigned long start = rounddown((unsigned long)buf, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)buf + size,
				    ARCH_DMA_MINALIGN);

	invalidate_dcache_range(start, end);
}

void eqos_flush_buffer_generic(void *buf, size_t size)
{
	unsigned long start = rounddown((unsigned long)buf, ARCH_DMA_MINALIGN);
	unsigned long end = roundup((unsigned long)buf + size,
				    ARCH_DMA_MINALIGN);

	flush_dcache_range(start, end);
}

static int eqos_mdio_wait_idle(struct eqos_priv *eqos)
{
	return wait_for_bit_le32(&eqos->mac_regs->mdio_address,
				 EQOS_MAC_MDIO_ADDRESS_GB, false,
				 1000000, true);
}

static int eqos_mdio_read(struct mii_dev *bus, int mdio_addr, int mdio_devad,
			  int mdio_reg)
{
	struct eqos_priv *eqos = bus->priv;
	u32 val;
	int ret;

	debug("%s(dev=%p, addr=%x, reg=%d):\n", __func__, eqos->dev, mdio_addr,
	      mdio_reg);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO not idle at entry");
		return ret;
	}

	val = readl(&eqos->mac_regs->mdio_address);
	val &= EQOS_MAC_MDIO_ADDRESS_SKAP |
		EQOS_MAC_MDIO_ADDRESS_C45E;
	val |= (mdio_addr << EQOS_MAC_MDIO_ADDRESS_PA_SHIFT) |
		(mdio_reg << EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT) |
		(eqos->config->config_mac_mdio <<
		 EQOS_MAC_MDIO_ADDRESS_CR_SHIFT) |
		(EQOS_MAC_MDIO_ADDRESS_GOC_READ <<
		 EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT) |
		EQOS_MAC_MDIO_ADDRESS_GB;
	writel(val, &eqos->mac_regs->mdio_address);

	udelay(eqos->config->mdio_wait);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO read didn't complete");
		return ret;
	}

	val = readl(&eqos->mac_regs->mdio_data);
	val &= EQOS_MAC_MDIO_DATA_GD_MASK;

	debug("%s: val=%x\n", __func__, val);

	return val;
}

static int eqos_mdio_write(struct mii_dev *bus, int mdio_addr, int mdio_devad,
			   int mdio_reg, u16 mdio_val)
{
	struct eqos_priv *eqos = bus->priv;
	u32 val;
	int ret;

	debug("%s(dev=%p, addr=%x, reg=%d, val=%x):\n", __func__, eqos->dev,
	      mdio_addr, mdio_reg, mdio_val);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO not idle at entry");
		return ret;
	}

	writel(mdio_val, &eqos->mac_regs->mdio_data);

	val = readl(&eqos->mac_regs->mdio_address);
	val &= EQOS_MAC_MDIO_ADDRESS_SKAP |
		EQOS_MAC_MDIO_ADDRESS_C45E;
	val |= (mdio_addr << EQOS_MAC_MDIO_ADDRESS_PA_SHIFT) |
		(mdio_reg << EQOS_MAC_MDIO_ADDRESS_RDA_SHIFT) |
		(eqos->config->config_mac_mdio <<
		 EQOS_MAC_MDIO_ADDRESS_CR_SHIFT) |
		(EQOS_MAC_MDIO_ADDRESS_GOC_WRITE <<
		 EQOS_MAC_MDIO_ADDRESS_GOC_SHIFT) |
		EQOS_MAC_MDIO_ADDRESS_GB;
	writel(val, &eqos->mac_regs->mdio_address);

	udelay(eqos->config->mdio_wait);

	ret = eqos_mdio_wait_idle(eqos);
	if (ret) {
		pr_err("MDIO read didn't complete");
		return ret;
	}

	return 0;
}

static int eqos_set_full_duplex(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&eqos->mac_regs->configuration, EQOS_MAC_CONFIGURATION_DM);

	return 0;
}

static int eqos_set_half_duplex(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&eqos->mac_regs->configuration, EQOS_MAC_CONFIGURATION_DM);

	/* WAR: Flush TX queue when switching to half-duplex */
	setbits_le32(&eqos->mtl_regs->txq0_operation_mode,
		     EQOS_MTL_TXQ0_OPERATION_MODE_FTQ);

	return 0;
}

static int eqos_set_gmii_speed(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_PS | EQOS_MAC_CONFIGURATION_FES);

	return 0;
}

static int eqos_set_mii_speed_100(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	setbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_PS | EQOS_MAC_CONFIGURATION_FES);

	return 0;
}

static int eqos_set_mii_speed_10(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	clrsetbits_le32(&eqos->mac_regs->configuration,
			EQOS_MAC_CONFIGURATION_FES, EQOS_MAC_CONFIGURATION_PS);

	return 0;
}


static int eqos_adjust_link(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;
	bool en_calibration;

	debug("%s(dev=%p):\n", __func__, dev);

	if (eqos->phy->duplex)
		ret = eqos_set_full_duplex(dev);
	else
		ret = eqos_set_half_duplex(dev);
	if (ret < 0) {
		pr_err("eqos_set_*_duplex() failed: %d", ret);
		return ret;
	}

	switch (eqos->phy->speed) {
	case SPEED_1000:
		en_calibration = true;
		ret = eqos_set_gmii_speed(dev);
		break;
	case SPEED_100:
		en_calibration = true;
		ret = eqos_set_mii_speed_100(dev);
		break;
	case SPEED_10:
		en_calibration = false;
		ret = eqos_set_mii_speed_10(dev);
		break;
	default:
		pr_err("invalid speed %d", eqos->phy->speed);
		return -EINVAL;
	}
	if (ret < 0) {
		pr_err("eqos_set_*mii_speed*() failed: %d", ret);
		return ret;
	}

	if (en_calibration) {
		ret = eqos->config->ops->eqos_calibrate_pads(dev);
		if (ret < 0) {
			pr_err("eqos_calibrate_pads() failed: %d",
			       ret);
			return ret;
		}
	} else {
		ret = eqos->config->ops->eqos_disable_calibration(dev);
		if (ret < 0) {
			pr_err("eqos_disable_calibration() failed: %d",
			       ret);
			return ret;
		}
	}
	ret = eqos->config->ops->eqos_set_tx_clk_speed(dev);
	if (ret < 0) {
		pr_err("eqos_set_tx_clk_speed() failed: %d", ret);
		return ret;
	}

	return 0;
}

static int eqos_write_hwaddr(struct udevice *dev)
{
	struct eth_pdata *plat = dev_get_platdata(dev);
	struct eqos_priv *eqos = dev_get_priv(dev);
	uint32_t val;

	/*
	 * This function may be called before start() or after stop(). At that
	 * time, on at least some configurations of the EQoS HW, all clocks to
	 * the EQoS HW block will be stopped, and a reset signal applied. If
	 * any register access is attempted in this state, bus timeouts or CPU
	 * hangs may occur. This check prevents that.
	 *
	 * A simple solution to this problem would be to not implement
	 * write_hwaddr(), since start() always writes the MAC address into HW
	 * anyway. However, it is desirable to implement write_hwaddr() to
	 * support the case of SW that runs subsequent to U-Boot which expects
	 * the MAC address to already be programmed into the EQoS registers,
	 * which must happen irrespective of whether the U-Boot user (or
	 * scripts) actually made use of the EQoS device, and hence
	 * irrespective of whether start() was ever called.
	 *
	 * Note that this requirement by subsequent SW is not valid for
	 * Tegra186, and is likely not valid for any non-PCI instantiation of
	 * the EQoS HW block. This function is implemented solely as
	 * future-proofing with the expectation the driver will eventually be
	 * ported to some system where the expectation above is true.
	 */
	if (!eqos->config->reg_access_always_ok && !eqos->reg_access_ok)
		return 0;

	/* Update the MAC address */
	val = (plat->enetaddr[5] << 8) |
		(plat->enetaddr[4]);
	writel(val, &eqos->mac_regs->address0_high);
	val = (plat->enetaddr[3] << 24) |
		(plat->enetaddr[2] << 16) |
		(plat->enetaddr[1] << 8) |
		(plat->enetaddr[0]);
	writel(val, &eqos->mac_regs->address0_low);

	return 0;
}

static int eqos_start(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret, i;
	ulong rate;
	u32 val, tx_fifo_sz, rx_fifo_sz, tqs, rqs, pbl;
	ulong last_rx_desc;

	debug("%s(dev=%p):\n", __func__, dev);

	eqos->tx_desc_idx = 0;
	eqos->rx_desc_idx = 0;

	ret = eqos->config->ops->eqos_start_clks(dev);
	if (ret < 0) {
		pr_err("eqos_start_clks() failed: %d", ret);
		goto err;
	}

	ret = eqos->config->ops->eqos_start_resets(dev);
	if (ret < 0) {
		pr_err("eqos_start_resets() failed: %d", ret);
		goto err_stop_clks;
	}

	udelay(10);

	eqos->reg_access_ok = true;

	ret = wait_for_bit_le32(&eqos->dma_regs->mode,
				EQOS_DMA_MODE_SWR, false,
				eqos->config->swr_wait, false);
	if (ret) {
		pr_err("EQOS_DMA_MODE_SWR stuck");
		goto err_stop_resets;
	}

	ret = eqos->config->ops->eqos_calibrate_pads(dev);
	if (ret < 0) {
		pr_err("eqos_calibrate_pads() failed: %d", ret);
		goto err_stop_resets;
	}
	rate = eqos->config->ops->eqos_get_tick_clk_rate(dev);

	val = (rate / 1000000) - 1;
	writel(val, &eqos->mac_regs->us_tic_counter);

	/*
	 * if PHY was already connected and configured,
	 * don't need to reconnect/reconfigure again
	 */
	if (!eqos->phy) {
		eqos->phy = phy_connect(eqos->mii, 0, dev,
					eqos->config->interface(dev));
		if (!eqos->phy) {
			pr_err("phy_connect() failed");
			goto err_stop_resets;
		}
		ret = phy_config(eqos->phy);
		if (ret < 0) {
			pr_err("phy_config() failed: %d", ret);
			goto err_shutdown_phy;
		}
	}

	ret = phy_startup(eqos->phy);
	if (ret < 0) {
		pr_err("phy_startup() failed: %d", ret);
		goto err_shutdown_phy;
	}

	if (!eqos->phy->link) {
		pr_err("No link");
		goto err_shutdown_phy;
	}

	ret = eqos_adjust_link(dev);
	if (ret < 0) {
		pr_err("eqos_adjust_link() failed: %d", ret);
		goto err_shutdown_phy;
	}

	/* Configure MTL */

	/* Enable Store and Forward mode for TX */
	/* Program Tx operating mode */
	setbits_le32(&eqos->mtl_regs->txq0_operation_mode,
		     EQOS_MTL_TXQ0_OPERATION_MODE_TSF |
		     (EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_ENABLED <<
		      EQOS_MTL_TXQ0_OPERATION_MODE_TXQEN_SHIFT));

	/* Transmit Queue weight */
	writel(0x10, &eqos->mtl_regs->txq0_quantum_weight);

	/* Enable Store and Forward mode for RX, since no jumbo frame */
	setbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
		     EQOS_MTL_RXQ0_OPERATION_MODE_RSF);

	/* Transmit/Receive queue fifo size; use all RAM for 1 queue */
	val = readl(&eqos->mac_regs->hw_feature1);
	tx_fifo_sz = (val >> EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_SHIFT) &
		EQOS_MAC_HW_FEATURE1_TXFIFOSIZE_MASK;
	rx_fifo_sz = (val >> EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_SHIFT) &
		EQOS_MAC_HW_FEATURE1_RXFIFOSIZE_MASK;

	/*
	 * r/tx_fifo_sz is encoded as log2(n / 128). Undo that by shifting.
	 * r/tqs is encoded as (n / 256) - 1.
	 */
	tqs = (128 << tx_fifo_sz) / 256 - 1;
	rqs = (128 << rx_fifo_sz) / 256 - 1;

	clrsetbits_le32(&eqos->mtl_regs->txq0_operation_mode,
			EQOS_MTL_TXQ0_OPERATION_MODE_TQS_MASK <<
			EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT,
			tqs << EQOS_MTL_TXQ0_OPERATION_MODE_TQS_SHIFT);
	clrsetbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
			EQOS_MTL_RXQ0_OPERATION_MODE_RQS_MASK <<
			EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT,
			rqs << EQOS_MTL_RXQ0_OPERATION_MODE_RQS_SHIFT);

	/* Flow control used only if each channel gets 4KB or more FIFO */
	if (rqs >= ((4096 / 256) - 1)) {
		u32 rfd, rfa;

		setbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
			     EQOS_MTL_RXQ0_OPERATION_MODE_EHFC);

		/*
		 * Set Threshold for Activating Flow Contol space for min 2
		 * frames ie, (1500 * 1) = 1500 bytes.
		 *
		 * Set Threshold for Deactivating Flow Contol for space of
		 * min 1 frame (frame size 1500bytes) in receive fifo
		 */
		if (rqs == ((4096 / 256) - 1)) {
			/*
			 * This violates the above formula because of FIFO size
			 * limit therefore overflow may occur inspite of this.
			 */
			rfd = 0x3;	/* Full-3K */
			rfa = 0x1;	/* Full-1.5K */
		} else if (rqs == ((8192 / 256) - 1)) {
			rfd = 0x6;	/* Full-4K */
			rfa = 0xa;	/* Full-6K */
		} else if (rqs == ((16384 / 256) - 1)) {
			rfd = 0x6;	/* Full-4K */
			rfa = 0x12;	/* Full-10K */
		} else {
			rfd = 0x6;	/* Full-4K */
			rfa = 0x1E;	/* Full-16K */
		}

		clrsetbits_le32(&eqos->mtl_regs->rxq0_operation_mode,
				(EQOS_MTL_RXQ0_OPERATION_MODE_RFD_MASK <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT) |
				(EQOS_MTL_RXQ0_OPERATION_MODE_RFA_MASK <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT),
				(rfd <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFD_SHIFT) |
				(rfa <<
				 EQOS_MTL_RXQ0_OPERATION_MODE_RFA_SHIFT));
	}

	/* Configure MAC */

	clrsetbits_le32(&eqos->mac_regs->rxq_ctrl0,
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_MASK <<
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT,
			eqos->config->config_mac <<
			EQOS_MAC_RXQ_CTRL0_RXQ0EN_SHIFT);

	/* Set TX flow control parameters */
	/* Set Pause Time */
	setbits_le32(&eqos->mac_regs->q0_tx_flow_ctrl,
		     0xffff << EQOS_MAC_Q0_TX_FLOW_CTRL_PT_SHIFT);
	/* Assign priority for TX flow control */
	clrbits_le32(&eqos->mac_regs->txq_prty_map0,
		     EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_MASK <<
		     EQOS_MAC_TXQ_PRTY_MAP0_PSTQ0_SHIFT);
	/* Assign priority for RX flow control */
	clrbits_le32(&eqos->mac_regs->rxq_ctrl2,
		     EQOS_MAC_RXQ_CTRL2_PSRQ0_MASK <<
		     EQOS_MAC_RXQ_CTRL2_PSRQ0_SHIFT);
	/* Enable flow control */
	setbits_le32(&eqos->mac_regs->q0_tx_flow_ctrl,
		     EQOS_MAC_Q0_TX_FLOW_CTRL_TFE);
	setbits_le32(&eqos->mac_regs->rx_flow_ctrl,
		     EQOS_MAC_RX_FLOW_CTRL_RFE);

	clrsetbits_le32(&eqos->mac_regs->configuration,
			EQOS_MAC_CONFIGURATION_GPSLCE |
			EQOS_MAC_CONFIGURATION_WD |
			EQOS_MAC_CONFIGURATION_JD |
			EQOS_MAC_CONFIGURATION_JE,
			EQOS_MAC_CONFIGURATION_CST |
			EQOS_MAC_CONFIGURATION_ACS);

	eqos_write_hwaddr(dev);

	/* Configure DMA */

	/* Enable OSP mode */
	setbits_le32(&eqos->dma_regs->ch0_tx_control,
		     EQOS_DMA_CH0_TX_CONTROL_OSP);

	/* RX buffer size. Must be a multiple of bus width */
	clrsetbits_le32(&eqos->dma_regs->ch0_rx_control,
			EQOS_DMA_CH0_RX_CONTROL_RBSZ_MASK <<
			EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT,
			EQOS_MAX_PACKET_SIZE <<
			EQOS_DMA_CH0_RX_CONTROL_RBSZ_SHIFT);

	setbits_le32(&eqos->dma_regs->ch0_control,
		     EQOS_DMA_CH0_CONTROL_PBLX8);

	/*
	 * Burst length must be < 1/2 FIFO size.
	 * FIFO size in tqs is encoded as (n / 256) - 1.
	 * Each burst is n * 8 (PBLX8) * 16 (AXI width) == 128 bytes.
	 * Half of n * 256 is n * 128, so pbl == tqs, modulo the -1.
	 */
	pbl = tqs + 1;
	if (pbl > 32)
		pbl = 32;
	clrsetbits_le32(&eqos->dma_regs->ch0_tx_control,
			EQOS_DMA_CH0_TX_CONTROL_TXPBL_MASK <<
			EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT,
			pbl << EQOS_DMA_CH0_TX_CONTROL_TXPBL_SHIFT);

	clrsetbits_le32(&eqos->dma_regs->ch0_rx_control,
			EQOS_DMA_CH0_RX_CONTROL_RXPBL_MASK <<
			EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT,
			8 << EQOS_DMA_CH0_RX_CONTROL_RXPBL_SHIFT);

	/* DMA performance configuration */
	val = (2 << EQOS_DMA_SYSBUS_MODE_RD_OSR_LMT_SHIFT) |
		EQOS_DMA_SYSBUS_MODE_EAME | EQOS_DMA_SYSBUS_MODE_BLEN16 |
		EQOS_DMA_SYSBUS_MODE_BLEN8 | EQOS_DMA_SYSBUS_MODE_BLEN4;
	writel(val, &eqos->dma_regs->sysbus_mode);

	/* Set up descriptors */

	memset(eqos->descs, 0, EQOS_DESCRIPTORS_SIZE);
	for (i = 0; i < EQOS_DESCRIPTORS_RX; i++) {
		struct eqos_desc *rx_desc = &(eqos->rx_descs[i]);
		rx_desc->des0 = (u32)(ulong)(eqos->rx_dma_buf +
					     (i * EQOS_MAX_PACKET_SIZE));
		rx_desc->des3 |= EQOS_DESC3_OWN | EQOS_DESC3_BUF1V;
	}
	eqos->config->ops->eqos_flush_desc(eqos->descs);

	writel(0, &eqos->dma_regs->ch0_txdesc_list_haddress);
	writel((ulong)eqos->tx_descs, &eqos->dma_regs->ch0_txdesc_list_address);
	writel(EQOS_DESCRIPTORS_TX - 1,
	       &eqos->dma_regs->ch0_txdesc_ring_length);

	writel(0, &eqos->dma_regs->ch0_rxdesc_list_haddress);
	writel((ulong)eqos->rx_descs, &eqos->dma_regs->ch0_rxdesc_list_address);
	writel(EQOS_DESCRIPTORS_RX - 1,
	       &eqos->dma_regs->ch0_rxdesc_ring_length);

	/* Enable everything */

	setbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_TE | EQOS_MAC_CONFIGURATION_RE);

	setbits_le32(&eqos->dma_regs->ch0_tx_control,
		     EQOS_DMA_CH0_TX_CONTROL_ST);
	setbits_le32(&eqos->dma_regs->ch0_rx_control,
		     EQOS_DMA_CH0_RX_CONTROL_SR);

	/* TX tail pointer not written until we need to TX a packet */
	/*
	 * Point RX tail pointer at last descriptor. Ideally, we'd point at the
	 * first descriptor, implying all descriptors were available. However,
	 * that's not distinguishable from none of the descriptors being
	 * available.
	 */
	last_rx_desc = (ulong)&(eqos->rx_descs[(EQOS_DESCRIPTORS_RX - 1)]);
	writel(last_rx_desc, &eqos->dma_regs->ch0_rxdesc_tail_pointer);

	eqos->started = true;

	debug("%s: OK\n", __func__);
	return 0;

err_shutdown_phy:
	phy_shutdown(eqos->phy);
err_stop_resets:
	eqos->config->ops->eqos_stop_resets(dev);
err_stop_clks:
	eqos->config->ops->eqos_stop_clks(dev);
err:
	pr_err("FAILED: %d", ret);
	return ret;
}

void eqos_stop(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int i;

	debug("%s(dev=%p):\n", __func__, dev);

	if (!eqos->started)
		return;
	eqos->started = false;
	eqos->reg_access_ok = false;

	/* Disable TX DMA */
	clrbits_le32(&eqos->dma_regs->ch0_tx_control,
		     EQOS_DMA_CH0_TX_CONTROL_ST);

	/* Wait for TX all packets to drain out of MTL */
	for (i = 0; i < 1000000; i++) {
		u32 val = readl(&eqos->mtl_regs->txq0_debug);
		u32 trcsts = (val >> EQOS_MTL_TXQ0_DEBUG_TRCSTS_SHIFT) &
			EQOS_MTL_TXQ0_DEBUG_TRCSTS_MASK;
		u32 txqsts = val & EQOS_MTL_TXQ0_DEBUG_TXQSTS;
		if ((trcsts != 1) && (!txqsts))
			break;
	}

	/* Turn off MAC TX and RX */
	clrbits_le32(&eqos->mac_regs->configuration,
		     EQOS_MAC_CONFIGURATION_TE | EQOS_MAC_CONFIGURATION_RE);

	/* Wait for all RX packets to drain out of MTL */
	for (i = 0; i < 1000000; i++) {
		u32 val = readl(&eqos->mtl_regs->rxq0_debug);
		u32 prxq = (val >> EQOS_MTL_RXQ0_DEBUG_PRXQ_SHIFT) &
			EQOS_MTL_RXQ0_DEBUG_PRXQ_MASK;
		u32 rxqsts = (val >> EQOS_MTL_RXQ0_DEBUG_RXQSTS_SHIFT) &
			EQOS_MTL_RXQ0_DEBUG_RXQSTS_MASK;
		if ((!prxq) && (!rxqsts))
			break;
	}

	/* Turn off RX DMA */
	clrbits_le32(&eqos->dma_regs->ch0_rx_control,
		     EQOS_DMA_CH0_RX_CONTROL_SR);

	if (eqos->phy) {
		phy_shutdown(eqos->phy);
	}
	eqos->config->ops->eqos_stop_resets(dev);
	eqos->config->ops->eqos_stop_clks(dev);

	debug("%s: OK\n", __func__);
}

int eqos_send(struct udevice *dev, void *packet, int length)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eqos_desc *tx_desc;
	int i;

	debug("%s(dev=%p, packet=%p, length=%d):\n", __func__, dev, packet,
	      length);

	memcpy(eqos->tx_dma_buf, packet, length);
	eqos->config->ops->eqos_flush_buffer(eqos->tx_dma_buf, length);

	tx_desc = &(eqos->tx_descs[eqos->tx_desc_idx]);
	eqos->tx_desc_idx++;
	eqos->tx_desc_idx %= EQOS_DESCRIPTORS_TX;

	tx_desc->des0 = (ulong)eqos->tx_dma_buf;
	tx_desc->des1 = 0;
	tx_desc->des2 = length;
	/*
	 * Make sure that if HW sees the _OWN write below, it will see all the
	 * writes to the rest of the descriptor too.
	 */
	mb();
	tx_desc->des3 = EQOS_DESC3_OWN | EQOS_DESC3_FD | EQOS_DESC3_LD | length;
	eqos->config->ops->eqos_flush_desc(tx_desc);

	writel((ulong)(tx_desc + 1), &eqos->dma_regs->ch0_txdesc_tail_pointer);

	for (i = 0; i < 1000000; i++) {
		eqos->config->ops->eqos_inval_desc(tx_desc);
		if (!(readl(&tx_desc->des3) & EQOS_DESC3_OWN))
			return 0;
		udelay(1);
	}

	debug("%s: TX timeout\n", __func__);

	return -ETIMEDOUT;
}

int eqos_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	struct eqos_desc *rx_desc;
	int length;

	debug("%s(dev=%p, flags=%x):\n", __func__, dev, flags);

	rx_desc = &(eqos->rx_descs[eqos->rx_desc_idx]);
	if (rx_desc->des3 & EQOS_DESC3_OWN) {
		debug("%s: RX packet not available\n", __func__);
		return -EAGAIN;
	}

	*packetp = eqos->rx_dma_buf +
		(eqos->rx_desc_idx * EQOS_MAX_PACKET_SIZE);
	length = rx_desc->des3 & 0x7fff;
	debug("%s: *packetp=%p, length=%d\n", __func__, *packetp, length);

	eqos->config->ops->eqos_inval_buffer(*packetp, length);

	return length;
}

int eqos_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	uchar *packet_expected;
	struct eqos_desc *rx_desc;

	debug("%s(packet=%p, length=%d)\n", __func__, packet, length);

	packet_expected = eqos->rx_dma_buf +
		(eqos->rx_desc_idx * EQOS_MAX_PACKET_SIZE);
	if (packet != packet_expected) {
		debug("%s: Unexpected packet (expected %p)\n", __func__,
		      packet_expected);
		return -EINVAL;
	}

	rx_desc = &(eqos->rx_descs[eqos->rx_desc_idx]);
	rx_desc->des0 = (u32)(ulong)packet;
	rx_desc->des1 = 0;
	rx_desc->des2 = 0;
	/*
	 * Make sure that if HW sees the _OWN write below, it will see all the
	 * writes to the rest of the descriptor too.
	 */
	mb();
	rx_desc->des3 |= EQOS_DESC3_OWN | EQOS_DESC3_BUF1V;
	eqos->config->ops->eqos_flush_desc(rx_desc);

	writel((ulong)rx_desc, &eqos->dma_regs->ch0_rxdesc_tail_pointer);

	eqos->rx_desc_idx++;
	eqos->rx_desc_idx %= EQOS_DESCRIPTORS_RX;

	return 0;
}

static int eqos_probe_resources_core(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	eqos->descs = eqos_alloc_descs(EQOS_DESCRIPTORS_TX +
				       EQOS_DESCRIPTORS_RX);
	if (!eqos->descs) {
		debug("%s: eqos_alloc_descs() failed\n", __func__);
		ret = -ENOMEM;
		goto err;
	}
	eqos->tx_descs = (struct eqos_desc *)eqos->descs;
	eqos->rx_descs = (eqos->tx_descs + EQOS_DESCRIPTORS_TX);
	debug("%s: tx_descs=%p, rx_descs=%p\n", __func__, eqos->tx_descs,
	      eqos->rx_descs);

	eqos->tx_dma_buf = memalign(EQOS_BUFFER_ALIGN, EQOS_MAX_PACKET_SIZE);
	if (!eqos->tx_dma_buf) {
		debug("%s: memalign(tx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_descs;
	}
	debug("%s: tx_dma_buf=%p\n", __func__, eqos->tx_dma_buf);

	eqos->rx_dma_buf = memalign(EQOS_BUFFER_ALIGN, EQOS_RX_BUFFER_SIZE);
	if (!eqos->rx_dma_buf) {
		debug("%s: memalign(rx_dma_buf) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_tx_dma_buf;
	}
	debug("%s: rx_dma_buf=%p\n", __func__, eqos->rx_dma_buf);

	eqos->rx_pkt = malloc(EQOS_MAX_PACKET_SIZE);
	if (!eqos->rx_pkt) {
		debug("%s: malloc(rx_pkt) failed\n", __func__);
		ret = -ENOMEM;
		goto err_free_rx_dma_buf;
	}
	debug("%s: rx_pkt=%p\n", __func__, eqos->rx_pkt);

	debug("%s: OK\n", __func__);
	return 0;

err_free_rx_dma_buf:
	free(eqos->rx_dma_buf);
err_free_tx_dma_buf:
	free(eqos->tx_dma_buf);
err_free_descs:
	eqos_free_descs(eqos->descs);
err:

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int eqos_remove_resources_core(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	free(eqos->rx_pkt);
	free(eqos->rx_dma_buf);
	free(eqos->tx_dma_buf);
	eqos_free_descs(eqos->descs);

	debug("%s: OK\n", __func__);
	return 0;
}

/* board-specific Ethernet Interface initializations. */
__weak int board_interface_eth_init(int interface_type, bool eth_clk_sel_reg,
				    bool eth_ref_clk_sel_reg)
{
	return 0;
}

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int eqos_ofdata_to_platdata(struct udevice *dev)
{
	struct eqos_pdata *pdata = dev_get_platdata(dev);

	if (!pdata) {
		pr_err("no platform data");
		return -ENOMEM;
	}

	pdata->eth.iobase = devfdt_get_addr(dev);
	if (pdata->eth.iobase == FDT_ADDR_T_NONE) {
		pr_err("devfdt_get_addr() failed");
		return -ENODEV;
	}

	pdata->config = (void *)dev_get_driver_data(dev);

	return 0;
}
#endif /* OF_CONTROL */

static int eqos_probe(struct udevice *dev)
{
	struct eqos_pdata *pdata = dev_get_platdata(dev);
	struct eqos_priv *eqos = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	eqos->dev = dev;

	eqos->config = pdata->config;
	if (!eqos->config) {
		pr_err("invalid config!\n");
		return -ENODEV;
	}

	eqos->regs = pdata->eth.iobase;
	if (!eqos->regs) {
		pr_err("iobase not retrieved");
		return -ENODEV;
	}

	eqos->mac_regs = (void *)(eqos->regs + EQOS_MAC_REGS_BASE);
	eqos->mtl_regs = (void *)(eqos->regs + EQOS_MTL_REGS_BASE);
	eqos->dma_regs = (void *)(eqos->regs + EQOS_DMA_REGS_BASE);
	eqos->tegra186_regs = (void *)(eqos->regs + EQOS_TEGRA186_REGS_BASE);

	ret = eqos_probe_resources_core(dev);
	if (ret < 0) {
		pr_err("eqos_probe_resources_core() failed: %d", ret);
		return ret;
	}

	ret = eqos->config->ops->eqos_probe_resources(dev);
	if (ret < 0) {
		pr_err("eqos_probe_resources() failed: %d", ret);
		goto err_remove_resources_core;
	}

	eqos->mii = mdio_alloc();
	if (!eqos->mii) {
		pr_err("mdio_alloc() failed");
		ret = -ENOMEM;
		goto err_remove_resources;
	}
	eqos->mii->read = eqos_mdio_read;
	eqos->mii->write = eqos_mdio_write;
	eqos->mii->priv = eqos;
	strcpy(eqos->mii->name, dev->name);

	ret = mdio_register(eqos->mii);
	if (ret < 0) {
		pr_err("mdio_register() failed: %d", ret);
		goto err_free_mdio;
	}

	debug("%s: OK\n", __func__);
	return 0;

err_free_mdio:
	mdio_free(eqos->mii);
err_remove_resources:
	eqos->config->ops->eqos_remove_resources(dev);
err_remove_resources_core:
	eqos_remove_resources_core(dev);

	debug("%s: returns %d\n", __func__, ret);
	return ret;
}

static int eqos_remove(struct udevice *dev)
{
	struct eqos_priv *eqos = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	mdio_unregister(eqos->mii);
	mdio_free(eqos->mii);
	eqos->config->ops->eqos_remove_resources(dev);

	eqos_probe_resources_core(dev);

	debug("%s: OK\n", __func__);
	return 0;
}

static const struct eth_ops eqos_ops = {
	.start = eqos_start,
	.stop = eqos_stop,
	.send = eqos_send,
	.recv = eqos_recv,
	.free_pkt = eqos_free_pkt,
	.write_hwaddr = eqos_write_hwaddr,
};

/* Driver declaration */

U_BOOT_DRIVER(eth_eqos) = {
	.name = "eth_eqos",
	.id = UCLASS_ETH,
	.of_match = of_match_ptr(eqos_ids),
	.ofdata_to_platdata = of_match_ptr(eqos_ofdata_to_platdata),
	.probe = eqos_probe,
	.remove = eqos_remove,
	.ops = &eqos_ops,
	.priv_auto_alloc_size = sizeof(struct eqos_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
