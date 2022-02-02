#
# (C) Copyright 2013-2015 Freescale Semiconductor, Inc.
# Copyright 2018-2022 NXP
#
# SPDX-License-Identifier:	GPL-2.0+
#

ccflags-$(CONFIG_S32_GEN1) += -Iarch/$(ARCH)/cpu/armv8/s32
ccflags-$(CONFIG_S32_GEN1) += -Iboard/freescale/s32-gen1
ccflags-$(CONFIG_S32_GEN1) += -Idrivers/clk/s32/include
ccflags-$(CONFIG_S32_GEN1) += -Idrivers/misc
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
