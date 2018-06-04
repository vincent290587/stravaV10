/*
 * mk64f_parser.c
 *
 *  Created on: 6 déc. 2017
 *      Author: Vincent
 */

#include "fec.h"
#include "glasses.h"
#include "boards.h"
#include "nrf_assert.h"
#include "nrf_delay.h"
#include "notifications.h"
#include "mk64f_parser.h"
#include "backlighting.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


#define INT_PIN_DELAY_US    200

/**
 *
 * @param output
 */
void mk64f_parse_rx_info(sSpisRxInfo* input) {

	ASSERT(input);

	switch (input->page_id) {
	case eSpiRxPage0:
		notifications_setNotify (&input->pages.page0.neo_info);
		set_glasses_buffer      (&input->pages.page0.glasses_info);
		fec_set_control         (&input->pages.page0.fec_info);
		backlighting_set_control(&input->pages.page0.back_info);
		mk64f_power_down_switch (&input->pages.page0.power_info);
		// TODO battery orders
		break;

	case eSpiRxPage1:

		break;

	default:
		break;
	}
}

/**
 *
 * @param button_action
 */
void mk64f_toggle_line(eMk64fLineToggle button_action) {

	for (uint8_t i=0; i < button_action; i++) {

		nrf_gpio_pin_set(INT_PIN);
		nrf_delay_us(INT_PIN_DELAY_US);

		nrf_gpio_pin_clear(INT_PIN);
		nrf_delay_us(INT_PIN_DELAY_US);
	}

}

/**
 *
 * @param power_info
 */
void mk64f_power_down_switch(sPowerOrders *power_info) {

	if (power_info->state == 1) {
		NRF_LOG_INFO("Going to OFF");
		nrf_gpio_pin_set(LED_PIN);
		nrf_gpio_pin_clear(LDO_PIN);
		nrf_delay_ms(500);
	}

}

