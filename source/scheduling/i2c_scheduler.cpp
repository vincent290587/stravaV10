/*
 * i2c_scheduler.c
 *
 *  Created on: 10 déc. 2017
 *      Author: Vincent
 */

#include <i2c_scheduler.h>
#include "segger_wrapper.h"
#include "parameters.h"
#include "millis.h"
#include "fxos.h"
#include "Model.h"

static fxos_handle_t fxos_handle;

static uint32_t m_last_polled_time = 0;


static void _i2c_scheduling_sensors_init(void) {

	//FXOS_Init(&fxos_handle);

	stc.init(STC3100_CUR_SENS_RES_MO, STC3100_MODE_STANDARD);

	veml.init();

	ms5637.init();

	m_last_polled_time = 1;
}

void i2c_scheduling_init(void) {

	m_last_polled_time = 0;

	_i2c_scheduling_sensors_init();
}

void i2c_scheduling_tasks(void) {

	if (m_last_polled_time == 0) {
		_i2c_scheduling_sensors_init();
	}

	if (millis() - m_last_polled_time >= SENSORS_READING_DELAY_MS) {

		LOG_DEBUG("Reading sensors t=%u ms\r\n", millis());

		//fxos_tasks(&fxos_handle);

		stc.refresh();

		//veml.poll();

		ms5637.getPressure(OSR_8192);

		m_last_polled_time = millis();

		model_dispatch_sensors_update();
	}

}



