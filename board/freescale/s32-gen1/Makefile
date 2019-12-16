#
# (C) Copyright 2013-2015 Freescale Semiconductor, Inc.
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier:	GPL-2.0+
#

ccflags-$(CONFIG_S32_GEN1) += -Iarch/$(ARCH)/cpu/armv8/s32
obj-y := common.o

obj-$(CONFIG_S32G274A) += s32g274a.o
obj-$(CONFIG_S32R45X) += s32r45x.o

ifeq (,$(CONFIG_OF_EMBED)$(CONFIG_OF_SEPARATE))
	obj-y	+= ../../../tools/lib/fdtdec.o ../../../drivers/core/ofnode.o
endif

obj-$(CONFIG_NETDEVICES) += eth.o

obj-$(CONFIG_TARGET_S32G274AEVB) += s32g274aevb.o
obj-$(CONFIG_TARGET_S32G274ARDB) += s32g274ardb.o
obj-$(CONFIG_TARGET_S32G274ASIM) += s32g274asim.o
obj-$(CONFIG_TARGET_S32G274AEMU) += s32g274aemu.o

obj-$(CONFIG_TARGET_S32R45XEVB) += s32r45xevb.o
obj-$(CONFIG_TARGET_S32R45XSIM) += s32r45xsim.o
obj-$(CONFIG_TARGET_S32R45XEMU) += s32r45xemu.o

obj-$(CONFIG_TARGET_S32V344EVB) += s32v344sim.o
obj-$(CONFIG_TARGET_S32V344SIM) += s32v344sim.o

ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
obj-$(CONFIG_SYS_FSL_DDRSS) += ddrss_regconf_emu.o ddrss_firmware_emu.o
else
obj-$(CONFIG_SYS_FSL_DDRSS) += ddrss_regconf.o ddrss_firmware.o
obj-$(CONFIG_SJA1105) += sja1105_cfg.o
endif

#########################################################################
