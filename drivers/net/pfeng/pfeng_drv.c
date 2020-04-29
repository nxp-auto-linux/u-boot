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

#include "oal.h"
#include "pfe_platform.h"
#include "pfe_hif_drv.h"

#include <dm/platform_data/pfeng_dm_eth.h>
#include <dm/device_compat.h>

#include "pfeng.h"

#define HIF_QUEUE_ID 0
#define HIF_HEADER_SIZE sizeof(pfe_ct_hif_rx_hdr_t)

static struct pfeng_priv *pfeng_drv_priv = NULL;

int s32_serdes1_wait_link(int idx);

/* firmware */

#if CONFIG_IS_ENABLED(FSL_PFENG_FW_LOC_SDCARD)
static int pfeng_fw_load(char *fname, char *iface, char *part, int ftype,
			 struct pfeng_priv *priv)
{
	int ret;
	void *addr = NULL;
	loff_t length, read = 0;
	char addr_str[32];
	char *load_argv[] = { "fsload", iface, part, addr_str, fname, NULL };

	debug("Loading PFEng fw from %s@%s:%s:%d\n", iface, part, fname, ftype);

	ret = fs_set_blk_dev(iface, part, ftype);
	if (ret < 0)
		goto err;

	ret = fs_size(fname, &length);
	if (ret)
		goto err;

	addr = valloc(length);
	if (!addr) {
		ret = -ENOMEM;
		goto err;
	}

#if 0 /* TODO: fix usage of fs_read and get rid of do_load */
	length = 0;
	ret = fs_read(fname, (ulong)addr, 0, length, &read);
#else
	snprintf(addr_str, sizeof(addr_str) - 1, "0x%llx", (addr_t)addr);
	ret = do_load(NULL, 0, ARRAY_SIZE(load_argv), load_argv, ftype);
	read = length;
#endif
	if (ret < 0)
		goto err;

	priv->fw.class_data = addr;
	priv->fw.class_size = read;

	debug("Found PFEng firmware: %s@%s:%s size %lld\n", iface, part, fname,
	       read);

	return 0;

err:
	dev_err(priv->dev, "PFEng firmware file '%s@%s:%s' loading failed: %d\n",
		 iface, part, fname, ret);
	priv->fw.class_data = NULL;
	priv->fw.class_size = 0;
	return ret;
}
#endif /* FSL_PFENG_FW_LOC_SDCARD */

#if CONFIG_IS_ENABLED(FSL_PFENG_FW_LOC_QSPI)
static int pfeng_fw_load(char *fname, char *iface, const char *part, int ftype,
			 struct pfeng_priv *priv)
{
	int ret = 0;

	/* point to the embedded fw array */
	priv->fw.class_data = (void *)simple_strtoul(part, NULL, 16);
	if (priv->fw.class_data) {
		if (!valid_elf_image((addr_t)priv->fw.class_data)) {
			dev_err(dev, "PFEng firmware is not valid at qspi@%p\n",
				priv->fw.class_data);
			return -EINVAL;
		} else
			priv->fw.class_size = 0x200000; //FIXME: parse elf for size
	}

	if (!priv->fw.class_data) {
		dev_err(dev, "PFEng firmware not found at qspi@%p\n",
			priv->fw.class_data);
		return -EINVAL;
	}

	debug("Found PFEng firmware: qspi@%p\n", priv->fw.class_data);

	return ret;
}
#endif /* FSL_PFENG_FW_LOC_QSPI */

/* debug (sysfs-like) */

#define DEBUG_BUF_SIZE (4096*8)
static char text_buf[DEBUG_BUF_SIZE];

int pfeng_debug_emac(u32 idx)
{
	int ret;

	if(!pfeng_drv_priv)
		return 0;

	ret = pfe_emac_get_text_statistics(pfeng_drv_priv->pfe->emac[idx],
					   text_buf, DEBUG_BUF_SIZE, 9);
	if (ret > 0)
		printf(text_buf);
	else
		printf("<no data?>\n");

	return 0;
}

int pfeng_debug_hif(void)
{
	int ret;

	if(!pfeng_drv_priv)
		return 0;

	ret = pfe_hif_get_text_statistics(pfeng_drv_priv->pfe->hif, text_buf,
					  DEBUG_BUF_SIZE, 9);
	if (ret > 0)
		printf(text_buf);
	else
		printf("<no data?>\n");

	return 0;
}

int pfeng_debug_class(void)
{
	int ret;

	if(!pfeng_drv_priv)
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

static int emac_mdio_read(struct mii_dev *bus, int mdio_addr, int mdio_devad,
			  int mdio_reg)
{
	pfe_emac_t *emac = bus->priv;
	u16 val;
	int ret;

	if (mdio_devad == MDIO_DEVAD_NONE)
		/* clause 22 */
		ret = pfe_emac_mdio_read22(emac, mdio_addr, mdio_reg, &val);
	else
		/* clause 45 */
		ret = pfe_emac_mdio_read45(emac, mdio_addr, mdio_devad,
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
	pfe_emac_t *emac = bus->priv;
	int ret;

	debug("%s(addr=%x, reg=%d, val=%x):\n", __func__,
	      mdio_addr, mdio_reg, mdio_val);

	if (mdio_devad == MDIO_DEVAD_NONE)
		/* clause 22 */
		ret = pfe_emac_mdio_write22(emac, mdio_addr, mdio_reg,
					    mdio_val);
	else
		/* clause 45 */
		ret = pfe_emac_mdio_write45(emac, mdio_addr, mdio_devad,
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

static int pfeng_write_hwaddr(struct udevice *dev)
{
        struct pfeng_priv *priv = dev_get_priv(dev);
	struct pfeng_pdata *pdata = dev_get_platdata(dev);
	uchar ea[ARP_HLEN];

	if (!priv->iface)
		return 0;

	/* Use 'pfe%daddr' for hwaddr */
	if (eth_env_get_enetaddr_by_index("pfe", priv->emac_index, ea)) {
		if (memcmp(pdata->eth.enetaddr, ea, ARP_HLEN))
			memcpy(pdata->eth.enetaddr, ea, ARP_HLEN);
		return pfe_log_if_set_mac_addr(priv->iface, ea);
	}

	dev_err(dev, "Can not read hwaddr from 'pfe%daddr'\n",
		priv->emac_index);
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
		fw_type = FS_TYPE_FAT;
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
	priv->pfe_cfg.common_irq_mode = FALSE; /* don't use common irq mode */
        priv->pfe_cfg.irq_vector_hif_chnls[0] = 0x0; /* no IRQ used */
        priv->pfe_cfg.cbus_base = (addr_t)S32G_PFE_REGS_BASE;
        priv->pfe_cfg.cbus_len = 0x0; /* not used */
        priv->pfe_cfg.fw = &priv->fw;
        priv->pfe_cfg.hif_chnls_mask = HIF_CHNL_0; /* channel bitmap */
        priv->pfe_cfg.irq_vector_hif_nocpy = 0; /* disable */
        priv->pfe_cfg.irq_vector_bmu = 0; /* no IRQ used */

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

	/* Platform init */
	priv->hif = pfe_platform_get_hif_drv(priv->pfe, 0);
	if (!priv->hif) {
                dev_err(priv->dev, "Could not get HIF instance\n");
                ret = -EINVAL;
		goto end;
        }
	debug("PFE HIF inited\n");

	pfeng_write_hwaddr(priv->dev);
end:
	return ret;
}

static int pfeng_hif_event_handler(pfe_hif_drv_client_t *client, void *data,
				   uint32_t event, uint32_t qno)
{
	switch(event) {
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

static void pfeng_stop(struct udevice *dev)
{
	struct pfeng_priv *priv = dev_get_priv(dev);

	debug("%s(dev=%p):\n", __func__, dev);

	if(priv->iface) {
		pfe_log_if_disable(priv->iface);
		priv->iface = NULL;
	}

	if(priv->client) {
		pfe_hif_drv_client_unregister(priv->client);
		priv->client = NULL;
	}

	if(priv->hif)
		pfe_hif_drv_stop(priv->hif);

	return;
}

static int pfeng_start(struct udevice *dev)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	u32 clid = 0;
	int ret;
	char *env_emac = env_get(PFENG_ENV_VAR_EMAC);

	debug("%s(dev=%p):\n", __func__, dev);

	/* The port index to be used (pfe0/pfe1/pfe2) */
	if (!env_emac)
		clid = 0;
	else
		clid = simple_strtoul(env_emac, NULL, 10);
	priv->emac_changed = priv->emac_index != clid;
	priv->emac_index = clid;

	printf("Attached to pfe%d\n", clid);

	/* Update clocks */
	pfeng_apply_clocks();

	/* Retrieve LOGIF */
	priv->iface = pfe_platform_get_log_if_by_id(priv->pfe, clid);
	if (!priv->iface) {
		dev_err(dev, "Incorrect log if id %d\n", clid);
		return -ENODEV;
	}

	priv->last_rx = NULL;
	priv->last_tx = NULL;

	/* check if the interface is up */
	if (pfeng_cfg_emac_get_interface(clid) == PHY_INTERFACE_MODE_SGMII) {
		if (clid == 0 || clid == 1) {
			s32_serdes1_wait_link(clid);
		} else {
			dev_err(dev, "PFE2 SGMII mode not supported\n");
			ret = -ENODEV;
			goto err;
		}
	}

	/* Sanitize hwaddr */
	pfeng_write_hwaddr(dev);

	/* Connect to HIF */
	priv->client = pfe_hif_drv_client_register(
                priv->hif,              /* HIF Driver instance */
                priv->iface,            /* Client ID */
                1,                      /* TX Queue Count */
                1,                      /* RX Queue Count */
                256,                    /* TX Queue Depth */
                256,                    /* RX Queue Depth */
                &pfeng_hif_event_handler, /* Client's event handler */
                (void *)priv);          /* Meta data */

	if (!priv->client)     {
		dev_err(dev, "Unable to register HIF client id %d\n", clid);
		return -ENODEV;
	}
	debug("Register HIF client id %d for log if %p\n", clid, priv->iface);

	/* start HIF driver */
	ret = pfe_hif_drv_start(priv->hif);
	if (ret) {
		dev_err(dev, "PFE HIF driver start failed\n");
		ret = -ENODEV;
		goto err;
	}
	debug("PFE HIF driver started sucessfully\n");

	ret = pfe_log_if_enable(priv->iface);
	if (ret) {
		dev_err(dev, "PFE LOGIF enabling failed\n");
		ret = -ENODEV;
		goto err;
	}
	debug("PFE LOGIF enabled\n");

end:
	return ret;

err:
	pfeng_stop(dev);
	goto end;
}

static int pfeng_send(struct udevice *dev, void *packet, int length)
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

static int pfeng_recv(struct udevice *dev, int flags, uchar **packetp)
{
	struct pfeng_priv *priv = dev_get_priv(dev);
	pfe_hif_pkt_t *hif_pkt;
	u32 plen = 0;

	debug("%s(dev=%p, flags=%x):\n", __func__, dev, flags);

	pfe_hif_drv_rx_poll(priv->hif);

	hif_pkt = pfe_hif_drv_client_receive_pkt(priv->client, HIF_QUEUE_ID);
	if (hif_pkt) {
		/* Process the packet */
		if(TRUE == pfe_hif_pkt_is_last(hif_pkt)) {
			/* Whole packet received */
			*packetp = (void *)(addr_t)pfe_hif_pkt_get_data(hif_pkt)
				   + HIF_HEADER_SIZE;
			plen = pfe_hif_pkt_get_data_len(hif_pkt) - HIF_HEADER_SIZE;
			priv->last_rx = hif_pkt;

		} else {
			/* multi-buffer pkt, discard it */
			dev_err(dev, "Got multi-buffer packet, dropped.\n");
		}
	}

	return plen;
}

static int pfeng_free_pkt(struct udevice *dev, uchar *packet, int length)
{
	struct pfeng_priv *priv = dev_get_priv(dev);

	debug("%s(packet=%p, length=%d)\n", __func__, packet, length);

	if(!packet)
		return 0;

	if(!priv->last_rx)
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
		dev_warn(dev, "pfeng: driver disabled by environment (pfeng_mode)\n");
		return -ENODEV;
	}

	pfeng_drv_priv = priv;
	priv->dev = dev;
	priv->dev_index = eth_get_dev_index();
	priv->emac_index = 0;
	priv->emac_changed = 1;

	priv->config = pdata->config;
	if (!priv->config) {
		dev_err(dev, "invalid config!\n");
		return -ENODEV;
	}

	/* retrieve emacs mode from env */
	env_mode = env_get(PFENG_ENV_VAR_MODE_NAME);
	if (env_mode && ((env_mode = strchr(env_mode, ','))) && *env_mode)
		pfeng_set_emacs_from_env(++env_mode);

	/* check if pcie is not using serdes_1 */
	if (REQUIRE_SERDES(1) && !hwconfig_subarg_cmp("pcie1", "mode", "sgmii"))
		return -ENODEV;

	/* enable PFE IP support */
	pfeng_cfg_set_mode(PFENG_MODE_RUN);

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
