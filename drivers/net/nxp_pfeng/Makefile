# SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
#
# Copyright 2023 NXP
#

# NXP S32G PFE Ethernet accelerator driver
obj-y += pfe_hw_hif.o \
	 pfeng_gpr.o \

ifdef CONFIG_NXP_PFENG_SLAVE
obj-y += pfeng_slave.o \
	 pfeng_netif_slave.o \
	 pfe_hw_slave.o
else
obj-y += pfeng.o \
	 pfeng_fw.o \
	 pfeng_netif.o \
	 pfeng_mdio.o \
	 pfe_hw.o \
	 pfe_hw_emac.o \
	 pfe_hw_pe.o
endif

