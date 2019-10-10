#
# (C) Copyright 2013-2015 Freescale Semiconductor, Inc.
# Copyright 2018-2019 NXP
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y := common.o

ifeq (,$(CONFIG_OF_EMBED)$(CONFIG_OF_SEPARATE))
	obj-y	+= ../../../tools/lib/fdtdec.o ../../../drivers/core/ofnode.o
endif

obj-$(CONFIG_DWC_ETH_QOS_S32CC)   += eth.o
obj-$(CONFIG_TARGET_S32G275) += s32g275.o
obj-$(CONFIG_TARGET_S32R45X) += s32r45x.o
obj-$(CONFIG_TARGET_S32V344) += s32v344.o

ifdef CONFIG_TARGET_TYPE_S32GEN1_EMULATOR
obj-$(CONFIG_SYS_FSL_DDRSS) += ddrss_regconf_emu.o ddrss_firmware_emu.o
else
obj-$(CONFIG_SYS_FSL_DDRSS) += ddrss_regconf.o ddrss_firmware.o
endif

#########################################################################
