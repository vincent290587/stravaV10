/*
 * fec.c
 *
 *  Created on: 9 déc. 2017
 *      Author: Vincent
 */




#include "ant.h"
#include "fec.h"
#include "Model.h"
#ifdef ANT_STACK_SUPPORT_REQD
#include "ant_fec.h"
#include "ant_fec_pages.h"
#include "ant_fec_utils.h"
#include "ant_interface.h"
#include "ant_search_config.h"
#include "ant_device_manager.h"

#include "app_timer.h"
#include "segger_wrapper.h"

static void ant_fec_evt_handler(ant_fec_profile_t * p_profile, ant_fec_evt_t event);


FEC_DISP_CHANNEL_CONFIG_DEF(m_ant_fec,
		FEC_CHANNEL_NUMBER,
		WILDCARD_TRANSMISSION_TYPE,
		TACX_DEVICE_NUMBER,
		ANTPLUS_NETWORK_NUMBER);
FEC_DISP_PROFILE_CONFIG_DEF(m_ant_fec,
		ant_fec_evt_handler);


#define FEC_CONTROL_DELAY           APP_TIMER_TICKS(1500)

ant_fec_profile_t        m_ant_fec;

APP_TIMER_DEF(m_fec_update);

static bool is_fec_init = false;

static bool is_fec_tx_pending = false;

ant_fec_message_layout_t m_fec_message_payload;

static void roller_manager(void * p_context);

/**
 *
 */
void ant_evt_fec (ant_evt_t * p_ant_evt)
{
	ret_code_t err_code = NRF_SUCCESS;

	switch (p_ant_evt->event)
	{
	case EVENT_TX:
	case EVENT_TRANSFER_TX_COMPLETED:

		break;
	case EVENT_TRANSFER_TX_FAILED:
		NRF_LOG_WARNING("ANT FEC TX failed");
		break;

	case EVENT_RX:
	{
		uint16_t pusDeviceNumber = 0;
		uint8_t pucDeviceType    = 0;
		uint8_t pucTransmitType  = 0;
		sd_ant_channel_id_get (FEC_CHANNEL_NUMBER,
				&pusDeviceNumber, &pucDeviceType, &pucTransmitType);

		ant_device_manager_search_add(pusDeviceNumber, -5);

		if (pusDeviceNumber && !is_fec_init) {
			is_fec_init = 1;

			err_code = app_timer_start(m_fec_update, FEC_CONTROL_DELAY, &fec_control);
			APP_ERROR_CHECK(err_code);
		}
	}
	ant_fec_disp_evt_handler(p_ant_evt, &m_ant_fec);
	break;
	case EVENT_RX_FAIL:
		break;
	case EVENT_RX_FAIL_GO_TO_SEARCH:
		is_fec_init = 0;
		err_code = app_timer_stop(m_fec_update);
		APP_ERROR_CHECK(err_code);
		NRF_LOG_WARNING("ANT FEC EVENT_RX_FAIL_GO_TO_SEARCH");
		break;
	case EVENT_RX_SEARCH_TIMEOUT:
		NRF_LOG_WARNING("ANT FEC EVENT_RX_SEARCH_TIMEOUT");
		break;
	case EVENT_CHANNEL_CLOSED:
		NRF_LOG_WARNING("ANT FEC EVENT_CHANNEL_CLOSED");
		break;

	default:
		NRF_LOG_WARNING("ANT FEC event %02X", p_ant_evt->event);
		break;
	}
}


/**@brief Function for handling Bicycle Power profile's events
 *
 */
static void ant_fec_evt_handler(ant_fec_profile_t * p_profile, ant_fec_evt_t event)
{
	switch (event)
	{
	case ANT_FEC_PAGE_1_UPDATED:
		// calibration data received from sensor
		LOG_INFO("Received calibration data");
		break;

	case ANT_FEC_PAGE_2_UPDATED:
		// calibration data received from sensor
		LOG_INFO("Updated calibration data");
		break;

	case ANT_FEC_PAGE_16_UPDATED:
	{
		fec_info.el_time = ant_fec_utils_raw_time_to_uint16_t(p_profile->page_16.elapsed_time);
		fec_info.speed   = ant_fec_utils_raw_speed_to_uint16_t(p_profile->page_16.speed);
		w_task_events_set(m_tasks_id.boucle_id, TASK_EVENT_FEC_INFO);
	}
	break;

	case ANT_FEC_PAGE_25_UPDATED:
	{
		fec_info.power = p_profile->page_25.inst_power;
		w_task_events_set(m_tasks_id.boucle_id, TASK_EVENT_FEC_POWER);
	}
	break;

	case ANT_FEC_PAGE_17_UPDATED:
		/* fall through */
	case ANT_FEC_PAGE_21_UPDATED:
	case ANT_FEC_PAGE_48_UPDATED:
	case ANT_FEC_PAGE_49_UPDATED:
	case ANT_FEC_PAGE_51_UPDATED:
		/* fall through */
	case ANT_FEC_PAGE_80_UPDATED:
		/* fall through */
	case ANT_FEC_PAGE_81_UPDATED:
		// data actualization
		LOG_INFO("Page %u was updated", event);
		break;

	case ANT_FEC_CALIB_TIMEOUT:
		// calibration request time-out
		LOG_INFO("ANT_FEC_CALIB_TIMEOUT");
		break;

	case ANT_FEC_CALIB_REQUEST_TX_FAILED:
		// Please consider retrying the request.
		LOG_INFO("ANT_FEC_CALIB_REQUEST_TX_FAILED");
		break;

	default:
		// never occurred
		LOG_INFO("ANT_FEC Page %u was updated", event);
		break;
	}
}


static void roller_manager(void * p_context) {

	ASSERT(p_context);

	sFecControl* control = (sFecControl*)p_context;

	switch (control->type) {
	case eFecControlTargetPower:
	{
		memset(&m_fec_message_payload, 0, sizeof(m_fec_message_payload));
		m_fec_message_payload.page_number = ANT_FEC_PAGE_49;
		ant_fec_page49_data_t page49;
		page49.target_power = ant_fec_utils_target_power_to_uint16_t(control->data.power_control.target_power_w);
		ant_fec_page49_encode(m_fec_message_payload.page_payload, &page49);

		is_fec_tx_pending = true;
	}
	break;
	case eFecControlSlope:
	{
		memset(&m_fec_message_payload, 0, sizeof(m_fec_message_payload));
		m_fec_message_payload.page_number = ANT_FEC_PAGE_51;
		ant_fec_page51_data_t page51;
		page51.grade_slope = ant_fec_utils_slope_to_uint16_t(control->data.slope_control.slope_ppc);
		page51.roll_res    = ant_fec_utils_rolling_res_to_uint8_t(control->data.slope_control.rolling_resistance);
		ant_fec_page51_encode(m_fec_message_payload.page_payload, &page51);

		is_fec_tx_pending = true;
	}
	break;
	default:
		// no control

		is_fec_tx_pending = false;
		break;
	}


}

void roller_manager_tasks(void) {

	if (is_fec_tx_pending && is_fec_init) {

		uint8_t is_pending = 0;
		sd_ant_pending_transmit (m_ant_fec.channel_number, &is_pending);
		if (!is_pending) {

			LOG_INFO("Transmitting track simulation...");

			uint32_t err_code = sd_ant_acknowledge_message_tx(m_ant_fec.channel_number,
					sizeof (m_fec_message_payload),
					(uint8_t *) &m_fec_message_payload);
			APP_ERROR_CHECK(err_code);

			is_fec_tx_pending = false;
		} else {
			NRF_LOG_WARNING("Transmission busy");
		}

	}
}


void fec_init(void) {

	// Create timer.
	ret_code_t err_code = app_timer_create(&m_fec_update, APP_TIMER_MODE_REPEATED, roller_manager);
	APP_ERROR_CHECK(err_code);

	memset(&m_fec_message_payload, 0, sizeof(m_fec_message_payload));

	fec_control.type = eFecControlTargetNone;
	fec_control.data.power_control.target_power_w = 100;

}

void fec_set_control(sFecControl* tbc) {

	LOG_INFO("FEC control type %u", tbc->type);

	memcpy(&fec_control, tbc, sizeof(fec_control));

}

/**
 *
 */
void fec_profile_setup(void) {

	ret_code_t err_code;
#if defined( USE_ANT_SEARCH )
	ant_search_config_t ant_search_config   = DEFAULT_ANT_SEARCH_CONFIG(0);

	// Disable high priority search to minimize disruption to other channels while searching
	ant_search_config.high_priority_timeout = ANT_HIGH_PRIORITY_SEARCH_DISABLE;
	ant_search_config.low_priority_timeout  = 80;
	ant_search_config.search_sharing_cycles = 3;
	ant_search_config.search_priority       = ANT_SEARCH_PRIORITY_LOWEST;
#endif

	// FEC
	err_code = ant_fec_disp_init(&m_ant_fec,
			FEC_DISP_CHANNEL_CONFIG(m_ant_fec),
			FEC_DISP_PROFILE_CONFIG(m_ant_fec));
	APP_ERROR_CHECK(err_code);

#if defined( USE_ANT_SEARCH )
	ant_search_config.channel_number = FEC_CHANNEL_NUMBER;
	err_code = ant_search_init(&ant_search_config);
	APP_ERROR_CHECK(err_code);
#endif
}

/**
 *
 */
void fec_profile_start(void) {

	ret_code_t err_code;

	err_code = ant_fec_disp_open(&m_ant_fec);
	APP_ERROR_CHECK(err_code);
}

#endif
