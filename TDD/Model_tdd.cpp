/*
 * Global.cpp
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#include "Model.h"
#include "uart_tdd.h"
#include "gpio.h"
#include "segger_wrapper.h"

SAtt att;

Attitude      attitude;

ListeSegments mes_segments;

ListeParcours mes_parcours;

ListePoints   mes_points;

Locator       locator;

Boucle        boucle;

SegmentManager     segMngr;

GPS_MGMT      gps_mgmt;

sBacklightOrders     backlight;

sNeopixelOrders      neopixel;

Vue           vue;

sHrmInfo hrm_info;
sBscInfo bsc_info;
sFecInfo fec_info;

// init counters
int Point2D::objectCount2D = 0;
int Point::objectCount = 0;

/**
 *
 */
void model_dispatch_sensors_update(void) {


}

/**
 *
 */
void perform_system_tasks(void) {

	locator.tasks();

	uart_tasks();

	btn_task();

}

/**
 *
 */
void perform_system_tasks_light(void) {

	uart_tasks();

	btn_task();

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
