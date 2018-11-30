/*
 * millis.c
 *
 *  Created on: 5 oct. 2017
 *      Author: Vincent
 */


#include <stdint.h>
#include "segger_wrapper.h"
#include "nrf.h"
#include "app_timer.h"
#include "nrf_delay.h"
#include "nrfx_clock.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_TIMER_MS(TICKS)                                \
            ((uint32_t)ROUNDED_DIV(                        \
            (TICKS) * 1000 * (APP_TIMER_CONFIG_RTC_FREQUENCY + 1),   \
            (uint64_t)APP_TIMER_CLOCK_FREQ))

static uint32_t m_cur_time_ms = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 *
 */
void millis_init(void) {

	uint32_t err_code;

	err_code = app_timer_init();
	APP_ERROR_CHECK(err_code);

}

/** @brief Returns the current time
 *
 * @return The internal time
 */
uint32_t millis(void) {

	// check
	static uint32_t ticks_prev = 0;
	uint32_t ticks_cur = app_timer_cnt_get();

	uint32_t ticks = app_timer_cnt_diff_compute(ticks_cur, ticks_prev);

	ticks_prev = ticks_cur;

	m_cur_time_ms += APP_TIMER_MS(ticks);

	return m_cur_time_ms;
}

/** @brief Delays execution by sleeping
 *
 * @param delay_ The delay in ms
 */
void delay_ms(uint32_t delay_) {
	nrf_delay_ms(delay_);
}

/**
 *
 * @param delay_ The delay in us
 */
void delay_us(uint32_t delay_) {

	nrf_delay_us(delay_);
}


