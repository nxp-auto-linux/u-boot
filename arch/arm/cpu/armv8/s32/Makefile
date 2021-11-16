# SPDX-License-Identifier:	GPL-2.0+
#
# (C) Copyright 2013-2016 Freescale Semiconductor, Inc.
# (C) Copyright 2016-2018, 2020-2021 NXP
#
ccflags-y += -I./board/freescale/common/
ccflags-y += -Iarch/arm/cpu/armv8/s32/

obj-$(CONFIG_S32_GEN1)		+= s32-gen1/
obj-$(CONFIG_S32_GEN1)		+= scmi_reset_agent.o
obj-y				+= cpu.o
obj-y				+= lowlevel.o
obj-$(CONFIG_SYS_PIT_TIMER)	+= timer.o
obj-$(CONFIG_GICSUPPORT)	+= gicsupport.o
obj-$(CONFIG_MP)		+= mp.o
obj-$(CONFIG_OF_LIBFDT)		+= fdt.o
obj-y				+= initsram.o
ccflags-y			+= -Idrivers/clk/s32/include
ccflags-y			+= -Idrivers/misc
ccflags-y			+= -Iboard/freescale/s32-gen1
ccflags-$(CONFIG_SYS_ERRATUM_ERR050543)	+= -Idrivers/ddr/fsl
