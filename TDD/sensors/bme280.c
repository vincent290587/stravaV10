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

static float m_press_sim;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void bme280_init_sensor() {

	LOG_WARNING("BME init done");

	m_press_sim = 1003.0f;
}

void bme280_sleep() {

}

void bme280_read_sensor(void) {

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

void bme280_set_pressure(float press) {
	m_press_sim = press;
	m_is_updated = true;
}

bool bme280_is_data_ready(void) {
	return m_data.is_updated;
}

void bme280_refresh(void) {

	if (m_is_updated) {
		m_is_updated = false;

		LOG_DEBUG("BME280 Updated");

		m_data.comp_press = m_press_sim * (256.0F * 100.0F);
		m_data.comp_temp = 20.7f * 100.0F;

		LOG_DEBUG("Simulated pressure: %f", m_press_sim);

		m_data.is_updated = true;

	}

}

void bme280_clear_flags(void) {
	m_data.is_updated = 0;
}
