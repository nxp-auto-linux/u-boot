# SPDX-License-Identifier:GPL-2.0
#
# Copyright 2019-2021 NXP
#

# S32G PFEng driver
obj-y += pfeng_cmd.o pfeng_drv.o pfeng_hw.o

TARGET_OS = UBOOT
PLATFORM = aarch64-none-elf-
ARCH = arm
BUILD_PROFILE ?= RELEASE

SELF_DIR := $(dir $(lastword $(MAKEFILE_LIST)))

#User defined include/preprocessor flags and libraries
INCLUDES += -I$(INC_PREFIX)pfe_platform/public

CCFLAGS_pfe += -D$(PFE_CFG_TARGET_ARCH_DEF) -D$(PFE_CFG_TARGET_OS_DEF) -D$(PFE_CFG_BUILD_PROFILE_DEF)

# Hash of compatible FW binary interface (0.9.3)
ccflags-y += -DPFE_CFG_PFE_CT_H_MD5=4e3e2e8a5e0e8e215c15a424048782d9
# Platform
ccflags-y += -DPFE_CFG_TARGET_ARCH_aarch64
# IP version
ccflags-y += -DPFE_CFG_IP_VERSION=PFE_CFG_IP_VERSION_NPU_7_14

ccflags-y += -Idrivers/net/pfeng/regmap
ccflags-y += -Idrivers/net/pfeng/hw_abi
ccflags-y += -Wno-date-time
ccflags-y += -I./drivers/net/s32
