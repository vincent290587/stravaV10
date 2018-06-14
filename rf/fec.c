/*
 * fec.c
 *
 *  Created on: 9 déc. 2017
 *      Author: Vincent
 */

#include "assert.h"
#include "ant.h"
#include "fec.h"
#include "Model.h"
#include "ant_fec_pages.h"
#include "ant_interface.h"
#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define FEC_CONTROL_DELAY           APP_TIMER_TICKS(1500)

ant_fec_profile_t        m_ant_fec;

sFecInfo             fec_info;

sFecControl          fec_control;

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
	uint32_t err_code = NRF_SUCCESS;

	switch (p_ant_evt->event)
	{
	case EVENT_TX:
	case EVENT_TRANSFER_TX_COMPLETED:

		break;
	case EVENT_TRANSFER_TX_FAILED:
		NRF_LOG_WARNING("ANT FEC TX failed");
		break;

	case EVENT_RX:
		if (!is_fec_init) {
			uint16_t pusDeviceNumber = 0;
			uint8_t pucDeviceType    = 0;
			uint8_t pucTransmitType  = 0;
			sd_ant_channel_id_get (FEC_CHANNEL_NUMBER,
					&pusDeviceNumber, &pucDeviceType, &pucTransmitType);

			if (pusDeviceNumber) {
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

	APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Bicycle Power profile's events
 *
 */
void ant_fec_evt_handler(ant_fec_profile_t * p_profile, ant_fec_evt_t event)
{
	switch (event)
	{
	case ANT_FEC_PAGE_1_UPDATED:
		// calibration data received from sensor
		NRF_LOG_INFO("Received calibration data");
		break;

	case ANT_FEC_PAGE_2_UPDATED:
		// calibration data received from sensor
		NRF_LOG_INFO("Updated calibration data");
		break;

	case ANT_FEC_PAGE_16_UPDATED:
	{
		fec_info.el_time = ant_fec_utils_raw_time_to_uint16_t(p_profile->page_16.elapsed_time);
		fec_info.speed   = ant_fec_utils_raw_speed_to_uint16_t(p_profile->page_16.speed);
	}
	break;

	case ANT_FEC_PAGE_25_UPDATED:
	{
		fec_info.power = p_profile->page_25.inst_power;
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
		NRF_LOG_INFO("Page %u was updated", event);
		break;

	case ANT_FEC_CALIB_TIMEOUT:
		// calibration request time-out
		NRF_LOG_INFO("ANT_FEC_CALIB_TIMEOUT");
		break;

	case ANT_FEC_CALIB_REQUEST_TX_FAILED:
		// Please consider retrying the request.
		NRF_LOG_INFO("ANT_FEC_CALIB_REQUEST_TX_FAILED");
		break;

	default:
		// never occurred
		NRF_LOG_WARNING("Page %u was updated", event);
		break;
	}
}


static void roller_manager(void * p_context) {

	assert(p_context);

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

			NRF_LOG_INFO("Transmitting track simulation...");

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
	uint32_t err_code = app_timer_create(&m_fec_update, APP_TIMER_MODE_REPEATED, roller_manager);
	APP_ERROR_CHECK(err_code);

	memset(&m_fec_message_payload, 0, sizeof(m_fec_message_payload));

	fec_control.type = eFecControlTargetNone;
	fec_control.data.power_control.target_power_w = 150;

}

void fec_set_control(sFecControl* tbc) {

	NRF_LOG_INFO("FEC control type %u", tbc->type);

	memcpy(&fec_control, tbc, sizeof(fec_control));

}
