# SPDX-License-Identifier: GPL-2.0+
#
# Copyright 2020 NXP
#

obj-y := campps32v2.o

ifdef CONFIG_S32_LPDDR2
obj-$(CONFIG_TARGET_CAMPPS32V2) += lpddr2_config.o
obj-$(CONFIG_TARGET_CAMPPS32V2_REVB) += lpddr2_config_revb.o
endif

obj-$(CONFIG_SJA1105) += sja1105_cfg.o

#########################################################################
