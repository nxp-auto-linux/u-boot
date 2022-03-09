# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
# (C) Copyright 2011 Freescale Semiconductor, Inc.
# Copyright 2019-2020,2022 NXP

obj-$(CONFIG_NXP_S32_CC) += s32-cc/

ifdef CONFIG_SPI_FLASH_MACRONIX
QSPI_MEMORY = MX25UW51245G
endif

ifdef CONFIG_SPI_FLASH_STMICRO
QSPI_MEMORY = MT35XU512ABA
endif

QSPI_PARAMS_FILE ?= qspi_params_$(QSPI_MEMORY).bin

s32gen1_qspiparam-objs := tools/s32gen1image_qspi_micron.o \
	tools/s32gen1image_qspi_macronix.o \
	tools/s32gen1_qspiparam.o
HOSTCFLAGS_s32gen1image_qspi_macronix.o += -Itools/
HOSTCFLAGS_s32gen1image_qspi_micron.o += -Itools/
HOSTCFLAGS_s32gen1_qspiparam.o += -Itools/
hostprogs-y += s32gen1_qspiparam
always := $(hostprogs-y)

quiet_cmd_cpp_cfg = CFGS    $@
      cmd_cpp_cfg = $(CPP) $(cpp_flags) -x c -o $@ $<

S32GEN1_CONFIG_FILE = $(CONFIG_S32GEN1_CONFIG_FILE:"%"=%)

ifdef CONFIG_QSPI_BOOT
$(QSPI_PARAMS_FILE): arch/arm/mach-s32/s32gen1_qspiparam FORCE
	$(Q)$(objtree)/arch/arm/mach-s32/s32gen1_qspiparam \
		-m $(QSPI_MEMORY) \
		-o $(QSPI_PARAMS_FILE)
else
$(QSPI_PARAMS_FILE):
endif

%.cfgout: $(S32GEN1_CONFIG_FILE) $(QSPI_PARAMS_FILE) FORCE
	$(Q)mkdir -p $(dir $@)
	$(call if_changed_dep,cpp_cfg)
