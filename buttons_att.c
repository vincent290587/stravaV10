/*
 * buttons_att.c
 *
 *  Created on: 15 déc. 2017
 *      Author: Vincent
 */

#include "boards.h"
#include "buttons_att.h"
#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_sdm.h"
#include "nrf_gpio.h"
#include "app_timer.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"


static uint16_t m_button_counter = 0;
static uint16_t m_reverse_counter = 0;

void buttons_att_tasks(void) {

	NRF_LOG_DEBUG("Buttons: %u %u %u",
			nrf_gpio_pin_read(BUTTON_1),
			nrf_gpio_pin_read(BUTTON_2),
			nrf_gpio_pin_read(BUTTON_3));

	if (!nrf_gpio_pin_read(BUTTON_1) && !nrf_gpio_pin_read(BUTTON_3)) {
		m_button_counter++;
		m_reverse_counter = 0;
	} else {
		m_reverse_counter++;
	}

	if (m_reverse_counter > BUTTONS_ACTION_COUNTER_NB) {
		m_button_counter = 0;
		m_reverse_counter = 0;
	}

	if (m_button_counter > BUTTONS_ACTION_COUNTER_NB) {
		m_button_counter = 0;
		m_reverse_counter = 0;

		// shutdown
//		ret_code_t err_code;
//		err_code = sd_softdevice_disable();
//		APP_ERROR_CHECK(err_code);
//		err_code = app_timer_stop_all();
//		APP_ERROR_CHECK(err_code);

		NRF_LOG_INFO("Going to OFF");
		nrf_gpio_pin_set(LED_PIN);
		nrf_gpio_pin_clear(LDO_PIN);

		while (1) {
			nrf_delay_ms(1000);
		}

		//sd_power_system_off();
	}

}




