/*
 * bsc.c
 *
 *  Created on: 12 mrt. 2019
 *      Author: v.golle
 */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "Model.h"
#include "segger_wrapper.h"

#include "ant.h"
#include "nordic_common.h"
#include "app_error.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "app_timer.h"
#include "ant_device_manager.h"

#ifdef ANT_STACK_SUPPORT_REQD
#include "ant_key_manager.h"
#include "ant_search_config.h"
#include "ant_bsc.h"
#include "ant_interface.h"

#include "Model.h"
#include "segger_wrapper.h"

#include "bsc.h"

#define ANT_DELAY                       APP_TIMER_TICKS(30000)


/** @snippet [ANT BSC RX Instance] */
#define WHEEL_CIRCUMFERENCE         2070                                                            /**< Bike wheel circumference [mm] */
#define BSC_EVT_TIME_FACTOR         1024                                                            /**< Time unit factor for BSC events */
#define BSC_RPM_TIME_FACTOR         60                                                              /**< Time unit factor for RPM unit */
#define BSC_MS_TO_KPH_NUM           36                                                              /**< Numerator of [m/s] to [kph] ratio */
#define BSC_MS_TO_KPH_DEN           10                                                              /**< Denominator of [m/s] to [kph] ratio */
#define BSC_MM_TO_M_FACTOR          1000                                                            /**< Unit factor [m/s] to [mm/s] */
#define BSC_SPEED_UNIT_FACTOR       (BSC_MS_TO_KPH_DEN * BSC_MM_TO_M_FACTOR)                        /**< Speed unit factor */
//#define SPEED_COEFFICIENT           (WHEEL_CIRCUMFERENCE * BSC_EVT_TIME_FACTOR * BSC_MS_TO_KPH_NUM) /**< Coefficient for speed value calculation */
#define CADENCE_COEFFICIENT         (BSC_EVT_TIME_FACTOR * BSC_RPM_TIME_FACTOR)                     /**< Coefficient for cadence value calculation */
#define SPEED_COEFFICIENT           (WHEEL_CIRCUMFERENCE * BSC_EVT_TIME_FACTOR * BSC_MS_TO_KPH_NUM \
		/ BSC_MS_TO_KPH_DEN)                      /**< Coefficient for speed value calculation */

APP_TIMER_DEF(m_sec_bsc);

static void ant_bsc_evt_handler(ant_bsc_profile_t * p_profile, ant_bsc_evt_t event);


BSC_DISP_CHANNEL_CONFIG_DEF(m_ant_bsc,
		BSC_CHANNEL_NUMBER,
		WILDCARD_TRANSMISSION_TYPE,
		BSC_DEVICE_TYPE,
		BSC_DEVICE_NUMBER,
		ANTPLUS_NETWORK_NUMBER,
		BSC_MSG_PERIOD_4Hz);

BSC_DISP_PROFILE_CONFIG_DEF(m_ant_bsc, ant_bsc_evt_handler);

ant_bsc_profile_t m_ant_bsc;


typedef struct
{
	uint8_t is_init;
	int32_t acc_rev_cnt;
	int32_t prev_rev_cnt;
	int32_t prev_acc_rev_cnt;
	int32_t acc_evt_time;
	int32_t prev_evt_time;
	int32_t prev_acc_evt_time;
} bsc_disp_calc_data_t;

static bsc_disp_calc_data_t m_speed_calc_data   = {0};
static bsc_disp_calc_data_t m_cadence_calc_data = {0};

static uint8_t is_cad_init = 0;


/**
 *
 */
static void bsc_connect(void * p_context)
{
	uint32_t err_code = NRF_SUCCESS;

	err_code = ant_bsc_disp_open(&m_ant_bsc);
	APP_ERROR_CHECK(err_code);
}

static inline uint32_t calculate_cadence(int32_t rev_cnt, int32_t evt_time)
{
	uint32_t computed_cadence = 0;

	if (m_cadence_calc_data.is_init == 0) {

		m_cadence_calc_data.is_init = 1;
		m_cadence_calc_data.prev_rev_cnt  = rev_cnt;
		m_cadence_calc_data.prev_evt_time = evt_time;
		return computed_cadence;
	}

	if (rev_cnt != m_cadence_calc_data.prev_rev_cnt)
	{
		m_cadence_calc_data.acc_rev_cnt  += rev_cnt - m_cadence_calc_data.prev_rev_cnt;
		m_cadence_calc_data.acc_evt_time += evt_time - m_cadence_calc_data.prev_evt_time;

		/* Process rollover */
		if (m_cadence_calc_data.prev_rev_cnt > rev_cnt)
		{
			m_cadence_calc_data.acc_rev_cnt += UINT16_MAX + 1;
		}
		if (m_cadence_calc_data.prev_evt_time > evt_time)
		{
			m_cadence_calc_data.acc_evt_time += UINT16_MAX + 1;
		}

		m_cadence_calc_data.prev_rev_cnt  = rev_cnt;
		m_cadence_calc_data.prev_evt_time = evt_time;

		computed_cadence = CADENCE_COEFFICIENT *
				(m_cadence_calc_data.acc_rev_cnt  - m_cadence_calc_data.prev_acc_rev_cnt) /
				(m_cadence_calc_data.acc_evt_time - m_cadence_calc_data.prev_acc_evt_time);

		m_cadence_calc_data.prev_acc_rev_cnt  = m_cadence_calc_data.acc_rev_cnt;
		m_cadence_calc_data.prev_acc_evt_time = m_cadence_calc_data.acc_evt_time;
	}

	if (computed_cadence > 200) {
		computed_cadence = 0;
	}

	return computed_cadence;
}

/**
 *
 */
static inline uint32_t calculate_speed(int32_t rev_cnt, int32_t evt_time)
{
	uint32_t computed_speed   = 0;

	if (m_cadence_calc_data.is_init == 0) {

		m_speed_calc_data.is_init = 1;
		m_speed_calc_data.prev_rev_cnt  = rev_cnt;
		m_speed_calc_data.prev_evt_time = evt_time;
		return computed_speed;
	}

	if (rev_cnt != m_speed_calc_data.prev_rev_cnt)
	{
		m_speed_calc_data.acc_rev_cnt  += rev_cnt - m_speed_calc_data.prev_rev_cnt;
		m_speed_calc_data.acc_evt_time += evt_time - m_speed_calc_data.prev_evt_time;

		/* Process rollover */
		if (m_speed_calc_data.prev_rev_cnt > rev_cnt)
		{
			m_speed_calc_data.acc_rev_cnt += UINT16_MAX + 1;
		}
		if (m_speed_calc_data.prev_evt_time > evt_time)
		{
			m_speed_calc_data.acc_evt_time += UINT16_MAX + 1;
		}

		m_speed_calc_data.prev_rev_cnt  = rev_cnt;
		m_speed_calc_data.prev_evt_time = evt_time;

		computed_speed = SPEED_COEFFICIENT *
				(m_speed_calc_data.acc_rev_cnt  - m_speed_calc_data.prev_acc_rev_cnt) /
				(m_speed_calc_data.acc_evt_time - m_speed_calc_data.prev_acc_evt_time);

		m_speed_calc_data.prev_acc_rev_cnt  = m_speed_calc_data.acc_rev_cnt;
		m_speed_calc_data.prev_acc_evt_time = m_speed_calc_data.acc_evt_time;
	}

	if (computed_speed > 150) {
		computed_speed = 0;
	}

	return (uint32_t)computed_speed;
}

/**
 *
 */
static void ant_bsc_evt_handler(ant_bsc_profile_t * p_profile, ant_bsc_evt_t event)
{
	switch (event)
	{
	case ANT_BSC_PAGE_0_UPDATED:
		/* fall through */
	case ANT_BSC_PAGE_1_UPDATED:
		/* fall through */
	case ANT_BSC_PAGE_2_UPDATED:
		/* fall through */
	case ANT_BSC_PAGE_3_UPDATED:
		/* fall through */
	case ANT_BSC_PAGE_4_UPDATED:
		/* fall through */
	case ANT_BSC_PAGE_5_UPDATED:
		/* Log computed value */
		break;

	case ANT_BSC_COMB_PAGE_0_UPDATED:
	{
		bsc_info.speed = calculate_speed(p_profile->BSC_PROFILE_speed_rev_count, p_profile->BSC_PROFILE_speed_event_time);
		bsc_info.cadence = calculate_cadence(p_profile->BSC_PROFILE_cadence_rev_count, p_profile->BSC_PROFILE_cadence_event_time);

		LOG_DEBUG("Evenement BSC speed=%lu cad=%lu\n",
				bsc_info.cadence, bsc_info.speed);
	} break;

	default:
		break;
	}
}

/**
 *
 */
void ant_evt_bsc (ant_evt_t * p_ant_evt)
{
	ret_code_t err_code = NRF_SUCCESS;

	uint16_t pusDeviceNumber = 0;
	uint8_t pucDeviceType    = 0;
	uint8_t pucTransmitType  = 0;

	switch (p_ant_evt->event)
	{
	case EVENT_RX:
		if (!is_cad_init) {
			sd_ant_channel_id_get (BSC_CHANNEL_NUMBER,
					&pusDeviceNumber, &pucDeviceType, &pucTransmitType);

			if (pusDeviceNumber) {
				is_cad_init = 1;

				memset(&m_speed_calc_data, 0, sizeof(m_speed_calc_data));
				memset(&m_cadence_calc_data, 0, sizeof(m_cadence_calc_data));
			}
		}
		ant_bsc_disp_evt_handler(p_ant_evt, &m_ant_bsc);
		break;
	case EVENT_RX_FAIL:
		break;
	case EVENT_RX_FAIL_GO_TO_SEARCH:
		memset(&m_speed_calc_data, 0, sizeof(m_speed_calc_data));
		memset(&m_cadence_calc_data, 0, sizeof(m_cadence_calc_data));
		break;
	case EVENT_RX_SEARCH_TIMEOUT:
		break;
	case EVENT_CHANNEL_CLOSED:
		is_cad_init = 0;
		err_code = app_timer_start(m_sec_bsc, ANT_DELAY, NULL);
		APP_ERROR_CHECK(err_code);
		break;
	}

}

/**@brief Function for initializing the timer module.
 */
void bsc_init(void)
{
	ret_code_t err_code;

	err_code = app_timer_create(&m_sec_bsc, APP_TIMER_MODE_SINGLE_SHOT, bsc_connect);
	APP_ERROR_CHECK(err_code);

}

/**
 *
 */
void bsc_profile_setup(void) {

	ret_code_t err_code;
#if defined( USE_ANT_SEARCH )
	ant_search_config_t ant_search_config   = DEFAULT_ANT_SEARCH_CONFIG(0);

	// Disable high priority search to minimize disruption to other channels while searching
	ant_search_config.high_priority_timeout = ANT_HIGH_PRIORITY_SEARCH_DISABLE;
	ant_search_config.low_priority_timeout  = 80;
	ant_search_config.search_sharing_cycles = 3;
	ant_search_config.search_priority       = ANT_SEARCH_PRIORITY_LOWEST;
#endif

	// CAD
	err_code = ant_bsc_disp_init(&m_ant_bsc,
			BSC_DISP_CHANNEL_CONFIG(m_ant_bsc),
			BSC_DISP_PROFILE_CONFIG(m_ant_bsc));
	APP_ERROR_CHECK(err_code);

#if defined( USE_ANT_SEARCH )
	ant_search_config.channel_number = BSC_CHANNEL_NUMBER;
	err_code = ant_search_init(&ant_search_config);
	APP_ERROR_CHECK(err_code);
#endif

}

/**
 *
 */
void bsc_profile_start(void) {

	ret_code_t err_code;

	err_code = ant_bsc_disp_open(&m_ant_bsc);
	APP_ERROR_CHECK(err_code);
}


#endif
