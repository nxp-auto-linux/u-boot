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

#include "pfeng.h"

#include <dm/platform_data/pfeng_dm_eth.h>
#include <dm/device_compat.h>

#define HIF_QUEUE_ID	0
#define HIF_HEADER_SIZE sizeof(pfe_ct_hif_rx_hdr_t)

static struct pfeng_priv *pfeng_drv_priv = NULL;
/* firmware */

#if CONFIG_IS_ENABLED(FSL_PFENG_FW_LOC_SDCARD)
static int
pfeng_fw_load(char *fname, char *iface, char *part, int ftype,
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

static int
pfeng_fw_load(char *fname, char *iface, const char *part, int ftype,
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

/* debug (sysfs-like) */

#define DEBUG_BUF_SIZE (4096 * 8)
static char text_buf[DEBUG_BUF_SIZE];

int
pfeng_debug_emac(u32 idx)
{
	int ret;

	if (!pfeng_drv_priv)
		return 0;

	ret = pfe_emac_get_text_statistics(pfeng_drv_priv->pfe->emac[idx],
					   text_buf, DEBUG_BUF_SIZE, 9);
	if (ret > 0)
		printf(text_buf);
	else
		printf("<no data?>\n");

	return 0;
}

int
pfeng_debug_hif(void)
{
	int ret;

	if (!pfeng_drv_priv)
		return 0;

	ret = pfe_hif_get_text_statistics(pfeng_drv_priv->pfe->hif, text_buf,
					  DEBUG_BUF_SIZE, 9);
	if (ret > 0)
		printf(text_buf);
	else
		printf("<no data?>\n");

	return 0;
}

int
pfeng_debug_class(void)
{
	int ret;

	if (!pfeng_drv_priv)
		return 0;

	ret = pfe_class_get_text_statistics(pfeng_drv_priv->pfe->classifier,
					    text_buf, DEBUG_BUF_SIZE, 9);
	if (ret > 0)
		printf(text_buf);
	else
		printf("<no data?>\n");
	return 0;
}

/* MDIO */

static int
emac_mdio_read(struct mii_dev *bus, int mdio_addr, int mdio_devad, int mdio_reg)
{
	pfe_emac_t *emac = bus->priv;
	u16 val;
	int ret;

	if (mdio_devad == MDIO_DEVAD_NONE)
		/* clause 22 */
		ret = pfe_emac_mdio_read22(emac, mdio_addr, mdio_reg, &val, 0);
	else
		/* clause 45 */
		ret = pfe_emac_mdio_read45(emac, mdio_addr, mdio_devad,
					   mdio_reg, &val, 0);

	if (ret) {
		pr_err("%s: MDIO read on MAC failed\n", bus->name);
		return -EAGAIN;
	}
	debug("%s: val=%x\n", __func__, val);

	return val;
}

static int
emac_mdio_write(struct mii_dev *bus, int mdio_addr, int mdio_devad,
		int mdio_reg, u16 mdio_val)
{
	pfe_emac_t *emac = bus->priv;
	int ret;

	debug("%s(addr=%x, reg=%d, val=%x):\n", __func__, mdio_addr, mdio_reg,
	      mdio_val);

	if (mdio_devad == MDIO_DEVAD_NONE)
		/* clause 22 */
		ret = pfe_emac_mdio_write22(emac, mdio_addr, mdio_reg, mdio_val,
					    0);
	else
		/* clause 45 */
		ret = pfe_emac_mdio_write45(emac, mdio_addr, mdio_devad,
					    mdio_reg, mdio_val, 0);

	if (ret) {
		pr_err("%s: MDIO write on MAC failed\n", bus->name);
		return -EAGAIN;
	}

	return 0;
}

static void
pfeng_mdio_unregister_all(struct pfeng_priv *priv)
{
	int i;

	for (i = 0; i < PFENG_EMACS_COUNT; i++)
		if (priv->config->config_mac_mask & (1 << i)) {
			mdio_unregister(priv->mii[i]);
			mdio_free(priv->mii[i]);
		}
}

static int
pfeng_mdio_register(struct pfeng_priv *priv, int mac_id)
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
	priv->mii[mac_id]->priv = priv->pfe->emac[mac_id];
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

static int
pfeng_write_hwaddr(struct udevice *dev)
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
		if (!priv->logif_emac)
			return 0;
		return pfe_log_if_set_mac_addr(priv->logif_emac, ea);
	}

	dev_err(dev, "Can not read hwaddr from 'pfe%daddr'\n", priv->if_index);
	return -EINVAL;
}

static int
pfeng_check_env(void)
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

static int
pfeng_set_fw_from_env(struct pfeng_priv *priv)
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
		fw_type = FS_TYPE_FAT;
	}

#if CONFIG_IS_ENABLED(FSL_PFENG_FW_LOC_SDCARD)
	if (!fw_int)
		fw_int = "mmc";
#endif

	/* FW load */
	return pfeng_fw_load(fw_name, fw_int, fw_part, fw_type, priv);
}

static int
pfeng_driver_init(struct pfeng_priv *priv)
{
	int ret;

	/* CFG setup */
	priv->pfe_cfg.common_irq_mode = FALSE; /* don't use common irq mode */
	priv->pfe_cfg.irq_vector_hif_chnls[0] = 0x0; /* no IRQ used */
	priv->pfe_cfg.cbus_base = (addr_t)S32G_PFE_REGS_BASE;
	priv->pfe_cfg.cbus_len = 0x0; /* not used */
	priv->pfe_cfg.fw = &priv->fw;
	priv->pfe_cfg.hif_chnls_mask = HIF_CHNL_0; /* channel bitmap */
	priv->pfe_cfg.irq_vector_hif_nocpy = 0;	   /* disable */
	priv->pfe_cfg.irq_vector_bmu = 0;	   /* no IRQ used */

	ret = pfe_platform_init(&priv->pfe_cfg);
	if (ret) {
		dev_err(priv->dev, "Could not init PFE platform\n");
		goto end;
	}

	priv->pfe = pfe_platform_get_instance();
	if (!priv->pfe) {
		dev_err(priv->dev, "Could not get PFE platform instance\n");
		ret = -EINVAL;
		goto end;
	}
	debug("PFE platform inited\n");

	/* Platform drv init */
	priv->channel = pfe_hif_get_channel(priv->pfe->hif, HIF_CHNL_0);
	if (!priv->channel) {
		dev_err(priv->dev, "Could not get PFE HIF channel instance\n");
		ret = -EINVAL;
		goto end;
	}
	debug("PFE HIF channel retrieved\n");

	priv->hif = pfe_hif_drv_create(priv->channel);
	if (!priv->hif) {
		dev_err(priv->dev, "Could not create HIF driver instance\n");
		ret = -EINVAL;
		goto end;
	}
	debug("PFE HIF channel inited\n");

	if (pfe_hif_drv_init(priv->hif) != EOK) {
		dev_err(priv->dev, "Could not init HIF driver instance\n");
		ret = -EINVAL;
		goto end;
	}
	debug("PFE HIF driver inited\n");

	pfeng_write_hwaddr(priv->dev);
end:
	return ret;
}

static int
pfeng_hif_event_handler(pfe_hif_drv_client_t *client, void *data,
			u32 event, uint32_t qno)
{
	switch (event) {
	case EVENT_HIGH_RX_WM: /* Rx queue has reached watermark level */
		/* unused */
		break;
	case EVENT_RX_PKT_IND: /* New packet(s) received */
		/* unused */
		break;
	case EVENT_TXDONE_IND: /* New Tx confirmation(s) */
		/* unused */
		break;
	case EVENT_RX_OOB: /* Ran out of Rx buffers */
		/* unused */
		break;
	default:
		/* unused */
		break;
	}

	return 0;
}

static void
pfeng_stop(struct udevice *dev)
{
	struct pfeng_priv *priv = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	if (priv->hif)
		pfe_hif_drv_stop(priv->hif);

	if (priv->logif_emac) {
		pfe_log_if_disable(priv->logif_emac);
		pfe_platform_unregister_log_if(priv->pfe, priv->logif_emac);
	}
	if (priv->logif_hif) {
		pfe_log_if_disable(priv->logif_hif);
		pfe_platform_unregister_log_if(priv->pfe, priv->logif_hif);
	}

	if (priv->client) {
		pfe_hif_drv_client_unregister(priv->client);
		priv->client = NULL;
	}

	if (priv->logif_emac) {
		pfe_log_if_destroy(priv->logif_emac);
		priv->logif_emac = NULL;
	}
	if (priv->logif_hif) {
		pfe_log_if_destroy(priv->logif_hif);
		priv->logif_hif = NULL;
	}

	return;
}

static int
pfeng_start(struct udevice *dev)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	u32 clid = 0;
	int ret;
	char devname[16];
	char *env_emac = env_get(PFENG_ENV_VAR_EMAC);
	static const pfe_ct_phy_if_id_t emac_ids[] = { PFE_PHY_IF_ID_EMAC0,
						       PFE_PHY_IF_ID_EMAC1,
						       PFE_PHY_IF_ID_EMAC2 };

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
	pfeng_apply_clocks();

	/* Retrieve PHYIF */
	priv->phyif_emac = pfe_platform_get_phy_if_by_id(
		priv->pfe, emac_ids[priv->if_index]);
	if (!priv->phyif_emac) {
		dev_err(dev, "Unsupported EMAC PHY id %d\n", priv->if_index);
		return -ENODEV;
	}

	priv->phyif_hif =
		pfe_platform_get_phy_if_by_id(priv->pfe, PFE_PHY_IF_ID_HIF0);
	if (!priv->phyif_hif) {
		dev_err(dev, "Unsupported HIF PHY id %d\n", priv->if_index);
		return -ENODEV;
	}

	/* Create LOGIF */
	priv->logif_emac = pfe_log_if_create(priv->phyif_emac, devname);
	if (!priv->logif_emac) {
		dev_err(dev, "Failed to create log if '%s'\n", devname);
		return -ENODEV;
	}
	ret = pfe_platform_register_log_if(priv->pfe, priv->logif_emac);
	if (ret)
		goto err;

	sprintf(devname, "hif%i", clid);
	priv->logif_hif = pfe_log_if_create(priv->phyif_hif, devname);
	if (!priv->logif_hif) {
		dev_err(dev, "Failed to create log if '%s'\n", devname);
		ret = ENODEV;
		goto err;
	}
	ret = pfe_platform_register_log_if(priv->pfe, priv->logif_hif);
	if (ret)
		goto err;

	priv->last_rx = NULL;
	priv->last_tx = NULL;

	/* check if the interface is up */
	if (pfeng_cfg_emac_get_interface(clid) == PHY_INTERFACE_MODE_SGMII)
		pfeng_serdes_wait_link(clid);

	/* Sanitize hwaddr */
	pfeng_write_hwaddr(dev);

	/* Connect to HIF */
	priv->client = pfe_hif_drv_client_register(
		priv->hif,		  /* HIF Driver instance */
		priv->logif_emac,	  /* Client ID */
		1,			  /* TX Queue Count */
		1,			  /* RX Queue Count */
		256,			  /* TX Queue Depth */
		256,			  /* RX Queue Depth */
		&pfeng_hif_event_handler, /* Client's event handler */
		(void *)priv);		  /* Meta data */

	if (!priv->client) {
		dev_err(dev, "Unable to register HIF client id %d\n", clid);
		ret = -ENODEV;
		goto err;
	}
	debug("Register HIF client id %d\n", clid);

	ret = pfe_log_if_add_egress_if(priv->logif_emac, priv->phyif_hif);
	if (ret) {
		dev_err(dev, "Can't set ingress interface for '%s'\n", devname);
		goto err;
	}
	ret = pfe_log_if_add_egress_if(priv->logif_hif, priv->phyif_emac);
	if (ret) {
		dev_err(dev, "Can't set egress interface for '%s'\n", devname);
		goto err;
	}

	/* Enable promiscuous mode.
	   This is default log interface and therefore should always accept all traffic. */
	pfe_log_if_promisc_enable(priv->logif_hif);

	ret = pfe_log_if_enable(priv->logif_emac);
	ret = pfe_log_if_enable(priv->logif_hif);
	if (ret) {
		dev_err(dev, "PFE LOGIF enabling failed\n");
		ret = -ENODEV;
		goto err;
	}
	debug("PFE LOGIF enabled\n");

	/* start HIF driver */
	ret = pfe_hif_drv_start(priv->hif);
	if (ret) {
		dev_err(dev, "PFE HIF driver start failed\n");
		ret = -ENODEV;
		goto err;
	}
	debug("PFE HIF driver started successfully\n");

end:
	return ret;

err:
	pfeng_stop(dev);
	goto end;
}

static int
pfeng_send(struct udevice *dev, void *packet, int length)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	int ret;

	debug("%s(dev=%p, packet=%p, length=%d):\n", __func__, dev, packet,
	      length);

	ret = pfe_hif_drv_client_xmit_pkt(priv->client, HIF_QUEUE_ID, packet,
					  packet, length, packet);
	if (ret) {
		dev_err(dev, "%s(packet=%p, length=%d): failed: %d\n", __func__,
			packet, length, ret);
	} else {
		while (NULL == pfe_hif_drv_client_receive_tx_conf(priv->client,
								  HIF_QUEUE_ID))
			pfe_hif_drv_tx_poll(priv->hif);
	}
	priv->last_tx = packet;

	return 0;
}

static int
pfeng_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	pfe_hif_pkt_t *hif_pkt;
	u32 plen = 0;

	debug("%s(dev=%p, flags=%x):\n", __func__, dev, flags);

	pfe_hif_drv_rx_poll(priv->hif);

	hif_pkt = pfe_hif_drv_client_receive_pkt(priv->client, HIF_QUEUE_ID);
	if (hif_pkt) {
		/* Process the packet */
		if (pfe_hif_pkt_is_last(hif_pkt) == TRUE) {
			/* Whole packet received */
			*packetp =
				(void *)(addr_t)pfe_hif_pkt_get_data(hif_pkt) +
				HIF_HEADER_SIZE;
			plen = pfe_hif_pkt_get_data_len(hif_pkt) -
			       HIF_HEADER_SIZE;
			priv->last_rx = hif_pkt;
		} else {
			/* multi-buffer pkt, discard it */
			dev_err(dev, "Got multi-buffer packet, dropped.\n");
		}
	}

	return plen;
}

static int
pfeng_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct pfeng_priv *priv = dev_get_priv(dev);

	debug("%s(packet=%p, length=%d)\n", __func__, packet, length);

	if (!packet)
		return 0;

	if (!priv->last_rx)
		return 0;

	pfe_hif_pkt_free(priv->last_rx);

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
static int
pfeng_ofdata_to_platdata(struct udevice *dev)
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

static int
pfeng_probe(struct udevice *dev)
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
	priv->if_changed = 1;

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
	pfeng_cfg_set_mode(PFENG_MODE_ENABLE);

	/* fw: parse location and load it */
	ret = pfeng_set_fw_from_env(priv);
	if (ret)
		return ret;

	/* init pfe platform driver */
	ret = pfeng_driver_init(priv);
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

static int
pfeng_remove(struct udevice *dev)
{
	struct pfeng_priv *priv = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	/* free all registered MDIO buses */
	pfeng_mdio_unregister_all(priv);

	return 0;
}

static int
pfeng_bind(struct udevice *dev)
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
