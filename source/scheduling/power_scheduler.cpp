/*
 * power_scheduler.cpp
 *
 *  Created on: 10 févr. 2020
 *      Author: Vincent
 */

#include "millis.h"
#include "boards.h"
#include "gpio.h"
//#include "nrf_pwr_mgmt.h"
#include "power_scheduler.h"

#define POWER_SCHEDULER_MAX_IDLE_MIN               15


static uint32_t m_last_ping = 0;


void power_scheduler__run(void) {


	if (millis() - m_last_ping > (60000 * POWER_SCHEDULER_MAX_IDLE_MIN)) {

		//nrf_pwr_mgmt_shutdown(NRF_PWR_MGMT_SHUTDOWN_STAY_IN_SYSOFF);
		gpio_set(KILL_PIN);
	}

}


void power_scheduler__ping(ePowerSchedulerPing ping_type) {


	switch (ping_type) {

	case ePowerSchedulerPingCRS:
	{
		m_last_ping = millis();
	} break;

	case ePowerSchedulerPingFEC:
	{
		m_last_ping = millis();
	} break;


	default:
		break;
	}

}
