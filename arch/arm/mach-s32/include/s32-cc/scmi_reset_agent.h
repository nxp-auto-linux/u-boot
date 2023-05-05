/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Copyright 2020,2022-2023 NXP
 */
#ifndef SCMI_RESET_AGENT_H
#define SCMI_RESET_AGENT_H

#define S32_SCMI_AGENT_OSPM       1

/*
 * Reset SMCI resource settings that were previously configured by
 * the U-Boot agent
 */
int scmi_reset_agent(void);

#endif
