#
# (C) Copyright 2013-2015 Freescale Semiconductor, Inc.
# Copyright 2018-2022 NXP
#
# SPDX-License-Identifier:	GPL-2.0+
#

ccflags-$(CONFIG_NXP_S32_CC) += -Iarch/$(ARCH)/cpu/armv8/s32
ccflags-$(CONFIG_NXP_S32_CC) += -Iboard/nxp/s32-cc
ccflags-$(CONFIG_NXP_S32_CC) += -Idrivers/net/s32
ccflags-$(CONFIG_NXP_S32_CC) += -Idrivers/misc
obj-y := common.o

ifeq (,$(CONFIG_OF_EMBED)$(CONFIG_OF_SEPARATE))
	obj-y	+= ../../../drivers/core/ofnode.o
endif

obj-$(CONFIG_NETDEVICES) += eth.o

obj-$(CONFIG_TARGET_S32G2XXAEVB) += s32gxxxaevb.o
obj-$(CONFIG_TARGET_S32G3XXAEVB) += s32gxxxaevb.o
obj-$(CONFIG_TARGET_S32G274ARDB2) += s32gxxxardb.o
obj-$(CONFIG_TARGET_S32G399ARDB3) += s32gxxxardb.o
obj-$(CONFIG_TARGET_S32G274ABLUEBOX3) += s32g274abluebox3.o

obj-$(CONFIG_TARGET_S32R45EVB) += s32r45evb.o

#########################################################################
