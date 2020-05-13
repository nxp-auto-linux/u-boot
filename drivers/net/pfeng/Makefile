# SPDX-License-Identifier:GPL-2.0+
#
# Copyright 2019 NXP
#

# S32G PFEng driver
obj-y += pfeng_cmd.o pfeng_drv.o
obj-y += platform/

TARGET_OS = UBOOT
PLATFORM = aarch64-none-elf-
ARCH = arm
BUILD_PROFILE ?= RELEASE

export PFE_CFG_IP_VERSION ?= PFE_CFG_IP_VERSION_NPU_7_14
export PFE_CFG_MULTI_INSTANCE_SUPPORT = 0
export PFE_CFG_FCI_ENABLE = 0
export PFE_CFG_L2BRIDGE_ENABLE = 0
export PFE_CFG_RTABLE_ENABLE = 0
export PFE_CFG_GLOB_ERR_POLL_WORKER = 0
export PFE_CFG_FLEX_PARSER_AND_FILTER = 0

export PFE_CFG_MC_HIF = 1
export PFE_CFG_SC_HIF = 0

export KERNELRELEASE = NONE

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

include $(SELF_DIR)/platform/build_env.mak

#User defined include/preprocessor flags and libraries
INCLUDES += -I$(INC_PREFIX)pfe_platform/public \
			-I$(INC_PREFIX)pfe_hif_drv/public \
			-I$(INC_PREFIX)common/public \
			-I$(INC_PREFIX)elfman/public \
			-I$(INC_PREFIX)hal/public \
			-I$(INC_PREFIX)oal/public

CCFLAGS_pfe += -D$(PFE_CFG_TARGET_ARCH_DEF) -D$(PFE_CFG_TARGET_OS_DEF) -D$(PFE_CFG_BUILD_PROFILE_DEF)

ccflags-y += $(INCLUDES)
ccflags-y += $(CCFLAGS_pfe)
ccflags-y += $(GLOBAL_CCFLAGS)
ccflags-y += -Wno-date-time
