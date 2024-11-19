// SPDX-License-Identifier: GPL 2.0
/*
 *  Copyright (c) 2019 Imagination Technologies Limited
 *  Copyright (c) 2020-2021 Imagination Technologies Limited
 *  Copyright 2018-2024 NXP
 */

#include <elf.h>
#include <asm/io.h>
#include <dm/device_compat.h>
#include <linux/delay.h>

#include "pfe_hw.h"
#include "internal/pfe_hw_priv.h"

/* Sanity checks for CLASS FW ELF file */
/* Maximal number of entries in the section header table */
#define FW_ELF_MAX_SH_ENTRIES 32
/* Maximal number of entries in the program header table */
#define FW_ELF_MAX_PH_ENTRIES 16

/* PE/ELF helper macros*/
#define PE_ADDR_LOW(memt) (PFE_CFG_CLASS_ELF_ ## memt ##_BASE)
#define PE_ADDR_HI(memt) (PFE_CFG_CLASS_ELF_ ## memt ## _BASE + \
			  PFE_CFG_CLASS_ ## memt ## _SIZE)

#define ELF_SKIP_FLAGS (SHF_WRITE | SHF_ALLOC | SHF_EXECINSTR)
#define SHT_PFE_SKIP (0x7000002aU)

#define PFE_LOADCONF_ENABLE 0xABCDU

enum pfe_pe_mem { PFE_PE_DMEM, PFE_PE_IMEM };

static inline u8 bytes_to_4b_alignment(u64 addr)
{
	return 4U - (addr & 3U);
}

static int pfe_hw_pe_elf_hdr_sanity_check(const Elf32_Ehdr *ehdr, const struct pfe_hw_cfg *cfg)
{
	/* Both e_shnum and e_phnum expected to be in Little-endian format */
	if (ehdr->e_shnum > FW_ELF_MAX_SH_ENTRIES ||
	    ehdr->e_phnum > FW_ELF_MAX_PH_ENTRIES) {
		dev_err(cfg->dev, "Failed FW ELF check (e_shnum: %u e_phnum: %u)\n",
			ehdr->e_shnum, ehdr->e_phnum);
		return -EINVAL;
	}

	return 0;
}

/* ELF helper functions */
static void elf32_shdr_swap_endian(Elf32_Shdr *shdr_p, u32 e_shnum)
{
	u32 index;
	Elf32_Shdr *shdr;

	for (index = 0U; index < e_shnum; index++) {
		shdr = &shdr_p[index];
		shdr->sh_name = be32_to_cpup(&shdr->sh_name);
		shdr->sh_type = be32_to_cpup(&shdr->sh_type);
		shdr->sh_flags = be32_to_cpup(&shdr->sh_flags);
		shdr->sh_addr = be32_to_cpup(&shdr->sh_addr);
		shdr->sh_offset = be32_to_cpup(&shdr->sh_offset);
		shdr->sh_size = be32_to_cpup(&shdr->sh_size);
		shdr->sh_link = be32_to_cpup(&shdr->sh_link);
		shdr->sh_info = be32_to_cpup(&shdr->sh_info);
		shdr->sh_addralign = be32_to_cpup(&shdr->sh_addralign);
		shdr->sh_entsize = be32_to_cpup(&shdr->sh_entsize);
	}
}

static void elf32_ehdr_swap_endian(Elf32_Ehdr *ehdr)
{
	ehdr->e_type = be16_to_cpup(&ehdr->e_type);
	ehdr->e_machine = be16_to_cpup(&ehdr->e_machine);
	ehdr->e_version = be32_to_cpup(&ehdr->e_version);
	ehdr->e_entry = be32_to_cpup(&ehdr->e_entry);
	ehdr->e_phoff = be32_to_cpup(&ehdr->e_phoff);
	ehdr->e_shoff = be32_to_cpup(&ehdr->e_shoff);
	ehdr->e_flags = be32_to_cpup(&ehdr->e_flags);
	ehdr->e_ehsize = be16_to_cpup(&ehdr->e_ehsize);
	ehdr->e_phentsize = be16_to_cpup(&ehdr->e_phentsize);
	ehdr->e_phnum = be16_to_cpup(&ehdr->e_phnum);
	ehdr->e_shentsize = be16_to_cpup(&ehdr->e_shentsize);
	ehdr->e_shnum = be16_to_cpup(&ehdr->e_shnum);
	ehdr->e_shstrndx = be16_to_cpup(&ehdr->e_shstrndx);
}

static void elf32_phdr_swap_endian(Elf32_Phdr *phdr_p, u32 e_phnum)
{
	u32 index;
	Elf32_Phdr *phdr;

	for (index = 0U; index < e_phnum; index++) {
		phdr = &phdr_p[index];
		phdr->p_type = be32_to_cpup(&phdr->p_type);
		phdr->p_offset = be32_to_cpup(&phdr->p_offset);
		phdr->p_vaddr = be32_to_cpup(&phdr->p_vaddr);
		phdr->p_paddr = be32_to_cpup(&phdr->p_paddr);
		phdr->p_filesz = be32_to_cpup(&phdr->p_filesz);
		phdr->p_memsz = be32_to_cpup(&phdr->p_memsz);
		phdr->p_flags = be32_to_cpup(&phdr->p_flags);
		phdr->p_align = be32_to_cpup(&phdr->p_align);
	}
}

/* PE helper functions */
static void pfe_hw_pe_mem_write_idx(struct pfe_hw_pe *class, u8 pfe_idx,
				    enum pfe_pe_mem mem, u32 val, u32 addr, u8 size)
{
	u8 bytesel = 0U;
	u32 memsel;
	u8 offset;

	/* Size is 0, do nothing */
	if (!size)
		return;

	/* Only one u32 val with size 4 can be written at a time */
	if (size > 4)
		size = 4;

	/* Check if the address is not 4-byte aligned */
	if (unlikely(addr & 0x3U)) {
		offset = bytes_to_4b_alignment(addr);

		if (size <= offset) {
			val = val << (8U * (addr & 0x3U));
			bytesel = (((1U << size) - 1U) << (offset - size)) & 0xFF;
		} else {
			pfe_hw_pe_mem_write_idx(class, pfe_idx, mem, val, addr, offset);
			val >>= 8U * offset;
			size -= offset;
			addr += offset;
			pfe_hw_pe_mem_write_idx(class, pfe_idx, mem, val, addr, size);
			return;
		}
	} else {
		bytesel = (((1U << size) - 1U) << (4U - size)) & 0xFF;
	}

	if (mem == PFE_PE_DMEM)
		memsel = PE_IBUS_ACCESS_DMEM;
	else
		memsel = PE_IBUS_ACCESS_IMEM;

	addr = (addr & GENMASK(19, 0))	/* Address (low 20bits) */
	       | PE_IBUS_WRITE		/* Direction (r/w) */
	       | memsel			/* Memory selector */
	       | PE_IBUS_PE_ID(pfe_idx)	/* PE instance */
	       | PE_IBUS_WREN(bytesel); /* Byte(s) selector */

	/* commit data to HW */
	pfe_hw_write(class, CLASS_MEM_ACCESS_WDATA, htonl(val));
	pfe_hw_write(class, CLASS_MEM_ACCESS_ADDR, addr);
}

static void pfe_hw_pe_memset(struct pfe_hw_pe *class, u8 pe_idx, enum pfe_pe_mem mem,
			     u8 val, u64 addr, u32 len)
{
	u32 val32 = (u32)val | ((u32)val << 8) | ((u32)val << 16) | ((u32)val << 24);
	u32 offset;

	if (addr & 0x3U) {
		/*	Write unaligned bytes to align the address */
		offset = bytes_to_4b_alignment(addr);
		offset = (len < offset) ? len : offset;
		pfe_hw_pe_mem_write_idx(class, pe_idx, mem, val32, addr, offset);
		len = (len >= offset) ? (len - offset) : 0U;
		addr += offset;
	}

	for (; len >= 4U; len -= 4U, addr += 4U) {
		/*	Write aligned words */
		pfe_hw_pe_mem_write_idx(class, pe_idx, mem, val32, addr, 4U);
	}

	if (len > 0U) {
		/*	Write the rest */
		pfe_hw_pe_mem_write_idx(class, pe_idx, mem, val32, addr, len);
	}
}

static void pfe_hw_pe_memcpy(struct pfe_hw_pe *class, u8 pe_idx, enum pfe_pe_mem memt,
			     u64 dst, const u8 *src, u32 len)
{
	u32 val;
	u32 offset;
	const u8 *src_byteptr = src;

	if (dst & 0x3U) {
		offset = bytes_to_4b_alignment(dst);
		offset = (len < offset) ? len : offset;
		val = *(u32 *)src_byteptr;
		pfe_hw_pe_mem_write_idx(class, pe_idx, memt, val, dst, offset);
		src_byteptr += offset;
		dst += offset;
		len = (len >= offset) ? (len - offset) : 0U;
	}

	for (; len >= 4U; len -= 4U, src_byteptr += 4U, dst += 4U) {
		/*	4-byte writes */
		val = *(u32 *)src_byteptr;
		pfe_hw_pe_mem_write_idx(class, pe_idx, memt, val, (u32)dst, 4U);
	}

	if (len != 0U) {
		/*	The rest */
		val = *(u32 *)src_byteptr;
		pfe_hw_pe_mem_write_idx(class, pe_idx, memt, val, (u32)dst, len);
	}
}

static int pfe_hw_pe_load_section(struct pfe_hw_pe *class, u8 pe_idx, const void *data,
				  u32 addr, u32 size, u32 type)
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
		printf("DEB: ERR:Unsupported memory range 0x%x\n", addr);
		return -EINVAL;
	}

	switch (type) {
	case SHT_PFE_SKIP:
		break;
	case SHT_PROGBITS:
		pfe_hw_pe_memcpy(class, pe_idx, memt, addr - base, data, size);
		break;
	case SHT_NOBITS:
		if (memt == PFE_PE_DMEM) {
			pfe_hw_pe_memset(class, pe_idx, PFE_PE_DMEM, 0, addr, size);
			break;
		}
		fallthrough;
	default:
		printf("DEB: ERR:Unsupported section type: 0x%x\n", type);
		return -EINVAL;
	}

	return 0;
}

static Elf32_Addr pfe_hw_pe_get_elf_sect_load_addr(Elf32_Phdr *phdr, Elf32_Half phdr_cnt,
						   Elf32_Shdr *shdr)
{
	Elf32_Addr virt_addr = shdr->sh_addr;
	Elf32_Addr load_addr;
	Elf32_Off offset;
	Elf32_Half i;

	/* Go through all program headers to find one containing the section */
	for (i = 0; i < phdr_cnt; i++) {
		if (virt_addr >= phdr[i].p_vaddr &&
		    ((Elf64_Sxword)virt_addr <=
		     (phdr[i].p_vaddr + phdr[i].p_memsz - shdr->sh_size))) {
			offset = phdr[i].p_vaddr - phdr[i].p_paddr;
			if (virt_addr < offset)
				return 0;
			load_addr = virt_addr - offset;
			return load_addr;
		}
	}
	/* No segment containing the section was found ! */
	printf("DEB:ERR: PFE: Translation of 0x%08x failed, fallback used\n", virt_addr);
	return 0;
}

static int pfe_hw_pe_load_firmware(struct pfe_hw_pe *class, u8 pe_idx, u8 *fw)
{
	Elf32_Ehdr *ehdr = (Elf32_Ehdr *)fw;
	Elf32_Shdr *shdr = (Elf32_Shdr *)((phys_addr_t)fw + ehdr->e_shoff);
	Elf32_Phdr *phdr = (Elf32_Phdr *)((phys_addr_t)fw + ehdr->e_phoff);
	u32 lcv = htonl(PFE_LOADCONF_ENABLE);
	bool lc_found = false;
	int ret = -ENODATA;
	char *names;
	Elf32_Addr l_addr;
	void *buf;
	Elf32_Half i;

	names = (char *)((phys_addr_t)fw + shdr[ehdr->e_shstrndx].sh_offset);
	for (i = 0; i < ehdr->e_shnum; ++i) {
		if (!strcmp(".loadconf", &names[shdr[i].sh_name])) {
			lc_found = true;
			break;
		}
	}

	if (!lc_found) {
		log_err("PFE: loadconf section is not available.\n");
		return -EINVAL;
	}

	memcpy(((u8 *)fw + shdr[i].sh_offset), &lcv, sizeof(lcv));

	/*	Try to upload all sections of the .elf */
	for (i = 0; i < ehdr->e_shnum; i++) {
		if (!(shdr[i].sh_flags & (ELF_SKIP_FLAGS)))
			continue;

		buf = (unsigned char *)fw + shdr[i].sh_offset;

		/* Translate elf virtual address to load address */
		l_addr = pfe_hw_pe_get_elf_sect_load_addr(phdr, ehdr->e_phnum,
							  &shdr[i]);
		if (l_addr == 0)
			return -EINVAL;

		ret = pfe_hw_pe_load_section(class, pe_idx, buf, l_addr,
					     shdr[i].sh_size,
					     shdr[i].sh_type);

		if (ret != 0)
			log_err("PFE: Couldn't upload firmware section\n");
	}

	return ret;
}

int pfe_hw_pe_init_class(struct pfe_hw_pe *class, const struct pfe_hw_cfg *cfg)
{
	void *fw = cfg->fw_class_data;
	Elf32_Shdr *shdr = NULL;
	Elf32_Ehdr *ehdr = NULL;
	Elf32_Phdr *phdr = NULL;
	int ret = 0;
	u32 reg;
	u8 i;

	if (!cfg->fw_class_data || cfg->fw_class_size == 0U) {
		dev_err(cfg->dev, "The CLASS firmware is not loaded\n");
		return -EIO;
	}

	class->class_pe_count = PFENG_PE_COUNT;
	class->base = (void __iomem *)cfg->cbus_base;

	/* Init CLASS Mem */
	for (i = 0; i < class->class_pe_count; i++) {
		pfe_hw_pe_memset(class, i, PFE_PE_DMEM, 0U, 0U,
				 PFE_CFG_CLASS_DMEM_SIZE);
		pfe_hw_pe_memset(class, i, PFE_PE_IMEM, 0U, 0U,
				 PFE_CFG_CLASS_IMEM_SIZE);
	}

	/*	Issue block reset */
	pfe_hw_write(class, CLASS_TX_CTRL, PFE_CORE_DISABLE);
	pfe_hw_write(class, CLASS_TX_CTRL, PFE_CORE_SW_RESET);

	/* After soft reset, need to wait for 10us to perform another CSR write/read */
	ndelay(10000);
	pfe_hw_write(class, CLASS_TX_CTRL, PFE_CORE_DISABLE);

	pfe_hw_write(class, CLASS_BMU1_BUF_FREE, PFE_CFG_CBUS_PHYS_BASE_ADDR +
		     CBUS_BMU1_BASE_ADDR + BMU_FREE_CTRL);

	/*	Config Queue/Reorder buffers */
	pfe_hw_write(class, CLASS_PE0_RO_DM_ADDR0, CLASS_PE0_RO_DM_ADDR0_VAL);
	pfe_hw_write(class, CLASS_PE0_RO_DM_ADDR1, CLASS_PE0_RO_DM_ADDR1_VAL);
	pfe_hw_write(class, CLASS_PE0_QB_DM_ADDR0, CLASS_PE0_QB_DM_ADDR0_VAL);
	pfe_hw_write(class, CLASS_PE0_QB_DM_ADDR1, CLASS_PE0_QB_DM_ADDR1_VAL);

	/*	TMU Config */
	pfe_hw_write(class, CLASS_TM_INQ_ADDR, PFE_CFG_CBUS_PHYS_BASE_ADDR + TMU_PHY_INQ_PKTPTR);
	pfe_hw_write(class, CLASS_MAX_BUF_CNT, 0x18U);
	pfe_hw_write(class, CLASS_AFULL_THRES, 0x14U);
	pfe_hw_write(class, CLASS_INQ_AFULL_THRES, 0x3c0U);
	pfe_hw_write(class, CLASS_USE_TMU_INQ, 0x1U);

	/*	System clock */
	pfe_hw_write(class, CLASS_PE_SYS_CLK_RATIO, 0x1U);

	/*	Disable TCP/UDP/IPv4 checksum drop */
	pfe_hw_write(class, CLASS_L4_CHKSUM, 0U);

	/* Configure the LMEM header size and RO  */
	if (cfg->on_g3)
		reg = (PFE_CFG_RO_HDR_SIZE_G3 << 16) | PFE_CFG_LMEM_HDR_SIZE_G3;
	else
		reg = (PFE_CFG_RO_HDR_SIZE_G2 << 16) | PFE_CFG_LMEM_HDR_SIZE_G2;
	pfe_hw_write(class, CLASS_HDR_SIZE, reg);

	pfe_hw_write(class, CLASS_LMEM_BUF_SIZE, PFE_CFG_LMEM_BUF_SIZE);

	if (cfg->on_g3)
		clrsetbits_32(pfe_hw_addr(class, CLASS_AXI_CTRL_ADDR),
			      AXI_DBUS_BURST_SIZE(0x3ffU),
			      AXI_DBUS_BURST_SIZE(0x200U));

	setbits_32(pfe_hw_addr(class, CLASS_AXI_CTRL_ADDR), 0x3U);

	pfe_hw_write(class, CLASS_ROUTE_MULTI, QB2BUS_ENDIANNESS);

	/* Prepare FW file*/
	class->fw = (u8 *)cfg->fw_class_data;
	ehdr = (Elf32_Ehdr *)class->fw;

	if (!IS_ELF(*ehdr)) {
		dev_err(cfg->dev, "Only ELF format is supported\n");
		return -ENODEV;
	}

	dev_info(cfg->dev, "Uploading CLASS firmware\n");

	/* .elf data must be in BIG ENDIAN */
	if (ehdr->e_ident[EI_DATA] == 1U) {
		dev_err(cfg->dev, "Unexpected .elf format (little endian)\n");
		return -EINVAL;
	}

	elf32_ehdr_swap_endian(ehdr);

	/* Sanity check of e_shoff & e_phoff in ELF Header */
	ret = pfe_hw_pe_elf_hdr_sanity_check(ehdr, cfg);
	if (ret)
		return ret;

	shdr = (Elf32_Shdr *)(fw + ehdr->e_shoff);
	phdr = (Elf32_Phdr *)(fw + ehdr->e_phoff);
	elf32_shdr_swap_endian(shdr, ehdr->e_shnum);
	elf32_phdr_swap_endian(phdr, ehdr->e_phnum);

	for (i = 0; i < class->class_pe_count; i++) {
		ret = pfe_hw_pe_load_firmware(class, i, fw);
		if (ret != 0) {
			dev_err(cfg->dev, "FW load failed PE %u : %d\n", i, ret);
			break;
		}
	}
	if (ret != 0) {
		dev_err(cfg->dev, "Error during upload of CLASS firmware\n");
		return -EIO;
	}

	return 0;
}

void pfe_hw_pe_enable_class(struct pfe_hw_pe *class)
{
	pfe_hw_write(class, CLASS_TX_CTRL, PFE_CORE_ENABLE);
}

void pfe_hw_pe_deinit_class(struct pfe_hw_pe *class)
{
	if (class->base)
		pfe_hw_write(class, CLASS_TX_CTRL, PFE_CORE_DISABLE);
}
