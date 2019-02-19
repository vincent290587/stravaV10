/*
 * bme280.c
 *
 *  Created on: 27 janv. 2019
 *      Author: Vincent
 */

#include <stdbool.h>
#include "bme280.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

static bme280_data m_data;


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void bme280_init_sensor() {

	LOG_WARNING("BME init done");

}

void bme280_read_sensor(void) {

}

bme280_data *bme280_get_data_handle(void) {
	return &m_data;
}
