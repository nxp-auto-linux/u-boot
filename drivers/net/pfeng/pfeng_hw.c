// SPDX-License-Identifier: GPL 2.0
/*
 *  Copyright (c) 2020 Imagination Technologies Limited
 *  Copyright 2018-2022 NXP
 */

/**
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 *
 * @file		pfeng_hw.c
 * @brief		Basic PFE HW abstraction
 *
 */

#include <common.h>
#include <cpu_func.h>
#include <elf.h>
#include <linux/delay.h>
#include <linux/ethtool.h>
#include <linux/mdio.h>

#include "pfe_cbus.h"
#include "pfe_platform_cfg.h"
#include "pfeng_hw.h"

#define WSP_VERSION			(0)
#define WSP_VERSION_SILICON_G2		0x00050300
#define WSP_VERSION_SILICON_G3		0x00000101

#define WSP_FAIL_STOP_MODE_EN		(0xB4)
#define WSP_FAIL_STOP_MODE_INT_EN	(0xC0)
#define WSP_ECC_ERR_INT_EN		(0x130)
#define WSP_DBUG_BUS1			(0xA4)

#define SOFT_RESET_DONE			BIT(19)
#define BMU1_SOFT_RESET_DONE		BIT(20)
#define BMU2_SOFT_RESET_DONE		BIT(21)

#define SOFT_RESET_DONE_CLEAR		BIT(27)
#define BMU1_SOFT_RESET_DONE_CLEAR	BIT(28)
#define BMU2_SOFT_RESET_DONE_CLEAR	BIT(29)

static struct pfe_platform pfe;

static const struct pfe_ct_hif_tx_hdr header[3U] = {
	{.queue = 0, .flags = HIF_TX_INJECT, .chid = 0, .e_phy_ifs = htonl(1)},
	{.queue = 0, .flags = HIF_TX_INJECT, .chid = 0, .e_phy_ifs = htonl(2)},
	{.queue = 0, .flags = HIF_TX_INJECT, .chid = 0, .e_phy_ifs = htonl(4)},
};

#define HIF_HEADER_SIZE sizeof(struct pfe_ct_hif_rx_hdr)

#define INIT_PLAT_OFF(plat, off) ((void *)((uint64_t)(plat)->cbus_baseaddr + \
				  (uint64_t)(off)))

/* EMAC  helper macros */
#define MHZ		(1000000UL)
#define PFE_EMAC_F_DEF (0U | PASS_CONTROL_PACKETS(FORWARD_ALL_EXCEPT_PAUSE) | \
			HASH_MULTICAST | HASH_OR_PERFECT_FILTER | \
			HASH_UNICAST)

#define PFE_EMAC_CFG_DEF (0U | SA_INSERT_REPLACE_CONTROL(CTRL_BY_SIGNALS) | \
			  CHECKSUM_OFFLOAD | \
			  GIANT_PACKET_LIMIT_CONTROL | \
			  CRC_STRIPPING_FOR_TYPE | \
			  AUTO_PAD_OR_CRC_STRIPPING | \
			  JABBER_DISABLE | \
			  DUPLEX_MODE | WATCHDOG_DISABLE | \
			  BACK_OFF_LIMIT(MIN_N_10) | \
			  PREAMBLE_LENGTH_TX(PREAMBLE_7B) | \
			  DISABLE_CARRIER_SENSE_TX)

/* PE/ELF helper macros*/
#define PE_ADDR_LOW(memt) (PFE_CFG_CLASS_ELF_ ## memt ##_BASE)
#define PE_ADDR_HI(memt) (PFE_CFG_CLASS_ELF_ ## memt ## _BASE + \
			  PFE_CFG_CLASS_ ## memt ## _SIZE)

#define BYTES_TO_4B_ALIGNMENT(x) (4U - ((x) & 0x3U))
#define ELF_SKIP_FLAGS (SHF_WRITE | SHF_ALLOC | SHF_EXECINSTR)
#define SHT_PFE_SKIP (0x7000002aU)

struct pfe_hw_chnl {
	struct pfe_hif_ring *rx_ring;
	struct pfe_hif_ring *tx_ring;
};

struct pfe_gpi_cfg {
	u32 alloc_retry_cycles;
	u32 gpi_tmlf_txthres;
	u32 gpi_dtx_aseq_len;
	bool emac_1588_ts_en;
	bool ingress;
};

struct pfe_bmu_cfg {
	void *pool_pa;
	u32 max_buf_cnt;
	u32 buf_size;
	u32 bmu_ucast_thres;
	u32 bmu_mcast_thres;
	u32 int_mem_loc_cnt;
	u32 buf_mem_loc_cnt;
};

struct pfe_debug {
	char *reg_name;
	u32 reg;
};

enum pfe_pe_mem { PFE_PE_DMEM, PFE_PE_IMEM };

static struct pfe_debug debug_hif[] = {
	{"HIF_TX_STATE", HIF_TX_STATE },
	{"HIF_TX_ACTV", HIF_TX_ACTV },
	{"HIF_TX_CURR_CH_NO", HIF_TX_CURR_CH_NO },
	{"HIF_DXR_TX_FIFO_CNT", HIF_DXR_TX_FIFO_CNT },
	{"HIF_TX_CTRL_WORD_FIFO_CNT1", HIF_TX_CTRL_WORD_FIFO_CNT1 },
	{"HIF_TX_CTRL_WORD_FIFO_CNT2", HIF_TX_CTRL_WORD_FIFO_CNT2 },
	{"HIF_TX_BVALID_FIFO_CNT", HIF_TX_BVALID_FIFO_CNT },
	{"HIF_TX_PKT_CNT1", HIF_TX_PKT_CNT1 },
	{"HIF_TX_PKT_CNT2", HIF_TX_PKT_CNT2 },
	{"HIF_RX_STATE", HIF_RX_STATE },
	{"HIF_RX_ACTV", HIF_RX_ACTV },
	{"HIF_RX_CURR_CH_NO", HIF_RX_CURR_CH_NO },
	{"HIF_DXR_RX_FIFO_CNT", HIF_DXR_RX_FIFO_CNT },
	{"HIF_RX_CTRL_WORD_FIFO_CNT", HIF_RX_CTRL_WORD_FIFO_CNT },
	{"HIF_RX_BVALID_FIFO_CNT", HIF_RX_BVALID_FIFO_CNT },
	{"HIF_RX_PKT_CNT1", HIF_RX_PKT_CNT1 },
	{"HIF_RX_PKT_CNT2", HIF_RX_PKT_CNT2 },
};

static struct pfe_debug debug_emac[] = {
	{"RX_PACKETS_COUNT_GOOD_BAD", RX_PACKETS_COUNT_GOOD_BAD },
	{"TX_PACKET_COUNT_GOOD_BAD", TX_PACKET_COUNT_GOOD_BAD },
	{"MAC_CONFIGURATION", MAC_CONFIGURATION },
	{"MAC_HW_FEATURE0", MAC_HW_FEATURE0 },

	{"TX_UNDERFLOW_ERROR_PACKETS", TX_UNDERFLOW_ERROR_PACKETS },
	{"TX_SINGLE_COLLISION_GOOD_PACKETS", TX_SINGLE_COLLISION_GOOD_PACKETS },
	{"TX_MULTIPLE_COLLISION_GOOD_PACKETS",
	 TX_MULTIPLE_COLLISION_GOOD_PACKETS },
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
	{"TX_65TO127OCTETS_PACKETS_GOOD_BAD",
	 TX_65TO127OCTETS_PACKETS_GOOD_BAD },
	{"TX_128TO255OCTETS_PACKETS_GOOD_BAD",
	 TX_128TO255OCTETS_PACKETS_GOOD_BAD },
	{"TX_256TO511OCTETS_PACKETS_GOOD_BAD",
	 TX_256TO511OCTETS_PACKETS_GOOD_BAD },
	{"TX_512TO1023OCTETS_PACKETS_GOOD_BAD",
	 TX_512TO1023OCTETS_PACKETS_GOOD_BAD },
	{"TX_1024TOMAXOCTETS_PACKETS_GOOD_BAD",
	 TX_1024TOMAXOCTETS_PACKETS_GOOD_BAD },

	{"RX_OCTET_COUNT_GOOD", RX_OCTET_COUNT_GOOD },
	{"RX_OCTET_COUNT_GOOD_BAD", RX_OCTET_COUNT_GOOD_BAD },
	{"RX_64OCTETS_PACKETS_GOOD_BAD", RX_64OCTETS_PACKETS_GOOD_BAD },
	{"RX_65TO127OCTETS_PACKETS_GOOD_BAD",
	 RX_65TO127OCTETS_PACKETS_GOOD_BAD },
	{"RX_128TO255OCTETS_PACKETS_GOOD_BAD",
	 RX_256TO511OCTETS_PACKETS_GOOD_BAD },
	{"RX_512TO1023OCTETS_PACKETS_GOOD_BAD",
	 RX_512TO1023OCTETS_PACKETS_GOOD_BAD },
	{"RX_1024TOMAXOCTETS_PACKETS_GOOD_BAD",
	 RX_1024TOMAXOCTETS_PACKETS_GOOD_BAD },
};

/* ELF helper functions */
static void elf32_shdr_swap_endian(Elf32_Shdr *shdr_p, u32 e_shnum)
{
	u32 index;
	Elf32_Shdr *shdr;

	for (index = 0U; index < e_shnum; index++) {
		shdr = &shdr_p[index];
		shdr->sh_name = swab32(shdr->sh_name);
		shdr->sh_type = swab32(shdr->sh_type);
		shdr->sh_flags = swab32(shdr->sh_flags);
		shdr->sh_addr = swab32(shdr->sh_addr);
		shdr->sh_offset = swab32(shdr->sh_offset);
		shdr->sh_size = swab32(shdr->sh_size);
		shdr->sh_link = swab32(shdr->sh_link);
		shdr->sh_info = swab32(shdr->sh_info);
		shdr->sh_addralign = swab32(shdr->sh_addralign);
		shdr->sh_entsize = swab32(shdr->sh_entsize);
	}
}

static void elf32_ehdr_swap_endian(Elf32_Ehdr *ehdr)
{
	ehdr->e_type = swab16(ehdr->e_type);
	ehdr->e_machine = swab16(ehdr->e_machine);
	ehdr->e_version = swab32(ehdr->e_version);
	ehdr->e_entry = swab32(ehdr->e_entry);
	ehdr->e_phoff = swab32(ehdr->e_phoff);
	ehdr->e_shoff = swab32(ehdr->e_shoff);
	ehdr->e_flags = swab32(ehdr->e_flags);
	ehdr->e_ehsize = swab16(ehdr->e_ehsize);
	ehdr->e_phentsize = swab16(ehdr->e_phentsize);
	ehdr->e_phnum = swab16(ehdr->e_phnum);
	ehdr->e_shentsize = swab16(ehdr->e_shentsize);
	ehdr->e_shnum = swab16(ehdr->e_shnum);
	ehdr->e_shstrndx = swab16(ehdr->e_shstrndx);
}

static void elf32_phdr_swap_endian(Elf32_Phdr *phdr_p, u32 e_phnum)
{
	u32 index;
	Elf32_Phdr *phdr;

	for (index = 0U; index < e_phnum; index++) {
		phdr = &phdr_p[index];
		phdr->p_type = swab32(phdr->p_type);
		phdr->p_offset = swab32(phdr->p_offset);
		phdr->p_vaddr = swab32(phdr->p_vaddr);
		phdr->p_paddr = swab32(phdr->p_paddr);
		phdr->p_filesz = swab32(phdr->p_filesz);
		phdr->p_memsz = swab32(phdr->p_memsz);
		phdr->p_flags = swab32(phdr->p_flags);
		phdr->p_align = swab32(phdr->p_align);
	}
}

int
pfe_emac_cfg_enable_duplex(void *base_va, bool enable)
{
	u32 reg = readl((u64)base_va + MAC_CONFIGURATION) & ~(DUPLEX_MODE);

	if (enable)
		reg |= DUPLEX_MODE;

	writel(reg, (u64)base_va + MAC_CONFIGURATION);

	return 0;
}

/* EMAC helper functions */
int pfeng_hw_emac_set_speed(void *base_va, u32 speed)
{
	u32 reg = readl((u64)base_va + MAC_CONFIGURATION) &
			~(PORT_SELECT | SPEED);

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

	writel(reg, (u64)base_va + MAC_CONFIGURATION);

	return 0;
}

int pfeng_hw_emac_mdio_read(void *base_va, u8 pa, s32 dev, uint16_t ra,
			    uint16_t *val)
{
	u32 reg = 0U;
	u32 retries = 500U;

	if (dev == MDIO_DEVAD_NONE) {
		/* C22 */
		reg = REG_DEV_ADDR(ra);
	} else {
		/* C45 */
		reg = GMII_REGISTER_ADDRESS(ra);
		writel(reg, base_va + MAC_MDIO_DATA);
		reg = CLAUSE45_ENABLE | REG_DEV_ADDR(dev);
	}

	reg |=	GMII_BUSY |
		GMII_OPERATION_CMD(GMII_READ) |
		CSR_CLOCK_RANGE(pfe.emac_mdio_div) |
		PHYS_LAYER_ADDR(pa);

	/*	Start a read operation */
	writel(reg, base_va + MAC_MDIO_ADDRESS);

	/*	Wait for completion */
	while (GMII_BUSY == (readl(base_va + MAC_MDIO_ADDRESS) & GMII_BUSY)) {
		if (retries-- == 0U)
			return -ETIME;

		ndelay(10000);
	}

	/*	Get the data */
	reg = readl(base_va + MAC_MDIO_DATA);
	*val = GMII_DATA(reg);

	return 0;
}

int pfeng_hw_emac_mdio_write(void *base_va, u8 pa, s32 dev, uint16_t ra,
			     uint16_t val)
{
	u32 reg = 0U;
	u32 retries = 500U;

	if (dev == MDIO_DEVAD_NONE) {
		/* C22 */
		reg = GMII_DATA(val);
		writel(reg, base_va + MAC_MDIO_DATA);
		reg = REG_DEV_ADDR(ra);
	} else {
		/* C45 */
		reg = GMII_DATA(val) | GMII_REGISTER_ADDRESS(ra);
		writel(reg, base_va + MAC_MDIO_DATA);
		reg = CLAUSE45_ENABLE | REG_DEV_ADDR(dev);
	}

	reg |=	GMII_BUSY | GMII_OPERATION_CMD(GMII_WRITE) |
		CSR_CLOCK_RANGE(pfe.emac_mdio_div) |
		PHYS_LAYER_ADDR(pa);

	/*	Start a write operation */
	writel(reg, base_va + MAC_MDIO_ADDRESS);
	/*	Wait for completion */
	while (GMII_BUSY == (readl(base_va + MAC_MDIO_ADDRESS) & GMII_BUSY)) {
		if (retries-- == 0U)
			return -ETIME;

		ndelay(10000);
	}

	return 0;
}

static u8 pfeng_hw_emac_mdio_div_decode(u64 csr_clk)
{
	u8 mdio_div = CSR_CLK_300_500_MHZ_MDC_CSR_DIV_204;

	if (csr_clk < 35 * MHZ)
		mdio_div = CSR_CLK_20_35_MHZ_MDC_CSR_DIV_16;
	else if ((csr_clk >= 35 * MHZ) && (csr_clk < 60 * MHZ))
		mdio_div = CSR_CLK_35_60_MHZ_MDC_CSR_DIV_26;
	else if ((csr_clk >= 60 * MHZ) && (csr_clk < 100 * MHZ))
		mdio_div = CSR_CLK_60_100_MHZ_MDC_CSR_DIV_42;
	else if ((csr_clk >= 100 * MHZ) && (csr_clk < 150 * MHZ))
		mdio_div = CSR_CLK_100_150_MHZ_MDC_CSR_DIV_62;
	else if ((csr_clk >= 150 * MHZ) && (csr_clk < 250 * MHZ))
		mdio_div = CSR_CLK_150_250_MHZ_MDC_CSR_DIV_102;
	else if ((csr_clk >= 250 * MHZ) && (csr_clk < 300 * MHZ))
		mdio_div = CSR_CLK_250_300_MHZ_MDC_CSR_DIV_124;
	else if ((csr_clk >= 300 * MHZ) && (csr_clk < 500 * MHZ))
		mdio_div = CSR_CLK_300_500_MHZ_MDC_CSR_DIV_204;
	else if ((csr_clk >= 500 * MHZ) && (csr_clk < 800 * MHZ))
		mdio_div = CSR_CLK_500_800_MHZ_MDC_CSR_DIV_324;
	else
		pr_err("PFE: Invalid csr_clk");

	pr_info("csr_clock %llu, mdio_div %u", csr_clk, mdio_div);

	return mdio_div;
}

/* PE helper functions */
static void pfeng_hw_pe_mem_write_idx(int pfe_idx, enum pfe_pe_mem mem,
				      u32 val, u32 addr, u8 size)
{
	u8 bytesel = 0U;
	u32 memsel;
	u32 offset;

	if (unlikely(addr & 0x3U)) {
		offset = BYTES_TO_4B_ALIGNMENT(addr);

		if (size <= offset) {
			val = val << (8U * (addr & 0x3U));
			bytesel = (((1U << size) - 1U) << (offset - size));
		} else {
			pfeng_hw_pe_mem_write_idx(pfe_idx, mem, val,
						  addr, offset);
			val >>= 8U * offset;
			size -= offset;
			addr += offset;
			pfeng_hw_pe_mem_write_idx(pfe_idx, mem, val,
						  addr, size);
			return;
		}
	} else {
		bytesel = ((1U << (size)) - 1U) << (4 - (size));
	}

	if (mem == PFE_PE_DMEM)
		memsel = PE_IBUS_ACCESS_DMEM;
	else
		memsel = PE_IBUS_ACCESS_IMEM;

	addr = (addr & 0xfffffU)	/* Address (low 20bits) */
	       | PE_IBUS_WRITE		/* Direction (r/w) */
	       | memsel			/* Memory selector */
	       | PE_IBUS_PE_ID(pfe_idx)	/* PE instance */
	       | PE_IBUS_WREN(bytesel); /* Byte(s) selector */

	writel(htonl(val), pfe.cbus_baseaddr + CLASS_MEM_ACCESS_WDATA);
	writel((u32)addr, pfe.cbus_baseaddr  + CLASS_MEM_ACCESS_ADDR);
}

static void pfeng_hw_pe_memset(int pe_idx, enum pfe_pe_mem mem, u8 val,
			       u64 addr, u32 len)
{
	u32 val32 = val | (val << 8) | (val << 16) | (val << 24);
	u32 offset;

	if (addr & 0x3U) {
		/*	Write unaligned bytes to align the address */
		offset = BYTES_TO_4B_ALIGNMENT(addr);
		offset = (len < offset) ? len : offset;
		pfeng_hw_pe_mem_write_idx(pe_idx, mem, val32, addr, offset);
		len = (len >= offset) ? (len - offset) : 0U;
		addr += offset;
	}

	for (; len >= 4U; len -= 4U, addr += 4U) {
		/*	Write aligned words */
		pfeng_hw_pe_mem_write_idx(pe_idx, mem, val32, addr, 4U);
	}

	if (len > 0U) {
		/*	Write the rest */
		pfeng_hw_pe_mem_write_idx(pe_idx, mem, val32, addr, len);
	}
}

static void pfeng_hw_pe_memcpy(int pe_idx, enum pfe_pe_mem memt, u64 dst,
			       const u8 *src, u32 len)
{
	u32 val;
	u32 offset;
	const u8 *src_byteptr = src;

	if (dst & 0x3U) {
		offset = BYTES_TO_4B_ALIGNMENT(dst);
		offset = (len < offset) ? len : offset;
		val = *(u32 *)src_byteptr;
		pfeng_hw_pe_mem_write_idx(pe_idx, memt, val, dst, offset);
		src_byteptr += offset;
		dst += offset;
		len = (len >= offset) ? (len - offset) : 0U;
	}

	for (; len >= 4U; len -= 4U, src_byteptr += 4U, dst += 4U) {
		/*	4-byte writes */
		val = *(u32 *)src_byteptr;
		pfeng_hw_pe_mem_write_idx(pe_idx, memt, val, (u32)dst, 4U);
	}

	if (len != 0U) {
		/*	The rest */
		val = *(u32 *)src_byteptr;
		pfeng_hw_pe_mem_write_idx(pe_idx, memt, val, (u32)dst, len);
	}
}

static int pfeng_hw_pe_load_section(int pe_idx, const void *data, u32 addr,
				    u32 size, u32 type)
{
	enum pfe_pe_mem memt;
	u32 base;

	if (addr >= PE_ADDR_LOW(DMEM) &&
	    ((addr + size) < PE_ADDR_HI(DMEM))) {
		/* Section belongs to DMEM */
		memt = PFE_PE_DMEM;
		base = PFE_CFG_CLASS_ELF_DMEM_BASE;

	} else if (addr >= PE_ADDR_LOW(IMEM) &&
		   ((addr + size) < PE_ADDR_HI(IMEM))) {
		/* Section belongs to IMEM */
		memt = PFE_PE_IMEM;
		base = PFE_CFG_CLASS_ELF_IMEM_BASE;

	} else {
		pr_err("Unsupported memory range 0x%x\n", addr);
		return -EINVAL;
	}

	switch (type) {
	case SHT_PFE_SKIP:
		break;
	case SHT_PROGBITS:
		pfeng_hw_pe_memcpy(pe_idx, memt, addr - base, data, size);
		break;
	case SHT_NOBITS:
		if (memt == PFE_PE_DMEM) {
			pfeng_hw_pe_memset(pe_idx, PFE_PE_DMEM, 0, addr, size);
			break;
		}
		/* Otherwise fallthrough to error */
	default:
		pr_err("Unsupported section type: 0x%x\n", type);
		return -EINVAL;
	}

	return 0;
}

static s32 pfeng_hw_pe_get_elf_sect_load_addr(Elf32_Phdr *phdr, s32 phdr_cnt,
					      Elf32_Shdr *shdr)
{
	s32 virt_addr = shdr->sh_addr;
	s32 load_addr;
	s32 offset;
	u32 ii;

	/* Go through all program headers to find one containing the section */
	for (ii = 0U; ii < phdr_cnt; ii++) {
		if (virt_addr >= phdr[ii].p_vaddr &&
		    (virt_addr <=
		     (phdr[ii].p_vaddr + phdr[ii].p_memsz - shdr->sh_size))) {
			offset = phdr[ii].p_vaddr - phdr[ii].p_paddr;
			load_addr = virt_addr - offset;
			return load_addr;
		}
	}
	/* No segment containing the section was found ! */
	pr_err("PFE: Translation of 0x%08x failed, fallback used\n", virt_addr);
	return 0;
}

static int pfeng_hw_pe_load_firmware(s32 pe_idx, u8 *fw,
				     struct pfe_ct_pe_mmap **memmap)
{
	s32 ii;
	s32 l_addr;
	u32 lcv = htonl(PFE_LOADCONF_ENABLE);
	void *buf;
	bool map_found = false;
	bool lc_found = false;
	int ret;
	char *names;
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)fw;
	Elf32_Shdr *shdr = (Elf32_Shdr *)((int64_t)fw + ehdr->e_shoff);
	Elf32_Phdr *phdr = (Elf32_Phdr *)((int64_t)fw + ehdr->e_phoff);
	struct pfe_ct_pe_mmap *fw_mmap = NULL;
	static const char mmap_version_str[] = TOSTRING(PFE_CFG_PFE_CT_H_MD5);

	names = (char *)((int64_t)fw + shdr[ehdr->e_shstrndx].sh_offset);
	for (ii = 0; ii < ehdr->e_shnum; ++ii) {
		if (!strcmp(".loadconf", &names[shdr[ii].sh_name])) {
			lc_found = true;
			break;
		}
	}

	if (lc_found) {
		memcpy(((u8 *)fw + shdr[ii].sh_offset), &lcv, sizeof(lcv));
		*memmap = NULL;
	} else {
		for (ii = 0; ii < ehdr->e_shnum; ++ii) {
			if (!strcmp(".pfe_pe_mmap", &names[shdr[ii].sh_name])) {
				map_found = true;
				break;
			}
		}

		if (!map_found) {
			pr_err("PFE: loadconf section is not available.\n");
			pr_err("PFE: PE Memory map is not available.\n");
			return -EINVAL;
		}

		fw_mmap = malloc(sizeof(struct pfe_ct_pe_mmap));
		if (!fw_mmap)
			return -ENOMEM;

		memcpy(fw_mmap, ((unsigned char *)fw + shdr[ii].sh_offset),
		       sizeof(struct pfe_ct_pe_mmap));
		if (strcmp(mmap_version_str, fw_mmap->version.cthdr) != 0) {
			pr_err("PFE: Unsupported CLASS firmware detected\n");
			pr_err("PFE: FW %d.%d.%d fwAPI:%s, required fwAPI %s\n",
			       fw_mmap->version.major, fw_mmap->version.minor,
			       fw_mmap->version.patch, fw_mmap->version.cthdr,
			       TOSTRING(PFE_CFG_PFE_CT_H_MD5));
			pr_warn("PFE function not guaranteed !!!\n");
		}
		pr_info("pfe_ct.h file version\"%s\"\n", mmap_version_str);
		/*	Indicate that mmap_data is available */
		*memmap = fw_mmap;
	}

	/*	Try to upload all sections of the .elf */
	for (ii = 0U; ii < ehdr->e_shnum; ii++) {
		if (!(shdr[ii].sh_flags & (ELF_SKIP_FLAGS)))
			continue;

		buf = (unsigned char *)fw + shdr[ii].sh_offset;

		/* Translate elf virtual address to load address */
		l_addr = pfeng_hw_pe_get_elf_sect_load_addr(phdr,
							    ehdr->e_phnum,
							    &shdr[ii]);
		if (l_addr == 0)
			return -EINVAL;

		ret = pfeng_hw_pe_load_section(pe_idx, buf, l_addr,
					       shdr[ii].sh_size,
					       shdr[ii].sh_type);

		if (ret != 0)
			pr_err("PFE: Couldn't upload firmware section\n");
	}

	return 0;
}

static int pfeng_hw_pe_check_mmap(struct pfe_ct_pe_mmap *pfe_pe_mmap)
{
	if (ntohl(pfe_pe_mmap->size) != sizeof(struct pfe_ct_pe_mmap)) {
		pr_err("Structure length mismatch: found %u required %u\n",
		       (u32)ntohl(pfe_pe_mmap->size),
		       (u32)sizeof(struct pfe_ct_pe_mmap));
		return -EINVAL;
	}

	pr_info("PFE [FW VERSION] %d.%d.%d, Build: %s, %s (%s), ID: 0x%x\n",
		pfe_pe_mmap->version.major, pfe_pe_mmap->version.minor,
		pfe_pe_mmap->version.patch,
		(char *)pfe_pe_mmap->version.build_date,
		(char *)pfe_pe_mmap->version.build_time,
		(char *)pfe_pe_mmap->version.vctrl,
		pfe_pe_mmap->version.id);

	pr_info("[PE MMAP]\n DMEM Heap Base: 0x%08x (%d bytes)\n"
		"PHY IF Base   : 0x%08x (%d bytes)\n",
		ntohl(pfe_pe_mmap->dmem_heap_base),
		ntohl(pfe_pe_mmap->dmem_heap_size),
		ntohl(pfe_pe_mmap->dmem_phy_if_base),
		ntohl(pfe_pe_mmap->dmem_phy_if_size));
	return 0;
}

static void pfeng_hw_flush_d(void *dat, u32 len)
{
	flush_dcache_range(rounddown((u64)dat, ARCH_DMA_MINALIGN),
			   roundup((u64)dat + len, ARCH_DMA_MINALIGN));
}

static void pfeng_hw_inval_d(void *dat, u32 len)
{
	invalidate_dcache_range(rounddown((u64)dat, ARCH_DMA_MINALIGN),
				roundup((u64)dat + len, ARCH_DMA_MINALIGN));
}

static int pfeng_hw_attach_ring(struct pfe_platform *platform,
				void *tx_ring, void *tx_ring_wb,
				void *rx_ring, void *rx_ring_wb)
{
	void *base = platform->hif_base;

	if (!tx_ring | !tx_ring_wb | !rx_ring | !rx_ring_wb) {
		pr_err("PFE: Invalid buffer descriptor rings");
		return -EINVAL;
	}

	/*	Set TX BD ring */
	writel((u32)((u64)tx_ring & 0xffffffffU),
	       base + HIF_TX_BDP_RD_LOW_ADDR_CHN(pfe.hif_chnl));
	writel(0U, base + HIF_TX_BDP_RD_HIGH_ADDR_CHN(pfe.hif_chnl));

	/*	Set TX WB BD ring */
	writel((u32)((u64)tx_ring_wb & 0xffffffffU),
	       base + HIF_TX_BDP_WR_LOW_ADDR_CHN(pfe.hif_chnl));
	writel(0U, base + HIF_TX_BDP_WR_HIGH_ADDR_CHN(pfe.hif_chnl));
	writel(RING_LEN,
	       base + HIF_TX_WRBK_BD_CHN_BUFFER_SIZE(pfe.hif_chnl));

	/*	Set RX BD ring */
	writel((u32)((u64)rx_ring & 0xffffffffU),
	       base + HIF_RX_BDP_RD_LOW_ADDR_CHN(pfe.hif_chnl));
	writel(0U, base + HIF_RX_BDP_RD_HIGH_ADDR_CHN(pfe.hif_chnl));

	/*	Set RX WB BD ring */
	writel((u32)((u64)rx_ring_wb & 0xffffffffU),
	       base + HIF_RX_BDP_WR_LOW_ADDR_CHN(pfe.hif_chnl));
	writel(0U, base + HIF_RX_BDP_WR_HIGH_ADDR_CHN(pfe.hif_chnl));
	writel(RING_LEN,
	       base + HIF_RX_WRBK_BD_CHN_BUFFER_SIZE(pfe.hif_chnl));

	return 0;
}

static struct pfe_hif_ring *pfeng_hw_init_ring(bool is_rx)
{
	u32 ii;
	u8 *offset = NULL;
	struct pfe_hif_ring *ring = malloc(sizeof(struct pfe_hif_ring));
	u32 page_size = 0x1000;
	size_t  size;

	if (!ring)
		return NULL;

	ring->write_idx = 0;
	ring->read_idx = 0;

	size = roundup(RING_LEN * sizeof(struct pfe_hif_bd), page_size);
	ring->bd = memalign(max((u32)RING_BD_ALIGN, page_size), size);

	if (!ring->bd) {
		pr_warn("HIF ring couldn't be allocated.\n");
		goto err_with_ring;
	}

	size = roundup(RING_LEN * sizeof(struct pfe_hif_wb_bd), page_size);
	ring->wb_bd = memalign(max((u32)RING_BD_ALIGN, page_size), size);

	if (!ring->wb_bd) {
		pr_warn("HIF ring couldn't be allocated.\n");
		goto err_with_bd;
	}

	ring->is_rx = is_rx;

	memset(ring->bd, 0, RING_LEN * sizeof(struct pfe_hif_bd));

	if (ring->is_rx) {
		/* fill buffers */
		ring->mem = memalign(page_size,
				     PFE_BUF_SIZE * PFE_HIF_RING_CFG_LENGTH);
		offset = ring->mem;
		if (!ring->mem)
			goto err_with_wb_bd;
	}

	/* Flushe cache to update MMU mapings */
	flush_dcache_all();

	mmu_set_region_dcache_behaviour((phys_addr_t)ring->bd,
					size, DCACHE_OFF);
	mmu_set_region_dcache_behaviour((phys_addr_t)ring->wb_bd,
					size, DCACHE_OFF);

	for (ii = 0; ii < RING_LEN; ii++) {
		if (ring->is_rx) {
			/*	Mark BD as RX */
			ring->bd[ii].dir = 1U;
			/* Add buffer to rx descriptor */
			ring->bd[ii].desc_en = 1U;
			ring->bd[ii].lifm = 1U;
			ring->bd[ii].buflen = (u16)PFE_BUF_SIZE;
			ring->bd[ii].data = (u32)(u64)offset;
			offset = (void *)((u64)offset + PFE_BUF_SIZE);
		}

		/*	Enable BD interrupt */
		ring->bd[ii].cbd_int_en = 1U;
		ring->bd[ii].next = (u32)(u64)&ring->bd[ii + 1U];
		pfeng_hw_flush_d(&ring->bd[ii], sizeof(struct pfe_hif_bd));
	}

	ring->bd[ii - 1].next = (u32)(u64)&ring->bd[0];
	ring->bd[ii - 1].last_bd = 1U;
	pfeng_hw_flush_d(&ring->bd[ii - 1], sizeof(struct pfe_hif_bd));

	memset(ring->wb_bd, 0, RING_LEN * sizeof(struct pfe_hif_wb_bd));

	for (ii = 0U; ii < RING_LEN; ii++) {
		ring->wb_bd[ii].seqnum = 0xffffU;
		ring->wb_bd[ii].desc_en = 1U;
		pfeng_hw_flush_d(&ring->wb_bd[ii],
				 sizeof(struct pfe_hif_wb_bd));
	}

	debug("BD ring %p\nWB ring %p\nBuff %p\n",
	      ring->bd, ring->wb_bd, ring->mem);

	return ring;

err_with_wb_bd:
	free(ring->wb_bd);
err_with_bd:
	free(ring->bd);
err_with_ring:
	free(ring);
	return NULL;
}

static void pfeng_hw_deinit_ring(struct pfe_hif_ring *ring)
{
	if (!ring)
		return;

	if (ring->mem)
		free(ring->mem);
	if (ring->wb_bd)
		free(ring->wb_bd);
	if (ring->bd)
		free(ring->bd);
	free(ring);
}

struct pfe_hw_chnl *pfeng_hw_init_chnl(void)
{
	int ret = 0;
	struct pfe_hw_chnl *chnl = malloc(sizeof(struct pfe_hw_chnl));

	if (!chnl)
		return NULL;

	/* init TX ring */
	chnl->tx_ring = pfeng_hw_init_ring(false);
	if (!chnl->tx_ring)
		goto err_free;

	/* init RX ring */
	chnl->rx_ring = pfeng_hw_init_ring(true);
	if (!chnl->rx_ring)
		goto err_tx;

	/* register rings to pfe HW */
	ret = pfeng_hw_attach_ring(&pfe,
				   chnl->tx_ring->bd, chnl->tx_ring->wb_bd,
				   chnl->rx_ring->bd, chnl->rx_ring->wb_bd);
	if (ret)
		goto err_rx;

	return chnl;

err_rx:
	pfeng_hw_deinit_ring(chnl->rx_ring);
err_tx:
	pfeng_hw_deinit_ring(chnl->tx_ring);
err_free:
	free(chnl);
	return NULL;
}

void pfeng_hw_deinit_chnl(struct pfe_hw_chnl *chnl)
{
	if (!chnl)
		return;

	if (chnl->rx_ring) {
		if (chnl->rx_ring->mem) {
			free(chnl->rx_ring->mem);
			chnl->rx_ring->mem = NULL;
		}
		if (chnl->rx_ring->bd) {
			free(chnl->rx_ring->bd);
			chnl->rx_ring->bd = NULL;
		}
		if (chnl->rx_ring->wb_bd) {
			free(chnl->rx_ring->wb_bd);
			chnl->rx_ring->wb_bd = NULL;
		}

		free(chnl->rx_ring);
		chnl->rx_ring = NULL;
	}

	if (chnl->tx_ring) {
		if (chnl->tx_ring->bd) {
			free(chnl->tx_ring->bd);
			chnl->tx_ring->bd = NULL;
		}
		if (chnl->tx_ring->wb_bd) {
			free(chnl->tx_ring->wb_bd);
			chnl->tx_ring->wb_bd = NULL;
		}

		free(chnl->tx_ring);
		chnl->tx_ring = NULL;
	}
}

int pfeng_hw_chnl_xmit(struct pfe_hw_chnl *chnl, int emac,
		       void *packet, int length)
{
	struct pfe_hif_ring *ring = chnl->tx_ring;
	struct pfe_hif_bd *bd_hd, *bd_pkt, *bp_rd;
	struct pfe_hif_wb_bd *wb_bd_hd, *wb_bd_pkt, *wb_bp_rd;
	bool lifm = false;

	/* Get descriptor for header */
	bd_hd = &ring->bd[ring->write_idx & RING_LEN_MASK];
	wb_bd_hd = &ring->wb_bd[ring->write_idx & RING_LEN_MASK];

	/* Get descriptor for packet */
	bd_pkt = &ring->bd[(ring->write_idx + 1) & RING_LEN_MASK];
	wb_bd_pkt = &ring->wb_bd[(ring->write_idx + 1) & RING_LEN_MASK];

	pfeng_hw_inval_d(bd_hd, sizeof(struct pfe_hif_bd));
	pfeng_hw_inval_d(bd_pkt, sizeof(struct pfe_hif_bd));

	if (RING_BD_DESC_EN(bd_hd->ctrl) != 0U ||
	    RING_BD_DESC_EN(bd_pkt->ctrl) != 0U)
		return -EAGAIN;

	/* Flush the data buffer */
	pfeng_hw_flush_d(packet, length);

	/* Fill header */
	bd_hd->data = (u64)&header[emac];
	bd_hd->buflen = (u16)sizeof(struct pfe_ct_hif_tx_hdr);
	bd_hd->status = 0U;
	bd_hd->lifm = 0;
	wb_bd_hd->desc_en = 1U;
	dmb();
	bd_hd->desc_en = 1U;

	/* Fill packet */
	bd_pkt->data = (u32)(u64)packet;
	bd_pkt->buflen = (uint16_t)length;
	bd_pkt->status = 0U;
	bd_pkt->lifm = 1;
	wb_bd_pkt->desc_en = 1U;
	dmb();
	bd_pkt->desc_en = 1U;

	/* Increment index for next buffer descriptor */
	ring->write_idx += 2;

	/* Tx Confirmation */
	while (1) {
		lifm = false;
		bp_rd = &ring->bd[ring->read_idx & RING_LEN_MASK];
		wb_bp_rd = &ring->wb_bd[ring->read_idx & RING_LEN_MASK];

		pfeng_hw_inval_d(bp_rd, sizeof(struct pfe_hif_bd));
		pfeng_hw_inval_d(wb_bp_rd, sizeof(struct pfe_hif_wb_bd));

		if (RING_BD_DESC_EN(bp_rd->ctrl) == 0 ||
		    RING_WBBD_DESC_EN(wb_bp_rd->ctrl) != 0)
			continue;

		lifm = bp_rd->lifm;
		bp_rd->desc_en = 0U;
		wb_bp_rd->desc_en = 1U;
		dmb();
		ring->read_idx++;

		if (lifm)
			break;
	}

	return 0;
}

int pfeng_hw_chnl_receive(struct pfe_hw_chnl *chnl, int flags, uchar **packetp)
{
	struct pfe_hif_bd *bd_pkt;
	struct pfe_hif_wb_bd *wb_bd_pkt;
	struct pfe_hif_ring *ring = chnl->rx_ring;
	int plen = 0;

	bd_pkt = &ring->bd[ring->read_idx & RING_LEN_MASK];
	wb_bd_pkt = &ring->wb_bd[ring->read_idx & RING_LEN_MASK];

	pfeng_hw_inval_d(bd_pkt, sizeof(struct pfe_hif_bd));
	pfeng_hw_inval_d(wb_bd_pkt, sizeof(struct pfe_hif_wb_bd));

	/* Check, if we received some data */
	if (RING_BD_DESC_EN(bd_pkt->ctrl) == 0U ||
	    RING_WBBD_DESC_EN(wb_bd_pkt->ctrl) != 0u)
		return -EAGAIN;

	/* Give the data to u-boot stack */
	bd_pkt->desc_en = 0U;
	wb_bd_pkt->desc_en = 1U;
	dmb();
	*packetp = ((void *)((u64)(bd_pkt->data) + HIF_HEADER_SIZE));
	plen = wb_bd_pkt->buflen - HIF_HEADER_SIZE;

	/* Advance read buffer */
	ring->read_idx++;

	/* Invalidate the buffer */
	pfeng_hw_inval_d(*packetp, plen);

	if (wb_bd_pkt->lifm != 1U) {
		printf("Multi buffer packets not supported, discarding.\n");
		/* Return EOK so the stack frees the buffer */
		return 0;
	}

	return plen;
}

int pfeng_hw_chnl_free_pkt(struct pfe_hw_chnl *chnl, uchar *packet, int length)
{
	struct pfe_hif_ring *ring = chnl->rx_ring;
	struct pfe_hif_bd *bd_pkt;
	struct pfe_hif_wb_bd *wb_bd_pkt;

	bd_pkt = &ring->bd[ring->write_idx & RING_LEN_MASK];
	wb_bd_pkt = &ring->wb_bd[ring->write_idx & RING_LEN_MASK];

	pfeng_hw_inval_d(bd_pkt, sizeof(struct pfe_hif_bd));
	pfeng_hw_inval_d(wb_bd_pkt, sizeof(struct pfe_hif_wb_bd));

	if (bd_pkt->desc_en != 0U) {
		pr_err("Can't free buffer since the BD entry is used\n");
		return -EIO;
	}

	/* Free buffer */
	bd_pkt->buflen = 2048;
	bd_pkt->status = 0U;
	bd_pkt->lifm = 1U;
	wb_bd_pkt->desc_en = 1U;
	dmb();
	bd_pkt->desc_en = 1U;

	/* This has to be here for correct HW functionality */
	pfeng_hw_flush_d(packet, length);
	pfeng_hw_inval_d(packet, length);

	/* Advance free pointer */
	ring->write_idx++;

	return 0;
}

/* PFE HW functions */
static int pfeng_hw_init_hif(struct pfe_platform *platform,
			     struct pfe_platform_config *config)
{
	u32 ii = 0, reg;
	void *base;

	platform->hif_base = INIT_PLAT_OFF(platform, CBUS_HIF_BASE_ADDR);
	base = platform->hif_base;

	/* Disable and clear HIF interrupts */
	writel(0U, base + HIF_ERR_INT_EN);
	writel(0U, base + HIF_TX_FIFO_ERR_INT_EN);
	writel(0U, base + HIF_RX_FIFO_ERR_INT_EN);
	writel(0xffffffffU, base + HIF_ERR_INT_SRC);
	writel(0xffffffffU, base + HIF_TX_FIFO_ERR_INT_SRC);
	writel(0xffffffffU, base + HIF_RX_FIFO_ERR_INT_SRC);

	/* SOFT RESET */
	writel(HIF_SOFT_RESET_CMD, base + HIF_SOFT_RESET);
	while (readl(base + HIF_SOFT_RESET) & ~CSR_SW_RESET) {
		if (++ii < 1000u) {
			mdelay(1);
		} else {
			pr_err("HIF reset timed out.\n");
			return -ETIMEDOUT;
		}
	}
	writel(0, base + HIF_SOFT_RESET);

	/* Numbers of poll cycles */
	writel((0xff << 16) | (0xff), base + HIF_TX_POLL_CTRL);
	writel((0xff << 16) | (0xff), base + HIF_RX_POLL_CTRL);

	/* MICS */
	writel(0U | HIF_TIMEOUT_EN | BD_START_SEQ_NUM(0x0), base + HIF_MISC);

	/* Timeout */
	writel(100000000U, base + HIF_TIMEOUT_REG);

	/* TMU queue mapping. 0,1->ch.0, 2,3->ch.1, 4,5->ch.2, 6,7->ch.3 */
	writel(0x33221100U, base + HIF_RX_QUEUE_MAP_CH_NO_ADDR);

	/* DMA burst size */
	writel(0x0U, base + HIF_DMA_BURST_SIZE_ADDR);

	/* DMA base address */
	writel(0x0U, base + HIF_DMA_BASE_ADDR);

	/* LTC reset (should not be used, but for sure...) */
	writel(0x0U, base + HIF_LTC_PKT_CTRL_ADDR);

	/* Disable channel interrupts */
	writel(0U, base + HIF_CHN_INT_EN(platform->hif_chnl));
	writel(0xffffffffU, base + HIF_CHN_INT_SRC(platform->hif_chnl));

	/* Disable RX & TX DMA engine and polling */
	reg = readl(base + HIF_CTRL_CHN(platform->hif_chnl));
	reg &= ~((u32)RX_DMA_ENABLE | RX_BDP_POLL_CNTR_EN);
	reg &= ~((u32)TX_DMA_ENABLE | TX_BDP_POLL_CNTR_EN);
	writel(reg, base + HIF_CTRL_CHN(platform->hif_chnl));

	/* Disable interrupt coalescing */
	writel(0x0U, base + HIF_INT_COAL_EN_CHN(platform->hif_chnl));
	writel(0x0U, base + HIF_ABS_INT_TIMER_CHN(platform->hif_chnl));
	writel(0x0U, base + HIF_ABS_FRAME_COUNT_CHN(platform->hif_chnl));

	/* LTC reset */
	writel(0x0U, base + HIF_LTC_MAX_PKT_CHN_ADDR(platform->hif_chnl));

	return 0;
}

static void pfeng_hw_deinit_hif(struct pfe_platform *platform)
{
	u32 reg;
	/*	Disable RX & TX DMA engine and polling */
	reg = readl(platform->hif_base + HIF_CTRL_CHN(platform->hif_chnl));
	reg &= ~((u32)RX_DMA_ENABLE | RX_BDP_POLL_CNTR_EN);
	reg &= ~((u32)TX_DMA_ENABLE | TX_BDP_POLL_CNTR_EN);
	writel(reg, platform->hif_base + HIF_CTRL_CHN(platform->hif_chnl));
}

/**
 * @brief		Assign BMU to the platform
 */
static int pfeng_hw_init_bmu(struct pfe_platform *platform)
{
	struct pfe_bmu_cfg bmu_cfg[2] = {
		{(void *)(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_LMEM_BASE_ADDR),
		PFE_CFG_BMU1_BUF_COUNT, PFE_CFG_BMU1_BUF_SIZE,
		0x200U, 0x200U, 64U, 64U},
		{NULL,
		PFE_CFG_BMU2_BUF_COUNT, PFE_CFG_BMU2_BUF_SIZE,
		0x10U, 0x10U, 1024U, 1024U }
	};
	int ii, mem;
	void *base;

	/*	Create storage for instances */
	platform->bmu_base = malloc(platform->bmu_count * sizeof(void *));
	if (!platform->bmu_base) {
		pr_err("malloc() failed\n");
		return -ENOMEM;
	}

	platform->bmu_base[0] = INIT_PLAT_OFF(platform, CBUS_BMU1_BASE_ADDR);
	platform->bmu_base[1] = INIT_PLAT_OFF(platform, CBUS_BMU2_BASE_ADDR);

	platform->bmu_buffers_size = PFE_CFG_BMU2_BUF_COUNT *
				     (1U << PFE_CFG_BMU2_BUF_SIZE);
	if (platform->bmu_buffers_size != platform->cfg->bmu_addr_size) {
		pr_err("PFE: BMU expecteted mem size is: 0x%llX check dtb config\n",
		       platform->bmu_buffers_size);
		return -EINVAL;
	}
	platform->bmu_buffers_va = (void *)(u64)platform->cfg->bmu_addr;
	/* memalign(platform->bmu_buffers_size, platform->bmu_buffers_size);*/

	if (!platform->bmu_buffers_va) {
		pr_err("PFE: Unable to get BMU2 pool memory\n");
		return -ENOMEM;
	}

	bmu_cfg[1].pool_pa = platform->bmu_buffers_va;

	/* PFE AXI MASTERs can only access range p0x00020000 - p0xbfffffff */
	if (((u64)bmu_cfg[1].pool_pa < 0x00020000U) ||
	    (((u64)bmu_cfg[1].pool_pa + platform->bmu_buffers_size)) >
		    0xbfffffffU) {
		pr_err("BMU2 buffers not in required range: starts @ p0x%p\n",
		       bmu_cfg[1].pool_pa);
	} else {
		pr_info("BMU2 buffer base: p0x%p\n",
			bmu_cfg[1].pool_pa);
	}

	pr_info("BMU1 buffer base: p0x%p\n", bmu_cfg[0].pool_pa);
	pr_info("BMU2 buffer base: p0x%p\n", bmu_cfg[1].pool_pa);

	for (ii = 0; ii < platform->bmu_count; ii++) {
		base = platform->bmu_base[ii];
		/* Disable */
		writel(0x0U, base + BMU_CTRL);

		/* Disable and clear BMU interrupts */
		writel(0x0U, base + BMU_INT_ENABLE);
		writel(0xffffffffU, base + BMU_INT_SRC);

		writel((u32)((u64)bmu_cfg[ii].pool_pa & 0xffffffffU),
		       base + BMU_UCAST_BASEADDR);
		writel(bmu_cfg[ii].max_buf_cnt & 0xffffU,
		       base + BMU_UCAST_CONFIG);
		writel(bmu_cfg[ii].buf_size & 0xffffU, base + BMU_BUF_SIZE);

		/* Thresholds 75%  */
		writel((bmu_cfg[ii].max_buf_cnt * 75U) / 100U,
		       base + BMU_THRES);

		/* Clear internal memories */
		for (mem = 0U; mem < bmu_cfg[ii].int_mem_loc_cnt; mem++) {
			writel(mem, base + BMU_INT_MEM_ACCESS_ADDR);
			writel(0U, base + BMU_INT_MEM_ACCESS);
			writel(0U, base + BMU_INT_MEM_ACCESS2);
		}

		for (mem = 0U; mem < bmu_cfg[ii].buf_mem_loc_cnt; mem++) {
			writel(mem, base + BMU_INT_MEM_ACCESS_ADDR);
			writel(0U, base + BMU_INT_MEM_ACCESS);
			writel(0U, base + BMU_INT_MEM_ACCESS2);
		}

		/* Enable BMU interrupts except the global enable bit */
		writel(0xffffffffU & ~(BMU_INT), base + BMU_INT_ENABLE);
	}

	return 0;
}

static void pfeng_hw_deinit_bmu(struct pfe_platform *platform)
{
	u32 ii;

	if (!platform->bmu_base)
		goto free_return;

	for (ii = 0; ii < platform->bmu_count; ii++) {
		if (!platform->bmu_base[ii])
			continue;

		writel(0x0U, platform->bmu_base[ii] + BMU_CTRL);
		/*	Disable and clear BMU interrupts */
		writel(0x0U, platform->bmu_base[ii] + BMU_INT_ENABLE);
		writel(0xffffffffU, platform->bmu_base[ii] + BMU_INT_SRC);
		platform->bmu_base[ii] = NULL;
	}

	free(platform->bmu_base);
	platform->bmu_base = NULL;

free_return:
	if (platform->bmu_buffers_va) {
		free(platform->bmu_buffers_va);
		platform->bmu_buffers_va = NULL;
	}
}

static int
pfeng_hw_init_gpi(struct pfe_platform *platform)
{
	int index;
	struct pfe_gpi_cfg gpi_cfg[] = {
		{0x200U, 0x178U, 0x40U, true, true}, /*EGPI1*/
		{0x200U, 0x178U, 0x40U, true, true}, /*EGPI2*/
		{0x200U, 0x178U, 0x40U, true, true}, /*EGPI3*/
		{0x200U, 0xbcU, 0x40U, true, false}, /*ETPI1*/
		{0x200U, 0xbcU, 0x40U, true, false}, /*ETPI2*/
		{0x200U, 0xbcU, 0x40U, true, false}, /*ETPI3*/
		{0x200U, 0x178U, 0x40U, false, false}, /*HGPI1*/
	};

	platform->gpi_base = malloc(platform->gpi_total * sizeof(void *));
	if (!platform->gpi_base) {
		pr_err("Was not possible to initialize gpis\n");
		return -ENODEV;
	}

	platform->gpi_base[0] = INIT_PLAT_OFF(platform, CBUS_EGPI1_BASE_ADDR);
	platform->gpi_base[1] = INIT_PLAT_OFF(platform, CBUS_EGPI2_BASE_ADDR);
	platform->gpi_base[2] = INIT_PLAT_OFF(platform, CBUS_EGPI3_BASE_ADDR);
	platform->gpi_base[3] = INIT_PLAT_OFF(platform, CBUS_ETGPI1_BASE_ADDR);
	platform->gpi_base[4] = INIT_PLAT_OFF(platform, CBUS_ETGPI2_BASE_ADDR);
	platform->gpi_base[5] = INIT_PLAT_OFF(platform, CBUS_ETGPI3_BASE_ADDR);
	platform->gpi_base[6] = INIT_PLAT_OFF(platform, CBUS_HGPI_BASE_ADDR);

	for (index = 0; index < platform->gpi_total; index++) {
		u32 timeout = 20U;
		u32 reg;
		u32 ii;
		u32 val;
		void *base = platform->gpi_base[index];

		/* Reset GPI */
		reg = readl(base + GPI_CTRL);
		reg |= 0x2U;
		writel(reg, base + GPI_CTRL);

		do {
			ndelay(100000);
			reg = readl(base + GPI_CTRL);
			} while ((reg & 0x2U) && (--timeout > 0U));

		if (timeout == 0U) {
			pr_err("Init failed GPI%d\n", index);
			return -ETIMEDOUT;
		}

		/* Disable GPI */
		reg = readl(platform->gpi_base[index] + GPI_CTRL);
		writel(reg & ~(0x1U), platform->gpi_base[index] + GPI_CTRL);

		/* INIT QOS */
		writel(0U, base + CSR_IGQOS_ENTRY_DATA_REG0);
		writel(0U, base + CSR_IGQOS_ENTRY_DATA_REG1);
		writel(0U, base + CSR_IGQOS_ENTRY_DATA_REG2);
		writel(0U, base + CSR_IGQOS_ENTRY_DATA_REG3);
		writel(0U, base + CSR_IGQOS_ENTRY_DATA_REG4);
		writel(0U, base + CSR_IGQOS_ENTRY_DATA_REG5);
		writel(0U, base + CSR_IGQOS_ENTRY_DATA_REG6);
		writel(0U, base + CSR_IGQOS_ENTRY_DATA_REG7);

		/*	Entry table */
		for (ii = 0U; gpi_cfg[index].ingress &&
		     ii < IGQOS_ENTRY_TABLE_LEN; ii++) {
			val = CMDCNTRL_CMD_WRITE | CMDCNTRL_CMD_TAB_ADDR(ii);
			writel(val, base + CSR_IGQOS_ENTRY_CMDCNTRL);
			val = CMDCNTRL_CMD_WRITE | CMDCNTRL_CMD_TAB_ADDR(ii) |
			      CMDCNTRL_CMD_TAB_SELECT_LRU;
			writel(val, base + CSR_IGQOS_ENTRY_CMDCNTRL);
		}

		/*	GPI_EMAC_1588_TIMESTAMP_EN */
		writel(0x0U, base + GPI_EMAC_1588_TIMESTAMP_EN);

		/*	GPI_EMAC_1588_TIMESTAMP_EN */
		if (gpi_cfg[index].emac_1588_ts_en) {
			reg = readl(base + GPI_EMAC_1588_TIMESTAMP_EN);
			reg |= 0xe01U;
			writel(reg, base + GPI_EMAC_1588_TIMESTAMP_EN);
		}
		/*	GPI_RX_CONFIG */
		writel(((gpi_cfg[index].alloc_retry_cycles << 16) |
			GPI_DDR_BUF_EN |
			GPI_LMEM_BUF_EN),
			base + GPI_RX_CONFIG);
		/*	GPI_HDR_SIZE */
		writel((PFE_CFG_DDR_HDR_SIZE << 16) | PFE_CFG_LMEM_HDR_SIZE,
		       base + GPI_HDR_SIZE);
		/*	GPI_BUF_SIZE */
		writel((PFE_CFG_DDR_BUF_SIZE << 16) | PFE_CFG_LMEM_BUF_SIZE,
		       base + GPI_BUF_SIZE);

		/*	DDR config */
		writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU1_BASE_ADDR +
		       BMU_ALLOC_CTRL, base + GPI_LMEM_ALLOC_ADDR);
		writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU1_BASE_ADDR +
		       BMU_FREE_CTRL, base + GPI_LMEM_FREE_ADDR);
		writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU2_BASE_ADDR +
		       BMU_ALLOC_CTRL, base + GPI_DDR_ALLOC_ADDR);
		writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU2_BASE_ADDR +
				    BMU_FREE_CTRL,
				    base + GPI_DDR_FREE_ADDR);

		/*	GPI_CLASS_ADDR */
		writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + CLASS_INQ_PKTPTR,
		       base + GPI_CLASS_ADDR);
		/*	GPI_DDR_DATA_OFFSET */
		writel(PFE_CFG_DDR_HDR_SIZE, base + GPI_DDR_DATA_OFFSET);
		writel(0x30U, base + GPI_LMEM_DATA_OFFSET);
		writel(PFE_CFG_LMEM_HDR_SIZE,
		       base + GPI_LMEM_SEC_BUF_DATA_OFFSET);
		writel(gpi_cfg[index].gpi_tmlf_txthres, base + GPI_TMLF_TX);
		writel(gpi_cfg[index].gpi_dtx_aseq_len, base + GPI_DTX_ASEQ);

		/*	IP/TCP/UDP Checksum Offload */
		writel(1, base + GPI_CSR_TOE_CHKSUM_EN);

		pr_info("GPI%d block was initialized\n", index);
	}

	return 0;
}

static void
pfeng_hw_deinit_gpi(struct pfe_platform *platform)
{
	u32 ii;
	u32 reg;
	void *base;

	if (platform->gpi_base) {
		for (ii = 0U; ii < platform->gpi_total; ii++) {
			base = platform->gpi_base[ii];
			if (!base)
				continue;

			/* Disable and reset */
			reg = readl(base + GPI_CTRL);
			reg |= 0x2U;
			writel(reg & ~(0x1U), base + GPI_CTRL);
			writel(reg, base + GPI_CTRL);
			platform->gpi_base[ii] = NULL;
		}

		free(platform->gpi_base);
		platform->gpi_base = NULL;
	}
}

static int
pfeng_hw_init_class(struct pfe_platform *platform)
{
	void *base = platform->cbus_baseaddr;
	int ret = 0;
	int ii;
	u8 *fw;
	Elf32_Shdr *shdr = NULL;
	Elf32_Ehdr *ehdr = NULL;
	Elf32_Phdr *phdr = NULL;

	if (!platform->fw) {
		pr_err("The CLASS firmware is NULL\n");
		return -EINVAL;
	}

	if (!platform->fw->class_data || platform->fw->class_size == 0U) {
		pr_err("The CLASS firmware is not loaded\n");
		return -EIO;
	}

	/* Init CLASS Mem */
	for (ii = 0; ii < platform->class_pe_count; ii++) {
		pfeng_hw_pe_memset(ii, PFE_PE_DMEM, 0U, 0U,
				   PFE_CFG_CLASS_DMEM_SIZE);
		pfeng_hw_pe_memset(ii, PFE_PE_IMEM, 0U, 0U,
				   PFE_CFG_CLASS_IMEM_SIZE);
	}

	/*	Issue block reset */
	writel(PFE_CORE_DISABLE, base + CLASS_TX_CTRL);
	writel(PFE_CORE_SW_RESET, base + CLASS_TX_CTRL);
	ndelay(10000);
	writel(PFE_CORE_DISABLE, base + CLASS_TX_CTRL);

	writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU1_BASE_ADDR +
	       BMU_FREE_CTRL,
	       base + CLASS_BMU1_BUF_FREE);

	/*	Config Queue/Reorder buffers */
	writel(CLASS_PE0_RO_DM_ADDR0_VAL, base + CLASS_PE0_RO_DM_ADDR0);
	writel(CLASS_PE0_RO_DM_ADDR1_VAL, base + CLASS_PE0_RO_DM_ADDR1);
	writel(CLASS_PE0_QB_DM_ADDR0_VAL, base + CLASS_PE0_QB_DM_ADDR0);
	writel(CLASS_PE0_QB_DM_ADDR1_VAL, base + CLASS_PE0_QB_DM_ADDR1);

	/*	TMU Config */
	writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + TMU_PHY_INQ_PKTPTR,
	       base + CLASS_TM_INQ_ADDR);
	writel(0x18U, base + CLASS_MAX_BUF_CNT);
	writel(0x14U, base + CLASS_AFULL_THRES);
	writel(0x3c0U, base + CLASS_INQ_AFULL_THRES);
	writel(0x1U, base + CLASS_USE_TMU_INQ);

	/*	System clock */
	writel(0x1U, base + CLASS_PE_SYS_CLK_RATIO);

	/*	Disable TCP/UDP/IPv4 checksum drop */
	writel(0U, base + CLASS_L4_CHKSUM);

	/* Configure the LMEM header size and RO  */
	writel((PFE_CFG_RO_HDR_SIZE << 16) | PFE_CFG_LMEM_HDR_SIZE,
	       base + CLASS_HDR_SIZE);
	writel(PFE_CFG_LMEM_BUF_SIZE, base + CLASS_LMEM_BUF_SIZE);

	writel(0U | QB2BUS_ENDIANNESS, base + CLASS_ROUTE_MULTI);

	/* Prepare FW file*/
	fw = (u8 *)platform->fw->class_data;
	ehdr = (Elf32_Ehdr *)fw;

	if (IS_ELF(*ehdr)) {
		pr_info("Uploading CLASS firmware\n");

		/*	.elf data must be in BIG ENDIAN */
		if (ehdr->e_ident[EI_DATA] == 1U) {
			pr_err("Unexpected .elf format (little endian)\n");
			return -EINVAL;
		}

		elf32_ehdr_swap_endian(ehdr);
		shdr = (Elf32_Shdr *)((uint64_t)fw + ehdr->e_shoff);
		phdr = (Elf32_Phdr *)((uint64_t)fw + ehdr->e_phoff);
		elf32_shdr_swap_endian(shdr, ehdr->e_shnum);
		elf32_phdr_swap_endian(phdr, ehdr->e_phnum);

		for (ii = 0U; ii < platform->class_pe_count; ii++) {
			ret = pfeng_hw_pe_load_firmware(ii, fw,
							&platform->memmap);

			if (ret != 0) {
				pr_err("FW load failed PE %u : %d\n", ii, ret);
				break;
			}
		}
		if (ret != 0) {
			pr_err("Error during upload of CLASS firmware\n");
			return -EIO;
		}

		if (platform->memmap) {
			ret = pfeng_hw_pe_check_mmap(platform->memmap);
			if (ret)
				return ret;
		}

	} else {
		pr_err("Only ELF format is supported\n");
		return -ENODEV;
	}

	return 0;
}

static void
pfeng_hw_deinit_class(struct pfe_platform *platform)
{
	writel(PFE_CORE_DISABLE, platform->cbus_baseaddr + CLASS_TX_CTRL);
	free(platform->memmap);
}

static int
pfeng_hw_init_tmu(struct pfe_platform *platform)
{
	u32 timeout = 20U;
	u32 reg;
	u32 ii, queue, cnt;
	void *qos_base;
	void *base = platform->cbus_baseaddr;
	static const int phys[] = {
		PFE_PHY_IF_ID_EMAC0, PFE_PHY_IF_ID_EMAC1, PFE_PHY_IF_ID_EMAC2,
		PFE_PHY_IF_ID_HIF_NOCPY, PFE_PHY_IF_ID_HIF
	};

	writel(0x1U, base + TMU_CTRL);

	do {
		ndelay(100000U);
		timeout--;
		reg = readl(base + TMU_CTRL);
	} while ((0U != (reg & 0x1U)) && (timeout > 0U));

	if (timeout == 0U)
		pr_err("FATAL: TMU reset timed-out\n");

	writel(0x0U, base + TMU_PHY0_TDQ_CTRL); /* EMAC0 */
	writel(0x0U, base + TMU_PHY1_TDQ_CTRL); /* EMAC1 */
	writel(0x0U, base + TMU_PHY2_TDQ_CTRL); /* EMAC2 */
	writel(0x0U, base + TMU_PHY3_TDQ_CTRL); /* HIF */

	/*	Reset */
	writel(0x1U, base + TMU_CTRL);

	do {
		ndelay(100000U);
		timeout--;
		reg = readl(base + TMU_CTRL);
	} while ((0U != (reg & 0x1U)) && (timeout > 0U));

	if (timeout == 0U)
		pr_err("FATAL: TMU reset timed-out\n");

	/*	INQ */
	writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_EGPI1_BASE_ADDR +
			    GPI_INQ_PKTPTR,
			    base + TMU_PHY0_INQ_ADDR); /* EGPI1 */
	writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_EGPI2_BASE_ADDR +
			    GPI_INQ_PKTPTR,
			    base + TMU_PHY1_INQ_ADDR); /* EGPI2 */
	writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_EGPI3_BASE_ADDR +
			    GPI_INQ_PKTPTR,
			    base + TMU_PHY2_INQ_ADDR); /* EGPI3 */
	writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_HGPI_BASE_ADDR +
			    GPI_INQ_PKTPTR,
			    base + TMU_PHY3_INQ_ADDR); /* HGPI */

	/*	Context memory initialization */
	for (ii = 0U; ii < ARRAY_SIZE(phys); ii++) {
		/*	Initialize queues */
		for (queue = 0U; queue < TLITE_PHY_QUEUES_CNT; queue++) {
			/*	Set direct context memory access */
			writel(0x1U, base + TMU_CNTX_ACCESS_CTRL);

			/*	Select PHY and QUEUE */
			writel((((u32)phys[ii] & 0x1fU) << 8) |
					    (queue & 0x7U),
					    base + TMU_PHY_QUEUE_SEL);
			dmb();

			/*	Clear direct access registers */
			writel(0U, base + TMU_CURQ_PTR);
			writel(0U, base + TMU_CURQ_PKT_CNT);
			writel(0U, base + TMU_CURQ_DROP_CNT);
			writel(0U, base + TMU_CURQ_TRANS_CNT);
			writel(0U, base + TMU_CURQ_QSTAT);
			writel(0U, base + TMU_HW_PROB_CFG_TBL0);
			writel(0U, base + TMU_HW_PROB_CFG_TBL1);
			writel(0U, base + TMU_CURQ_DEBUG);
		}

		for (cnt = 0; cnt < platform->sch_per_phy; cnt++) {
			/*	Initialize HW schedulers/shapers */
			qos_base = base;
			qos_base += TLITE_PHYN_SCHEDM_BASE_ADDR(phys[ii], cnt);
			writel(0xffffffffU, qos_base + TMU_SCH_Q_ALLOC0);
			writel(0xffffffffU, qos_base + TMU_SCH_Q_ALLOC1);
		}

		for (cnt = 0; cnt < platform->shape_per_phy; cnt++) {
			/*	Initialize HW schedulers/shapers */
			qos_base = base;
			qos_base += TLITE_PHYN_SHPM_BASE_ADDR(phys[ii], cnt);
			reg = readl(qos_base + TMU_SHP_CTRL);
			writel(reg & ~(u32)0x1U, qos_base + TMU_SHP_CTRL);

			/*	Set invalid position */
			writel((0x1fU << 1), qos_base + TMU_SHP_CTRL2);

			/*	Set default limits */
			writel(0U, qos_base + TMU_SHP_MAX_CREDIT);
			writel(0U, qos_base + TMU_SHP_MIN_CREDIT);
		}

		for (queue = 0U; queue < TLITE_PHY_QUEUES_CNT; queue++) {
			/*	Scheduler 0 */
			qos_base = base +
				   TLITE_PHYN_SCHEDM_BASE_ADDR(phys[ii], 0U);
			reg = readl(qos_base + TMU_SCH_Q_ALLOCN(queue / 4U));
			reg &= ~(0xffU << (8U * (queue % 4U)));
			reg |= ((queue & 0x1fU) << (8U * (queue % 4U)));
			writel(reg, qos_base + TMU_SCH_Q_ALLOCN(queue / 4U));

			/*	Scheduler 1 */
			qos_base = base +
				   TLITE_PHYN_SCHEDM_BASE_ADDR(phys[ii], 1U);
			reg = readl(qos_base + TMU_SCH_Q_ALLOCN(0));
			reg &= ~0xffU;
			reg |= 0xffU & 0x1fU;
			writel(reg, qos_base + TMU_SCH_Q_ALLOCN(0));
		}

		/*	Connect Scheduler 0 output to Scheduler 1 input 0 */
		writel(0, base + TLITE_PHYN_SCHEDM_BASE_ADDR(phys[ii], 0U) +
			  TMU_SCH_POS);

		for (queue = 0U; queue < TLITE_PHY_QUEUES_CNT; queue++) {
			/* Configure Queue mode */
			writel(0U, base + TMU_CNTX_ACCESS_CTRL);
			writel(((phys[ii] & 0x1fU) << 16) | ((8U * queue) + 4U),
			       base + TMU_CNTX_ADDR);
			writel((255U << 11) | (0U << 2) | (0x1U << 0),
			       base + TMU_CNTX_DATA);
			writel(0x3U, base + TMU_CNTX_CMD);
			timeout = 20;
			/*	Wait until command has been finished */
			do {
				ndelay(10000U);
				timeout--;
				reg = readl(base + TMU_CNTX_CMD);
			} while ((0U == (reg & 0x4U)) && (timeout > 0U));

			if (timeout == 0U)
				return -ETIMEDOUT;
		}
	}

	/*	BMU1 */
	writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU1_BASE_ADDR +
			    BMU_FREE_CTRL,
			    base + TMU_BMU_INQ_ADDR);

	/*	BMU2 */
	writel(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_BMU2_BASE_ADDR +
			    BMU_FREE_CTRL,
			    base + TMU_BMU2_INQ_ADDR);

	/*	Thresholds */
	writel(0x100U, base + TMU_AFULL_THRES);
	writel(0xfcU, base + TMU_INQ_WATERMARK);

	/*	TDQ CTRL */
	writel(0xfU, base + TMU_PHY0_TDQ_CTRL); /* EMAC0 */
	writel(0xfU, base + TMU_PHY1_TDQ_CTRL); /* EMAC1 */
	writel(0xfU, base + TMU_PHY2_TDQ_CTRL); /* EMAC2 */
	writel(0xfU, base + TMU_PHY3_TDQ_CTRL); /* HIF */

	pr_info("TMU was initialized\n");
	return 0;
}

static int
pfeng_hw_init_emac(struct pfe_platform *platform)
{
	int index;
	void *base;
	u32 reg;

	platform->emac_base = malloc(platform->emac_count * sizeof(void *));
	if (!platform->emac_base) {
		pr_err("Was not possible to initialize emacs");
		return -ENODEV;
	}

	platform->emac_base[0] = INIT_PLAT_OFF(platform, CBUS_EMAC1_BASE_ADDR);
	platform->emac_base[1] = INIT_PLAT_OFF(platform, CBUS_EMAC2_BASE_ADDR);
	platform->emac_base[2] = INIT_PLAT_OFF(platform, CBUS_EMAC3_BASE_ADDR);

	for (index = 0; index < platform->emac_count; index++) {
		base = platform->emac_base[index];
		if (!base) {
			pr_err("Was not possible to initialize emac%d", index);
			return -ENODEV;
		}

		reg = readl(base + MAC_CONFIGURATION);
		reg &= ~(TRANSMITTER_ENABLE | RECEIVER_ENABLE);
		writel(reg, base + MAC_CONFIGURATION);

		writel(0U, base + MAC_CONFIGURATION);

		writel(0x8000ffeeU, base + MAC_ADDRESS0_HIGH);
		writel(0xddccbbaaU, base + MAC_ADDRESS0_LOW);

		writel(PFE_EMAC_F_DEF, base + MAC_PACKET_FILTER);

		reg = readl(base + MAC_Q0_TX_FLOW_CTRL);
		reg &= ~TX_FLOW_CONTROL_ENABLE;
		writel(reg, base + MAC_Q0_TX_FLOW_CTRL);
		writel(0U, base + MAC_INTERRUPT_ENABLE);

		writel(PFE_EMAC_CFG_DEF, base + MAC_CONFIGURATION);

		writel(0U | FORWARD_ERROR_PACKETS,
		       base + MTL_RXQ0_OPERATION_MODE);

		writel(0U, base + MTL_TXQ0_OPERATION_MODE);

		writel(GIANT_PACKET_SIZE_LIMIT(0x3000U),
		       base + MAC_EXT_CONFIGURATION);

		writel(0x1U, base + MTL_DPP_CONTROL);

		writel(0U | ENABLE_TIMESTAMP | INITIALIZE_TIMESTAMP |
		       ENABLE_TIMESTAMP_FOR_ALL |
		       ENABLE_PTP_PROCESSING | SELECT_PTP_PACKETS(3U),
		       base + MAC_TIMESTAMP_CONTROL);

		writel(0x140000U, base + MAC_SUB_SECOND_INCREMENT);

		pr_info("EMAC%d block was initialized\n", index);
	}

	return 0;
}

static void
pfeng_hw_deinit_emac(struct pfe_platform *platform)
{
	u32 ii;

	if (!platform->emac_base)
		return;

	for (ii = 0U; ii < platform->emac_count; ii++) {
		if (platform->emac_base[ii])
			continue;
		u32 reg = readl(platform->emac_base[ii] +
				     MAC_CONFIGURATION);
		reg &= ~(TRANSMITTER_ENABLE | RECEIVER_ENABLE);
		writel(reg, platform->emac_base[ii] +
			    MAC_CONFIGURATION);
	}

	free(platform->emac_base);
	platform->emac_base = NULL;
}

static int pfeng_hw_init_ifaces(struct pfe_platform *platform)
{
	s32 ii, pe;
	int ret = 0;
	u32 dmem_base, heap_base;

	if (!platform->memmap)
		return 0; /* init done by firmware */

	if ((platform->hif_chnl + PFE_PHY_IF_ID_HIF0) < PFE_PHY_IF_ID_HIF0 ||
	    (platform->hif_chnl + PFE_PHY_IF_ID_HIF0) > PFE_PHY_IF_ID_MAX) {
		pr_err("Invalid HIF channel\n");
		return -ENODEV;
	}

	for (ii = 0; ii != PFE_PHY_IF_ID_INVALID; ii++) {
		struct pfe_ct_phy_if phy = { 0 };

		phy.id = ii;
		phy.mode = IF_OP_DEFAULT;
		phy.flags = 0;
		phy.mirror = PFE_PHY_IF_ID_INVALID;

		/* Prepare logical interfaces for EMACs */
		if (ii <= PFE_PHY_IF_ID_EMAC2) {
			struct pfe_ct_log_if log = { 0 };

			heap_base = ntohl(platform->memmap->dmem_heap_base) +
					  (ii * sizeof(struct pfe_ct_log_if));
			log.e_phy_ifs = htonl(1U << (platform->hif_chnl +
					      PFE_PHY_IF_ID_HIF0));
			log.mode = IF_OP_DEFAULT;
			log.flags = IF_FL_ENABLED;
			log.id = ii;
			log.next = 0U;

			for (pe = 0; pe < platform->class_pe_count; pe++) {
				pfeng_hw_pe_memcpy(pe, PFE_PE_DMEM,
						   heap_base, (u8 *)&log,
						   sizeof(struct
							  pfe_ct_log_if));
			}

			pr_info("log if stored at %8x\n", heap_base);
		} else {
			heap_base = 0U;
		}

		/* Store reference to the interfaces */
		phy.def_log_if = htonl((u32)heap_base);
		phy.log_ifs = htonl((u32)heap_base);

		dmem_base = ntohl(platform->memmap->dmem_phy_if_base) +
				  (ii * sizeof(struct pfe_ct_phy_if));

		for (pe = 0; pe < platform->class_pe_count; pe++) {
			pfeng_hw_pe_memcpy(pe, PFE_PE_DMEM,
					   dmem_base, (u8 *)&phy,
					   sizeof(struct pfe_ct_phy_if));
		}
	}

	return ret;
}

int pfeng_hw_start(struct pfe_platform *platform, int emac, u8 *ea)
{
	u32 reg;
	u32 bottom;
	u32 top;

	if (!platform || !ea)
		return -EINVAL;

	bottom = (ea[3] << 24) | (ea[2] << 16) | (ea[1] << 8) | (ea[0] << 0);
	top = (ea[5] << 8) | (ea[4] << 0) | 0x80000000U;

	/*	Store MAC address */
	writel(top, platform->emac_base[emac] + MAC_ADDRESS_HIGH(0));
	writel(bottom, platform->emac_base[emac] + MAC_ADDRESS_LOW(0));
	ndelay(100000U);
	writel(bottom, platform->emac_base[emac] + MAC_ADDRESS_LOW(0));

	/*	Enable emac */
	reg = readl(platform->emac_base[emac] + MAC_CONFIGURATION);
	reg &= ~(TRANSMITTER_ENABLE | RECEIVER_ENABLE);
	reg |= TRANSMITTER_ENABLE | RECEIVER_ENABLE;
	writel(reg, platform->emac_base[emac] + MAC_CONFIGURATION);

	/*	Enable RX & TX DMA engine and polling */
	reg = readl(platform->hif_base + HIF_CTRL_CHN(platform->hif_chnl));
	reg |= RX_BDP_POLL_CNTR_EN | RX_DMA_ENABLE;
	reg |= TX_BDP_POLL_CNTR_EN | TX_DMA_ENABLE;
	writel(reg, platform->hif_base + HIF_CTRL_CHN(platform->hif_chnl));

	return 0;
}

int pfeng_hw_stop(struct pfe_platform *platform, int emac)
{
	u32 reg;

	if (!platform)
		return -EINVAL;

	/* disable emac */
	reg = readl(platform->emac_base[emac] + MAC_CONFIGURATION);
	reg &= ~(TRANSMITTER_ENABLE | RECEIVER_ENABLE);
	writel(reg, platform->emac_base[emac] + MAC_CONFIGURATION);

	/* Clear MAC address */
	writel(0, platform->emac_base[emac] + MAC_ADDRESS_HIGH(0));
	writel(0, platform->emac_base[emac] + MAC_ADDRESS_LOW(0));
	ndelay(100000U);
	writel(0, platform->emac_base[emac] + MAC_ADDRESS_LOW(0));

	/*	Disable RX & TX DMA engine and polling */
	reg = readl(platform->hif_base + HIF_CTRL_CHN(platform->hif_chnl));
	reg &= ~((u32)RX_DMA_ENABLE | RX_BDP_POLL_CNTR_EN);
	reg &= ~((u32)TX_DMA_ENABLE | TX_BDP_POLL_CNTR_EN);
	writel(reg, platform->hif_base + HIF_CTRL_CHN(platform->hif_chnl));

	return 0;
}

static int pfeng_hw_soft_reset_g2(struct pfe_platform *platform)
{
	void *addr;
	u32 regval;

	(void)platform;
	addr = (void *)(CBUS_GLOBAL_CSR_BASE_ADDR + 0x20U +
			(uint64_t)(pfe.cbus_baseaddr));
	regval = readl(addr) | (1U << 30);
	writel(regval, addr);

	mdelay(100); /* 100ms (taken from reference code) */

	regval &= ~(1U << 30);
	writel(regval, addr);

	return 0;
}

static int pfeng_hw_soft_reset_g3(struct pfe_platform *platform)
{
	void *addr, *addr2;
	u32 regval;
	u32 retries = 1000;

	addr = (void *)(CBUS_GLOBAL_CSR_BASE_ADDR + 0x20U +
			(uint64_t)(pfe.cbus_baseaddr));
	regval = readl(addr) | (1U << 30);
	writel(regval, addr);

	mdelay(10);

	addr2 = (void *)(CBUS_GLOBAL_CSR_BASE_ADDR + WSP_DBUG_BUS1 +
			(uint64_t)(pfe.cbus_baseaddr));
	while (!(readl(addr2) & (SOFT_RESET_DONE | BMU1_SOFT_RESET_DONE |
				 BMU2_SOFT_RESET_DONE))) {
		if (retries-- == 0)
			return -ETIME;

		mdelay(1);
	}

	regval = readl(addr) | SOFT_RESET_DONE_CLEAR |
		 BMU1_SOFT_RESET_DONE_CLEAR | BMU2_SOFT_RESET_DONE_CLEAR;
	writel(regval, addr);

	return 0;
}

static void safety_clear(struct pfe_platform *pfe)
{
	u32 *addr;

	if (pfe->on_g3) {
		/* Disable S32G3 safety */

		addr = (u32 *)(void *)((uint64_t)pfe->cbus_baseaddr +
				    CBUS_GLOBAL_CSR_BASE_ADDR +
				    WSP_FAIL_STOP_MODE_INT_EN);
		writel(0x0, addr);

		addr = (u32 *)(void *)((uint64_t)pfe->cbus_baseaddr +
				    CBUS_GLOBAL_CSR_BASE_ADDR +
				    WSP_FAIL_STOP_MODE_EN);
		writel(0x0, addr);

		addr = (u32 *)(void *)((uint64_t)pfe->cbus_baseaddr +
				    CBUS_GLOBAL_CSR_BASE_ADDR +
				    WSP_ECC_ERR_INT_EN);
		writel(0x0, addr);
	}
}

int pfeng_hw_init(struct pfe_platform_config *config)
{
	int ret = 0;
	u32 reg;
	u32 *addr;
	u32 val;
	u32 *ii;
	u32 index = 0;

	memset(&pfe, 0U, sizeof(struct pfe_platform));
	pfe.fw = config->fw;
	pfe.cfg = config;

	/* Map CBUS address space */
	pfe.cbus_baseaddr = (void *)config->cbus_base;
	if (!pfe.cbus_baseaddr) {
		pr_err("Can't map PPFE CBUS\n");
		goto exit;
	}

	/* Check version */
	addr = (u32 *)((uint64_t)pfe.cbus_baseaddr +
				 CBUS_GLOBAL_CSR_BASE_ADDR + WSP_VERSION);
	val = readl(addr);
	if (val == WSP_VERSION_SILICON_G2) {
		/* S32G2 */
		pr_debug("PFE: S32G2 detected\n");
		pfe.on_g3 = false;
	} else if (val == WSP_VERSION_SILICON_G3) {
		/* S32G3 */
		pr_debug("PFE: S32G3 detected\n");
		pfe.on_g3 = true;
	} else {
		/* Unknown */
		pr_err("PFE: Silicon HW version is unknown: 0x%x\n", val);
		pfe.on_g3 = false;
	}

	safety_clear(&pfe);

	/* Init LMEM */
	addr = (u32 *)(void *)((uint64_t)pfe.cbus_baseaddr +
				    CBUS_LMEM_BASE_ADDR);
	pr_debug("Initializing LMEM (%d bytes)\n", (int)CBUS_LMEM_SIZE);
	for (ii = addr; ii - addr < CBUS_LMEM_SIZE; ++ii)
		*ii = 0U;

	/* Create HW components */
	pfe.emac_count = 3U;
	pfe.gpi_count = 3U;
	pfe.etgpi_count = 3U;
	pfe.hgpi_count = 1U;
	pfe.gpi_total = pfe.gpi_count + pfe.etgpi_count + pfe.hgpi_count;
	pfe.bmu_count = 2U;
	pfe.class_pe_count = 8U;
	pfe.util_pe_count = 1U;
	pfe.tmu_pe_count = 0U;
	pfe.shape_per_phy = 4;
	pfe.sch_per_phy = 2;
	pfe.hif_chnl = 0;
	pfe.emac_mdio_div = pfeng_hw_emac_mdio_div_decode(config->csr_clk_f);

	/* BMU */
	ret = pfeng_hw_init_bmu(&pfe);
	if (ret)
		goto exit;

	/* GPI */
	ret = pfeng_hw_init_gpi(&pfe);
	if (ret)
		goto exit;

	/* TMU */
	ret = pfeng_hw_init_tmu(&pfe);
	if (ret)
		goto exit;

	/* Classifier */
	ret = pfeng_hw_init_class(&pfe);
	if (ret)
		goto exit;

	/* EMAC */
	ret = pfeng_hw_init_emac(&pfe);
	if (ret)
		goto exit;

	/* SOFT RESET */
	if (pfe.on_g3)
		ret = pfeng_hw_soft_reset_g3(&pfe);
	else
		ret = pfeng_hw_soft_reset_g2(&pfe);
	if (ret) {
		pr_err("Platform reset failed\n");
		goto exit;
	}

	/* HIF (HIF DOES NOT LIKE SOFT RESET ONCE HAS BEEN CONFIGURED...) */
	ret = pfeng_hw_init_hif(&pfe, config);
	if (ret)
		goto exit;

	/* Interfaces (Before starting the cores) */
	ret = pfeng_hw_init_ifaces(&pfe);
	if (ret)
		goto exit;

	/* Activate the classifier */
	pr_info("Enabling the CLASS block\n");
	writel(PFE_CORE_ENABLE, pfe.cbus_baseaddr + CLASS_TX_CTRL);
	/* Wait to let classifier firmware to initialize */
	mdelay(50);

	/* Enable BMU */

	for (index = 0; index < pfe.bmu_count; index++) {
		writel(0x1U, pfe.bmu_base[index] + BMU_CTRL);
		pr_info("Enabling the BMU%d block\n", index);
	}

	/* Enable GPI */
	for (index = 0; index < pfe.gpi_total; index++) {
		reg = readl(pfe.gpi_base[index] + GPI_CTRL);
		writel(reg | 0x1U, pfe.gpi_base[index] + GPI_CTRL);
		pr_info("Enabling the GPI%d block\n", index);
	}

	addr = (void *)(CBUS_GLOBAL_CSR_BASE_ADDR + 0x20U +
			(uint64_t)(pfe.cbus_baseaddr));
	val = readl(addr);
	writel((val | 0x80000003U), addr);

	pfe.probed = true;

	return 0;

exit:
	(void)pfeng_hw_remove();
	return ret;
}

int pfeng_hw_debug(struct pfe_platform *platform)
{
	s32 stat, emac;
	u32 reg;

	puts("HIF statistics\n");

	for (stat = 0; stat < ARRAY_SIZE(debug_hif); stat++)
		printf("%-40s : 0x%08x\n",
		       debug_hif[stat].reg_name,
		       readl((u64)platform->hif_base +
			     debug_hif[stat].reg));

	for (emac = 0; emac < platform->emac_count; emac++) {
		printf("\nEMAC%d statistics\n", emac);
		for (stat = 0; stat < ARRAY_SIZE(debug_emac); stat++) {
			reg = readl((u64)platform->emac_base[emac] +
				    debug_emac[stat].reg);
			printf("%-40s : 0x%08x\n",
			       debug_emac[stat].reg_name, reg);
		}
	}

	return 0;
}

int pfeng_hw_remove(void)
{
	/*	Clear the generic control register */
	if (pfe.cbus_baseaddr) {
		writel(0U, (void *)(CBUS_GLOBAL_CSR_BASE_ADDR + 0x20U +
			   (uint64_t)(pfe.cbus_baseaddr)));
	}

	pfeng_hw_deinit_hif(&pfe);
	pfeng_hw_deinit_gpi(&pfe);
	pfeng_hw_deinit_bmu(&pfe);

	pfeng_hw_deinit_class(&pfe);
	pfeng_hw_deinit_emac(&pfe);

	pfe.cbus_baseaddr = 0x0ULL;
	pfe.probed = false;

	return 0;
}

struct pfe_platform *pfeng_hw_get_instance(void)
{
	if (pfe.probed)
		return &pfe;
	else
		return NULL;
}

/** @}*/
