/*
 * BoucleFEC.cpp
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdbool.h>
#include <BoucleFEC.h>
#include "fec.h"
#include "millis.h"
#include "Model.h"
#include "power_scheduler.h"
#include "segger_wrapper.h"
#include "parameters.h"
#include "Locator.h"

#if defined (BLE_STACK_SUPPORT_REQD)
#include "ble_api_base.h"
#endif
#if defined(ANT_STACK_SUPPORT_REQD) || defined(TDD)
#include "fec.h"
#endif

static tHistoValue m_st_buffer[FEC_PW_BUFFER_NB_ELEM];


/**
 *
 */
BoucleFEC::BoucleFEC() : m_pw_buffer(FEC_PW_BUFFER_NB_ELEM, m_st_buffer) {

}


/**
 *
 */
void BoucleFEC::init_internal(void) {

	LOG_INFO("Boucle FEC init\r\n");

	// turn GPS OFF
	gps_mgmt.standby();

#if defined(ANT_STACK_SUPPORT_REQD) || defined(TDD)
	fec_profile_start();
#endif
}

void BoucleFEC::invalidate_internal(void) {


}

/**
 *
 */
void BoucleFEC::run_internal(void) {

	// wait for location to be updated
	(void)w_task_events_wait(TASK_EVENT_FEC_INFO | TASK_EVENT_FEC_POWER);

	LOG_INFO("Boucle FEC run\r\n");

#if defined(ANT_STACK_SUPPORT_REQD) || defined(TDD)
	// add FEC point to record
	attitude.addNewFECPoint(fec_info);

	if (m_pw_buffer.isFull()) {
		m_pw_buffer.popLast();
	}
	uint16_t power = fec_info.power;
	m_pw_buffer.add(&power);
	zPower.addPowerData(fec_info.power, millis());
#endif

	// ready for displaying
	if (m_tasks_id.ls027_id != TASK_ID_INVALID) {
		w_task_delay_cancel(m_tasks_id.ls027_id);
	}

	power_scheduler__ping(ePowerSchedulerPingFEC);
}
