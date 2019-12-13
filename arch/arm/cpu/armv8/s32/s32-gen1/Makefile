# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright 2018-2019 NXP
#

obj-y					+= clock.o
obj-$(CONFIG_S32G275)			+= s32g274a/s32g274a-clock.o
obj-$(CONFIG_S32V344)			+= s32g274a/s32g274a-clock.o
obj-$(CONFIG_S32R45X)			+= s32r45x/s32r45x-clock.o
obj-y					+= ncore.o
obj-y					+= soc.o
obj-$(CONFIG_S32_GEN_1_USES_FLASH)	+= qspi.o
ccflags-y				+= -I./board/freescale/s32-gen1/
ccflags-y				+= -I./arch/arm/include/asm/arch-s32/s32-gen1/
