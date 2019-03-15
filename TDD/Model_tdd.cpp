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
#include "neopixel.h"
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

sAppErrorDescr m_app_error;

SufferScore   suffer_score;

PowerZone     zPower;

UserSettings   u_settings;

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
 * @param nav
 */
void model_get_navigation(sKomootNavigation *nav) {

	nav->isUpdated = true;
	nav->distance = 750;
	nav->direction = ++nav->direction % 33;

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

	int tot_point_mem = Point::getObjectCount() * sizeof(Point);
	tot_point_mem += Point2D::getObjectCount() * sizeof(Point2D);
	tot_point_mem += segMngr.getNbSegs() * sizeof(sSegmentData);

	if (tot_point_mem > TOT_HEAP_MEM_AVAILABLE - 500) {

		LOG_ERROR("Memory exhausted");

		return true;
	} else if (5 * tot_point_mem > 4 * TOT_HEAP_MEM_AVAILABLE) {

		LOG_WARNING("Memory high");

	}

	return false;
}

void print_mem_state(void) {

	static int max_mem_used = 0;
	int tot_point_mem = 0;
	tot_point_mem += Point::getObjectCount() * sizeof(Point);
	tot_point_mem += Point2D::getObjectCount() * sizeof(Point2D);
	tot_point_mem += segMngr.getNbSegs() * sizeof(sSegmentData);

	if (tot_point_mem > max_mem_used) max_mem_used = tot_point_mem;

	LOG_INFO(">> Allocated pts: %d 2D %d 3D / mem %d o / %d o",
			Point2D::getObjectCount(), Point::getObjectCount(),
			tot_point_mem, max_mem_used);
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

		suffer_score.addHrmData(hrm_info.bpm, millis());

		// check screen update & unlock task
		if (millis() - vue.getLastRefreshed() > LS027_TIMEOUT_DELAY_MS) {
			vue.refresh();
		}

		baro.sensorRead();
		if (baro.isUpdated()) {
			baro.sensorRefresh();
		}

		neopixel_radio_callback_handler(false);

		// update date
		SDate dat;
		locator.getDate(dat);
		attitude.addNewDate(&dat);

		notifications_tasks();

		events_wait(TASK_EVENT_PERIPH_TRIGGER);
	}
}


