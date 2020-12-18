# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright 2018-2020 NXP
#

obj-$(CONFIG_NXP_S32G2XX)		+= cmu.o
obj-$(CONFIG_NXP_S32R45)		+= cmu.o
obj-$(CONFIG_HSE_SECBOOT)		+= hse_adv_secboot.o
obj-y					+= ncore.o
obj-y					+= soc.o
obj-y					+= sram.o
ifneq ($(filter y,$(CONFIG_NXP_S32G2XX) $(CONFIG_NXP_S32R45)),)
obj-$(CONFIG_NETDEVICES)		+= sgmii/sgmii.o sgmii/serdes_xpcs.o
endif
ccflags-y				+= -I./board/freescale/s32-gen1/
ccflags-y				+= -I./arch/arm/include/asm/arch-s32/s32-gen1/
ccflags-y				+= -I./drivers/clk/s32/include/
