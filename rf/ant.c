/* Copyright (c) 2012 Nordic Semiconductor. All Rights Reserved.
 *
 * The information contained herein is property of Nordic Semiconductor ASA.
 * Terms and conditions of usage are described in detail in NORDIC
 * SEMICONDUCTOR STANDARD SOFTWARE LICENSE AGREEMENT.
 *
 * Licensees are granted free, non-transferable use of the information. NO
 * WARRANTY of ANY KIND is provided. This heading must NOT be removed from
 * the file.
 */

/** @file
 *
 * @defgroup ble_sdk_apple_notification_main main.c
 * @{
 * @ingroup ble_sdk_app_apple_notification
 * @brief Apple Notification Client Sample Application main file. Disclaimer: 
 * This client implementation of the Apple Notification Center Service can and 
 * will be changed at any time by Nordic Semiconductor ASA.
 *
 * Server implementations such as the ones found in iOS can be changed at any 
 * time by Apple and may cause this client implementation to stop working.
 *
 * This file contains the source code for a sample application using the Apple 
 * Notification Center Service Client.
 */



#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "ant.h"
#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "app_scheduler.h"
#include "nrf_soc.h"
#include "nrf_delay.h"
#include "nrf_sdh.h"
#include "app_timer.h"
#include "nrf_drv_wdt.h"

#ifdef ANT_STACK_SUPPORT_REQD
#include "ant_key_manager.h"
#include "ant_search_config.h"
#include "ant_hrm.h"
#include "ant_bsc.h"
#include "ant_fec.h"
#include "ant_glasses.h"
#include "ant_interface.h"
#endif

#include "Model.h"
#include "segger_wrapper.h"

#include "glasses.h"
#include "fec.h"

#define GPIO_BUTTON                     30

#define ANT_DELAY                       APP_TIMER_TICKS(30000)


#define APP_ANT_OBSERVER_PRIO       1                                                               /**< Application's ANT observer priority. You shouldn't need to modify this value. */


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


#ifdef ANT_STACK_SUPPORT_REQD


APP_TIMER_DEF(m_sec_hrm);
APP_TIMER_DEF(m_sec_bsc);

nrf_drv_wdt_channel_id wdt_channel_id;


void ant_bsc_evt_handler(ant_bsc_profile_t * p_profile, ant_bsc_evt_t event);
void ant_evt_glasses (ant_evt_t * p_ant_evt);


BSC_DISP_CHANNEL_CONFIG_DEF(m_ant_bsc,
		BSC_CHANNEL_NUMBER,
		WILDCARD_TRANSMISSION_TYPE,
		BSC_DEVICE_TYPE,
		BSC_DEVICE_NUMBER,
		ANTPLUS_NETWORK_NUMBER,
		BSC_MSG_PERIOD_4Hz);
BSC_DISP_PROFILE_CONFIG_DEF(m_ant_bsc, ant_bsc_evt_handler);
ant_bsc_profile_t m_ant_bsc;

/** @snippet [ANT HRM RX Instance] */
HRM_DISP_CHANNEL_CONFIG_DEF(m_ant_hrm,
		HRM_CHANNEL_NUMBER,
		WILDCARD_TRANSMISSION_TYPE,
		HRM_DEVICE_NUMBER,
		ANTPLUS_NETWORK_NUMBER,
		HRM_MSG_PERIOD_4Hz);
ant_hrm_profile_t           m_ant_hrm;

FEC_DISP_CHANNEL_CONFIG_DEF(m_ant_fec,
		FEC_CHANNEL_NUMBER,
		WILDCARD_TRANSMISSION_TYPE,
		TACX_DEVICE_NUMBER,
		ANTPLUS_NETWORK_NUMBER);
FEC_DISP_PROFILE_CONFIG_DEF(m_ant_fec,
		ant_fec_evt_handler);


const ant_channel_config_t  ant_glasses_channel_config  = GLASSES_TX_CHANNEL_CONFIG(GLASSES_CHANNEL_NUMBER,
		                            GLASSES_DEVICE_NUMBER, ANTPLUS_NETWORK_NUMBER);


typedef struct
{
    int32_t acc_rev_cnt;
    int32_t prev_rev_cnt;
    int32_t prev_acc_rev_cnt;
    int32_t acc_evt_time;
    int32_t prev_evt_time;
    int32_t prev_acc_evt_time;
} bsc_disp_calc_data_t;

static bsc_disp_calc_data_t m_speed_calc_data   = {0};
static bsc_disp_calc_data_t m_cadence_calc_data = {0};

static uint8_t is_hrm_init = 0;
static uint8_t is_cad_init = 0;


/**
 *
 */
static void hrm_connect(void * p_context)
{
	uint32_t err_code = NRF_SUCCESS;

	err_code = ant_hrm_disp_open(&m_ant_hrm);
	APP_ERROR_CHECK(err_code);
}

/**
 *
 */
static void bsc_connect(void * p_context)
{
	uint32_t err_code = NRF_SUCCESS;

	err_code = ant_bsc_disp_open(&m_ant_bsc);
	APP_ERROR_CHECK(err_code);
}


/**
 *
 */
void ant_evt_bsc (ant_evt_t * p_ant_evt)
{
	uint32_t err_code = NRF_SUCCESS;

	uint16_t pusDeviceNumber = 0;
	uint8_t pucDeviceType    = 0;
	uint8_t pucTransmitType  = 0;

	switch (p_ant_evt->event)
	{
	case EVENT_RX:
		if (!is_cad_init) {
			sd_ant_channel_id_get (BSC_CHANNEL_NUMBER,
					&pusDeviceNumber, &pucDeviceType, &pucTransmitType);
//			LOG_INFO("$ANCS,0,CAD 0x%x connected\n\r", pusDeviceNumber);
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
		break;
	}

	APP_ERROR_CHECK(err_code);
}

/**
 *
 */
void ant_evt_hrm (ant_evt_t * p_ant_evt)
{
	uint32_t err_code = NRF_SUCCESS;

	uint16_t pusDeviceNumber = 0;
	uint8_t pucDeviceType    = 0;
	uint8_t pucTransmitType  = 0;

	switch (p_ant_evt->event)
	{
	case EVENT_RX:
		if (!is_hrm_init) {
			sd_ant_channel_id_get (HRM_CHANNEL_NUMBER,
					&pusDeviceNumber, &pucDeviceType, &pucTransmitType);

			if (pusDeviceNumber) is_hrm_init = 1;
		}
		LOG_INFO("HRM RX\r\n");
		ant_hrm_disp_evt_handler(p_ant_evt, &m_ant_hrm);
		break;
	case EVENT_RX_FAIL:
		break;
	case EVENT_RX_FAIL_GO_TO_SEARCH:
		break;
	case EVENT_RX_SEARCH_TIMEOUT:
		break;
	case EVENT_CHANNEL_CLOSED:
		is_hrm_init = 0;
		err_code = app_timer_start(m_sec_hrm, ANT_DELAY, NULL);
		break;
	}

	APP_ERROR_CHECK(err_code);
}



/**@brief Function for dispatching a ANT stack event to all modules with a ANT stack event handler.
 *
 * @details This function is called from the ANT Stack event interrupt handler after a ANT stack
 *          event has been received.
 *
 * @param[in] p_ant_evt  ANT stack event.
 */
void ant_evt_handler(ant_evt_t * p_ant_evt, void * p_context)
{
	W_SYSVIEW_RecordEnterISR();

	switch(p_ant_evt->channel) {
	case HRM_CHANNEL_NUMBER:
		ant_evt_hrm (p_ant_evt);
		break;

	case BSC_CHANNEL_NUMBER:
		ant_evt_bsc (p_ant_evt);
		break;

	case GLASSES_CHANNEL_NUMBER:
		ant_evt_glasses (p_ant_evt);
		break;

	case FEC_CHANNEL_NUMBER:
		ant_evt_fec (p_ant_evt);
		break;

	default:
		break;
	}

    W_SYSVIEW_RecordExitISR();
}
NRF_SDH_ANT_OBSERVER(m_ant_observer, APP_ANT_OBSERVER_PRIO, ant_evt_handler, 0);

/**
 *
 */
static uint32_t calculate_speed(int32_t rev_cnt, int32_t evt_time)
{
    static uint32_t computed_speed   = 0;

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

    return (uint32_t)computed_speed;
}

static uint32_t calculate_cadence(int32_t rev_cnt, int32_t evt_time)
{
    static uint32_t computed_cadence = 0;

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

    return (uint32_t)computed_cadence;
}

/**
 *
 */
void ant_evt_glasses (ant_evt_t * p_ant_evt)
{
	uint32_t err_code = NRF_SUCCESS;

	switch (p_ant_evt->event)
	{
	case EVENT_TX:
		NRF_LOG_DEBUG("Sending glasses payload");
		ant_glasses_tx_evt_handle(&m_ant_glasses, p_ant_evt, m_glasses_payload);
		break;
	case EVENT_RX:
		break;
	case EVENT_RX_FAIL:
		break;
	case EVENT_RX_FAIL_GO_TO_SEARCH:
		break;
	case EVENT_CHANNEL_CLOSED:
	case EVENT_RX_SEARCH_TIMEOUT:
		break;
	}

	APP_ERROR_CHECK(err_code);
}

/**
 *
 */
void ant_bsc_evt_handler(ant_bsc_profile_t * p_profile, ant_bsc_evt_t event)
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

		LOG_INFO("Evenement BSC speed=%lu cad=%lu\n",
				bsc_info.cadence, bsc_info.speed);
	}
		break;

	default:
		break;
	}
}



/**@brief Handle received ANT+ HRM data.
 * 
 * @param[in]   p_profile       Pointer to the ANT+ HRM profile instance.
 * @param[in]   event           Event related with ANT+ HRM Display profile. 
 */
static void ant_hrm_evt_handler(ant_hrm_profile_t * p_profile, ant_hrm_evt_t event)
{

	static uint32_t     s_previous_beat_count  = 0;    // Heart beat count from previously received page
	uint16_t            beat_time              = p_profile->page_0.beat_time;
	uint32_t            beat_count             = p_profile->page_0.beat_count;

	hrm_info.bpm = p_profile->page_0.computed_heart_rate;

	switch (event)
	{
	case ANT_HRM_PAGE_0_UPDATED:

		hrm_info.bpm = p_profile->page_0.computed_heart_rate;

		/* fall through */
	case ANT_HRM_PAGE_1_UPDATED:
		/* fall through */
	case ANT_HRM_PAGE_2_UPDATED:
		/* fall through */
	case ANT_HRM_PAGE_3_UPDATED:
		break;

	case ANT_HRM_PAGE_4_UPDATED:

		LOG_INFO( "Evenement HR BPM=%u\n", hrm_info.bpm);

		// Ensure that there is only one beat between time intervals.
		if ((beat_count - s_previous_beat_count) == 1)
		{
			uint16_t prev_beat = p_profile->page_4.prev_beat;
			uint16_t rrInterval = (beat_time - prev_beat);

			hrm_info.rr = rrInterval * 1000. / 1024.;

			// Subtracting the event time gives the R-R interval
			//ble_hrs_rr_interval_add(&m_hrs, beat_time - prev_beat);
			LOG_INFO( "Evenement HR RR=%u\n", hrm_info.rr);

		}

		s_previous_beat_count = beat_count;
		break;

	default:
		break;
	}


}

/**@brief Function for initializing the timer module.
 */
void ant_timers_init(void)
{
	uint32_t err_code;

	err_code = app_timer_create(&m_sec_hrm, APP_TIMER_MODE_SINGLE_SHOT, hrm_connect);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_create(&m_sec_bsc, APP_TIMER_MODE_SINGLE_SHOT, bsc_connect);
	APP_ERROR_CHECK(err_code);

	fec_init();

}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */
void ant_stack_init(void)
{
	ret_code_t err_code;

	err_code = nrf_sdh_enable_request();
	APP_ERROR_CHECK(err_code);

	ASSERT(nrf_sdh_is_enabled());

	err_code = nrf_sdh_ant_enable();
	APP_ERROR_CHECK(err_code);

	err_code = ant_plus_key_set(ANTPLUS_NETWORK_NUMBER);
	APP_ERROR_CHECK(err_code);

}

/**@brief Function for HRM profile initialization.
 *
 * @details Initializes the HRM profile and open ANT channel.
 */
static void ant_profile_setup(void)
{
	/** @snippet [ANT HRM RX Profile Setup] */
	uint32_t err_code;
    ant_search_config_t ant_search_config   = DEFAULT_ANT_SEARCH_CONFIG(0);

    // Disable high priority search to minimize disruption to other channels while searching
    ant_search_config.high_priority_timeout = ANT_HIGH_PRIORITY_SEARCH_DISABLE;
    ant_search_config.low_priority_timeout  = 80;
    ant_search_config.search_sharing_cycles = 0x10;
    ant_search_config.search_priority       = ANT_SEARCH_PRIORITY_LOWEST;

	// HRM
	err_code = ant_hrm_disp_init(&m_ant_hrm,
			HRM_DISP_CHANNEL_CONFIG(m_ant_hrm),
			ant_hrm_evt_handler);
	APP_ERROR_CHECK(err_code);

	ant_search_config.channel_number = HRM_CHANNEL_NUMBER;
    err_code = ant_search_init(&ant_search_config);
    APP_ERROR_CHECK(err_code);

	// CAD
	err_code = ant_bsc_disp_init(&m_ant_bsc,
			BSC_DISP_CHANNEL_CONFIG(m_ant_bsc),
			BSC_DISP_PROFILE_CONFIG(m_ant_bsc));
	APP_ERROR_CHECK(err_code);

	ant_search_config.channel_number = BSC_CHANNEL_NUMBER;
    err_code = ant_search_init(&ant_search_config);
    APP_ERROR_CHECK(err_code);


	// GLASSES
	err_code = ant_glasses_init(&m_ant_glasses, &ant_glasses_channel_config);
	APP_ERROR_CHECK(err_code);

	ant_search_config.channel_number = GLASSES_CHANNEL_NUMBER;
    err_code = ant_search_init(&ant_search_config);
    APP_ERROR_CHECK(err_code);

	//FEC
	err_code = ant_fec_disp_init(&m_ant_fec,
			FEC_DISP_CHANNEL_CONFIG(m_ant_fec),
			FEC_DISP_PROFILE_CONFIG(m_ant_fec));
	APP_ERROR_CHECK(err_code);

	ant_search_config.channel_number = FEC_CHANNEL_NUMBER;
    err_code = ant_search_init(&ant_search_config);
    APP_ERROR_CHECK(err_code);

    // Open the ANT channels

	err_code = ant_hrm_disp_open(&m_ant_hrm);
	APP_ERROR_CHECK(err_code);

	err_code = ant_bsc_disp_open(&m_ant_bsc);
	APP_ERROR_CHECK(err_code);

	err_code = ant_fec_disp_open(&m_ant_fec);
	APP_ERROR_CHECK(err_code);

//	err_code = ant_glasses_open(&m_ant_glasses);
//	APP_ERROR_CHECK(err_code);
	LOG_INFO("ANT ready");
}


/**@brief Function for application main entry.
 */
int ant_setup_start(void)
{
	ant_profile_setup();

	return 0;
}


#endif
