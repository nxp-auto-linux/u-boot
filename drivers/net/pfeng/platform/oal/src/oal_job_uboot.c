// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2019-2020 NXP
 */

/**
 * @addtogroup  dxgr_OAL_JOB
 * @{
 * 
 * @file        oal_job_uboot.c
 * @brief       The oal_job module source file (UBOOT).
 * @details     This file contains empty implementation of oal_job API for UBOOT.
 *
 */

#include "oal.h"
#include "oal_mm.h"
#include "oal_job.h"

typedef uint32_t __oal_job_tag;

static __oal_job_tag singl;

errno_t oal_job_run(oal_job_t *job)
{
    return 0;
}

errno_t oal_job_drain(oal_job_t *job)
{
    return 0;
}

oal_job_t *oal_job_create(oal_job_func func, void *arg, const char_t *name, oal_prio_t prio)
{
    return (oal_job_t *)&singl;
}

errno_t oal_job_destroy(oal_job_t *job)
{
    return 0;
}

/** @}*/
