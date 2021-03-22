// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (c) 2020 Imagination Technologies Limited
 * Copyright 2019-2021 NXP
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

static int pfeng_driver_init(struct pfeng_priv *priv)
{
	int ret;

	/* CFG setup */
	priv->pfe_cfg.cbus_base = (u64)S32G_PFE_REGS_BASE;
	priv->pfe_cfg.cbus_len = 0x0; /* not used */
	priv->pfe_cfg.fw = &priv->fw;
	priv->pfe_cfg.csr_clk_f = get_pfe_axi_clk_f(priv->dev);

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
	int ret = 0;

	debug("%s(dev=%p, packet=%p, length=%d):\n", __func__, dev, packet,
	      length);
	ret = pfeng_hw_chnl_xmit(priv->chnl, priv->if_index, packet, length);

	/* Store ref only if the packet was transmitted  */
	if (!ret)
		priv->last_tx = packet;

	return ret;
}

static int pfeng_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	int ret = 0;

	debug("%s(dev=%p, flags=%x):\n", __func__, dev, flags);
	ret = pfeng_hw_chnl_receive(priv->chnl, flags, packetp);

	/* Store ref only in case we actually recived something */
	/* neg ret = no free, zero ret = error free, pos ret = success free*/
	if (ret >= 0)
		priv->last_rx = *packetp;

	return ret;
}

static int pfeng_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	int ret = 0;

	debug("%s(packet=%p, length=%d)\n", __func__, packet, length);

	if (!packet)
		return 0;

	if (!priv->last_rx)
		return 0;

	ret = pfeng_hw_chnl_free_pkt(priv->chnl, packet, length);
	if (!ret)
		priv->last_rx = NULL;

	return ret;
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
	priv->chnl = pfeng_hw_init_chnl();
	if (!priv->chnl)
		return -ENOMEM;

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
