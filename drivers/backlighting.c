/*
 * backlighting.c
 *
 *  Created on: 10 déc. 2017
 *      Author: Vincent
 */

#include <stdbool.h>
#include "boards.h"
#include "nrf_assert.h"
#include "app_timer.h"
#include "app_error.h"
#include "backlighting.h"
#include "segger_wrapper.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

APP_TIMER_DEF(m_back_timer);

static uint32_t m_ticks = 0;

static volatile bool m_timer_is_off = 0;


static void _backlight_callback(void* p_context) {

	m_timer_is_off = true;
	nrf_gpio_pin_toggle(BCK_PIN);

}


void backlighting_init(void) {

	nrf_gpio_cfg_output(BCK_PIN);
	nrf_gpio_pin_clear(BCK_PIN);

	// Create timer.
	uint32_t err_code = app_timer_create(&m_back_timer, APP_TIMER_MODE_SINGLE_SHOT, _backlight_callback);
	APP_ERROR_CHECK(err_code);

	m_timer_is_off = true;

}

/**
 *
 */
void backlighting_tasks(void) {

	if (m_ticks && m_timer_is_off) {

		m_timer_is_off = false;

		uint32_t err_code = app_timer_start(m_back_timer, m_ticks, NULL);
		APP_ERROR_CHECK(err_code);

	}
}

/**
 *
 * @param control
 */
void backlighting_set_control(sBacklightOrders* control) {

	ASSERT(control);

	m_ticks = 0;

	if (control->state && control->freq) {

		m_ticks = APP_TIMER_TICKS(control->freq * 10);

	} else if (control->state) {

		// turn on/off
		LOG_INFO("Backlighting ON");
		nrf_gpio_pin_set(BCK_PIN);

	} else {

		LOG_INFO("Backlighting OFF");
		nrf_gpio_pin_clear(BCK_PIN);

	}

}

