/*
 * Global.cpp
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#include <unistd.h>
#include "Model.h"
#include "uart_tdd.h"
#include "Simulator.h"
#include "gpio.h"
#include "segger_wrapper.h"

SAtt att;

sTasksIDs     m_tasks_id;

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

/**
 *
 * @param p_context
 */
void idle_task(void * p_context)
{
    for(;;)
    {
    	sleep(5);

		simulator_tasks();

    	task_yield();
    }
}

/**
 * System continuous tasks
 *
 * @param p_context
 */
void system_task(void * p_context)
{
    for(;;)
    {
		perform_system_tasks();

		task_yield();
    }
}

/**
 * Triggered externally when device has new valid data
 *
 * @param p_context
 */
void boucle_task(void * p_context)
{
	for (;;)
	{
		LOG_DEBUG("\r\nTask %u", millis());

#ifdef ANT_STACK_SUPPORT_REQD
		roller_manager_tasks();
#endif

		boucle.run();
	}
}

/**
 * Task triggered every APP_TIMEOUT_DELAY_MS.
 *
 * @param p_context
 */
void ls027_task(void * p_context)
{
	for(;;)
	{
#ifdef LS027_GUI
		// check screen update & unlock task
		vue.writeWhole();
#endif

		events_wait(TASK_EVENT_LS027_TRIGGER);
	}
}

/**
 * Task triggered every APP_TIMEOUT_DELAY_MS.
 *
 * @param p_context
 */
void peripherals_task(void * p_context)
{
	for(;;)
	{
		model_dispatch_sensors_update();

		// check screen update & unlock task
		if (millis() - vue.getLastRefreshed() > LS027_TIMEOUT_DELAY_MS) {
			vue.refresh();
		}

		// update date
		SDate dat;
		locator.getDate(dat);
		attitude.addNewDate(dat);

		notifications_tasks();

		events_wait(TASK_EVENT_PERIPH_TRIGGER);
	}
}


