#include <config.h>
#include <common.h>
#include <phy.h>

#define FIXED_PHY_UID   (0x484a53)

int get_phy_id(struct mii_dev *bus, int addr, int devad, u32 *phy_id)
{
	*phy_id = FIXED_PHY_UID;

	return 0;
}

static int fixed_phy_startup(struct phy_device *phydev)
{
	phydev->link	= 1;
#ifdef CONFIG_BCM_SPEED
	phydev->speed	= CONFIG_BCM_SPEED;
#else
	phydev->speed	= SPEED_1000;
#endif

#ifdef CONFIG_BCM_DUPLEX_MODE
	phydev->duplex	= CONFIG_BCM_DUPLEX_MODE;
#else
	phydev->duplex	= DUPLEX_FULL;
#endif
	return 0;
}

static int fixed_phy_shutdown(struct phy_device *phydev)
{
	return 0;
}

static int fixed_phy_config(struct phy_device *phydev)
{
	u32 features;

	/* For now, I'll claim that the generic driver supports
	 * all possible port types */
	features = (SUPPORTED_TP | SUPPORTED_MII
			| SUPPORTED_AUI | SUPPORTED_FIBRE |
			SUPPORTED_BNC);

	features |= SUPPORTED_1000baseT_Full;

	phydev->supported	= features;
	phydev->advertising = features;
	return 0;
}

static struct phy_driver fixed_phy_driver =
{
	.name		= "RGMII_Direct_Connect_1Gbit",
	.uid		= FIXED_PHY_UID,
	.mask		= 0xfffff0,
	.features	= PHY_GBIT_FEATURES,
	.config		= &fixed_phy_config,
	.startup	= &fixed_phy_startup,
	.shutdown	= &fixed_phy_shutdown,
};


int phy_fixed_init(void)
{
	phy_register(&fixed_phy_driver);
	return 0;
}
