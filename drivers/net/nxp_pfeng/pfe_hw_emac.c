// SPDX-License-Identifier: GPL 2.0
/*
 *  Copyright (c) 2019 Imagination Technologies Limited
 *  Copyright (c) 2020-2021 Imagination Technologies Limited
 *  Copyright 2018-2024 NXP
 */

#include <dm/device_compat.h>
#include <linux/delay.h>
#include <linux/iopoll.h>

#include "pfe_hw.h"
#include "pfeng.h"
#include "internal/pfe_hw_priv.h"

#define PFE_EMAC_F_DEF (PASS_CONTROL_PACKETS(FORWARD_ALL_EXCEPT_PAUSE) | \
			HASH_MULTICAST | HASH_OR_PERFECT_FILTER | \
			HASH_UNICAST | PROMISCUOUS_MODE)

#define PFE_EMAC_CFG_DEF (SA_INSERT_REPLACE_CONTROL(CTRL_BY_SIGNALS) | \
			  CHECKSUM_OFFLOAD | \
			  GIANT_PACKET_LIMIT_CONTROL | \
			  CRC_STRIPPING_FOR_TYPE | \
			  AUTO_PAD_OR_CRC_STRIPPING | \
			  JABBER_DISABLE | \
			  DUPLEX_MODE | WATCHDOG_DISABLE | \
			  BACK_OFF_LIMIT(MIN_N_10) | \
			  PREAMBLE_LENGTH_TX(PREAMBLE_7B) | \
			  DISABLE_CARRIER_SENSE_TX)

#define PFE_EMAC_ADDRHI_DEF (ADDRESS_ENABLE | ADDR_HI(0xffeeU))
#define PFE_EMAC_ADDRLO_DEF 0xddccbbaaU

#define PFE_HW_MDIO_TIMEOUT_US 5000UL

static const struct pfe_hw_stats pfe_stats_emac[] = {
	{"RX_PACKETS_COUNT_GOOD_BAD", RX_PACKETS_COUNT_GOOD_BAD },
	{"TX_PACKET_COUNT_GOOD_BAD", TX_PACKET_COUNT_GOOD_BAD },
	{"MAC_CONFIGURATION", MAC_CONFIGURATION },
	{"MAC_HW_FEATURE0", MAC_HW_FEATURE0 },

	{"TX_UNDERFLOW_ERROR_PACKETS", TX_UNDERFLOW_ERROR_PACKETS },
	{"TX_SINGLE_COLLISION_GOOD_PACKETS", TX_SINGLE_COLLISION_GOOD_PACKETS },
	{"TX_MULTIPLE_COLLISION_GOOD_PACKETS", TX_MULTIPLE_COLLISION_GOOD_PACKETS },
	{"TX_DEFERRED_PACKETS", TX_DEFERRED_PACKETS },
	{"TX_LATE_COLLISION_PACKETS", TX_LATE_COLLISION_PACKETS },
	{"TX_EXCESSIVE_COLLISION_PACKETS", TX_EXCESSIVE_COLLISION_PACKETS },
	{"TX_CARRIER_ERROR_PACKETS", TX_CARRIER_ERROR_PACKETS },
	{"TX_EXCESSIVE_DEFERRAL_ERROR", TX_EXCESSIVE_DEFERRAL_ERROR },
	{"TX_OSIZE_PACKETS_GOOD", TX_OSIZE_PACKETS_GOOD },
	{"RX_CRC_ERROR_PACKETS", RX_CRC_ERROR_PACKETS },
	{"HIF_RX_BVALID_FIFO_CNT", HIF_RX_BVALID_FIFO_CNT },
	{"HIF_RX_PKT_CNT1", HIF_RX_PKT_CNT1 },
	{"RX_ALIGNMENT_ERROR_PACKETS", RX_ALIGNMENT_ERROR_PACKETS },
	{"RX_RUNT_ERROR_PACKETS", RX_RUNT_ERROR_PACKETS },
	{"RX_JABBER_ERROR_PACKETS", RX_JABBER_ERROR_PACKETS },
	{"RX_LENGTH_ERROR_PACKETS", RX_LENGTH_ERROR_PACKETS },
	{"RX_OUT_OF_RANGE_TYPE_PACKETS", RX_OUT_OF_RANGE_TYPE_PACKETS },
	{"RX_FIFO_OVERFLOW_PACKETS", RX_FIFO_OVERFLOW_PACKETS },
	{"RX_RECEIVE_ERROR_PACKETS", RX_RECEIVE_ERROR_PACKETS },
	{"TX_UNICAST_PACKETS_GOOD_BAD", TX_UNICAST_PACKETS_GOOD_BAD },
	{"TX_BROADCAST_PACKETS_GOOD", TX_BROADCAST_PACKETS_GOOD },
	{"TX_BROADCAST_PACKETS_GOOD_BAD", TX_BROADCAST_PACKETS_GOOD_BAD },
	{"TX_MULTICAST_PACKETS_GOOD", TX_MULTICAST_PACKETS_GOOD },
	{"TX_MULTICAST_PACKETS_GOOD_BAD", TX_MULTICAST_PACKETS_GOOD_BAD },
	{"TX_VLAN_PACKETS_GOOD", TX_VLAN_PACKETS_GOOD },
	{"TX_PAUSE_PACKETS", TX_PAUSE_PACKETS },
	{"RX_UNICAST_PACKETS_GOOD", RX_UNICAST_PACKETS_GOOD },
	{"RX_BROADCAST_PACKETS_GOOD", RX_BROADCAST_PACKETS_GOOD },
	{"RX_MULTICAST_PACKETS_GOOD", RX_MULTICAST_PACKETS_GOOD },
	{"RX_VLAN_PACKETS_GOOD_BAD", RX_VLAN_PACKETS_GOOD_BAD },
	{"RX_CONTROL_PACKETS_GOOD", RX_CONTROL_PACKETS_GOOD },
	{"TX_OSIZE_PACKETS_GOOD", TX_OSIZE_PACKETS_GOOD },
	{"RX_UNDERSIZE_PACKETS_GOOD", RX_UNDERSIZE_PACKETS_GOOD },

	{"TX_OCTET_COUNT_GOOD", TX_OCTET_COUNT_GOOD },
	{"TX_OCTET_COUNT_GOOD_BAD", TX_OCTET_COUNT_GOOD_BAD },
	{"TX_64OCTETS_PACKETS_GOOD_BAD", TX_64OCTETS_PACKETS_GOOD_BAD },
	{"TX_65TO127OCTETS_PACKETS_GOOD_BAD", TX_65TO127OCTETS_PACKETS_GOOD_BAD },
	{"TX_128TO255OCTETS_PACKETS_GOOD_BAD", TX_128TO255OCTETS_PACKETS_GOOD_BAD },
	{"TX_256TO511OCTETS_PACKETS_GOOD_BAD", TX_256TO511OCTETS_PACKETS_GOOD_BAD },
	{"TX_512TO1023OCTETS_PACKETS_GOOD_BAD", TX_512TO1023OCTETS_PACKETS_GOOD_BAD },
	{"TX_1024TOMAXOCTETS_PACKETS_GOOD_BAD", TX_1024TOMAXOCTETS_PACKETS_GOOD_BAD },

	{"RX_OCTET_COUNT_GOOD", RX_OCTET_COUNT_GOOD },
	{"RX_OCTET_COUNT_GOOD_BAD", RX_OCTET_COUNT_GOOD_BAD },
	{"RX_64OCTETS_PACKETS_GOOD_BAD", RX_64OCTETS_PACKETS_GOOD_BAD },
	{"RX_65TO127OCTETS_PACKETS_GOOD_BAD", RX_65TO127OCTETS_PACKETS_GOOD_BAD },
	{"RX_128TO255OCTETS_PACKETS_GOOD_BAD", RX_256TO511OCTETS_PACKETS_GOOD_BAD },
	{"RX_512TO1023OCTETS_PACKETS_GOOD_BAD", RX_512TO1023OCTETS_PACKETS_GOOD_BAD },
	{"RX_1024TOMAXOCTETS_PACKETS_GOOD_BAD", RX_1024TOMAXOCTETS_PACKETS_GOOD_BAD },
};

void pfe_hw_emac_print_stats(struct pfe_hw_emac *emac)
{
	u32 reg;
	size_t i;

	for (i = 0; i < ARRAY_SIZE(pfe_stats_emac); i++) {
		const struct pfe_hw_stats *stat = &pfe_stats_emac[i];

		reg = pfe_hw_read(emac, stat->reg_off);

		printf("%-40s : 0x%08x\n", stat->reg_name, reg);
	}
}

void pfe_hw_emac_enable(struct pfe_hw_emac *emac)
{
	setbits_32(pfe_hw_addr(emac, MAC_CONFIGURATION),
		   TRANSMITTER_ENABLE | RECEIVER_ENABLE);
}

void pfe_hw_emac_disable(struct pfe_hw_emac *emac)
{
	clrbits_32(pfe_hw_addr(emac, MAC_CONFIGURATION),
		   TRANSMITTER_ENABLE | RECEIVER_ENABLE);
}

void pfe_hw_emac_get_addr(struct pfe_hw_emac *emac, u8 *addr)
{
	u32 bottom, top;
	int i;

	top = pfe_hw_read(emac, MAC_ADDRESS_HIGH(0));
	bottom = pfe_hw_read(emac, MAC_ADDRESS_LOW(0));

	for (i = 0; i < 4; i++)
		addr[i] = (u8)((bottom >> i * 8) & 0xFF);

	for (i = 0; i < 2; i++)
		addr[i + 4] = (u8)((top >> i * 8) & 0xFF);
}

void pfe_hw_emac_set_addr(struct pfe_hw_emac *emac, const u8 *addr)
{
	u32 bottom, top;

	bottom = ((u32)addr[3] << 24) | ((u32)addr[2] << 16)
		 | ((u32)addr[1] << 8) | ((u32)addr[0] << 0);
	top = ((u32)addr[5] << 8) | ((u32)addr[4] << 0) | ADDRESS_ENABLE;

	/*	Store MAC address */
	pfe_hw_write(emac, MAC_ADDRESS_HIGH(0), top);
	pfe_hw_write(emac, MAC_ADDRESS_LOW(0), bottom);
}

int pfe_hw_emac_set_duplex(struct pfe_hw_emac *emac, bool is_full)
{
	if (is_full)
		setbits_32(pfe_hw_addr(emac, MAC_CONFIGURATION), DUPLEX_MODE);
	else
		clrbits_32(pfe_hw_addr(emac, MAC_CONFIGURATION), DUPLEX_MODE);

	return 0;
}

/* EMAC helper functions */
int pfe_hw_emac_set_speed(struct pfe_hw_emac *emac, int speed)
{
	u32 reg;

	reg = pfe_hw_read(emac, MAC_CONFIGURATION) & ~(PORT_SELECT | SPEED);

	switch (speed) {
	case SPEED_10: {
		reg |= PORT_SELECT;
		break;
	}
	case SPEED_100: {
		reg |= PORT_SELECT | SPEED;
		break;
	}
	case SPEED_1000: {
		break;
	}
	case SPEED_2500: {
		reg |= SPEED;
		break;
	}
	default:
		return -EINVAL;
	}

	pfe_hw_write(emac, MAC_CONFIGURATION, reg);

	return 0;
}

int pfe_hw_emac_mdio_read(void __iomem *base_va, u8 pa, s32 dev, u16 ra, u16 *val)
{
	u32 mac_stat;
	u32 reg = 0U;

	if (dev == MDIO_DEVAD_NONE) {
		/* C22 */
		reg = REG_DEV_ADDR(ra);
	} else {
		/* C45 */
		reg = GMII_REGISTER_ADDRESS(ra);
		pfe_hw_write_reg(base_va, MAC_MDIO_DATA, reg);
		reg = CLAUSE45_ENABLE | REG_DEV_ADDR(dev);
	}

	reg |=	GMII_BUSY |
		GMII_OPERATION_CMD(GMII_READ) |
		CSR_CLOCK_RANGE(PFE_HW_PLATFORM(emac_mdio_div)) |
		PHYS_LAYER_ADDR(pa);

	/*	Start a read operation */
	pfe_hw_write_reg(base_va, MAC_MDIO_ADDRESS, reg);

	/*	Wait for completion */
	if (readl_poll_timeout(pfe_hw_reg_addr(base_va, MAC_MDIO_ADDRESS), mac_stat,
			       !(mac_stat & GMII_BUSY),
			       PFE_HW_MDIO_TIMEOUT_US) < 0)
		return -ETIMEDOUT;

	/*	Get the data */
	reg = pfe_hw_read_reg(base_va, MAC_MDIO_DATA);
	*val = GMII_DATA(reg);

	return 0;
}

int pfe_hw_emac_mdio_write(void __iomem *base_va, u8 pa, s32 dev, u16 ra, u16 val)
{
	u32 mac_stat;
	u32 reg = 0U;

	if (dev == MDIO_DEVAD_NONE) {
		/* C22 */
		reg = GMII_DATA(val);
		pfe_hw_write_reg(base_va, MAC_MDIO_DATA, reg);
		reg = REG_DEV_ADDR(ra);
	} else {
		/* C45 */
		reg = GMII_DATA(val) | GMII_REGISTER_ADDRESS(ra);
		pfe_hw_write_reg(base_va, MAC_MDIO_DATA, reg);
		reg = CLAUSE45_ENABLE | REG_DEV_ADDR(dev);
	}

	reg |= GMII_BUSY | GMII_OPERATION_CMD(GMII_WRITE) |
	       CSR_CLOCK_RANGE(PFE_HW_PLATFORM(emac_mdio_div)) |
	       PHYS_LAYER_ADDR(pa);

	/*	Start a write operation */
	pfe_hw_write_reg(base_va, MAC_MDIO_ADDRESS, reg);
	/*	Wait for completion */
	if (readl_poll_timeout(pfe_hw_reg_addr(base_va, MAC_MDIO_ADDRESS), mac_stat,
			       !(mac_stat & GMII_BUSY),
			       PFE_HW_MDIO_TIMEOUT_US) < 0)
		return -ETIMEDOUT;

	return 0;
}

int pfe_hw_emac_init(struct pfe_hw_emac *emac, enum pfe_hw_emac_block emac_id,
		     const struct pfe_hw_cfg *cfg)
{
	emac->base = pfe_hw_phys_addr(pfe_hw_emac_get_iobase(cfg->cbus_base, emac_id));
	if (!emac->base)
		return -ENODEV;

	pfe_hw_write(emac, MAC_CONFIGURATION, 0U);
	pfe_hw_write(emac, MTL_DPP_CONTROL, 0x1U);

	pfe_hw_write(emac, MAC_ADDRESS0_HIGH, PFE_EMAC_ADDRHI_DEF);
	pfe_hw_write(emac, MAC_ADDRESS0_LOW, PFE_EMAC_ADDRLO_DEF);

	pfe_hw_write(emac, MAC_PACKET_FILTER, PFE_EMAC_F_DEF);

	clrbits_32(pfe_hw_addr(emac, MAC_Q0_TX_FLOW_CTRL), TX_FLOW_CONTROL_ENABLE);

	pfe_hw_write(emac, MAC_INTERRUPT_ENABLE, 0U);

	pfe_hw_write(emac, MAC_CONFIGURATION, PFE_EMAC_CFG_DEF);
	pfe_hw_write(emac, MTL_RXQ0_OPERATION_MODE, FORWARD_ERROR_PACKETS);

	pfe_hw_write(emac, MTL_TXQ0_OPERATION_MODE, 0U);

	pfe_hw_write(emac, MAC_EXT_CONFIGURATION, GIANT_PACKET_SIZE_LIMIT(PKTSIZE_ALIGN));

	dev_info(cfg->dev, "EMAC%d block was initialized\n", emac_id);

	return 0;
}

void pfe_hw_emac_deinit(struct pfe_hw_emac *emac)
{
	pfe_hw_emac_disable(emac);
	emac->base = NULL;
}
