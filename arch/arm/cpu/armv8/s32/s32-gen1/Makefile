# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright 2018-2022 NXP
#

obj-$(CONFIG_S32_CMU)			+= cmu.o
obj-$(CONFIG_NXP_S32G2XX)		+= s32g2_cmu.o
obj-$(CONFIG_NXP_S32G3XX)		+= s32g3_cmu.o
obj-$(CONFIG_NXP_S32R45)		+= s32r45_cmu.o
obj-$(CONFIG_HSE_SECBOOT)		+= hse_adv_secboot.o
obj-y					+= soc.o
ifdef CONFIG_S32_GEN1
obj-$(CONFIG_NETDEVICES)		+= sgmii/sgmii.o sgmii/serdes_xpcs.o
endif
obj-y					+= start_m7.o
ccflags-y				+= -I./board/freescale/s32-gen1/
ccflags-y				+= -I./arch/arm/cpu/armv8/s32/s32-gen1/include
ccflags-y				+= -I./arch/arm/include/asm/arch-s32/s32-gen1/
ccflags-y				+= -I./drivers/misc/
