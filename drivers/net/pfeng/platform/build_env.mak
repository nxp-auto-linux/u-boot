# SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
#
# Copyright 2018-2020 NXP
# 
#
#all makefiles in the environment require "all" as default target
#.DEFAULT_GOAL:= all

# ***********************
# USER configuration
# ***********************
#Build profile, possible values: release, debug, profile, coverage
export BUILD_PROFILE?=release
#Build architecture/variant string, possible values: x86, armv7le, etc...
export PLATFORM?=aarch64le
#Target os, possible values: QNX, LINUX
export TARGET_OS?=QNX
#Target HW, possible values: s32g
export TARGET_HW ?= s32g
#NULL argument checks. Enable when debugging is needed. 1 - enable, 0 - disable.
export PFE_CFG_NULL_ARG_CHECK?=0
#Paranoid IRQ handing. Adds HW resource protection at potentially critical places.
export PFE_CFG_PARANOID_IRQ?=0
#Code for debugging. Adds program parts useful for debugging but reduces performance.
export PFE_CFG_DEBUG?=0
#Multi-instance driver support (includes IHC API). 1 - enable, 0 - disable.
export PFE_CFG_MULTI_INSTANCE_SUPPORT?=0
#Master/Slave variant switch
export PFE_CFG_PFE_MASTER?=1
#HIF NOCPY support
export PFE_CFG_HIF_NOCPY_SUPPORT?=0
#HIF NOCPY direct mode. When disabled then LMEM copy mode is used.
export PFE_CFG_HIF_NOCPY_DIRECT?=0
#Force TX CSUM calculation on all frames (this forcibly overwrite all IP/TCP/UDP checksums in FW)
export PFE_CFG_CSUM_ALL_FRAMES?=0
#HIF sequence number check. 1 - enable, 0 - disable
export PFE_CFG_HIF_SEQNUM_CHECK?=0
#IP version
export PFE_CFG_IP_VERSION?=PFE_CFG_IP_VERSION_NPU_7_14
#Build of rtable feature. 1 - enable, 0 - disable
export PFE_CFG_RTABLE_ENABLE?=1
#Build of l2bridge feature. 1 - enable, 0 - disable
export PFE_CFG_L2BRIDGE_ENABLE?=1
#Build support for FCI. 1 - enable, 0 - disable
export PFE_CFG_FCI_ENABLE?=1
#Build support for thread detecting PFE errors in polling mode. 1 - enable, 0 - disable
export PFE_CFG_GLOB_ERR_POLL_WORKER?=1
#Build support for flexible parser and flexible router
export PFE_CFG_FLEX_PARSER_AND_FILTER?=1
#Enable Interface Database worker thread. 1 - enable, 0 - disable
export PFE_CFG_IF_DB_WORKER?=0
#Use multi-client HIF driver. Required when multiple logical interfaces need to
#send/receive packets using the same HIF channel.
export PFE_CFG_MC_HIF?=0
#Use single-client HIF driver. Beneficial when more HIF channels are available and
#every logical interface can send/receive packets using dedicated HIF channel.
export PFE_CFG_SC_HIF?=1
#Enable or disable HIF traffic routing. When enabled, traffic sent from host via
#HIF will be routed according to HIF physical interface setup. When disabled, the
#traffic will be directly injected to specified list of interfaces.
export PFE_CFG_ROUTE_HIF_TRAFFIC?=0

ifneq ($(PFE_CFG_MC_HIF),0)
  ifneq ($(PFE_CFG_SC_HIF),0)
    $(error Impossible configuration)
  endif
endif

ifneq ($(PFE_CFG_SC_HIF),0)
  ifneq ($(PFE_CFG_MC_HIF),0)
    $(error Impossible configuration)
  endif
endif

#Include HIF TX FIFO fix. This is SW workaround for HIF stall issue.
ifeq ($(PFE_CFG_IP_VERSION),PFE_CFG_IP_VERSION_NPU_7_14)
export PFE_CFG_HIF_TX_FIFO_FIX=1
else
export PFE_CFG_HIF_TX_FIFO_FIX=0
endif
#Set default verbosity level for sysfs. Valid values are from 1 to 10.
export PFE_CFG_VERBOSITY_LEVEL?=4

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(notdir $(patsubst %/,%,$(dir $(mkfile_path))))

# ***********************
# Global stuff
# ***********************
ifneq ($(PFE_CFG_VERBOSITY_LEVEL),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_VERBOSITY_LEVEL=$(PFE_CFG_VERBOSITY_LEVEL)
endif

ifneq ($(PFE_CFG_NULL_ARG_CHECK),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_NULL_ARG_CHECK
endif

ifneq ($(PFE_CFG_PARANOID_IRQ),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_PARANOID_IRQ
endif

ifneq ($(PFE_CFG_DEBUG),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_DEBUG
endif

ifneq ($(PFE_CFG_MULTI_INSTANCE_SUPPORT),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_MULTI_INSTANCE_SUPPORT
endif

ifneq ($(PFE_CFG_PFE_MASTER),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_PFE_MASTER
else
    GLOBAL_CCFLAGS+=-DPFE_CFG_PFE_SLAVE
endif

ifneq ($(PFE_CFG_HIF_NOCPY_SUPPORT),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_HIF_NOCPY_SUPPORT
endif

ifneq ($(PFE_CFG_HIF_NOCPY_DIRECT),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_HIF_NOCPY_DIRECT
endif

ifneq ($(PFE_CFG_CSUM_ALL_FRAMES),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_CSUM_ALL_FRAMES
endif

ifneq ($(PFE_CFG_HIF_SEQNUM_CHECK),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_HIF_SEQNUM_CHECK
endif

ifneq ($(PFE_CFG_IP_VERSION),)
GLOBAL_CCFLAGS+=-DPFE_CFG_IP_VERSION=$(PFE_CFG_IP_VERSION)
else
$(error IP version must be set)
endif

ifneq ($(PFE_CFG_RTABLE_ENABLE),0)
    GLOBAL_CCFLAGS+= -DPFE_CFG_RTABLE_ENABLE
endif

ifneq ($(PFE_CFG_L2BRIDGE_ENABLE),0)
    GLOBAL_CCFLAGS+= -DPFE_CFG_L2BRIDGE_ENABLE
endif

ifneq ($(PFE_CFG_FCI_ENABLE),0)
    GLOBAL_CCFLAGS+= -DPFE_CFG_FCI_ENABLE
endif

ifneq ($(PFE_CFG_GLOB_ERR_POLL_WORKER),0)
    GLOBAL_CCFLAGS+= -DPFE_CFG_GLOB_ERR_POLL_WORKER
endif

ifneq ($(PFE_CFG_FLEX_PARSER_AND_FILTER),0)
    GLOBAL_CCFLAGS+= -DPFE_CFG_FLEX_PARSER_AND_FILTER
endif

ifneq ($(PFE_CFG_IF_DB_WORKER),0)
    GLOBAL_CCFLAGS+= -DPFE_CFG_IF_DB_WORKER
endif

ifneq ($(PFE_CFG_MC_HIF),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_MC_HIF
endif

ifneq ($(PFE_CFG_SC_HIF),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_SC_HIF
endif

ifneq ($(PFE_CFG_ROUTE_HIF_TRAFFIC),0)
    GLOBAL_CCFLAGS+=-DPFE_CFG_ROUTE_HIF_TRAFFIC
endif

ifneq ($(PFE_CFG_HIF_TX_FIFO_FIX),0)
    GLOBAL_CCFLAGS+= -DPFE_CFG_HIF_TX_FIFO_FIX
endif

# This variable will be propagated to every Makefile in the project
export GLOBAL_CCFLAGS;

# ***********************
# UBOOT environment
# ***********************

ifeq ($(TARGET_OS),UBOOT)

    export CONFIG_NAME?=$(PLATFORM)-$(BUILD_PROFILE)
    export PFE_CFG_TARGET_ARCH_DEF=PFE_CFG_TARGET_ARCH_$(shell echo $(PLATFORM) | cut -d '-' -f 1-1)
    export PFE_CFG_TARGET_OS_DEF=PFE_CFG_TARGET_OS_$(TARGET_OS)
    #export TARGET_ENDIAN_DEF=ENDIAN_LITTLE
    export PFE_CFG_BUILD_PROFILE_DEF=PFE_CFG_BUILD_PROFILE_$(shell echo $(BUILD_PROFILE) | tr '[a-z]' '[A-Z]')

    export INC_PREFIX=drivers/net/pfeng/platform/

endif # TARGET_OS
