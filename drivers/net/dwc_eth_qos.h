/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright 2023 NXP
 */

#include <phy_interface.h>
#include <linux/bitops.h>

#define EQOS_DMA_MODE_SWR				BIT(0)

#define EQOS_MAC_RXQ_CTRL0_RXQ0EN_ENABLED_DCB		2

#define EQOS_MAC_MDIO_ADDRESS_CR_500_800		7

#define EQOS_AXI_WIDTH_64	8

struct eqos_dma_regs {
	uint32_t mode;					/* 0x1000 */
	uint32_t sysbus_mode;				/* 0x1004 */
	uint32_t unused_1008[(0x1100 - 0x1008) / 4];	/* 0x1008 */
	uint32_t ch0_control;				/* 0x1100 */
	uint32_t ch0_tx_control;			/* 0x1104 */
	uint32_t ch0_rx_control;			/* 0x1108 */
	uint32_t unused_110c;				/* 0x110c */
	uint32_t ch0_txdesc_list_haddress;		/* 0x1110 */
	uint32_t ch0_txdesc_list_address;		/* 0x1114 */
	uint32_t ch0_rxdesc_list_haddress;		/* 0x1118 */
	uint32_t ch0_rxdesc_list_address;		/* 0x111c */
	uint32_t ch0_txdesc_tail_pointer;		/* 0x1120 */
	uint32_t unused_1124;				/* 0x1124 */
	uint32_t ch0_rxdesc_tail_pointer;		/* 0x1128 */
	uint32_t ch0_txdesc_ring_length;		/* 0x112c */
	uint32_t ch0_rxdesc_ring_length;		/* 0x1130 */
};

struct eqos_config {
	bool reg_access_always_ok;
	int mdio_wait;
	int swr_wait;
	int config_mac;
	int config_mac_mdio;
	int tx_fifo_size;
	int rx_fifo_size;
	unsigned int axi_bus_width;
	phy_interface_t (*interface)(struct udevice *dev);
	struct eqos_ops *ops;
};

struct eqos_ops {
	void (*eqos_inval_desc)(void *desc);
	void (*eqos_flush_desc)(void *desc);
	void (*eqos_inval_buffer)(void *buf, size_t size);
	void (*eqos_flush_buffer)(void *buf, size_t size);
	int (*eqos_probe_resources)(struct udevice *dev);
	int (*eqos_remove_resources)(struct udevice *dev);
	int (*eqos_stop_resets)(struct udevice *dev);
	int (*eqos_start_resets)(struct udevice *dev);
	int (*eqos_stop_clks)(struct udevice *dev);
	int (*eqos_start_clks)(struct udevice *dev);
	int (*eqos_calibrate_pads)(struct udevice *dev);
	int (*eqos_disable_calibration)(struct udevice *dev);
	int (*eqos_set_tx_clk_speed)(struct udevice *dev);
	int (*eqos_restart_rx_clk)(struct udevice *dev);
	ulong (*eqos_get_tick_clk_rate)(struct udevice *dev);
};

struct eqos_priv {
	struct udevice *dev;
	const struct eqos_config *config;
	fdt_addr_t regs;
	struct eqos_mac_regs *mac_regs;
	struct eqos_mtl_regs *mtl_regs;
	struct eqos_dma_regs *dma_regs;
	struct eqos_tegra186_regs *tegra186_regs;
	struct reset_ctl reset_ctl;
	struct gpio_desc phy_reset_gpio;
	struct clk clk_master_bus;
	struct clk clk_rx;
	struct clk clk_ptp_ref;
	struct clk clk_tx;
	struct clk clk_ck;
	struct clk clk_slave_bus;
	struct mii_dev *mii;
	struct phy_device *phy;
	u32 max_speed;
	void *descs;
	int tx_desc_idx, rx_desc_idx;
	unsigned int desc_size;
	void *tx_dma_buf;
	void *rx_dma_buf;
	void *rx_pkt;
	bool started;
	bool reg_access_ok;
	bool clk_ck_enabled;
};

void eqos_inval_desc_generic(void *desc);
void eqos_flush_desc_generic(void *desc);
void eqos_inval_buffer_generic(void *buf, size_t size);
void eqos_flush_buffer_generic(void *buf, size_t size);
int eqos_null_ops(struct udevice *dev);

extern const struct eqos_config eqos_s32cc_config;
