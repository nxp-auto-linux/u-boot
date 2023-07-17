/* SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause */
/*
 * Copyright 2023 NXP
 */

#ifndef __S32CC_SOC_H__
#define __S32CC_SOC_H__

#define SOC_MACHINE_S32G233A	"233A"
#define SOC_MACHINE_S32G254A	"254A"
#define SOC_MACHINE_S32G274A	"274A"
#define SOC_MACHINE_S32G358A	"358A"
#define SOC_MACHINE_S32G359A	"359A"
#define SOC_MACHINE_S32G378A	"378A"
#define SOC_MACHINE_S32G379A	"379A"
#define SOC_MACHINE_S32G398A	"398A"
#define SOC_MACHINE_S32G399A	"399A"
#define SOC_MACHINE_S32R455A	"455A"
#define SOC_MACHINE_S32R458A	"458A"

struct soc_s32cc_plat {
	bool lockstep_enabled;
};

#endif
