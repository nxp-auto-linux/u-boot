// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2019-2020 NXP
 *
 */

#include <common.h>
#include <malloc.h>
#include <fs.h>
#include <dm.h>
#include <phy.h>
#include <miiphy.h>
#include <net.h>
#include <elf.h>
#include <hwconfig.h>
#include <spi_flash.h>
#include <clk.h>
#include <cpu_func.h>

#include "pfeng.h"

#include <dm/platform_data/pfeng_dm_eth.h>
#include <dm/device_compat.h>

#define HIF_QUEUE_ID	0
#define HIF_HEADER_SIZE sizeof(struct pfe_ct_hif_rx_hdr)

static const struct pfe_ct_hif_tx_hdr header[PFENG_EMACS_COUNT] = {
	{.queue = 0, .flags = HIF_TX_INJECT, .chid = 0, .e_phy_ifs = htonl(1)},
	{.queue = 0, .flags = HIF_TX_INJECT, .chid = 0, .e_phy_ifs = htonl(2)},
	{.queue = 0, .flags = HIF_TX_INJECT, .chid = 0, .e_phy_ifs = htonl(4)},
};

static struct pfeng_priv *pfeng_drv_priv = NULL;
/* firmware */

void pfeng_debug(void)
{
	if (!pfeng_drv_priv || !pfeng_drv_priv->pfe)
		return;

	pfeng_hw_debug(pfeng_drv_priv->pfe);
}

#if CONFIG_IS_ENABLED(FSL_PFENG_FW_LOC_SDCARD)
static int pfeng_fw_load(char *fname, char *iface, char *part, int ftype,
			 struct pfeng_priv *priv)
{
	int ret;
	void *addr = NULL;
	loff_t length, read = 0;

	debug("Loading PFEng fw from %s@%s:%s:%d\n", iface, part, fname, ftype);

	ret = fs_set_blk_dev(iface, part, ftype);
	if (ret < 0)
		goto exit;

	ret = fs_size(fname, &length);
	if (ret)
		goto exit;

	addr = valloc(length);
	if (!addr) {
		ret = -ENOMEM;
		goto exit;
	}

	ret = fs_set_blk_dev(iface, part, ftype);
	if (ret < 0)
		goto exit;

	ret = fs_read(fname, (ulong)addr, 0, length, &read);
	if (ret < 0)
		goto exit;

	priv->fw.class_data = addr;
	priv->fw.class_size = read;

	debug("Found PFEng firmware: %s@%s:%s size %lld\n",
	      iface, part, fname, read);

exit:
	if (ret) {
		if (addr)
			free(addr);

		dev_err(priv->dev, "PFEng firmware file '%s@%s:%s' loading failed: %d\n",
			iface, part, fname, ret);
		priv->fw.class_data = NULL;
		priv->fw.class_size = 0;
	}
	return ret;
}
#endif /* FSL_PFENG_FW_LOC_SDCARD */

#if CONFIG_IS_ENABLED(FSL_PFENG_FW_LOC_QSPI)
static int setup_flash_device(struct spi_flash **flash)
{
	struct udevice *new;
	int	ret;

	/* Use default QSPI device. Speed and mode will be read from DT */
	ret = spi_flash_probe_bus_cs(CONFIG_ENV_SPI_BUS, CONFIG_ENV_SPI_CS,
				     CONFIG_ENV_SPI_MAX_HZ, CONFIG_ENV_SPI_MODE,
				     &new);
	if (ret) {
		log_err("spi_flash_probe_bus_cs() failed\n");
		return ret;
	}

	*flash = dev_get_uclass_priv(new);
	return 0;
}

static void swab_elf_hdr(Elf32_Ehdr *elf_hdr)
{
	elf_hdr->e_type = swab16(elf_hdr->e_type);
	elf_hdr->e_machine = swab16(elf_hdr->e_machine);
	elf_hdr->e_version = swab32(elf_hdr->e_version);
	elf_hdr->e_entry = swab32(elf_hdr->e_entry);
	elf_hdr->e_phoff = swab32(elf_hdr->e_phoff);
	elf_hdr->e_shoff = swab32(elf_hdr->e_shoff);
	elf_hdr->e_flags = swab32(elf_hdr->e_flags);
	elf_hdr->e_ehsize = swab16(elf_hdr->e_ehsize);
	elf_hdr->e_phentsize = swab16(elf_hdr->e_phentsize);
	elf_hdr->e_phnum = swab16(elf_hdr->e_phnum);
	elf_hdr->e_shentsize = swab16(elf_hdr->e_shentsize);
	elf_hdr->e_shnum = swab16(elf_hdr->e_shnum);
	elf_hdr->e_shstrndx = swab16(elf_hdr->e_shstrndx);
}

static int load_pfe_fw(struct spi_flash *flash, unsigned long qspi_addr,
		       void **fw_buffer, size_t *elf_size)
{
	int ret;
	Elf32_Ehdr elf_hdr;

	ret = spi_flash_read(flash, qspi_addr, sizeof(elf_hdr), &elf_hdr);
	if (ret) {
		log_err("Failed to read PFE FW Header from QSPI\n");
		return -EIO;
	}
	swab_elf_hdr(&elf_hdr);

	*elf_size = elf_hdr.e_shoff + (elf_hdr.e_shentsize * elf_hdr.e_shnum);

	if (!valid_elf_image((unsigned long)&elf_hdr)) {
		log_err("PFEng firmware is not valid\n");
		ret = -EINVAL;
		goto exit;
	}

	*fw_buffer = valloc(*elf_size);
	if (!*fw_buffer) {
		log_err("Failed to allocate 0x%lx bytes for PFE FW\n",
			*elf_size);
		ret = -ENOMEM;
		goto exit;
	}

	ret = spi_flash_read(flash, qspi_addr, *elf_size, *fw_buffer);
	if (ret) {
		log_err("Failed to load PFE FW from QSPI\n");
		goto exit;
	}

exit:
	if (ret && *fw_buffer)
		free(*fw_buffer);

	return ret;
}

static int pfeng_fw_load(char *fname, char *iface, const char *part, int ftype,
			 struct pfeng_priv *priv)
{
	int ret = 0;
	void *fw_buffer = NULL;
	struct spi_flash *flash;
	unsigned long qspi_addr;
	size_t elf_size;

	qspi_addr = simple_strtoul(part, NULL, 16);

	ret = setup_flash_device(&flash);
	if (ret)
		goto exit;

	ret = load_pfe_fw(flash, qspi_addr, &fw_buffer, &elf_size);
	if (ret)
		goto exit;

	priv->fw.class_data = fw_buffer;
	priv->fw.class_size = elf_size;

	if (!priv->fw.class_data) {
		dev_err(dev, "PFEng firmware not found at qspi@%p\n",
			priv->fw.class_data);
		ret = -EINVAL;
		goto exit;
	}

	debug("Found PFEng firmware: qspi@%p\n", priv->fw.class_data);

exit:

	if (ret) {
		if (fw_buffer)
			free(fw_buffer);

		dev_err(priv->dev, "PFEng firmware file '%s@%s:%s' loading failed: %d\n",
			iface, part, fname, ret);
		priv->fw.class_data = NULL;
		priv->fw.class_size = 0;
	}

	return ret;
}
#endif /* FSL_PFENG_FW_LOC_QSPI */

/* MDIO */
static int emac_mdio_read(struct mii_dev *bus, int mdio_addr,
			  int mdio_devad, int mdio_reg)
{
	void *emac = bus->priv;
	u16 val;
	int ret;

	ret = pfeng_hw_emac_mdio_read(emac, mdio_addr, mdio_devad,
				      mdio_reg, &val);

	if (ret) {
		pr_err("%s: MDIO read on MAC failed\n", bus->name);
		return -EAGAIN;
	}
	debug("%s: val=%x\n", __func__, val);

	return val;
}

static int emac_mdio_write(struct mii_dev *bus, int mdio_addr, int mdio_devad,
			   int mdio_reg, u16 mdio_val)
{
	void *emac = bus->priv;
	int ret;

	debug("%s(addr=%x, reg=%d, val=%x):\n", __func__, mdio_addr, mdio_reg,
	      mdio_val);

	ret = pfeng_hw_emac_mdio_write(emac, mdio_addr, mdio_devad,
				       mdio_reg, mdio_val);

	if (ret) {
		pr_err("%s: MDIO write on MAC failed\n", bus->name);
		return -EAGAIN;
	}

	return 0;
}

static void pfeng_flush_d(void *dat, u32 len)
{
	flush_dcache_range(rounddown((u64)dat, ARCH_DMA_MINALIGN),
			   roundup((u64)dat + len, ARCH_DMA_MINALIGN));
}

static void pfeng_inval_d(void *dat, u32 len)
{
	invalidate_dcache_range(rounddown((u64)dat, ARCH_DMA_MINALIGN),
				roundup((u64)dat + len, ARCH_DMA_MINALIGN));
}

static void pfeng_mdio_unregister_all(struct pfeng_priv *priv)
{
	int i;

	for (i = 0; i < PFENG_EMACS_COUNT; i++)
		if (priv->config->config_mac_mask & (1 << i)) {
			mdio_unregister(priv->mii[i]);
			mdio_free(priv->mii[i]);
		}
}

static int pfeng_mdio_register(struct pfeng_priv *priv, int mac_id)
{
	char dev_name[32];
	int ret;

	priv->mii[mac_id] = mdio_alloc();
	if (!priv->mii[mac_id]) {
		dev_err(priv->dev, "mdio_alloc() failed");
		return -ENOMEM;
	}

	snprintf(dev_name, sizeof(dev_name) - 1, "pfeng_emac_%d", mac_id);

	priv->mii[mac_id]->read = emac_mdio_read;
	priv->mii[mac_id]->write = emac_mdio_write;
	priv->mii[mac_id]->priv = priv->pfe->emac_base[mac_id];
	strcpy(priv->mii[mac_id]->name, dev_name);

	ret = mdio_register(priv->mii[mac_id]);
	if (ret < 0) {
		dev_err(priv->dev, "mdio_register('%s') failed: %d", dev_name,
			ret);
		return ret;
	}
	debug("%s: registered mdio bus %s\n", __func__, dev_name);

	return 0;
}

/* driver */
static int pfeng_write_hwaddr(struct udevice *dev)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	struct pfeng_pdata *pdata = dev_get_platdata(dev);
	uchar ea[ARP_HLEN];

	if (!priv || !pdata)
		return 0;

	/* Use 'pfe%daddr' for hwaddr */
	if (eth_env_get_enetaddr_by_index("pfe", priv->if_index, ea)) {
		if (memcmp(pdata->eth.enetaddr, ea, ARP_HLEN))
			memcpy(pdata->eth.enetaddr, ea, ARP_HLEN);
		return 0;
	}

	dev_err(dev, "Can not read hwaddr from 'pfe%daddr'\n", priv->if_index);
	return -EINVAL;
}

static int pfeng_check_env(void)
{
	char *env_mode = env_get(PFENG_ENV_VAR_MODE_NAME);
	char *tok, *loc_mode;

	if (!env_mode || !strlen(env_mode)) {
		/* return default mode */
		return PFENG_MODE_DEFAULT;
	}
	loc_mode = strdup(env_mode);

	tok = strchr(loc_mode, ',');
	if (tok)
		*tok = '\0';
	if (!strcmp("disable", loc_mode)) {
		return PFENG_MODE_DISABLE;
	} else {
		if (strcmp("enable", loc_mode)) {
			/* not "disable" nor "enable" so do default */
			return PFENG_MODE_DEFAULT;
		}
	}

	return PFENG_MODE_ENABLE;
}

static int pfeng_set_fw_from_env(struct pfeng_priv *priv)
{
	char *env_fw;
	char *fw_int = NULL, *fw_name = NULL, *fw_part = NULL;
	int fw_type = FS_TYPE_ANY;
	char *p;

	/* Parse fw destination from environment */
	env_fw = env_get(PFENG_ENV_VAR_FW_SOURCE);
	if (env_fw && strlen(env_fw)) {
		/* We want to modify content, so duplicate */
		env_fw = strdup(env_fw);
		/* check for interface (mmc/usb/...) */
		if ((p = strchr(env_fw, '@'))) {
			fw_int = env_fw;
			*p = '\0';
			env_fw = ++p;
		}

		/* check for dev:part:fwname */
		if ((p = strrchr(env_fw, ':'))) {
			*p = '\0';
			fw_part = env_fw;
			fw_name = ++p;
		} else {
#if CONFIG_IS_ENABLED(FSL_PFENG_FW_LOC_QSPI)
			fw_part = env_fw;
#endif
		}
	}

	if (!fw_name & !fw_part) {
		/* use the default config variant */
#if CONFIG_IS_ENABLED(FSL_PFENG_FW_LOC_SDCARD)
		fw_name = CONFIG_FSL_PFENG_FW_NAME;
#endif
		fw_part = CONFIG_FSL_PFENG_FW_PART;
	}

#if CONFIG_IS_ENABLED(FSL_PFENG_FW_LOC_SDCARD)
	if (!fw_int)
		fw_int = "mmc";
#endif

	/* FW load */
	return pfeng_fw_load(fw_name, fw_int, fw_part, fw_type, priv);
}

static struct pfe_hif_ring *pfeng_init_ring(bool is_rx)
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
		return NULL;
	}

	mmu_set_region_dcache_behaviour((phys_addr_t)ring->bd,
					size, DCACHE_OFF);

	size = roundup(RING_LEN * sizeof(struct pfe_hif_wb_bd), page_size);
	ring->wb_bd = memalign(max((u32)RING_BD_ALIGN, page_size), size);

	if (!ring->wb_bd) {
		pr_warn("HIF ring couldn't be allocated.\n");
		return NULL;
	}

	mmu_set_region_dcache_behaviour((phys_addr_t)ring->wb_bd,
					size, DCACHE_OFF);

	/* Flushe cache to update MMU mapings */
	flush_dcache_all();

	ring->is_rx = is_rx;

	memset(ring->bd, 0, RING_LEN * sizeof(struct pfe_hif_bd));

	if (ring->is_rx) {
		/* fill buffers */
		ring->mem = memalign(page_size,
				     PFE_BUF_SIZE * PFE_HIF_RING_CFG_LENGTH);
		offset = ring->mem;
		if (!ring) {
			free(ring);
			return NULL;
		}
	}

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
		pfeng_flush_d(&ring->bd[ii], sizeof(struct pfe_hif_bd));
	}

	ring->bd[ii - 1].next = (u32)(u64)&ring->bd[0];
	ring->bd[ii - 1].last_bd = 1U;
	pfeng_flush_d(&ring->bd[ii - 1], sizeof(struct pfe_hif_bd));

	memset(ring->wb_bd, 0, RING_LEN * sizeof(struct pfe_hif_wb_bd));

	for (ii = 0U; ii < RING_LEN; ii++) {
		ring->wb_bd[ii].seqnum = 0xffffU;
		ring->wb_bd[ii].desc_en = 1U;
		pfeng_flush_d(&ring->wb_bd[ii], sizeof(struct pfe_hif_wb_bd));
	}

	debug("BD ring %p\nWB ring %p\nBuff %p\n",
	      ring->bd, ring->wb_bd, ring->mem);

	return ring;
}

static int pfeng_driver_init(struct pfeng_priv *priv)
{
	int ret;

	/* CFG setup */
	priv->pfe_cfg.cbus_base = (u64)S32G_PFE_REGS_BASE;
	priv->pfe_cfg.cbus_len = 0x0; /* not used */
	priv->pfe_cfg.fw = &priv->fw;

	ret = pfeng_hw_init(&priv->pfe_cfg);
	if (ret) {
		dev_err(priv->dev, "Could not init PFE platform\n");
		goto end;
	}

	priv->pfe = pfeng_hw_get_instance();
	if (!priv->pfe) {
		dev_err(priv->dev, "Could not get PFE platform instance\n");
		ret = -EINVAL;
		goto end;
	}
	debug("PFE platform inited\n");

	pfeng_write_hwaddr(priv->dev);
end:
	return ret;
}

static void pfeng_stop(struct udevice *dev)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p):\n", __func__, dev);

	ret = pfeng_hw_stop(priv->pfe, priv->if_index);
	if (ret)
		pr_err("PFE HW stop failed\n");
}

static int pfeng_start(struct udevice *dev)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	struct pfeng_pdata *pdata = dev_get_platdata(dev);
	char devname[16];
	char *env_emac = env_get(PFENG_ENV_VAR_EMAC);
	int clid, ret = 0;

	debug("%s(dev=%p):\n", __func__, dev);

	/* The port index to be used (pfe0/pfe1/pfe2) */
	if (!env_emac)
		clid = 0;
	else
		clid = simple_strtoul(env_emac, NULL, 10);
	priv->if_changed = priv->if_index != clid;
	priv->if_index = clid;
	sprintf(devname, "pfe%i", clid);

	printf("Attached to %s\n", devname);

	/* Update clocks */
	pfeng_apply_clocks(dev);

	priv->last_rx = NULL;
	priv->last_tx = NULL;

	/* check if the interface is up */
	if (pfeng_cfg_emac_get_interface(clid) == PHY_INTERFACE_MODE_SGMII)
		pfeng_serdes_wait_link(clid);

	/* Get address of current interface */
	ret = pfeng_write_hwaddr(dev);
	if (ret) {
		pr_err("Was not possible to set MAC address\n");
		return ret;
	}

	/* Prepare platform for RX & TX */
	ret = pfeng_hw_start(priv->pfe, clid, pdata->eth.enetaddr);
	if (ret) {
		pr_err("PFE HW start failed\n");
		return ret;
	}

	return 0;
}

static int pfeng_send(struct udevice *dev, void *packet, int length)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	struct pfe_hif_ring *ring = priv->tx_ring;
	struct pfe_hif_bd *bd_hd, *bd_pkt, *bp_rd;
	struct pfe_hif_wb_bd *wb_bd_hd, *wb_bd_pkt, *wb_bp_rd;
	bool lifm = false;

	debug("%s(dev=%p, packet=%p, length=%d):\n", __func__, dev, packet,
	      length);

	/* Get descriptor for header */
	bd_hd = &ring->bd[ring->write_idx & RING_LEN_MASK];
	wb_bd_hd = &ring->wb_bd[ring->write_idx & RING_LEN_MASK];

	/* Get descriptor for packet */
	bd_pkt = &ring->bd[(ring->write_idx + 1) & RING_LEN_MASK];
	wb_bd_pkt = &ring->wb_bd[(ring->write_idx + 1) & RING_LEN_MASK];

	pfeng_inval_d(bd_hd, sizeof(struct pfe_hif_bd));
	pfeng_inval_d(bd_pkt, sizeof(struct pfe_hif_bd));

	if (RING_BD_DESC_EN(bd_hd->ctrl) != 0U ||
	    RING_BD_DESC_EN(bd_pkt->ctrl) != 0U)
		return -EAGAIN;

	/* Flush the data buffer */
	pfeng_flush_d(packet, length);

	/* Fill header */
	bd_hd->data = (u64)&header[priv->if_index];
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

		pfeng_inval_d(bp_rd, sizeof(struct pfe_hif_bd));
		pfeng_inval_d(wb_bp_rd, sizeof(struct pfe_hif_wb_bd));

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

	priv->last_tx = packet;

	return 0;
}

static int pfeng_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	struct pfe_hif_ring *ring = priv->rx_ring;
	struct pfe_hif_bd *bd_pkt;
	struct pfe_hif_wb_bd *wb_bd_pkt;
	int plen = 0;

	bd_pkt = &ring->bd[ring->read_idx & RING_LEN_MASK];
	wb_bd_pkt = &ring->wb_bd[ring->read_idx & RING_LEN_MASK];

	pfeng_inval_d(bd_pkt, sizeof(struct pfe_hif_bd));
	pfeng_inval_d(wb_bd_pkt, sizeof(struct pfe_hif_wb_bd));

	debug("%s(dev=%p, flags=%x):\n", __func__, dev, flags);

	/* Check, if we received some data */
	if (RING_BD_DESC_EN(bd_pkt->ctrl) == 0U ||
	    RING_WBBD_DESC_EN(wb_bd_pkt->ctrl) != 0u)
		return -EAGAIN;

	/* Give the data to u-boot stack */
	bd_pkt->desc_en = 0U;
	wb_bd_pkt->desc_en = 1U;
	dmb();
	*packetp = ((void *)((u64)(bd_pkt->data) + HIF_HEADER_SIZE));
	priv->last_rx = *packetp;
	plen = wb_bd_pkt->buflen - HIF_HEADER_SIZE;

	/* Advance read buffer */
	ring->read_idx++;

	/* Invalidate the buffer */
	pfeng_inval_d(*packetp, plen);

	if (wb_bd_pkt->lifm != 1U) {
		printf("Multi buffer packets not supported, discarding.\n");
		/* Return EOK so the stack frees the buffer */
		return 0;
	}

	return plen;
}

static int pfeng_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	struct pfe_hif_ring *ring = priv->rx_ring;
	struct pfe_hif_bd *bd_pkt;
	struct pfe_hif_wb_bd *wb_bd_pkt;

	bd_pkt = &ring->bd[ring->write_idx & RING_LEN_MASK];
	wb_bd_pkt = &ring->wb_bd[ring->write_idx & RING_LEN_MASK];

	pfeng_inval_d(bd_pkt, sizeof(struct pfe_hif_bd));
	pfeng_inval_d(wb_bd_pkt, sizeof(struct pfe_hif_wb_bd));

	debug("%s(packet=%p, length=%d)\n", __func__, packet, length);

	if (!packet)
		return 0;

	if (!priv->last_rx)
		return 0;

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
	pfeng_flush_d(packet, length);
	pfeng_inval_d(packet, length);

	/* Advance free pointer */
	ring->write_idx++;

	priv->last_rx = NULL;

	return 0;
}

static const struct eth_ops pfeng_ops = {
	.start = pfeng_start,
	.stop = pfeng_stop,
	.send = pfeng_send,
	.recv = pfeng_recv,
	.free_pkt = pfeng_free_pkt,
	.write_hwaddr = pfeng_write_hwaddr,
};

struct pfeng_config pfeng_s32g274a_config = {
	.config_mac_mask = (1 << PFENG_EMACS_COUNT) - 1,
};

#if CONFIG_IS_ENABLED(OF_CONTROL)
static int pfeng_ofdata_to_platdata(struct udevice *dev)
{
	struct pfeng_pdata *pdata = dev_get_platdata(dev);

	if (!pdata) {
		dev_err(dev, "no platform data");
		return -ENOMEM;
	}

	pdata->eth.iobase = devfdt_get_addr(dev);
	if (pdata->eth.iobase == FDT_ADDR_T_NONE) {
		dev_err(dev, "devfdt_get_addr() failed");
		return -ENODEV;
	}

	pdata->config = (void *)dev_get_driver_data(dev);

	return 0;
}
#endif /* OF_CONTROL */

static int pfeng_probe(struct udevice *dev)
{
	struct pfeng_pdata *pdata = dev_get_platdata(dev);
	struct pfeng_priv *priv = dev_get_priv(dev);
	int ret, i;
	char *env_mode;

	debug("%s(dev=%p):\n", __func__, dev);
	/* check environment vars */
	if (pfeng_check_env() == PFENG_MODE_DISABLE) {
		dev_warn(
			dev,
			"pfeng: driver disabled by environment (pfeng_mode)\n");
		return -ENODEV;
	}

	pfeng_drv_priv = priv;
	priv->dev = dev;
	priv->dev_index = eth_get_dev_index();
	priv->if_index = 0;
	priv->if_changed = true;
	priv->clocks_done = false;

	priv->config = pdata->config;
	if (!priv->config) {
		dev_err(dev, "invalid config!\n");
		return -ENODEV;
	}

	/* retrieve emacs mode from env */
	env_mode = env_get(PFENG_ENV_VAR_MODE_NAME);
	if (env_mode && ((env_mode = strchr(env_mode, ','))) && *env_mode)
		pfeng_set_emacs_from_env(++env_mode);

	/* Check if SerDes is configured for SGMII */
	for (i = 0; i < PFENG_EMACS_COUNT; i++)
		if (pfeng_cfg_emac_get_interface(i) == PHY_INTERFACE_MODE_SGMII)
			if (pfeng_serdes_emac_is_init(i))
				return -ENODEV;

	/* enable PFE IP support */
	pfeng_cfg_set_mode(PFENG_MODE_ENABLE, dev);

	/* fw: parse location and load it */
	ret = pfeng_set_fw_from_env(priv);
	if (ret)
		return ret;

	/* init pfe platform driver */
	ret = pfeng_driver_init(priv);
	if (ret)
		return ret;

	/* init TX ring */
	priv->tx_ring = pfeng_init_ring(false);
	if (!priv->tx_ring)
		return -ENOMEM;

	/* init RX ring */
	priv->rx_ring = pfeng_init_ring(true);
	if (!priv->rx_ring)
		return -ENOMEM;

	/* register rings to pfe HW */
	ret = pfeng_hw_attach_ring(priv->pfe,
				   priv->tx_ring->bd, priv->tx_ring->wb_bd,
				   priv->rx_ring->bd, priv->rx_ring->wb_bd);
	if (ret)
		return ret;

	/* register mdios */
	for (i = 0; i < PFENG_EMACS_COUNT; i++)
		if (priv->config->config_mac_mask & (1 << i))
			pfeng_mdio_register(priv, i);
		else
			dev_warn(dev, "skipped MDIO bus for EMAC %d\n", i);

	return 0;
}

static int pfeng_remove(struct udevice *dev)
{
	struct pfeng_priv *priv = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	/* free all registered MDIO buses */
	pfeng_mdio_unregister_all(priv);

	return 0;
}

static int pfeng_bind(struct udevice *dev)
{
	return device_set_name(dev, "eth_pfeng");
}

/* Driver declaration */

U_BOOT_DRIVER(eth_pfeng) = {
	.name = "eth_pfeng",
	.id = UCLASS_ETH,
	.of_match = of_match_ptr(pfeng_eth_ids),
	.ofdata_to_platdata = of_match_ptr(pfeng_ofdata_to_platdata),
	.bind = pfeng_bind,
	.probe = pfeng_probe,
	.remove = pfeng_remove,
	.ops = &pfeng_ops,
	.priv_auto_alloc_size = sizeof(struct pfeng_priv),
	.platdata_auto_alloc_size = sizeof(struct eth_pdata),
};
