# SPDX-License-Identifier:	GPL-2.0+
#
# (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
# (C) Copyright 2016-2018, 2020-2022 NXP
#
ccflags-y += -I./board/nxp/common/
ccflags-y += -I./arch/arm/mach-s32/s32-cc/

obj-$(CONFIG_NXP_S32_CC)		+= scmi_reset_agent.o
obj-y				+= cpu.o
obj-$(CONFIG_GICSUPPORT)	+= gicsupport.o
obj-$(CONFIG_MP)		+= mp.o
obj-$(CONFIG_OF_LIBFDT)		+= fdt.o
ccflags-y			+= -Idrivers/misc
ccflags-y			+= -Iboard/nxp/s32-gen1

obj-$(CONFIG_S32_CMU)		+= cmu.o
obj-$(CONFIG_ARCH_S32G2)	+= s32g2_cmu.o
obj-$(CONFIG_ARCH_S32G3)	+= s32g3_cmu.o
obj-$(CONFIG_ARCH_S32R45)	+= s32r45_cmu.o
obj-$(CONFIG_HSE_SECBOOT)	+= hse_adv_secboot.o
obj-y				+= soc.o
ifdef CONFIG_NXP_S32_CC
obj-$(CONFIG_NETDEVICES)	+= sgmii/sgmii.o sgmii/serdes_xpcs.o
endif
obj-y				+= start_m7.o
ccflags-y			+= -I./board/nxp/s32-gen1/
ccflags-y			+= -I./arch/arm/mach-s32/s32-cc/include
ccflags-y			+= -I./arch/arm/include/asm/arch-s32/s32-cc/
ccflags-y			+= -I./drivers/misc/
