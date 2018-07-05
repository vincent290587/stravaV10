/*
 * Global.cpp
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#include "Model.h"
#include "segger_wrapper.h"

#include "i2c_scheduler.h"
#include "uart.h"


SAtt att;

Attitude      attitude;

ListeSegments mes_segments;

ListeParcours mes_parcours;

ListePoints   mes_points;

Locator       locator;

Boucle        boucle;

SegmentManager     segMngr;

Vue           vue;

STC3100       stc;

VEML6075      veml;

MS5637        ms5637;

GPS_MGMT           gps_mgmt;

sBacklightOrders     backlight;

sNeopixelOrders      neopixel;

// init counters
int Point2D::objectCount2D = 0;
int Point::objectCount = 0;

/**
 *
 */
void model_dispatch_sensors_update(void) {

	uint16_t light_level = veml.getRawVisComp();

	LOG_INFO("Light level: %u", light_level);
	NRF_LOG_DEBUG("Temperature: %ld", (int)ms5637.m_temperature);
	NRF_LOG_DEBUG("Pressure: %ld", (int)ms5637.m_pressure);

	// check if backlighting is used for notifying
	if (backlight.freq == 0) {
		// setup backlight
		if (light_level < BACKLIGHT_AUTO_START_RAW_VIS) {
			// il fait tout noir: TG
			backlight.state = 1;
		} else {
			// sun is shining
			backlight.state = 0;
		}
	}
}

/**
 *
 */
void perform_system_tasks(void) {

	gps_mgmt.tasks();

	locator.tasks();

	uart_tasks();

}

/**
 *
 * @return true if memory is full, false otherwise
 */
bool check_memory_exception(void) {

	int tot_point_mem = 0;
	tot_point_mem += Point::getObjectCount() * sizeof(Point);
	tot_point_mem += (Point2D::getObjectCount()-Point::getObjectCount()) * sizeof(Point2D);

	if (tot_point_mem > TOT_HEAP_MEM_AVAILABLE) {

		LOG_ERROR("Memory exhausted");

		return true;
	}

	return false;
}
