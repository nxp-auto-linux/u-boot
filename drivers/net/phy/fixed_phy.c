#include <config.h>
#include <common.h>
#include <fixed_phy.h>
#include <phy.h>

int fixed_phy_startup(struct phy_device *phydev)
{
	phydev->link 	= 1;
	phydev->speed 	= SPEED_1000;
	phydev->duplex 	= DUPLEX_FULL;
	return 0;
}

int fixed_phy_shutdown(struct phy_device *phydev)
{
	return 0;
}

int fixed_phy_config(struct phy_device *phydev)
{
	u32 features;

	/* For now, I'll claim that the generic driver supports
	 * all possible port types */
	features = (SUPPORTED_TP | SUPPORTED_MII
			| SUPPORTED_AUI | SUPPORTED_FIBRE |
			SUPPORTED_BNC);

	features |= SUPPORTED_1000baseT_Full;
		
	phydev->supported 	= features;
	phydev->advertising = features;
	return 0;
}


static struct phy_driver fixed_phy_driver = 
{
	.name 		= "RGMII_Direct_Connect_1Gbit",
	.uid 		= FIXED_PHY_UID,
	.mask 		= 0xfffff0,
	.features 	= PHY_GBIT_FEATURES,
	.config 	= &fixed_phy_config,
	.startup 	= &fixed_phy_startup,
	.shutdown 	= &fixed_phy_shutdown,
};

int phy_fixed_init(void)
{
	phy_register(&fixed_phy_driver);
	return 0;
}
