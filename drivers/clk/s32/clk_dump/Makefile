#
# Copyright 2020-2021 NXP
#
# SPDX-License-Identifier:	GPL-2.0+
#
obj-y += clk_dump.o
obj-y += s32gen1_clk_names.o
obj-$(CONFIG_NXP_S32G2XX) += s32g_clk_names.o
obj-$(CONFIG_NXP_S32G3XX) += s32g_clk_names.o s32g3_clk_names.o
obj-$(CONFIG_NXP_S32R45) += s32r45_clk_names.o

ccflags-y += -Idrivers/clk/s32/include
ccflags-y += -Idrivers/clk/s32/clk_dump/include
