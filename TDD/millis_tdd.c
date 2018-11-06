/*
 * millis_tdd.c
 *
 *  Created on: 18 sept. 2018
 *      Author: Vincent
 */

#include "millis.h"
#include "timer.h"
#include "Model_tdd.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper_tdd.h"

#define MS_TIMER_GRANULOSITY       10

/*******************************************************************************
 * Code
 ******************************************************************************/

static volatile uint32_t m_cur_time_ms = 0;
static volatile uint32_t m_per_time_ms = 0;

void timer_handler(void)
{
	m_cur_time_ms += MS_TIMER_GRANULOSITY;

	// set peripherals in action
	if (m_cur_time_ms > m_per_time_ms + SENSORS_READING_DELAY_MS) {
		m_per_time_ms = m_cur_time_ms;
		// notify task
	    events_set(m_tasks_id.peripherals_id, TASK_EVENT_PERIPH_TRIGGER);
	}
}
/**
 *
 */
void millis_init(void) {

	start_timer(MS_TIMER_GRANULOSITY, &timer_handler);

}

/** @brief Returns the current time
 *
 * @return The internal time
 */
uint32_t millis(void) {

	return m_cur_time_ms;
}

/** @brief Delays execution by sleeping
 *
 * @param delay_ The delay in ms
 */
void delay_ms(uint32_t delay_) {

	uint32_t cur_time = millis();

	while (millis() < cur_time + delay_) {

	}
}

/**
 *
 * @param delay_ The delay in us
 */
void delay_us(uint32_t delay_) {


}
