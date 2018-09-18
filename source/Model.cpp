/*
 * Global.cpp
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#include "Model.h"
#include "nrf_pwr_mgmt.h"
#include "sdk_config.h"
#include "segger_wrapper.h"

#include "i2c_scheduler.h"
#include "uart.h"

#ifdef USB_ENABLED
#include "usb_cdc.h"
#endif

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

GPS_MGMT      gps_mgmt;

VParser       vparser;

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

#ifdef USB_ENABLED
	usb_cdc_tasks();
#endif

#if APP_SCHEDULER_ENABLED
	app_sched_execute();
#endif

	if (NRF_LOG_PROCESS() == false)
	{
		nrf_pwr_mgmt_run();
	}
}

/**
 *
 */
void perform_system_tasks_light(void) {

	uart_tasks();

#if APP_SCHEDULER_ENABLED
	app_sched_execute();
#endif

	if (NRF_LOG_PROCESS() == false)
	{
		nrf_pwr_mgmt_run();
	}
}

/**
 *
 */
void model_go_to_msc_mode(void) {

	boucle.uninit();

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
