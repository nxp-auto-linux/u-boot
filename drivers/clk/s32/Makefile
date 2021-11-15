# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright 2020-2021 NXP
#
obj-y += s32gen1_clk_drv.o
obj-y += enable_clk.o
obj-y += get_rate.o
obj-y += set_par_rate.o
obj-y += s32gen1_clk.o
obj-y += early_clocks.o
obj-y += clk_utils.o
obj-y += s32gen1_scmi_ids.o
obj-y += s32gen1_scmi_clk.o
obj-$(CONFIG_NXP_S32G2XX) += s32g274a_clk.o s32g_clk.o s32g_scmi_ids.o
obj-$(CONFIG_NXP_S32G3XX) += s32g3_clk.o s32g_clk.o s32g_scmi_ids.o
obj-$(CONFIG_NXP_S32R45) += s32r45_clk.o s32r45_scmi_ids.o
obj-$(CONFIG_CMD_CLK) += clk_dump/

ccflags-y += -Iarch/arm/include/asm/arch-s32/s32-gen1
ccflags-y += -Idrivers/clk/s32/include
