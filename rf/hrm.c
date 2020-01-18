

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "nordic_common.h"
#include "app_error.h"
#include "app_scheduler.h"
#include "nrf_soc.h"
#include "nrf_sdh.h"
#include "millis.h"
#include "app_timer.h"
#include "ant_device_manager.h"

#ifdef ANT_STACK_SUPPORT_REQD
#include "ant_hrm.h"
#include "ant_interface.h"
#include "ant_search_config.h"

#include "Model.h"
#include "segger_wrapper.h"

#include "hrm.h"

#define ANT_DELAY                       APP_TIMER_TICKS(30000)

APP_TIMER_DEF(m_sec_hrm);


/** @snippet [ANT HRM RX Instance] */
HRM_DISP_CHANNEL_CONFIG_DEF(m_ant_hrm,
		HRM_CHANNEL_NUMBER,
		WILDCARD_TRANSMISSION_TYPE,
		HRM_DEVICE_NUMBER,
		ANTPLUS_NETWORK_NUMBER,
		HRM_MSG_PERIOD_4Hz);
ant_hrm_profile_t           m_ant_hrm;

static uint8_t is_hrm_init = 0;

/**
 *
 */
static void hrm_connect(void * p_context)
{
	uint32_t err_code = NRF_SUCCESS;

	err_code = ant_hrm_disp_open(&m_ant_hrm);
	APP_ERROR_CHECK(err_code);
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

			hrm_info.rr = (uint16_t)(rrInterval * 1000.f / 1024.f);

			hrm_info.timestamp = millis();

			// Subtracting the event time gives the R-R interval
			LOG_INFO( "Evenement HR RR=%u\n", hrm_info.rr);

		}

		s_previous_beat_count = beat_count;
		break;

	default:
		break;
	}
}

/**
 *
 */
void ant_evt_hrm (ant_evt_t * p_ant_evt)
{
	ret_code_t err_code = NRF_SUCCESS;

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

/**@brief Function for initializing the timer module.
 */
void hrm_init(void)
{
	ret_code_t err_code;

	err_code = app_timer_create(&m_sec_hrm, APP_TIMER_MODE_SINGLE_SHOT, hrm_connect);
	APP_ERROR_CHECK(err_code);

}

void hrm_profile_setup(void) {

	ret_code_t err_code;
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

}

void hrm_profile_start(void) {

	ret_code_t err_code;

	// Open the ANT channel
	err_code = ant_hrm_disp_open(&m_ant_hrm);
	APP_ERROR_CHECK(err_code);

}

#endif
