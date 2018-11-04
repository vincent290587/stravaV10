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

sTasksIDs m_tasks_id;

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

STC3100       stc;

AltiBaro      baro;

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

	gps_mgmt.tasks();

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
	tot_point_mem += Point2D::getObjectCount() * sizeof(Point2D);

	if (tot_point_mem > TOT_HEAP_MEM_AVAILABLE - 500) {

		LOG_ERROR("Memory exhausted");

		return true;
	} else if (5 * tot_point_mem > 4 * TOT_HEAP_MEM_AVAILABLE) {

		LOG_WARNING("Memory high");

	}

	return false;
}

void print_mem_state(void) {

	int tot_point_mem = 0;
	tot_point_mem += Point::getObjectCount() * sizeof(Point);
	tot_point_mem += Point2D::getObjectCount() * sizeof(Point2D);

	LOG_INFO("Allocated points: %d o %u 2D %u 3D", tot_point_mem, Point2D::getObjectCount(), Point::getObjectCount());
}
