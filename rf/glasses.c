
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
#include "ant_glasses.h"
#include "ant_interface.h"

#include "glasses.h"

#define GLASSES_PAYLOAD_SIZE    8


const ant_channel_config_t  ant_glasses_channel_config  = GLASSES_TX_CHANNEL_CONFIG(GLASSES_CHANNEL_NUMBER,
		                            GLASSES_DEVICE_NUMBER, ANTPLUS_NETWORK_NUMBER);

// glasses profile
ant_glasses_profile_t       m_ant_glasses;

uint8_t m_glasses_payload[GLASSES_PAYLOAD_SIZE];

/**
 *
 * @param orders
 */
void set_glasses_buffer (sGlassesOrders* orders) {

	memset(m_glasses_payload, 0, GLASSES_PAYLOAD_SIZE);

	m_glasses_payload[0] = orders->led;
	m_glasses_payload[1] = orders->av_ent;
	m_glasses_payload[2] = orders->av_dec;

}

/**
 *
 */
void ant_evt_glasses (ant_evt_t * p_ant_evt)
{
	ret_code_t err_code = NRF_SUCCESS;

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
void glasses_init(void) {

	memset(m_glasses_payload, 0, GLASSES_PAYLOAD_SIZE);

}

/**
 *
 */
void glasses_profile_setup(void) {

	ret_code_t err_code;
	ant_search_config_t ant_search_config   = DEFAULT_ANT_SEARCH_CONFIG(0);

	// Disable high priority search to minimize disruption to other channels while searching
	ant_search_config.high_priority_timeout = ANT_HIGH_PRIORITY_SEARCH_DISABLE;
	ant_search_config.low_priority_timeout  = 80;
	ant_search_config.search_sharing_cycles = 0x10;
	ant_search_config.search_priority       = ANT_SEARCH_PRIORITY_LOWEST;

	// GLASSES
	err_code = ant_glasses_init(&m_ant_glasses, &ant_glasses_channel_config);
	APP_ERROR_CHECK(err_code);

	ant_search_config.channel_number = GLASSES_CHANNEL_NUMBER;
	err_code = ant_search_init(&ant_search_config);
	APP_ERROR_CHECK(err_code);
}

/**
 *
 */
void glasses_profile_start(void) {

	ret_code_t err_code;

	err_code = ant_glasses_open(&m_ant_glasses);
	APP_ERROR_CHECK(err_code);
}

#endif
