// SPDX-License-Identifier: GPL 2.0 OR BSD-3-Clause
/*
 *  Copyright 2018-2020 NXP
 */

/**
 * @addtogroup	dxgrPFE_PLATFORM
 * @{
 *
 * @file		pfe_platform.c
 * @brief		The PFE platform management (S32G master)
 * @details		This file contains HW-specific code. It is used to create structured SW representation
 *				of a given PFE HW implementation. File is intended to be created by user
 *				each time a new HW setup with different PFE configuration needs to be
 *				supported.
 * @note		Various variants of this file should exist for various HW implementations (please
 *				keep this file clean, not containing platform-specific preprocessor switches).
 *
 */

#include "pfe_cfg.h"
#include "elfman_cfg.h"
#include "elfman.h"

#include "oal.h"
#include "hal.h"

#include "pfe_cbus.h"
#include "pfe_hif.h"
#include "pfe_platform_cfg.h"
#include "pfe_platform.h"
#include "pfe_ct.h"
#include "pfe_idex.h"

#ifdef PFE_CFG_MULTI_INSTANCE_SUPPORT
#include "pfe_platform_rpc.h" /* RPC codes and arguments */
#endif			      /* PFE_CFG_MULTI_INSTANCE_SUPPORT */
#ifdef PFE_CFG_FLEX_PARSER_AND_FILTER
#include "pfe_fp.h"
#include "pfe_flexible_filter.h"
#endif /* PFE_CFG_FLEX_PARSER_AND_FILTER */
#ifdef PFE_CFG_FCI_ENABLE
#include "fci.h"
#endif /* PFE_CFG_FCI_ENABLE */

/**
 * This is a platform specific file. All routines shall be implemented
 * according to application-given HW setup.
 */

/*
 * @brief The main PFE structure
 */
static pfe_platform_t pfe = { .probed = FALSE };

#ifdef PFE_CFG_MULTI_INSTANCE_SUPPORT
static errno_t pfe_platform_unregister_log_if(pfe_platform_t *platform,
					      u32 session_id,
					      pfe_log_if_t *log_if);
#endif /* PFE_CFG_MULTI_INSTANCE_SUPPORT */

/**
 * @brief		BMU interrupt service routine
 * @details		Manage BMU interrupt
 * @details		See the oal_irq_handler_t
 */
static bool_t
pfe_platform_bmu_isr(void *arg)
{
	pfe_platform_t *platform = (pfe_platform_t *)arg;
	bool_t handled = FALSE;

	/*	BMU interrupt line is shared by 2 BMU instances. Additionally, the BMU keeps the interrupt
		line permanently asserted (watermarks and so on = bad design). Therefore disable all sources
		here before particular ISRs will be called to dispatch the interrupts. */
	if (platform->bmu[0]) {
		pfe_bmu_irq_mask(platform->bmu[0]);
	}

	if (platform->bmu[1]) {
		pfe_bmu_irq_mask(platform->bmu[1]);
	}

	/*	Run ISRs */
	if (pfe_bmu_isr(platform->bmu[0]) == EOK) {
		handled |= TRUE;
	}

	if (pfe_bmu_isr(platform->bmu[1]) == EOK) {
		handled |= TRUE;
	}

	/*	Re-enable interrupts from both BMU instances */
	if (platform->bmu[0]) {
		pfe_bmu_irq_unmask(platform->bmu[0]);
	}

	if (platform->bmu[1]) {
		pfe_bmu_irq_unmask(platform->bmu[1]);
	}

	return handled;
}

#if defined(PFE_CFG_GLOB_ERR_POLL_WORKER)
/**
 * @brief		Global polling service routine
 * @details		Runs various periodic tasks
 */
static void *
pfe_poller_func(void *arg)
{
	pfe_platform_t *platform = (pfe_platform_t *)arg;

	if (!platform) {
		NXP_LOG_WARNING("Global poller init failed\n");
		return NULL;
	}

	/*	Periodically call error detecting routines */
	while (TRUE) {
		switch (platform->poller_state) {
		case POLLER_STATE_DISABLED: {
			/*  Do nothing */
			break;
		}

		case POLLER_STATE_ENABLED: {
			/*  Process HIF global isr */
			if (platform->hif) {
				pfe_hif_irq_mask(platform->hif);
				(void)pfe_hif_isr(platform->hif);
				pfe_hif_irq_unmask(platform->hif);
			}

			/*	Classifier */
			if (platform->classifier) {
				pfe_class_irq_mask(platform->classifier);
				(void)pfe_class_isr(platform->classifier);
				pfe_class_irq_unmask(platform->classifier);
			}

			/*	Safety */
			if (platform->safety) {
				pfe_safety_irq_mask(platform->safety);
				/*	TODO: Note that safety IRQ is not connected to host but to FCCU. There
						will be conflict in case when FCCU will try to concurrently handle the
						interrupt. */
				(void)pfe_safety_isr(platform->safety);
				pfe_safety_irq_unmask(platform->safety);
			}

#if (PFE_CFG_IP_VERSION != PFE_CFG_IP_VERSION_FPGA_5_0_4)
			/*	Watchdogs */
			if (platform->wdt) {
				pfe_wdt_irq_mask(platform->wdt);
				/*	TODO: Note that watchdog IRQ is not connected to host but to FCCU. There
						will be conflict in case when FCCU will try to concurrently handle the
						interrupt. */
				(void)pfe_wdt_isr(platform->wdt);
				pfe_wdt_irq_unmask(platform->wdt);
			}
#endif /* PFE_CFG_IP_VERSION */

			break;
		}

		case POLLER_STATE_STOPPED: {
			/*  Stop the loop and exit */
			NXP_LOG_WARNING("Global poller finished\n");
			return NULL;
		}

		default: {
			NXP_LOG_ERROR("Unexpected poller state\n");
			return NULL;
		}
		}

		/*  Wait for 1 sec and loop again */
		oal_time_mdelay(1000);
	}

	return NULL;
}
#endif /* PFE_CFG_GLOB_ERR_POLL_WORKER */

/**
 * @brief		Global interrupt service routine
 * @details		This must be here on platforms (FPGA...) where all PFE interrupts
 * 				are combined to a single physical interrupt line :(
 * 				Because we want to catch interrupts during platform initialization some
 * 				of platform modules might have not been initialized yet. Therefore the NULL
 * 				checks...
 * @details		See the oal_irq_handler_t
 */
static bool_t
pfe_platform_global_isr(void *arg)
{
	pfe_platform_t *platform = (pfe_platform_t *)arg;
	bool_t handled = FALSE;
	uint32_t ii;
	static pfe_hif_chnl_id_t ids[] = { HIF_CHNL_0, HIF_CHNL_1, HIF_CHNL_2,
					   HIF_CHNL_3 };
	pfe_hif_chnl_t *chnls[sizeof(ids) / sizeof(pfe_hif_chnl_id_t)] = {
		NULL
	};

	/*	Disable all participating IRQ sources */
	if (platform->hif) {
		pfe_hif_irq_mask(platform->hif);
	}

	if (platform->bmu[0]) {
		pfe_bmu_irq_mask(platform->bmu[0]);
	}

	if (platform->bmu[1]) {
		pfe_bmu_irq_mask(platform->bmu[1]);
	}

	if (platform->hif) {
		for (ii = 0U; ii < (sizeof(ids) / sizeof(pfe_hif_chnl_id_t));
		     ii++) {
			chnls[ii] = pfe_hif_get_channel(platform->hif, ids[ii]);
			if (chnls[ii]) {
				pfe_hif_chnl_irq_mask(chnls[ii]);
			}
		}
	}

	if (platform->safety) {
		pfe_safety_irq_mask(platform->safety);
	}

#if (PFE_CFG_IP_VERSION != PFE_CFG_IP_VERSION_FPGA_5_0_4)
	if (platform->wdt) {
		pfe_wdt_irq_mask(platform->wdt);
	}
#endif /* PFE_CFG_IP_VERSION */

#if defined(PFE_CFG_HIF_NOCPY_SUPPORT)
	if (platform->hif_nocpy) {
		pfe_hif_chnl_irq_mask(pfe_hif_nocpy_get_channel(
			platform->hif_nocpy, PFE_HIF_CHNL_NOCPY_ID));
	}
#endif /* PFE_CFG_HIF_NOCPY_SUPPORT */

	/*	Call modules ISRs */
	if (platform->hif) {
		if (pfe_hif_isr(platform->hif) == EOK) {
			handled = TRUE;
		}
	}

	if (platform->bmu[0]) {
		if (pfe_bmu_isr(platform->bmu[0]) == EOK) {
			handled = TRUE;
		}
	}

	if (platform->bmu[1]) {
		if (pfe_bmu_isr(platform->bmu[1]) == EOK) {
			handled = TRUE;
		}
	}

	for (ii = 0U; ii < (sizeof(ids) / sizeof(pfe_hif_chnl_id_t)); ii++) {
		if (chnls[ii]) {
			if (pfe_hif_chnl_isr(chnls[ii]) == EOK) {
				handled = TRUE;
			}
		}
	}
	if (platform->safety) {
		if (pfe_safety_isr(platform->safety) == EOK) {
			handled = TRUE;
		}
	}

#if (PFE_CFG_IP_VERSION != PFE_CFG_IP_VERSION_FPGA_5_0_4)
	if (platform->wdt) {
		if (pfe_wdt_isr(platform->wdt) == EOK) {
			handled = TRUE;
		}
	}
#endif /* PFE_CFG_IP_VERSION */

#if defined(PFE_CFG_HIF_NOCPY_SUPPORT)
	if (platform->hif_nocpy) {
		if (EOK ==
		    pfe_hif_chnl_isr(pfe_hif_nocpy_get_channel(
			    platform->hif_nocpy, PFE_HIF_CHNL_NOCPY_ID))) {
			handled = TRUE;
		}
	}
#endif /* PFE_CFG_HIF_NOCPY_SUPPORT */

	/*	Enable all participating IRQ sources */
	if (platform->hif) {
		pfe_hif_irq_unmask(platform->hif);
	}

	if (platform->bmu[0]) {
		pfe_bmu_irq_unmask(platform->bmu[0]);
	}

	if (platform->bmu[1]) {
		pfe_bmu_irq_unmask(platform->bmu[1]);
	}

	for (ii = 0U; ii < (sizeof(ids) / sizeof(pfe_hif_chnl_id_t)); ii++) {
		if (chnls[ii]) {
			pfe_hif_chnl_irq_unmask(chnls[ii]);
		}
	}

	if (platform->safety) {
		pfe_safety_irq_unmask(platform->safety);
	}

#if (PFE_CFG_IP_VERSION != PFE_CFG_IP_VERSION_FPGA_5_0_4)
	if (platform->wdt) {
		pfe_wdt_irq_unmask(platform->wdt);
	}
#endif /* PFE_CFG_IP_VERSION */

#if defined(PFE_CFG_HIF_NOCPY_SUPPORT)
	if (platform->hif_nocpy) {
		pfe_hif_chnl_irq_unmask(pfe_hif_nocpy_get_channel(
			platform->hif_nocpy, PFE_HIF_CHNL_NOCPY_ID));
	}
#endif /* PFE_CFG_HIF_NOCPY_SUPPORT */

	return handled;
}

#ifdef PFE_CFG_MULTI_INSTANCE_SUPPORT
/**
 * @brief		IDEX RPC callback
 * @details		All requests from slave drivers are coming and being processed
 *				within this callback. Any request policing should be implemented
 *				here.
 * @warning		Don't block or sleep within the body
 * @param[in]	sender RPC originator identifier. The physical interface ID
 * 					   where the request is coming from.
 * @param[in]	id Request identifier
 * @param[in]	buf Pointer to request argument. Can be NULL.
 * @param[in]	buf_len Length of request argument. Can be zero.
 * @param[in]	arg Custom argument provided via pfe_idex_set_rpc_cbk()
 * @note		This callback runs in dedicated context/thread.
 */
static void
idex_rpc_cbk(pfe_ct_phy_if_id_t sender, uint32_t id, void *buf,
	     u16 buf_len, void *arg)
{
	pfe_platform_t *platform = (pfe_platform_t *)arg;
	pfe_phy_if_t *phy_if_arg = NULL;
	pfe_log_if_t *log_if_arg = NULL;
	pfe_if_db_entry_t *entry = NULL;
	errno_t ret = EOK;

	(void)buf_len;
#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!platform)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	/* Check if phy_if should be extracted from argument */
	if (id == PFE_PLATFORM_RPC_PFE_LOG_IF_CREATE ||
	    (id >= PFE_PLATFORM_RPC_PFE_PHY_IF_ENABLE &&
	     id <= PFE_PLATFORM_RPC_PFE_PHY_IF_IS_PROMISC)) {
		ret = pfe_if_db_get_first(
			platform->phy_if_db, sender, IF_DB_CRIT_BY_ID,
			(void *)(addr_t)(
				(pfe_platform_rpc_pfe_phy_if_generic_t *)buf)
				->phy_if_id,
			&entry);
		if ((ret == EOK) && (entry)) {
			phy_if_arg = pfe_if_db_entry_get_phy_if(entry);
		} else {
			/* Entry doesn't exist */
			ret = ENOENT;
		}
	}

	/* Check if log_if should be extracted from argument */
	if (id >= PFE_PLATFORM_RPC_PFE_LOG_IF_DESTROY &&
	    id <= PFE_PLATFORM_RPC_PFE_LOG_IF_SET_MATCH_AND) {
		ret = pfe_if_db_get_first(
			platform->log_if_db, sender, IF_DB_CRIT_BY_ID,
			(void *)(addr_t)(
				(pfe_platform_rpc_pfe_log_if_generic_t *)buf)
				->log_if_id,
			&entry);
		if ((ret == EOK) && (entry)) {
			log_if_arg = pfe_if_db_entry_get_log_if(entry);
		} else {
			/* Entry doesn't exist */
			ret = ENOENT;
		}
	}

	switch (id) {
	case PFE_PLATFORM_RPC_PFE_IF_LOCK: {
		ret = pfe_if_db_lock_owned((uint32_t)sender);

		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}
		/* start timeout*/
		break;
	}
	case PFE_PLATFORM_RPC_PFE_IF_UNLOCK: {
		ret = pfe_if_db_unlock((uint32_t)sender);

		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}
		break;
	}
	case PFE_PLATFORM_RPC_PFE_LOG_IF_CREATE: {
		pfe_platform_rpc_pfe_log_if_create_ret_t rpc_ret = { 0 };
		pfe_log_if_t *log_if = NULL;
		static char_t namebuf[16];

		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_CREATE\n");

		if (ret == EOK) {
			/*	Generate some name to easily identify non-local interfaces */
			(void)oal_util_snprintf(namebuf, sizeof(namebuf),
						"[cl%d]", sender);
			log_if = pfe_log_if_create(phy_if_arg, namebuf);
			if (!log_if) {
				NXP_LOG_ERROR(
					"Could not create logical interface\n");
				ret = ENODEV;
			} else {
				rpc_ret.log_if_id = pfe_log_if_get_id(log_if);
				ret = pfe_if_db_add(platform->log_if_db, sender,
						    log_if, sender);
				if (ret != EOK) {
					NXP_LOG_DEBUG(
						"Unable to register logical interface: %d\n",
						ret);
					pfe_log_if_destroy(log_if);
					log_if = NULL;
				}
			}
		}

		/*	Report execution status to caller */
		if (EOK !=
		    pfe_idex_set_rpc_ret_val(ret, &rpc_ret, sizeof(rpc_ret))) {
			NXP_LOG_ERROR("Could not send RPC response\n");
			if (log_if) {
				ret = pfe_if_db_get_first(
					platform->log_if_db, sender,
					IF_DB_CRIT_BY_INSTANCE, (void *)log_if,
					&entry);
				if (!entry) {
					ret = ENOENT;
				} else if (ret == EOK) {
					ret = pfe_if_db_remove(
						platform->log_if_db, sender,
						entry);
				}

				if (ret != EOK) {
					/*	This failure is normal in case the logical interface has not been registered */
					NXP_LOG_DEBUG(
						"Can't unregister %s: %d\n",
						pfe_log_if_get_name(log_if),
						ret);
				} else {
					pfe_log_if_destroy(log_if);
					log_if = NULL;
				}
			}
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_DESTROY: {
		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_DESTROY\n");

		if (ret == EOK) {
			ret = pfe_if_db_get_first(platform->log_if_db, sender,
						  IF_DB_CRIT_BY_INSTANCE,
						  (void *)log_if_arg, &entry);
			if (!entry) {
				ret = ENOENT;
			} else if (ret == EOK) {
				ret = pfe_if_db_remove(platform->log_if_db,
						       sender, entry);
			}

			if (ret != EOK) {
				NXP_LOG_DEBUG(
					"Unable to unregister %s with ID: %d\n",
					pfe_log_if_get_name(log_if_arg),
					pfe_log_if_get_id(log_if_arg));
			} else {
				pfe_log_if_destroy(log_if_arg);
				log_if_arg = NULL;
			}
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_SET_MATCH_RULES: {
		pfe_platform_rpc_pfe_log_if_set_match_rules_arg_t *arg =
			(pfe_platform_rpc_pfe_log_if_set_match_rules_arg_t *)buf;

		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_SET_MATCH_RULES\n");

		if (ret == EOK) {
			ret = pfe_log_if_set_match_rules(
				log_if_arg, oal_ntohl(arg->rules), &arg->args);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_GET_MATCH_RULES: {
		pfe_platform_rpc_pfe_log_if_get_match_rules_ret_t rpc_ret = {
			0
		};
		pfe_ct_if_m_rules_t rules;

		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_GET_MATCH_RULES\n");

		if (ret == EOK) {
			ret = pfe_log_if_get_match_rules(log_if_arg, &rules,
							 &rpc_ret.args);
			rpc_ret.rules = oal_htonl(rules);
		}

		/*	Report execution status to caller */
		if (EOK !=
		    pfe_idex_set_rpc_ret_val(ret, &rpc_ret, sizeof(rpc_ret))) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_ADD_MATCH_RULE: {
		pfe_platform_rpc_pfe_log_if_add_match_rule_arg_t *arg =
			(pfe_platform_rpc_pfe_log_if_add_match_rule_arg_t *)buf;

		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_ADD_MATCH_RULE\n");

		if (ret == EOK) {
			ret = pfe_log_if_add_match_rule(
				log_if_arg, oal_ntohl(arg->rule), arg->arg,
				oal_ntohl(arg->arg_len));
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_DEL_MATCH_RULE: {
		pfe_platform_rpc_pfe_log_if_del_match_rule_arg_t *arg =
			(pfe_platform_rpc_pfe_log_if_del_match_rule_arg_t *)buf;

		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_DEL_MATCH_RULE\n");

		if (ret == EOK) {
			ret = pfe_log_if_del_match_rule(log_if_arg,
							oal_ntohl(arg->rule));
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_SET_MAC_ADDR: {
		pfe_platform_rpc_pfe_log_if_set_mac_addr_arg_t *arg =
			(pfe_platform_rpc_pfe_log_if_set_mac_addr_arg_t *)buf;

		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_SET_MAC_ADDR\n");

		if (ret == EOK) {
			ret = pfe_log_if_set_mac_addr(log_if_arg, arg->addr);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_GET_MAC_ADDR: {
		pfe_platform_rpc_pfe_log_if_get_mac_addr_arg_t *arg =
			(pfe_platform_rpc_pfe_log_if_get_mac_addr_arg_t *)buf;
		pfe_platform_rpc_pfe_log_if_get_mac_addr_ret_t rpc_ret = { 0 };

		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_GET_MAC_ADDR\n");

		if (ret == EOK) {
			rpc_ret.log_if_id = arg->log_if_id;
			ret = pfe_log_if_get_mac_addr(log_if_arg, rpc_ret.addr);
		}

		/*	Report execution status to caller */
		if (EOK !=
		    pfe_idex_set_rpc_ret_val(ret, &rpc_ret, sizeof(rpc_ret))) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_CLEAR_MAC_ADDR: {
		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_CLEAR_MAC_ADDR\n");

		if (ret == EOK) {
			ret = pfe_log_if_clear_mac_addr(log_if_arg);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_ENABLE: {
		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_ENABLE\n");

		if (ret == EOK) {
			ret = pfe_log_if_enable(log_if_arg);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_DISABLE: {
		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_DISABLE\n");

		if (ret == EOK) {
			ret = pfe_log_if_disable(log_if_arg);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_IS_ENABLED: {
		pfe_platform_rpc_pfe_log_if_is_enabled_ret_t rpc_ret = { 0 };

		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_IS_ENABLED\n");

		if (ret == EOK) {
			rpc_ret.status = pfe_log_if_is_enabled(log_if_arg);
		}

		/*	Report execution status to caller */
		if (EOK !=
		    pfe_idex_set_rpc_ret_val(ret, &rpc_ret, sizeof(rpc_ret))) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_PROMISC_ENABLE: {
		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_PROMISC_ENABLE\n");

		if (ret == EOK) {
			ret = pfe_log_if_promisc_enable(log_if_arg);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_PROMISC_DISABLE: {
		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_PROMISC_DISABLE\n");

		if (ret == EOK) {
			ret = pfe_log_if_promisc_disable(log_if_arg);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_IS_PROMISC: {
		pfe_platform_rpc_pfe_log_if_is_promisc_ret_t rpc_ret = { 0 };

		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_IS_PROMISC\n");

		if (ret == EOK) {
			rpc_ret.status = pfe_log_if_is_promisc(log_if_arg);
		}

		/*	Report execution status to caller */
		if (EOK !=
		    pfe_idex_set_rpc_ret_val(ret, &rpc_ret, sizeof(rpc_ret))) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_ADD_EGRESS_IF: {
		pfe_platform_rpc_pfe_log_if_add_egress_if_arg_t *arg =
			(pfe_platform_rpc_pfe_log_if_add_egress_if_arg_t *)buf;

		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_ADD_EGRESS_IF\n");

		if (ret == EOK) {
			ret = pfe_if_db_get_first(
				platform->phy_if_db, sender, IF_DB_CRIT_BY_ID,
				(void *)(addr_t)arg->phy_if_id, &entry);
			phy_if_arg = pfe_if_db_entry_get_phy_if(entry);

			if ((!phy_if_arg) || (ret != EOK)) {
				ret = ENOENT;
			} else {
				ret = pfe_log_if_add_egress_if(log_if_arg,
							       phy_if_arg);
			}
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_GET_EGRESS: {
		pfe_platform_rpc_pfe_log_if_get_egress_ret_t rpc_ret = { 0 };
		u32 egress = 0U;

		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_GET_EGRESS\n");

		if (ret == EOK) {
			ret = pfe_log_if_get_egress_ifs(log_if_arg, &egress);
			rpc_ret.egress = egress;
		}

		/*	Report execution status to caller */
		if (EOK !=
		    pfe_idex_set_rpc_ret_val(ret, &rpc_ret, sizeof(rpc_ret))) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_IS_MATCH_OR: {
		pfe_platform_rpc_pfe_log_if_is_match_or_ret_t rpc_ret = { 0 };
		bool_t status = FALSE;

		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_IS_MATCH_OR\n");

		if (ret == EOK) {
			status = pfe_log_if_is_match_or(log_if_arg);
			rpc_ret.status = status;
		}

		/*	Report execution status to caller */
		if (EOK !=
		    pfe_idex_set_rpc_ret_val(ret, &rpc_ret, sizeof(rpc_ret))) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_SET_MATCH_OR: {
		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_SET_MATCH_OR\n");

		if (ret == EOK) {
			ret = pfe_log_if_set_match_or(log_if_arg);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_LOG_IF_SET_MATCH_AND: {
		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_LOG_IF_SET_MATCH_AND\n");

		if (ret == EOK) {
			ret = pfe_log_if_set_match_and(log_if_arg);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_PHY_IF_CREATE: {
		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_PHY_IF_CREATE\n");

		/*	Right now the clients are only allowed to work with interfaces created by the master driver */
		if (ret == EOK) {
			ret = EOK;
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_PHY_IF_ENABLE: {
		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_PHY_IF_ENABLE\n");

		if (ret == EOK) {
			ret = pfe_phy_if_enable(phy_if_arg);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_PHY_IF_DISABLE: {
		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_PHY_IF_DISABLE\n");

		if (ret == EOK) {
			ret = pfe_phy_if_disable(phy_if_arg);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_PHY_IF_PROMISC_ENABLE: {
		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_PHY_IF_PROMISC_ENABLE\n");

		if (ret == EOK) {
			ret = pfe_phy_if_promisc_enable(phy_if_arg);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_PHY_IF_PROMISC_DISABLE: {
		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_PHY_IF_PROMISC_DISABLE\n");

		if (ret == EOK) {
			ret = pfe_phy_if_promisc_disable(phy_if_arg);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_PHY_IF_ADD_MAC_ADDR: {
		pfe_platform_rpc_pfe_phy_if_add_mac_addr_arg_t *rpc_arg =
			(pfe_platform_rpc_pfe_phy_if_add_mac_addr_arg_t *)buf;
		pfe_mac_addr_t mac_addr;

		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_PHY_IF_ADD_MAC_ADDR\n");

		if (ret == EOK) {
			_ct_assert(sizeof(pfe_mac_addr_t) ==
				   sizeof(rpc_arg->mac_addr));
			memcpy(&mac_addr, rpc_arg->mac_addr,
			       sizeof(pfe_mac_addr_t));
			ret = pfe_phy_if_add_mac_addr(phy_if_arg, mac_addr);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_PHY_IF_DEL_MAC_ADDR: {
		pfe_platform_rpc_pfe_phy_if_del_mac_addr_arg_t *rpc_arg =
			(pfe_platform_rpc_pfe_phy_if_del_mac_addr_arg_t *)buf;
		pfe_mac_addr_t mac_addr;

		NXP_LOG_DEBUG(
			"RPC: PFE_PLATFORM_RPC_PFE_PHY_IF_DEL_MAC_ADDR\n");

		if (ret == EOK) {
			_ct_assert(sizeof(pfe_mac_addr_t) ==
				   sizeof(rpc_arg->mac_addr));
			memcpy(&mac_addr, rpc_arg->mac_addr,
			       sizeof(pfe_mac_addr_t));
			ret = pfe_phy_if_del_mac_addr(phy_if_arg, mac_addr);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_PHY_IF_SET_OP_MODE: {
		pfe_platform_rpc_pfe_phy_if_set_op_mode_arg_t *rpc_arg =
			(pfe_platform_rpc_pfe_phy_if_set_op_mode_arg_t *)buf;

		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_PHY_IF_SET_OP_MODE\n");

		if (ret == EOK) {
			ret = pfe_phy_if_set_op_mode(phy_if_arg,
						     rpc_arg->op_mode);
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_PHY_IF_HAS_LOG_IF: {
		pfe_platform_rpc_pfe_phy_if_has_log_if_arg_t *rpc_arg =
			(pfe_platform_rpc_pfe_phy_if_has_log_if_arg_t *)buf;

		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_PHY_IF_HAS_LOG_IF\n");

		ret = pfe_if_db_get_first(platform->log_if_db, sender,
					  IF_DB_CRIT_BY_ID,
					  (void *)(addr_t)rpc_arg->log_if_id,
					  &entry);
		log_if_arg = pfe_if_db_entry_get_log_if(entry);

		/* Check local log_if as well as globally extracted phy_if*/
		if ((log_if_arg == NULL) || (ret != EOK) || (ret != EOK)) {
			/*	Instance does not exist */
			ret = ENOENT;
		} else {
			if (pfe_phy_if_has_log_if(phy_if_arg, log_if_arg)) {
				ret = EOK;
			} else {
				ret = ENOENT;
			}
		}

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(ret, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_PHY_IF_GET_OP_MODE: {
		pfe_platform_rpc_pfe_phy_if_get_op_mode_ret_t rpc_ret = { 0 };
		pfe_ct_if_op_mode_t mode = IF_OP_DISABLED;

		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_PHY_IF_GET_OP_MODE\n");

		if (ret == EOK) {
			mode = pfe_phy_if_get_op_mode(phy_if_arg);
			rpc_ret.mode = mode;
		}

		/*	Report execution status to caller */
		if (EOK !=
		    pfe_idex_set_rpc_ret_val(ret, &rpc_ret, sizeof(rpc_ret))) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_PHY_IF_IS_ENABLED: {
		pfe_platform_rpc_pfe_phy_if_is_enabled_ret_t rpc_ret = { 0 };
		bool_t status = FALSE;

		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_PHY_IF_IS_ENABLED\n");

		if (ret == EOK) {
			status = pfe_phy_if_is_enabled(phy_if_arg);
			rpc_ret.status = status;
		}

		/*	Report execution status to caller */
		if (EOK !=
		    pfe_idex_set_rpc_ret_val(ret, &rpc_ret, sizeof(rpc_ret))) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	case PFE_PLATFORM_RPC_PFE_PHY_IF_IS_PROMISC: {
		pfe_platform_rpc_pfe_phy_if_is_promisc_ret_t rpc_ret = { 0 };
		bool_t status = FALSE;

		NXP_LOG_DEBUG("RPC: PFE_PLATFORM_RPC_PFE_PHY_IF_IS_PROMISC\n");

		if (ret == EOK) {
			status = pfe_phy_if_is_promisc(phy_if_arg);
			rpc_ret.status = status;
		}

		/*	Report execution status to caller */
		if (EOK !=
		    pfe_idex_set_rpc_ret_val(ret, &rpc_ret, sizeof(rpc_ret))) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}

	default: {
		NXP_LOG_WARNING("Unsupported RPC code: %d\n", id);

		/*	Report execution status to caller */
		if (pfe_idex_set_rpc_ret_val(EINVAL, NULL, 0U) != EOK) {
			NXP_LOG_ERROR("Could not send RPC response\n");
		}

		break;
	}
	}

	return;
}
#endif /* PFE_CFG_MULTI_INSTANCE_SUPPORT */

/**
 * @brief		Assign HIF to the platform
 */
static errno_t
pfe_platform_create_hif(pfe_platform_t *platform, pfe_platform_config_t *config)
{
	uint32_t ii;
	static pfe_hif_chnl_id_t ids[HIF_CFG_MAX_CHANNELS] = {
		HIF_CHNL_0, HIF_CHNL_1, HIF_CHNL_2, HIF_CHNL_3
	};
	pfe_hif_chnl_t *chnl;

	platform->hif =
		pfe_hif_create(platform->cbus_baseaddr + CBUS_HIF_BASE_ADDR,
			       config->hif_chnls_mask);
	if (!platform->hif) {
		NXP_LOG_ERROR("Couldn't create HIF instance\n");
		return ENODEV;
	}

	if (config->common_irq_mode == FALSE) {
		/*	IRQ mode: per channel isr (S32G) */
		;
	} else { /* config->common_irq_mode */
		/*	IRQ mode: global isr (FPGA) */

		/*	Now particular channel interrupt sources can be enabled */
		for (ii = 0U; ii < HIF_CFG_MAX_CHANNELS; ii++) {
			chnl = pfe_hif_get_channel(platform->hif, ids[ii]);
			if (!chnl) {
				/* not requested HIF channel, skipping */
				continue;
			}

			pfe_hif_chnl_irq_unmask(chnl);
		}
	}

	pfe_hif_irq_unmask(platform->hif);

	return EOK;
}

/**
 * @brief		Release HIF-related resources
 */
static void
pfe_platform_destroy_hif(pfe_platform_t *platform)
{
	if (platform->hif) {
		pfe_hif_irq_mask(platform->hif);
		pfe_hif_destroy(platform->hif);
		platform->hif = NULL;
	}
}

#if defined(PFE_CFG_HIF_NOCPY_SUPPORT)

/**
 * @brief		Assign HIF NOCPY to the platform
 */
static errno_t
pfe_platform_create_hif_nocpy(pfe_platform_t *platform,
			      pfe_platform_config_t *config)
{
	platform->hif_nocpy = pfe_hif_nocpy_create(
		pfe.cbus_baseaddr + CBUS_HIF_NOCPY_BASE_ADDR, platform->bmu[1]);

	if (!platform->hif_nocpy) {
		NXP_LOG_ERROR("Couldn't create HIF NOCPY instance\n");
		return ENODEV;
	}

	if (config->common_irq_mode == FALSE) {
		/*	IRQ mode: per channel isr (S32G) */

		if (config->irq_vector_hif_nocpy != 0U) {
			/* misconfigured channel (requested in config, but irq not set),
				   so report and exit */
			NXP_LOG_ERROR("HIF NOCPY has no IRQ configured\n");
			return ENODEV;
		}

		/*	HIF NOCPY interrupt handler */
		platform->irq_hif_nocpy = oal_irq_create(
			config->irq_vector_hif_nocpy, 0, "PFE HIF NOCPY IRQ");
		if (!platform->irq_hif_nocpy) {
			NXP_LOG_ERROR(
				"Could not create HIF NOCPY IRQ vector %d\n",
				config->irq_vector_hif_nocpy);
			return ENODEV;
		} else {
			if (EOK !=
			    oal_irq_add_handler(platform->irq_hif_nocpy,
						&pfe_platform_hif_chnl_isr,
						pfe_hif_nocpy_get_channel(
							platform->hif_nocpy,
							PFE_HIF_CHNL_NOCPY_ID),
						NULL)) {
				NXP_LOG_ERROR(
					"Could not add IRQ handler for the BMU[0]\n");
				return ENODEV;
			}
		}
	} else /* config->common_irq_mode */
	{
		/*	IRQ mode: global isr (FPGA) */

		/* Note: used global isr, so do nothing here */
	}

	pfe_hif_chnl_irq_unmask(pfe_hif_nocpy_get_channel(
		platform->hif_nocpy, PFE_HIF_CHNL_NOCPY_ID));

	return EOK;
}

/**
 * @brief		Release HIF-related resources
 */
static void
pfe_platform_destroy_hif_nocpy(pfe_platform_t *platform)
{
	if (platform->hif_nocpy) {
		if (platform->irq_hif_nocpy) {
			oal_irq_destroy(platform->irq_hif_nocpy);
			platform->irq_hif_nocpy = NULL;
		}

		pfe_hif_nocpy_destroy(platform->hif_nocpy);
		platform->hif_nocpy = NULL;
	}
}
#endif /* PFE_CFG_HIF_NOCPY_SUPPORT */

/**
 * @brief		Assign BMU to the platform
 */
static errno_t
pfe_platform_create_bmu(pfe_platform_t *platform, pfe_platform_config_t *config)
{
	pfe_bmu_cfg_t bmu_cfg;

	/*	Create storage for instances */
	platform->bmu =
		oal_mm_malloc(platform->bmu_count * sizeof(pfe_bmu_t *));
	if (!platform->bmu) {
		NXP_LOG_ERROR("oal_mm_malloc() failed\n");
		return ENOMEM;
	}

	/*	BMU1 - LMEM buffers */
	bmu_cfg.pool_pa =
		(void *)(PFE_CFG_CBUS_PHYS_BASE_ADDR + CBUS_LMEM_BASE_ADDR +
			 PFE_CFG_BMU1_LMEM_BASEADDR);
	NXP_LOG_INFO("BMU1 buffer base: p0x%p\n", bmu_cfg.pool_pa);
	bmu_cfg.max_buf_cnt = PFE_CFG_BMU1_BUF_COUNT;
	bmu_cfg.buf_size = PFE_CFG_BMU1_BUF_SIZE;
	bmu_cfg.bmu_ucast_thres = 0x200U;
	bmu_cfg.bmu_mcast_thres = 0x200U;
	bmu_cfg.int_mem_loc_cnt = 64U;
	bmu_cfg.buf_mem_loc_cnt = 64U;

	platform->bmu[0] = pfe_bmu_create(
		platform->cbus_baseaddr, (void *)CBUS_BMU1_BASE_ADDR, &bmu_cfg);
	if (!platform->bmu[0]) {
		NXP_LOG_ERROR("Couldn't create BMU1 instance\n");
		return ENODEV;
	}

	if (platform->bmu_count < 2U) {
		NXP_LOG_WARNING("Only single BMU was configured.\n");
		return EOK;
	}

	/*	BMU2 - DDR buffers */
	{
		/*
			Due to RTL bug the BMU2 base address must be 2k aligned.
			TRM also says that the base shall be aligned to BUF_COUNT * BUF_SIZE.

			UPDATE: When not aligned to BUF_COUNT * BUF_SIZE the BMU is reporting buffer free errors.
		*/
		platform->bmu_buffers_size =
			PFE_CFG_BMU2_BUF_COUNT * (1U << PFE_CFG_BMU2_BUF_SIZE);
		platform->bmu_buffers_va =
			(void *)(addr_t)0x83400000; //TODO: oal_mm_malloc_contig_named_aligned_nocache("pfe_ddr", platform->bmu_buffers_size, platform->bmu_buffers_size);
		if (!platform->bmu_buffers_va) {
			NXP_LOG_ERROR("Unable to get BMU2 pool memory\n");
			return ENOMEM;
		}

		bmu_cfg.pool_va = platform->bmu_buffers_va;
		bmu_cfg.pool_pa =
			oal_mm_virt_to_phys_contig(platform->bmu_buffers_va);

		/*	S32G: Some of PFE AXI MASTERs can only access range p0x00020000 - p0xbfffffff */
		if (((addr_t)bmu_cfg.pool_pa < 0x00020000U) ||
		    (((addr_t)bmu_cfg.pool_pa + platform->bmu_buffers_size)) >
			    0xbfffffffU) {
			NXP_LOG_WARNING(
				"BMU2 buffers not in required range: starts @ p0x%p\n",
				bmu_cfg.pool_pa);
		} else {
			NXP_LOG_INFO("BMU2 buffer base: p0x%p (%" PRINTADDR_T
				     " bytes)\n",
				     bmu_cfg.pool_pa,
				     platform->bmu_buffers_size);
		}
	}

	bmu_cfg.max_buf_cnt = PFE_CFG_BMU2_BUF_COUNT;
	bmu_cfg.buf_size = PFE_CFG_BMU2_BUF_SIZE;
	bmu_cfg.bmu_ucast_thres = 0x800U;
	bmu_cfg.bmu_mcast_thres = 0x200U;
	bmu_cfg.int_mem_loc_cnt = 1024U;
	bmu_cfg.buf_mem_loc_cnt = 1024U;

	platform->bmu[1] = pfe_bmu_create(
		platform->cbus_baseaddr, (void *)CBUS_BMU2_BASE_ADDR, &bmu_cfg);

	if (!platform->bmu[1]) {
		NXP_LOG_ERROR("Couldn't create BMU2 instance\n");
		return ENODEV;
	}

	/*	BMU interrupt handling. Both instances share single interrupt line. */
	if (config->common_irq_mode == FALSE) {
		/*	IRQ mode: per block isr (S32G) */

		platform->irq_bmu =
			oal_irq_create(config->irq_vector_bmu,
				       (oal_irq_flags_t)0, "PFE BMU IRQ");
		if (!platform->irq_bmu) {
			NXP_LOG_ERROR("Could not create BMU IRQ vector %d\n",
				      config->irq_vector_bmu);
			return ENODEV;
		} else {
			if (EOK != oal_irq_add_handler(platform->irq_bmu,
						       &pfe_platform_bmu_isr,
						       platform, NULL)) {
				NXP_LOG_ERROR(
					"Could not add IRQ handler for the BMU[0]\n");
				return ENODEV;
			}
		}
	} else /* config->common_irq_mode */
	{
		/*	IRQ mode: global isr (FPGA) */

		/* Note: used global isr, so do nothing here */
	}

	pfe_bmu_irq_unmask(platform->bmu[0]);
	pfe_bmu_irq_unmask(platform->bmu[1]);

	return EOK;
}

/**
 * @brief		Release BMU-related resources
 */
static void
pfe_platform_destroy_bmu(pfe_platform_t *platform)
{
	uint32_t ii;

	if (platform->bmu) {
		if (platform->irq_bmu) {
			oal_irq_destroy(platform->irq_bmu);
			platform->irq_bmu = NULL;
		}

		for (ii = 0; ii < pfe.bmu_count; ii++) {
			if (platform->bmu[ii]) {
				pfe_bmu_destroy(platform->bmu[ii]);
				platform->bmu[ii] = NULL;
			}
		}

		oal_mm_free(platform->bmu);
		platform->bmu = NULL;
	}

	if (platform->bmu_buffers_va) {
		oal_mm_free_contig(platform->bmu_buffers_va);
		platform->bmu_buffers_va = NULL;
	}
}

/**
 * @brief		Assign GPI to the platform
 */
static errno_t
pfe_platform_create_gpi(pfe_platform_t *platform)
{
	pfe_gpi_cfg_t gpi_cfg;

	/*	Create storage for instances */
	platform->gpi =
		oal_mm_malloc(platform->gpi_count * sizeof(pfe_gpi_t *));
	if (!platform->gpi) {
		NXP_LOG_ERROR("oal_mm_malloc() failed\n");
		return ENOMEM;
	}

	/*	GPI1 */
	gpi_cfg.alloc_retry_cycles = 0x200U;
	gpi_cfg.gpi_tmlf_txthres = 0x178U;
	gpi_cfg.gpi_dtx_aseq_len =
		0x40; /* AAVB-2028 - DTX_ASEQ_LEN shall be 0x40 to have HW CSUMs working */
	gpi_cfg.emac_1588_ts_en = TRUE;

	platform->gpi[0] =
		pfe_gpi_create(platform->cbus_baseaddr,
			       (void *)CBUS_EGPI1_BASE_ADDR, &gpi_cfg);
	if (!platform->gpi[0]) {
		NXP_LOG_ERROR("Couldn't create GPI1 instance\n");
		return ENODEV;
	}

	/*	GPI2 */
	gpi_cfg.alloc_retry_cycles = 0x200U;
	gpi_cfg.gpi_tmlf_txthres = 0x178U;
	gpi_cfg.gpi_dtx_aseq_len =
		0x40; /* AAVB-2028 - DTX_ASEQ_LEN shall be 0x40 to have HW CSUMs working */
	gpi_cfg.emac_1588_ts_en = TRUE;

	platform->gpi[1] =
		pfe_gpi_create(platform->cbus_baseaddr,
			       (void *)CBUS_EGPI2_BASE_ADDR, &gpi_cfg);
	if (!platform->gpi[1]) {
		NXP_LOG_ERROR("Couldn't create GPI2 instance\n");
		return ENODEV;
	}

	/*	GPI3 */
	gpi_cfg.alloc_retry_cycles = 0x200U;
	gpi_cfg.gpi_tmlf_txthres = 0x178U;
	gpi_cfg.gpi_dtx_aseq_len =
		0x40; /* AAVB-2028 - DTX_ASEQ_LEN shall be 0x40 to have HW CSUMs working */
	gpi_cfg.emac_1588_ts_en = TRUE;

	platform->gpi[2] =
		pfe_gpi_create(platform->cbus_baseaddr,
			       (void *)CBUS_EGPI3_BASE_ADDR, &gpi_cfg);
	if (!platform->gpi[2]) {
		NXP_LOG_ERROR("Couldn't create GPI3 instance\n");
		return ENODEV;
	}

	return EOK;
}

/**
 * @brief		Release GPI-related resources
 */
static void
pfe_platform_destroy_gpi(pfe_platform_t *platform)
{
	uint32_t ii;

	if (platform->gpi) {
		for (ii = 0U; ii < platform->gpi_count; ii++) {
			if (platform->gpi[ii]) {
				pfe_gpi_destroy(platform->gpi[ii]);
				platform->gpi[ii] = NULL;
			}
		}

		oal_mm_free(platform->gpi);
		platform->gpi = NULL;
	}
}

/**
 * @brief		Assign ETGPI to the platform
 */
static errno_t
pfe_platform_create_etgpi(pfe_platform_t *platform)
{
	pfe_gpi_cfg_t gpi_cfg;

	/*	Create storage for instances */
	platform->etgpi =
		oal_mm_malloc(platform->etgpi_count * sizeof(pfe_gpi_t *));
	if (!platform->etgpi) {
		NXP_LOG_ERROR("oal_mm_malloc() failed\n");
		return ENOMEM;
	}

	/*	ETGPI1 */
	gpi_cfg.alloc_retry_cycles = 0x200U;
	gpi_cfg.gpi_tmlf_txthres = 0xbcU;
	gpi_cfg.gpi_dtx_aseq_len =
		0x40; /* AAVB-2028 - DTX_ASEQ_LEN shall be 0x40 to have HW CSUMs working */
	gpi_cfg.emac_1588_ts_en = TRUE;

	platform->etgpi[0] =
		pfe_gpi_create(platform->cbus_baseaddr,
			       (void *)CBUS_ETGPI1_BASE_ADDR, &gpi_cfg);
	if (!platform->etgpi[0]) {
		NXP_LOG_ERROR("Couldn't create ETGPI1 instance\n");
		return ENODEV;
	}

	/*	ETGPI2 */
	gpi_cfg.alloc_retry_cycles = 0x200U;
	gpi_cfg.gpi_tmlf_txthres = 0xbcU;
	gpi_cfg.gpi_dtx_aseq_len =
		0x40; /* AAVB-2028 - DTX_ASEQ_LEN shall be 0x40 to have HW CSUMs working */
	gpi_cfg.emac_1588_ts_en = TRUE;

	platform->etgpi[1] =
		pfe_gpi_create(platform->cbus_baseaddr,
			       (void *)CBUS_ETGPI2_BASE_ADDR, &gpi_cfg);
	if (!platform->etgpi[1]) {
		NXP_LOG_ERROR("Couldn't create ETGPI2 instance\n");
		return ENODEV;
	}

	/*	ETGPI3 */
	gpi_cfg.alloc_retry_cycles = 0x200U;
	gpi_cfg.gpi_tmlf_txthres = 0xbcU;
	gpi_cfg.gpi_dtx_aseq_len =
		0x40; /* AAVB-2028 - DTX_ASEQ_LEN shall be 0x40 to have HW CSUMs working */
	gpi_cfg.emac_1588_ts_en = TRUE;

	platform->etgpi[2] =
		pfe_gpi_create(platform->cbus_baseaddr,
			       (void *)CBUS_ETGPI3_BASE_ADDR, &gpi_cfg);
	if (!platform->etgpi[2]) {
		NXP_LOG_ERROR("Couldn't create ETGPI3 instance\n");
		return ENODEV;
	}

	return EOK;
}

/**
 * @brief		Release ETGPI-related resources
 */
static void
pfe_platform_destroy_etgpi(pfe_platform_t *platform)
{
	uint32_t ii;

	if (platform->etgpi) {
		for (ii = 0U; ii < platform->etgpi_count; ii++) {
			if (platform->etgpi[ii]) {
				pfe_gpi_destroy(platform->etgpi[ii]);
				platform->etgpi[ii] = NULL;
			}
		}

		oal_mm_free(platform->etgpi);
		platform->etgpi = NULL;
	}
}

/**
 * @brief		Assign HGPI to the platform
 */
static errno_t
pfe_platform_create_hgpi(pfe_platform_t *platform)
{
	pfe_gpi_cfg_t hgpi_cfg;

	/*	Create storage for instances */
	platform->hgpi =
		oal_mm_malloc(platform->hgpi_count * sizeof(pfe_gpi_t *));
	if (!platform->hgpi) {
		NXP_LOG_ERROR("oal_mm_malloc() failed\n");
		return ENOMEM;
	}

	hgpi_cfg.alloc_retry_cycles = 0x200U;
	hgpi_cfg.gpi_tmlf_txthres = 0x178U;
	hgpi_cfg.gpi_dtx_aseq_len = HGPI_ASEQ_LEN;
	hgpi_cfg.emac_1588_ts_en = FALSE;

	platform->hgpi[0] =
		pfe_gpi_create(platform->cbus_baseaddr,
			       (void *)CBUS_HGPI_BASE_ADDR, &hgpi_cfg);
	if (!platform->hgpi[0]) {
		NXP_LOG_ERROR("Couldn't create HGPI instance\n");
		return ENODEV;
	}

	return EOK;
}

/**
 * @brief		Release GPI-related resources
 */
static void
pfe_platform_destroy_hgpi(pfe_platform_t *platform)
{
	uint32_t ii;

	if (platform->hgpi) {
		for (ii = 0U; ii < platform->hgpi_count; ii++) {
			if (platform->hgpi[ii]) {
				pfe_gpi_destroy(platform->hgpi[ii]);
				platform->hgpi[ii] = NULL;
			}
		}

		oal_mm_free(platform->hgpi);
		platform->hgpi = NULL;
	}
}

/**
 * @brief		Assign CLASS to the platform
 */
static errno_t
pfe_platform_create_class(pfe_platform_t *platform)
{
	uint32_t ret;
	pfe_class_cfg_t class_cfg = {
		.resume = FALSE,
		.toe_mode = FALSE,
		.pe_sys_clk_ratio = PFE_CFG_CLMODE,
		.pkt_parse_offset =
			6U, /* This is actually the sizeof(struct hif_hdr) to skip the HIF header */
	};

	ELF_File_t elf;
	uint8_t *temp;

	if (!platform->fw) {
		NXP_LOG_ERROR("The CLASS firmware is NULL\n");
	}

	if ((!platform->fw->class_data) ||
	    (platform->fw->class_size == 0U)) {
		NXP_LOG_ERROR("The CLASS firmware is not loaded\n");
		return EIO;
	}

	platform->classifier = pfe_class_create(
		platform->cbus_baseaddr, platform->class_pe_count, &class_cfg);

	if (!platform->classifier) {
		NXP_LOG_ERROR("Couldn't create classifier instance\n");
		return ENODEV;
	} else {
		/*	Here we need to initialize data structures within the DMEM */
		temp = (uint8_t *)platform->fw->class_data;

		if ((temp[0] == 0x7fU) && (temp[1] == 'E') &&
		    (temp[2] == 'L') && (temp[3] == 'F')) {
			/*	FW is ELF file */
			NXP_LOG_INFO("Firmware .elf detected\n");

			if (FALSE == ELF_Open(&elf, platform->fw->class_data,
					      platform->fw->class_size)) {
				NXP_LOG_ERROR("Can't parse CLASS firmware\n");
				return EIO;
			} else {
				NXP_LOG_INFO("Uploading CLASS firmware\n");
				ret = pfe_class_load_firmware(
					platform->classifier, &elf);

				ELF_Close(&elf);

				if (ret != EOK) {
					NXP_LOG_ERROR(
						"Error during upload of CLASS firmware: %d\n",
						ret);
					return EIO;
				}
			}
		} else {
			NXP_LOG_ERROR("Only ELF format is supported\n");
			return ENODEV;
		}
	}

	return EOK;
}

/**
 * @brief		Release CLASS-related resources
 */
static void
pfe_platform_destroy_class(pfe_platform_t *platform)
{
	if (platform->classifier) {
		pfe_class_destroy(platform->classifier);
		platform->classifier = NULL;
	}
}

#if defined(PFE_CFG_L2BRIDGE_ENABLE)
/**
 * @brief		Assign L2 Bridge to the platform
 */
static errno_t
pfe_platform_create_l2_bridge(pfe_platform_t *platform)
{
	platform->mactab = pfe_l2br_table_create(platform->cbus_baseaddr,
						 PFE_L2BR_TABLE_MAC2F);
	if (!platform->mactab) {
		NXP_LOG_ERROR("Couldn't create MAC table instance\n");
		return ENODEV;
	}

	platform->vlantab = pfe_l2br_table_create(platform->cbus_baseaddr,
						  PFE_L2BR_TABLE_VLAN);
	if (!platform->vlantab) {
		NXP_LOG_ERROR("Couldn't create VLAN table instance\n");
		return ENODEV;
	}

	platform->l2_bridge = pfe_l2br_create(
		platform->classifier, 1U, platform->mactab, platform->vlantab);
	if (!platform->l2_bridge) {
		NXP_LOG_ERROR("Could not create L2 Bridge\n");
		return ENODEV;
	}

	return EOK;
}

/**
 * @brief		Release L2 Bridge-related resources
 */
static void
pfe_platform_destroy_l2_bridge(pfe_platform_t *platform)
{
	if (platform->l2_bridge) {
		pfe_l2br_destroy(platform->l2_bridge);
		platform->l2_bridge = NULL;
	}

	if (platform->mactab) {
		pfe_l2br_table_destroy(platform->mactab);
		platform->mactab = NULL;
	}

	if (platform->vlantab) {
		pfe_l2br_table_destroy(platform->vlantab);
		platform->vlantab = NULL;
	}
}
#endif /* PFE_CFG_L2BRIDGE_ENABLE */

#if defined(PFE_CFG_RTABLE_ENABLE)
/**
 * @brief		Assign Routing Table to the platform
 */
static errno_t
pfe_platform_create_rtable(pfe_platform_t *platform)
{
	void *htable_mem;
	void *pool_mem;
	uint32_t pool_offs = 256U * pfe_rtable_get_entry_size();

	platform->rtable_size = 2U * 256U * pfe_rtable_get_entry_size();
	platform->rtable_va = oal_mm_malloc_contig_named_aligned_nocache(
		"pfe_ddr", platform->rtable_size, 2048U);
	if (!platform->rtable_va) {
		NXP_LOG_ERROR("Unable to get routing table memory\n");
		return ENOMEM;
	}

	htable_mem = platform->rtable_va;
	pool_mem = (void *)((addr_t)platform->rtable_va + pool_offs);

	if (!platform->classifier) {
		NXP_LOG_ERROR("Valid classifier instance required\n");
		return ENODEV;
	}

	platform->rtable = pfe_rtable_create(platform->classifier, htable_mem,
					     256U, pool_mem, 256U);

	if (!platform->rtable) {
		NXP_LOG_ERROR("Couldn't create routing table instance\n");
		return ENODEV;
	} else {
		NXP_LOG_INFO(
			"Routing table created, Hash Table @ p%p, Pool @ p%p (%d bytes)\n",
			oal_mm_virt_to_phys_contig(htable_mem),
			oal_mm_virt_to_phys_contig(htable_mem) + pool_offs,
			(uint32_t)platform->rtable_size);
	}

	return EOK;
}

/**
 * @brief		Release Routing table-related resources
 */
static void
pfe_platform_destroy_rtable(pfe_platform_t *platform)
{
	if (platform->rtable) {
		pfe_rtable_destroy(platform->rtable);
		platform->rtable = NULL;
	}

	if (platform->rtable_va) {
		oal_mm_free_contig(platform->rtable_va);
		platform->rtable_va = NULL;
	}
}
#endif /* PFE_CFG_RTABLE_ENABLE */

/**
 * @brief		Assign TMU to the platform
 */
static errno_t
pfe_platform_create_tmu(pfe_platform_t *platform)
{
	pfe_tmu_cfg_t tmu_cfg = {
		.pe_sys_clk_ratio = PFE_CFG_CLMODE,
	};

	platform->tmu = pfe_tmu_create(platform->cbus_baseaddr,
				       platform->tmu_pe_count, &tmu_cfg);

	if (!platform->tmu) {
		NXP_LOG_ERROR("Couldn't create TMU instance\n");
		return ENODEV;
	}

	return EOK;
}

/**
 * @brief		Release TMU-related resources
 */
static void
pfe_platform_destroy_tmu(pfe_platform_t *platform)
{
	if (platform->tmu) {
		pfe_tmu_destroy(platform->tmu);
		platform->tmu = NULL;
	}
}

/**
 * @brief		Assign UTIL to the platform
 */
static errno_t
pfe_platform_create_util(pfe_platform_t *platform)
{
	errno_t ret;
	pfe_util_cfg_t util_cfg = {
		.pe_sys_clk_ratio = PFE_CFG_CLMODE,
	};

	platform->util = pfe_util_create(platform->cbus_baseaddr,
					 platform->util_pe_count, &util_cfg);

	if (!platform->util) {
		NXP_LOG_ERROR("Couldn't create UTIL instance\n");
		return ENODEV;
	} else {
		ELF_File_t elf;

		if (NULL == platform->fw->util_data ||
		    0 == platform->fw->util_size) {
			NXP_LOG_WARNING("The UTIL firmware is not loaded\n");
			return EOK;
		}

		if (FALSE == ELF_Open(&elf, platform->fw->util_data,
				      platform->fw->util_size)) {
			NXP_LOG_ERROR("Can't parse UTIL firmware\n");
			return EIO;
		} else {
			NXP_LOG_INFO("Uploading UTIL firmware\n");
			ret = pfe_util_load_firmware(platform->util, &elf);

			ELF_Close(&elf);

			if (ret != EOK) {
				NXP_LOG_ERROR(
					"Error during upload of UTIL firmware: %d\n",
					ret);
				return EIO;
			}
		}
	}

	return EOK;
}

/**
 * @brief		Release UTIL-related resources
 */
static void
pfe_platform_destroy_util(pfe_platform_t *platform)
{
	if (platform->util) {
		pfe_util_destroy(platform->util);
		platform->util = NULL;
	}
}

/**
 * @brief		Assign EMAC to the platform
 */
static errno_t
pfe_platform_create_emac(pfe_platform_t *platform)
{
	/*	Create storage for instances */
	platform->emac =
		oal_mm_malloc(platform->emac_count * sizeof(pfe_emac_t *));
	if (!platform->emac) {
		NXP_LOG_ERROR("oal_mm_malloc() failed\n");
		return ENOMEM;
	}

	/*	EMAC1 */
#if PFE_CFG_IP_VERSION == PFE_CFG_IP_VERSION_NPU_7_14
	platform->emac[0] =
		pfe_emac_create(platform->cbus_baseaddr,
				(void *)CBUS_EMAC1_BASE_ADDR, EMAC_MODE_SGMII,
				EMAC_SPEED_1000_MBPS, EMAC_DUPLEX_FULL);
#else /* FPGA */
	platform->emac[0] =
		pfe_emac_create(platform->cbus_baseaddr,
				(void *)CBUS_EMAC1_BASE_ADDR, EMAC_MODE_SGMII,
				EMAC_SPEED_100_MBPS, EMAC_DUPLEX_FULL);
#endif
	if (!platform->emac[0]) {
		NXP_LOG_ERROR("Couldn't create EMAC1 instance\n");
		return ENODEV;
	} else {
		pfe_emac_set_max_frame_length(platform->emac[0], 1522);
		pfe_emac_enable_flow_control(platform->emac[0]);
		pfe_emac_enable_broadcast(platform->emac[0]);

		/*	MAC address will be added with phy/log interface */
	}

	/*	EMAC2 */
#if PFE_CFG_IP_VERSION == PFE_CFG_IP_VERSION_NPU_7_14
	platform->emac[1] =
		pfe_emac_create(platform->cbus_baseaddr,
				(void *)CBUS_EMAC2_BASE_ADDR, EMAC_MODE_RGMII,
				EMAC_SPEED_1000_MBPS, EMAC_DUPLEX_FULL);
#else /* FPGA */
	platform->emac[1] =
		pfe_emac_create(platform->cbus_baseaddr,
				(void *)CBUS_EMAC2_BASE_ADDR, EMAC_MODE_SGMII,
				EMAC_SPEED_100_MBPS, EMAC_DUPLEX_FULL);
#endif
	if (!platform->emac[1]) {
		NXP_LOG_ERROR("Couldn't create EMAC2 instance\n");
		return ENODEV;
	} else {
		pfe_emac_set_max_frame_length(platform->emac[1], 1522);
		pfe_emac_enable_flow_control(platform->emac[1]);
		pfe_emac_enable_broadcast(platform->emac[1]);

		/*	MAC address will be added with phy/log interface */
	}

	/*	EMAC3 */
#if PFE_CFG_IP_VERSION == PFE_CFG_IP_VERSION_NPU_7_14
	platform->emac[2] =
		pfe_emac_create(platform->cbus_baseaddr,
				(void *)CBUS_EMAC3_BASE_ADDR, EMAC_MODE_RGMII,
				EMAC_SPEED_1000_MBPS, EMAC_DUPLEX_FULL);
#else /* FPGA */
	platform->emac[2] =
		pfe_emac_create(platform->cbus_baseaddr,
				(void *)CBUS_EMAC3_BASE_ADDR, EMAC_MODE_SGMII,
				EMAC_SPEED_100_MBPS, EMAC_DUPLEX_FULL);
#endif
	if (!platform->emac[2]) {
		NXP_LOG_ERROR("Couldn't create EMAC3 instance\n");
		return ENODEV;
	} else {
		pfe_emac_set_max_frame_length(platform->emac[2], 1522);
		pfe_emac_enable_flow_control(platform->emac[2]);
		pfe_emac_enable_broadcast(platform->emac[2]);

		/*	MAC address will be added with phy/log interface */
	}

	return EOK;
}

/**
 * @brief		Release EMAC-related resources
 */
static void
pfe_platform_destroy_emac(pfe_platform_t *platform)
{
	uint32_t ii;

	if (platform->emac) {
		for (ii = 0U; ii < platform->emac_count; ii++) {
			if (platform->emac[ii]) {
				pfe_emac_destroy(platform->emac[ii]);
				platform->emac[ii] = NULL;
			}
		}

		oal_mm_free(platform->emac);
		platform->emac = NULL;
	}
}

#if !defined(TARGET_OS_UBOOT)
/**
 * @brief		Assign SAFETY and Watchdogs to the platform
 */
static errno_t
pfe_platform_create_safety(pfe_platform_t *platform,
			   pfe_platform_config_t *config)
{
	(void)config;
	/*	Safety */
	platform->safety = pfe_safety_create(platform->cbus_baseaddr,
					     (void *)CBUS_GLOBAL_CSR_BASE_ADDR);

	if (!platform->safety) {
		NXP_LOG_ERROR("Couldn't create SAFETY instance\n");
		return ENODEV;
	} else {
		NXP_LOG_INFO("SAFETY instance created\n");
	}

#if (PFE_CFG_IP_VERSION != PFE_CFG_IP_VERSION_FPGA_5_0_4)
	/*	Watchdogs */
	platform->wdt = pfe_wdt_create(platform->cbus_baseaddr,
				       (void *)CBUS_GLOBAL_CSR_BASE_ADDR);

	if (!platform->wdt) {
		NXP_LOG_ERROR("Couldn't create Watchdog instance\n");
		return ENODEV;
	} else {
		NXP_LOG_INFO("Watchdog instance created\n");
	}
#endif /* PFE_CFG_IP_VERSION */

	/*	Interrupts. Safety and Watchdog interrupts are not connected to host but to FCCU.
		We'll use polling to at least detect that something has happened... See the polling
		thread where safety and wdt ISRs are being periodically called. */

	pfe_safety_irq_unmask(platform->safety);
#if (PFE_CFG_IP_VERSION != PFE_CFG_IP_VERSION_FPGA_5_0_4)
	pfe_wdt_irq_unmask(platform->wdt);
#endif /* PFE_CFG_IP_VERSION */

	return EOK;
}

/**
 * @brief		Release SAFETY-related resources
 */
static void
pfe_platform_destroy_safety(pfe_platform_t *platform)
{
	if (platform->safety) {
		pfe_safety_destroy(platform->safety);
		platform->safety = NULL;
	}

#if (PFE_CFG_IP_VERSION != PFE_CFG_IP_VERSION_FPGA_5_0_4)
	if (platform->wdt) {
		pfe_wdt_destroy(platform->wdt);
		platform->wdt = NULL;
	}
#endif /* PFE_CFG_IP_VERSION */
}
#endif

#ifdef PFE_CFG_FCI_ENABLE
/**
 * @brief		Start the FCI endpoint
 *
 */
static errno_t
pfe_platform_create_fci(pfe_platform_t *platform)
{
	fci_init_info_t fci_init_info;
	errno_t ret = EOK;

	fci_init_info.rtable = platform->rtable;
	fci_init_info.l2_bridge = platform->l2_bridge;
	fci_init_info.class = platform->classifier;
	fci_init_info.phy_if_db = platform->phy_if_db;
	fci_init_info.log_if_db = platform->log_if_db;
	ret = fci_init(&fci_init_info, "pfe_fci");
	if (ret != EOK) {
		NXP_LOG_ERROR("Could not create the FCI endpoint\n");
		return ret;
	}

	platform->fci_created = TRUE;
	return EOK;
}

/**
 * @brief		Release FCI-related resources
 */
static void
pfe_platform_destroy_fci(pfe_platform_t *platform)
{
	fci_fini();
	platform->fci_created = FALSE;
}
#endif /* PFE_CFG_FCI_ENABLE */

/**
 * @brief		Regiseter logical interface
 * @details		Add logical interface to internal database
 */
errno_t
pfe_platform_register_log_if(pfe_platform_t *platform, pfe_log_if_t *log_if)
{
	u32 session_id;
	errno_t ret;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!platform) || (!log_if))) {
		NXP_LOG_ERROR("Null argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	ret = pfe_if_db_lock(&session_id);
	if (ret != EOK) {
		NXP_LOG_DEBUG("DB lock failed: %d\n", ret);
		return ret;
	}

	/*	Register in platform to db */
	ret = pfe_if_db_add(platform->log_if_db, session_id, log_if,
			    PFE_CFG_LOCAL_PHY_IF_ID);
	if (ret != EOK) {
		NXP_LOG_ERROR("Could not register %s: %d\n",
			      pfe_log_if_get_name(log_if), ret);
		pfe_log_if_destroy(log_if);
	}

	if (pfe_if_db_unlock(session_id) != EOK) {
		NXP_LOG_DEBUG("DB unlock failed\n");
	}

	return ret;
}

/**
 * @brief		Unregister logical interface
 * @details		Logical interface will be removed from internal database
 * @warning		Should be called only with locked DB
 */
errno_t
pfe_platform_unregister_log_if(pfe_platform_t *platform, pfe_log_if_t *log_if)
{
	errno_t ret = EOK;
	pfe_if_db_entry_t *entry = NULL;
	u32 session_id;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!platform) || (!log_if))) {
		NXP_LOG_ERROR("Null argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	ret = pfe_if_db_lock(&session_id);
	if (ret != EOK) {
		NXP_LOG_DEBUG("DB lock failed: %d\n", ret);
		return ret;
	}

	ret = pfe_if_db_get_first(platform->log_if_db, session_id,
				  IF_DB_CRIT_BY_INSTANCE, (void *)log_if,
				  &entry);
	if (!entry) {
		ret = ENOENT;
	} else if (ret == EOK) {
		ret = pfe_if_db_remove(platform->log_if_db, session_id, entry);
	}

	if (pfe_if_db_unlock(session_id) != EOK) {
		NXP_LOG_DEBUG("DB unlock failed\n");
	}

	return ret;
}

/**
 * @brief		Register physical interface
 * @details		Function will crate mapping table between physical interface IDs and
 *				instances and add the physical interface instance with various validity
 *				checks.
 */
static errno_t
pfe_platform_register_phy_if(pfe_platform_t *platform, uint32_t session_id,
			     pfe_phy_if_t *phy_if)
{
	errno_t ret;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely((!platform) || (!phy_if))) {
		NXP_LOG_ERROR("Null argument received\n");
		return EINVAL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	/*	Owner of the interface is local driver instance */
	ret = pfe_if_db_add(platform->phy_if_db, session_id, phy_if,
			    PFE_CFG_LOCAL_PHY_IF_ID);

	return ret;
}

/**
 * @brief		Get logical interface by its ID
 * @param[in]	platform Platform instance
 * @param[in]	id Logical interface ID. See pfe_log_if_t.
 * @return		Logical interface instance or NULL if failed.
 */
pfe_log_if_t *
pfe_platform_get_log_if_by_id(pfe_platform_t *platform, uint8_t id)
{
	pfe_if_db_entry_t *entry = NULL;
	uint32_t session_id;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!platform)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return NULL;
	}

	if (unlikely(!platform->log_if_db)) {
		NXP_LOG_ERROR("Logical interface DB not found\n");
		return NULL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (pfe_if_db_lock(&session_id) != EOK) {
		NXP_LOG_DEBUG("DB lock failed\n");
	}

	pfe_if_db_get_first(platform->log_if_db, session_id, IF_DB_CRIT_BY_ID,
			    (void *)(addr_t)id, &entry);

	if (pfe_if_db_unlock(session_id) != EOK) {
		NXP_LOG_DEBUG("DB unlock failed\n");
	}

	return pfe_if_db_entry_get_log_if(entry);
}

/**
 * @brief		Get logical interface by name
 * @param[in]	platform Platform instance
 * @param[in]	name Logical interface name
 * @return		Logical interface instance or NULL if failed.
 */
pfe_log_if_t *
pfe_platform_get_log_if_by_name(pfe_platform_t *platform, char_t *name)
{
	pfe_if_db_entry_t *entry = NULL;
	uint32_t session_id;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!platform)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return NULL;
	}

	if (unlikely(!platform->log_if_db)) {
		NXP_LOG_ERROR("Logical interface DB not found\n");
		return NULL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (pfe_if_db_lock(&session_id) != EOK) {
		NXP_LOG_DEBUG("DB lock failed\n");
	}

	pfe_if_db_get_first(platform->log_if_db, session_id, IF_DB_CRIT_BY_NAME,
			    (void *)name, &entry);

	if (pfe_if_db_unlock(session_id) != EOK) {
		NXP_LOG_DEBUG("DB unlock failed\n");
	}

	return pfe_if_db_entry_get_log_if(entry);
}

/**
 * @brief		Get physical interface by its ID
 * @param[in]	platform Platform instance
 * @param[in]	id Physical interface ID
 * @return		Logical interface instance or NULL if failed.
 */
pfe_phy_if_t *
pfe_platform_get_phy_if_by_id(pfe_platform_t *platform, pfe_ct_phy_if_id_t id)
{
	pfe_if_db_entry_t *entry = NULL;
	uint32_t session_id;

#if defined(PFE_CFG_NULL_ARG_CHECK)
	if (unlikely(!platform)) {
		NXP_LOG_ERROR("NULL argument received\n");
		return NULL;
	}

	if (unlikely(!platform->phy_if_db)) {
		NXP_LOG_ERROR("Physical interface DB not found\n");
		return NULL;
	}
#endif /* PFE_CFG_NULL_ARG_CHECK */

	if (pfe_if_db_lock(&session_id) != EOK) {
		NXP_LOG_DEBUG("DB lock failed\n");
	}

	pfe_if_db_get_first(platform->phy_if_db, session_id, IF_DB_CRIT_BY_ID,
			    (void *)(addr_t)id, &entry);

	if (pfe_if_db_unlock(session_id) != EOK) {
		NXP_LOG_DEBUG("DB unlock failed\n");
	}

	return pfe_if_db_entry_get_phy_if(entry);
}

/**
 * @brief		Assign interfaces to the platform.
 */
static errno_t
pfe_platform_create_ifaces(pfe_platform_t *platform)
{
	int32_t ii;
	pfe_phy_if_t *phy_if = NULL;
	errno_t ret = EOK;
	uint32_t session_id;

	struct {
		char_t *name;
		pfe_ct_phy_if_id_t id;
		pfe_mac_addr_t mac;
		union {
			pfe_emac_t *emac;
			pfe_hif_chnl_t *chnl;
		};
	} phy_ifs[] = {
		{ .name = "emac0",
		  .id = PFE_PHY_IF_ID_EMAC0,
		  .mac = GEMAC0_MAC,
		  .emac = platform->emac[0] },
		{ .name = "emac1",
		  .id = PFE_PHY_IF_ID_EMAC1,
		  .mac = GEMAC1_MAC,
		  .emac = platform->emac[1] },
		{ .name = "emac2",
		  .id = PFE_PHY_IF_ID_EMAC2,
		  .mac = GEMAC2_MAC,
		  .emac = platform->emac[2] },
		{ .name = "hif0",
		  .id = PFE_PHY_IF_ID_HIF0,
		  .mac = { 0 },
		  .chnl = pfe_hif_get_channel(platform->hif, HIF_CHNL_0) },
		{ .name = "hif1",
		  .id = PFE_PHY_IF_ID_HIF1,
		  .mac = { 0 },
		  .chnl = pfe_hif_get_channel(platform->hif, HIF_CHNL_1) },
		{ .name = "hif2",
		  .id = PFE_PHY_IF_ID_HIF2,
		  .mac = { 0 },
		  .chnl = pfe_hif_get_channel(platform->hif, HIF_CHNL_2) },
		{ .name = "hif3",
		  .id = PFE_PHY_IF_ID_HIF3,
		  .mac = { 0 },
		  .chnl = pfe_hif_get_channel(platform->hif, HIF_CHNL_3) },
#if defined(PFE_CFG_HIF_NOCPY_SUPPORT)
		{ .name = "hifncpy",
		  .id = PFE_PHY_IF_ID_HIF_NOCPY,
		  .mac = { 0 },
		  .chnl = pfe_hif_nocpy_get_channel(platform->hif_nocpy,
						    PFE_HIF_CHNL_NOCPY_ID) },
#endif /* PFE_CFG_HIF_NOCPY_SUPPORT */
		{ .name = NULL,
		  .id = PFE_PHY_IF_ID_INVALID,
		  .mac = { 0 },
		  { NULL } }
	};

	if (pfe_if_db_lock(&session_id) != EOK) {
		NXP_LOG_DEBUG("DB lock failed\n");
	}

	/*	Create interfaces. This also configures in interfaces in firmware. */
	for (ii = 0; phy_ifs[ii].id != PFE_PHY_IF_ID_INVALID; ii++) {
		/*	Create physical IF */
		phy_if = pfe_phy_if_create(platform->classifier, phy_ifs[ii].id,
					   phy_ifs[ii].name);
		if (!phy_if) {
			NXP_LOG_ERROR("Couldn't create %s\n", phy_ifs[ii].name);
			return ENODEV;
		} else {
			/*	Set default operation mode */
			if (EOK !=
			    pfe_phy_if_set_op_mode(phy_if, IF_OP_DEFAULT)) {
				NXP_LOG_ERROR(
					"Could not set default operational mode (%s)\n",
					phy_ifs[ii].name);
				return ENODEV;
			}

			if ((pfe_phy_if_get_id(phy_if) ==
			     PFE_PHY_IF_ID_EMAC0) ||
			    (pfe_phy_if_get_id(phy_if) ==
			     PFE_PHY_IF_ID_EMAC1) ||
			    (pfe_phy_if_get_id(phy_if) ==
			     PFE_PHY_IF_ID_EMAC2)) {
				/*	Bind EMAC instance with the physical IF */
				if (EOK != pfe_phy_if_bind_emac(
						   phy_if, phy_ifs[ii].emac)) {
					NXP_LOG_ERROR(
						"Can't bind interface with EMAC (%s)\n",
						phy_ifs[ii].name);
					return ENODEV;
				}

				/*	Do not set MAC address here. Will be configured via logical interfaces later. */

			} else {
				/*	Bind HIF channel instance with the physical IF */
				if (phy_ifs[ii].chnl) {
					if (EOK !=
					    pfe_phy_if_bind_hif(
						    phy_if, phy_ifs[ii].chnl)) {
						NXP_LOG_ERROR(
							"Can't bind interface with HIF (%s)\n",
							phy_ifs[ii].name);
						return ENODEV;
					}
				} else {
					/*	This driver instance is not managing given channel */
				}
			}

			/*	Register in platform */
			if (EOK != pfe_platform_register_phy_if(
					   platform, session_id, phy_if)) {
				NXP_LOG_ERROR("Could not register %s\n",
					      pfe_phy_if_get_name(phy_if));
				if (pfe_phy_if_destroy(phy_if) != EOK) {
					NXP_LOG_DEBUG(
						"Could not destroy physical interface\n");
				}

				phy_if = NULL;
				return ENODEV;
			}
		}
	}

	if (pfe_if_db_unlock(session_id) != EOK) {
		NXP_LOG_DEBUG("DB unlock failed\n");
	}

	return ret;
}

/**
 * @brief		Release interface-related resources
 */
static void
pfe_platform_destroy_ifaces(pfe_platform_t *platform)
{
	pfe_if_db_entry_t *entry = NULL;
	pfe_log_if_t *log_if;
	pfe_phy_if_t *phy_if;
	uint32_t session_id;
	errno_t ret;

	if (platform->log_if_db) {
		if (pfe_if_db_lock(&session_id) != EOK) {
			NXP_LOG_DEBUG("DB lock failed\n");
		}

		ret = pfe_if_db_get_first(platform->log_if_db, session_id,
					  IF_DB_CRIT_ALL, NULL, &entry);
		while (entry) {
			log_if = pfe_if_db_entry_get_log_if(entry);

			if (EOK != pfe_if_db_remove(platform->log_if_db,
						    session_id, entry)) {
				NXP_LOG_DEBUG(
					"Could not remove log_if DB entry\n");
			}

			pfe_log_if_destroy(log_if);

			ret = pfe_if_db_get_next(platform->log_if_db,
						 session_id, &entry);
		}

		if (ret != EOK) {
			NXP_LOG_DEBUG(
				"Could not remove log_if DB entry, DB was locked\n");
		}

		if (pfe_if_db_unlock(session_id) != EOK) {
			NXP_LOG_DEBUG("DB unlock failed\n");
		}
	}

	if (platform->phy_if_db) {
		if (pfe_if_db_lock(&session_id) != EOK) {
			NXP_LOG_DEBUG("DB lock failed\n");
		}

		ret = pfe_if_db_get_first(platform->phy_if_db, session_id,
					  IF_DB_CRIT_ALL, NULL, &entry);
		while (entry) {
			phy_if = pfe_if_db_entry_get_phy_if(entry);

			if (EOK != pfe_if_db_remove(platform->phy_if_db,
						    session_id, entry)) {
				NXP_LOG_DEBUG(
					"Could not remove phy_if DB entry\n");
			}

			if (pfe_phy_if_destroy(phy_if) != EOK) {
				NXP_LOG_DEBUG("Can't destroy %s\n",
					      pfe_phy_if_get_name(phy_if));
			}

			ret = pfe_if_db_get_next(platform->phy_if_db,
						 session_id, &entry);
		}

		if (ret != EOK) {
			NXP_LOG_DEBUG(
				"Could not remove log_if DB entry, DB was locked\n");
		}

		if (pfe_if_db_unlock(session_id) != EOK) {
			NXP_LOG_DEBUG("DB unlock failed\n");
		}
	}
}

/**
 * @brief		Perform PFE soft reset
 */
errno_t
pfe_platform_soft_reset(pfe_platform_t *platform)
{
	void *addr;
	uint32_t regval;

	(void)platform;
	addr = (void *)(CBUS_GLOBAL_CSR_BASE_ADDR + 0x20U +
			(addr_t)(pfe.cbus_baseaddr));
	regval = hal_read32(addr) | (1U << 30);
	hal_write32(regval, addr);

	oal_time_usleep(100000); /* 100ms (taken from reference code) */

	regval &= ~(1U << 30);
	hal_write32(regval, addr);

	return EOK;
}

/**
 * @brief	The platform init function
 * @details	Initializes the PFE HW platform and prepares it for usage according to configuration.
 */
errno_t
pfe_platform_init(pfe_platform_config_t *config)
{
	errno_t ret = EOK;
	u32 *addr;
	uint32_t val;
	u32 *ii;

	memset(&pfe, 0U, sizeof(pfe_platform_t));
	pfe.fci_created = FALSE;

	/*	Create interface databases */
	pfe.phy_if_db = pfe_if_db_create(PFE_IF_DB_PHY);
	if (!pfe.phy_if_db) {
		NXP_LOG_DEBUG("Can't create physical interface DB\n");
		goto exit;
	}

	pfe.log_if_db = pfe_if_db_create(PFE_IF_DB_LOG);
	if (!pfe.log_if_db) {
		NXP_LOG_DEBUG("Can't create logical interface DB\n");
		goto exit;
	}

	pfe.fw = config->fw;

	/*	Map CBUS address space */
	pfe.cbus_baseaddr =
		oal_mm_dev_map((void *)config->cbus_base, config->cbus_len);
	if (!pfe.cbus_baseaddr) {
		NXP_LOG_ERROR("Can't map PPFE CBUS\n");
		goto exit;
	} else {
		NXP_LOG_INFO("PFE CBUS p0x%p mapped @ v0x%p\n",
			     (void *)config->cbus_base, pfe.cbus_baseaddr);
	}

	/*	Initialize LMEM TODO: Get LMEM size from global WSP_LMEM_SIZE register */
	addr = (uint32_t *)(void *)((addr_t)pfe.cbus_baseaddr +
				    CBUS_LMEM_BASE_ADDR);
	NXP_LOG_DEBUG("Initializing LMEM (%d bytes)\n", (int)CBUS_LMEM_SIZE);
	for (ii = addr; ii - addr < CBUS_LMEM_SIZE; ++ii) {
		*ii = 0U;
	}

	/*	Create HW components */
	pfe.emac_count = 3U;
	pfe.gpi_count = 3U;
	pfe.etgpi_count = 3U;
	pfe.hgpi_count = 1U;
	pfe.bmu_count = 2U;
#if PFE_CFG_IP_VERSION == PFE_CFG_IP_VERSION_NPU_7_14
	pfe.class_pe_count = 8U;
	pfe.util_pe_count = 1U;
#else /* FPGA */
	pfe.class_pe_count = 1U;
	pfe.util_pe_count =
		0U; /* Current IMG FPGA bitfile does not implement UTIL PE */
#endif
	pfe.tmu_pe_count = 0U;

	if (config->common_irq_mode == TRUE) {
		/*	IRQ mode: global isr (FPGA) */

		NXP_LOG_INFO("Detected Common IRQ mode (FPGA/PCI)\n");

		pfe.irq_global = oal_irq_create(config->irq_vector_global,
						OAL_IRQ_FLAG_SHARED, "PFE IRQ");
		if (!pfe.irq_global) {
			NXP_LOG_ERROR("Could not create global PFE IRQ\n");
			goto exit;
		} else {
			if (EOK != oal_irq_add_handler(pfe.irq_global,
						       &pfe_platform_global_isr,
						       &pfe, NULL)) {
				NXP_LOG_ERROR(
					"Could not add global IRQ handler\n");
				goto exit;
			}
		}
	} else /* config->common_irq_mode */
	{
		/*	IRQ mode: per block isr (S32G) */

		NXP_LOG_INFO("Detected per block IRQ mode (S32G)\n");

		/* Note:
		 *
		 * The irq handlers are created inside corresponding constructors,
		 * like pfe_platform_create_hif() or pfe_platform_create_bmu()
		 *
		 */
	}

	/*	BMU */
	ret = pfe_platform_create_bmu(&pfe, config);
	if (ret != EOK) {
		goto exit;
	}

	/*	GPI */
	ret = pfe_platform_create_gpi(&pfe);
	if (ret != EOK) {
		goto exit;
	}

	/*	ETGPI */
	ret = pfe_platform_create_etgpi(&pfe);
	if (ret != EOK) {
		goto exit;
	}

	/*	HGPI */
	ret = pfe_platform_create_hgpi(&pfe);
	if (ret != EOK) {
		goto exit;
	}

	/*	TMU */
	ret = pfe_platform_create_tmu(&pfe);
	if (ret != EOK) {
		goto exit;
	}

	/*	Classifier */
	ret = pfe_platform_create_class(&pfe);
	if (ret != EOK) {
		goto exit;
	}
	/*	EMAC */
	ret = pfe_platform_create_emac(&pfe);
	if (ret != EOK) {
		goto exit;
	}

	/*	SAFETY & Watchdogs */
	ret = pfe_platform_create_safety(&pfe, config);
	if (ret != EOK) {
		goto exit;
	}

#ifdef PFE_CFG_FCI_ENABLE
#if defined(PFE_CFG_RTABLE_ENABLE)
	/*	Routing Table */
	ret = pfe_platform_create_rtable(&pfe);
	if (ret != EOK) {
		goto exit;
	}
#endif /* PFE_CFG_RTABLE_ENABLE */
#endif /* PFE_CFG_FCI_ENABLE */
	if (config->enable_util) {
		/*	UTIL */
		ret = pfe_platform_create_util(&pfe);
		if (ret != EOK) {
			goto exit;
		}
	}

	/*	SOFT RESET */
	if (pfe_platform_soft_reset(&pfe) != EOK)
		NXP_LOG_ERROR("Platform reset failed\n");

#ifdef PFE_CFG_FCI_ENABLE
#if defined(PFE_CFG_L2BRIDGE_ENABLE)
	/*	L2 Bridge. Must be initialized after soft reset. */
	ret = pfe_platform_create_l2_bridge(&pfe);
	if (ret != EOK) {
		goto exit;
	}
#endif /* PFE_CFG_L2BRIDGE_ENABLE */
#endif /* PFE_CFG_FCI_ENABLE */

	/*	HIF (HIF DOES NOT LIKE SOFT RESET ONCE HAS BEEN CONFIGURED...) */
	ret = pfe_platform_create_hif(&pfe, config);
	if (ret != EOK) {
		goto exit;
	}

#if defined(PFE_CFG_HIF_NOCPY_SUPPORT)
	/*	HIF NOCPY */
	ret = pfe_platform_create_hif_nocpy(&pfe, config);
	if (ret != EOK) {
		goto exit;
	}
#endif /* PFE_CFG_HIF_NOCPY_SUPPORT */

#if defined(PFE_CFG_GLOB_ERR_POLL_WORKER)
	/*	Enable poller. First initialize state, then run the thread... */
	pfe.poller_state = POLLER_STATE_ENABLED;

	/*	Create poller thread for errors detection */
	pfe.poller = oal_thread_create(&pfe_poller_func, &pfe,
				       "Global polling thread", 0);
	if (!pfe.poller) {
		NXP_LOG_ERROR("Couldn't start polling thread\n");
		return ENODEV;
	}
#else  /* PFE_CFG_GLOB_ERR_POLL_WORKER */
	pfe.poller_state = POLLER_STATE_DISABLED;
#endif /* PFE_CFG_GLOB_ERR_POLL_WORKER */

	/*	Activate the classifier */
	NXP_LOG_INFO("Enabling the CLASS block\n");
	pfe_class_enable(pfe.classifier);
	/*	Wait a (micro) second to let classifier firmware to initialize */
	oal_time_usleep(50000);

	/*	Interfaces */
	ret = pfe_platform_create_ifaces(&pfe);
	if (ret != EOK) {
		goto exit;
	}

#ifdef PFE_CFG_FCI_ENABLE
	ret = pfe_platform_create_fci(&pfe);
	if (ret != EOK) {
		goto exit;
	}
#endif /* PFE_CFG_FCI_ENABLE */
#ifdef PFE_CFG_FLEX_PARSER_AND_FILTER
	pfe_fp_init();
	pfe_flexible_filter_init();
#endif /* PFE_CFG_FLEX_PARSER_AND_FILTER */

	/*	Activate PFE blocks */
	pfe_bmu_enable(pfe.bmu[0]);
	pfe_bmu_enable(pfe.bmu[1]);
	pfe_gpi_enable(pfe.gpi[0]);
	pfe_gpi_enable(pfe.gpi[1]);
	pfe_gpi_enable(pfe.gpi[2]);
	pfe_gpi_enable(pfe.etgpi[0]);
	pfe_gpi_enable(pfe.etgpi[1]);
	pfe_gpi_enable(pfe.etgpi[2]);
	pfe_gpi_enable(pfe.hgpi[0]);
	pfe_tmu_enable(pfe.tmu);
	if (config->enable_util) {
		pfe_util_enable(pfe.util);
	}
	/*	Enable MAC's TODO: FIXME: Really? What does this write do? (it does not work without this but we need to know
		what is the purpose of this - description is missing in TRM of course... */
	addr = (void *)(CBUS_GLOBAL_CSR_BASE_ADDR + 0x20U +
			(addr_t)(pfe.cbus_baseaddr));
	val = hal_read32(addr);
	hal_write32((val | 0x80000003U), addr);

	pfe.probed = TRUE;

	return EOK;

exit:
	(void)pfe_platform_remove();
	return ret;
}

/**
 * @brief		Destroy the platform
 */
errno_t
pfe_platform_remove(void)
{
	errno_t ret;
	/*	Remove and disable IRQ just before platform modules are destroyed. */
	if (pfe.irq_global) {
		oal_irq_destroy(pfe.irq_global);
		pfe.irq_global = NULL;
	}

	/*	Clear the generic control register */
	if (pfe.cbus_baseaddr) {
		hal_write32(0U, (void *)(CBUS_GLOBAL_CSR_BASE_ADDR + 0x20U +
					 (addr_t)(pfe.cbus_baseaddr)));
	}

#if defined(PFE_CFG_GLOB_ERR_POLL_WORKER)
	if (pfe.poller) {
		pfe.poller_state = POLLER_STATE_STOPPED;
		oal_thread_join(pfe.poller, NULL);
		pfe.poller = NULL;
	}
#endif /* PFE_CFG_GLOB_ERR_POLL_WORKER */

	pfe_platform_destroy_ifaces(
		&pfe); /* Need classifier instance to be available */
	pfe_platform_destroy_hif(&pfe);
#if defined(PFE_CFG_HIF_NOCPY_SUPPORT)
	pfe_platform_destroy_hif_nocpy(&pfe);
#endif /* PFE_CFG_HIF_NOCPY_SUPPORT */
	pfe_platform_destroy_gpi(&pfe);
	pfe_platform_destroy_etgpi(&pfe);
	pfe_platform_destroy_hgpi(&pfe);
	pfe_platform_destroy_bmu(&pfe);
#if defined(PFE_CFG_RTABLE_ENABLE)
	pfe_platform_destroy_rtable(&pfe);
#endif /* PFE_CFG_RTABLE_ENABLE */
#if defined(PFE_CFG_L2BRIDGE_ENABLE)
	pfe_platform_destroy_l2_bridge(&pfe);
#endif			  /* PFE_CFG_L2BRIDGE_ENABLE */
#ifdef PFE_CFG_FCI_ENABLE /* temp solution, disabled for Linux and MCAL now */
	pfe_platform_destroy_fci(&pfe);
#endif /* PFE_CFG_FCI_ENABLE */
	pfe_platform_destroy_class(&pfe);
	pfe_platform_destroy_tmu(&pfe);
	pfe_platform_destroy_util(&pfe);
	pfe_platform_destroy_emac(&pfe);
	pfe_platform_destroy_safety(&pfe);

	if (pfe.cbus_baseaddr) {
		ret = oal_mm_dev_unmap(
			pfe.cbus_baseaddr,
			PFE_CFG_CBUS_LENGTH /* <- FIXME, should use value used on init instead */);
		if (ret != EOK) {
			NXP_LOG_ERROR("Can't unmap PPFE CBUS: %d\n", ret);
			return ret;
		}
	}

	if (pfe.phy_if_db) {
		pfe_if_db_destroy(pfe.phy_if_db);
		pfe.phy_if_db = NULL;
	}

	if (pfe.log_if_db) {
		pfe_if_db_destroy(pfe.log_if_db);
		pfe.log_if_db = NULL;
	}

	pfe.cbus_baseaddr = 0x0ULL;
	pfe.probed = FALSE;

	return EOK;
}

/**
 * @brief		Read and print PFE HW IP blocks versions
 */
void
pfe_platform_print_versions(pfe_platform_t *platform)
{
	NXP_LOG_INFO("CLASS version    : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr + CLASS_VERSION));
	NXP_LOG_INFO("TMU version      : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr + TMU_VERSION));
	NXP_LOG_INFO("BMU1 version     : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr + CBUS_BMU1_BASE_ADDR +
				BMU_VERSION));
	NXP_LOG_INFO("BMU2 version     : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr + CBUS_BMU2_BASE_ADDR +
				BMU_VERSION));
	NXP_LOG_INFO("EGPI1 version    : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr + CBUS_EGPI1_BASE_ADDR +
				GPI_VERSION));
	NXP_LOG_INFO("EGPI2 version    : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr + CBUS_EGPI2_BASE_ADDR +
				GPI_VERSION));
	NXP_LOG_INFO("EGPI3 version    : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr + CBUS_EGPI3_BASE_ADDR +
				GPI_VERSION));
	NXP_LOG_INFO("ETGPI1 version   : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr +
				CBUS_ETGPI1_BASE_ADDR + GPI_VERSION));
	NXP_LOG_INFO("ETGPI2 version   : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr +
				CBUS_ETGPI2_BASE_ADDR + GPI_VERSION));
	NXP_LOG_INFO("ETGPI3 version   : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr +
				CBUS_ETGPI3_BASE_ADDR + GPI_VERSION));
	NXP_LOG_INFO("HGPI version     : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr + CBUS_HGPI_BASE_ADDR +
				GPI_VERSION));
	/* NXP_LOG_INFO("GPT version      : 0x%x\n", hal_read32(platform->cbus_baseaddr + CBUS_GPT_VERSION)); */
	NXP_LOG_INFO("HIF version      : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr + CBUS_HIF_BASE_ADDR +
				HIF_VERSION));
	NXP_LOG_INFO("HIF NOPCY version: 0x%x\n",
		     hal_read32(platform->cbus_baseaddr +
				CBUS_HIF_NOCPY_BASE_ADDR + HIF_NOCPY_VERSION));
	NXP_LOG_INFO("UTIL version     : 0x%x\n",
		     hal_read32(platform->cbus_baseaddr + UTIL_VERSION));
}

/**
 * @brief		Get the platform instance
 */
pfe_platform_t *
pfe_platform_get_instance(void)
{
	if (pfe.probed == TRUE) {
		return &pfe;
	} else {
		return NULL;
	}
}

/** @}*/
