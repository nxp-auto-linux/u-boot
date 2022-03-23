#
# Copyright 2022 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += scmi_reset_agent.o
obj-y += soc.o

ifdef CONFIG_SPI_FLASH_MACRONIX
QSPI_MEMORY = MX25UW51245G
endif

ifdef CONFIG_SPI_FLASH_STMICRO
QSPI_MEMORY = MT35XU512ABA
endif

QSPI_PARAMS_FILE ?= qspi_params_$(QSPI_MEMORY).bin

s32cc_qspiparams-objs := tools/micron_params.o \
	tools/macronix_params.o \
	tools/qspiparams.o
HOSTCFLAGS_tools/macronix_params.o += -I$(srctree)/tools/
HOSTCFLAGS_tools/micron_params.o += -I$(srctree)/tools/
HOSTCFLAGS_tools/qspiparams.o += -I$(srctree)/tools/
hostprogs += s32cc_qspiparams
always-y := $(hostprogs)

quiet_cmd_cpp_cfg = CFGS    $@
      cmd_cpp_cfg = $(CPP) $(cpp_flags) -x c -o $@ $<

S32CC_CONFIG_FILE = $(CONFIG_S32CC_CONFIG_FILE:"%"=%)

ifdef CONFIG_QSPI_BOOT
$(QSPI_PARAMS_FILE): arch/arm/mach-s32/s32-cc/s32cc_qspiparams FORCE
	$(Q)$(objtree)/arch/arm/mach-s32/s32-cc/s32cc_qspiparams \
		-m $(QSPI_MEMORY) \
		-o $(QSPI_PARAMS_FILE)
else
$(QSPI_PARAMS_FILE):
endif

%.cfgout: $(S32CC_CONFIG_FILE) $(QSPI_PARAMS_FILE) FORCE
	$(Q)mkdir -p $(dir $@)
	$(call if_changed_dep,cpp_cfg)
