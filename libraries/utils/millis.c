/*
 * millis.c
 *
 *  Created on: 5 oct. 2017
 *      Author: Vincent
 */


#include <stdint.h>
#include "segger_wrapper.h"
#include "nrf_delay.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/



/*******************************************************************************
 * Variables
 ******************************************************************************/

volatile uint32_t lptmrCounter = 0U;

/*******************************************************************************
 * Code
 ******************************************************************************/

/**
 *
 */
void millis_init(void) {



}

/** @brief Returns the current time
 *
 * @return The internal time
 */
uint32_t millis(void) {
	return lptmrCounter;
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


