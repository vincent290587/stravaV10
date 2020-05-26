/*
 * ant_tdd.c
 *
 *  Created on: 12 mrt. 2019
 *      Author: v.golle
 */

#include <stdbool.h>
#include "ant.h"
#include "ant_device_manager.h"
#include "millis.h"
#include "segger_wrapper.h"

static bool m_is_searching = false;

void ant_search_start(eAntPairingSensorType search_type) {

	ant_device_manager_search_add(2846U, 0);
	ant_device_manager_search_add(7777U, 0);
	ant_device_manager_search_add(12U, 0);

}

void ant_search_end(eAntPairingSensorType search_type, uint16_t dev_id) {

}

void ant_timers_init(void) {

}

void ant_stack_init(void) {

}

void ant_setup_init(void) {

}

void ant_setup_start(uint16_t hrm_id, uint16_t bsc_id, uint16_t fec_id) {
	LOG_INFO("ANT started");
}


void fec_profile_start(void) {

}
