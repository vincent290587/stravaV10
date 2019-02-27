/*
 * bme280.c
 *
 *  Created on: 27 janv. 2019
 *      Author: Vincent
 */

#include <stdbool.h>
#include "millis.h"
#include "bme280.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

#define BME280_TDD_UPDATE_TIME_MS        3500

static bme280_data m_data;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void bme280_init_sensor() {

	LOG_WARNING("BME init done");

}

void bme280_read_sensor(void) {

}

bool bme280_is_updated(void) {

	if (millis() >= BME280_TDD_UPDATE_TIME_MS &&
			!m_data.is_updated) {
		m_data.comp_press = 1011.0f;
		m_data.comp_temp = 20.7f;
		m_data.is_updated = true;
	}

	return m_data.is_updated;
}

bme280_data *bme280_get_data_handle(void) {
	return &m_data;
}
