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

static bool m_is_updated = false;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void bme280_init_sensor() {

	LOG_WARNING("BME init done");

}

void bme280_read_sensor(void) {
	m_is_updated = true;
}

bool is_bme280_updated(void) {
	return m_is_updated;
}

bme280_data *bme280_get_data_handle(void) {
	return &m_data;
}

float bme280_get_pressure(void) {
	return m_data.comp_press / (256.0F * 100.0F);
}

bool bme280_is_data_ready(void) {
	return m_data.is_updated;
}

void bme280_refresh(void) {

	if (m_is_updated) {
		m_is_updated = false;

		m_data.comp_press = 1011.0f * (256.0F * 100.0F);
		m_data.comp_temp = 20.7f * 100.0F;

		m_data.is_updated = true;

	}

}
