/*
 * Global.cpp
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#include "Model.h"
#include "nrf_pwr_mgmt.h"
#include "sdk_config.h"
#include "task_manager.h"
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

AltiBaro      baro;

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

	uint16_t light_level = veml.getRawUVA();

	LOG_INFO("Light level: %u", light_level);
	NRF_LOG_DEBUG("Temperature: %ld", (int)baro.m_temperature);
	NRF_LOG_DEBUG("Pressure: %ld", (int)baro.m_pressure);

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


/**
 *
 */
void idle_task(void * p_context)
{
    for(;;)
    {
    	while (NRF_LOG_PROCESS()) { }

		perform_system_tasks();

		// BSP tasks
		bsp_tasks();

    	//No more logs to process, go to sleep
    	nrf_pwr_mgmt_run();

    	//Trigger context switch after any interrupt
        task_yield();
    }
}

/**
 *
 */
void boucle_task(void * p_context)
{

	for (;;)
	{
		LOG_DEBUG("\r\nTask %u", millis());

//			if (millis() > 4950 && millis() < 5050) usb_cdc_start_msc();

		wdt_reload();

#ifdef ANT_STACK_SUPPORT_REQD
		roller_manager_tasks();
#endif

		boucle.tasks();

		if (!millis()) NRF_LOG_WARNING("No millis");

		task_events_wait(TASK_EVENT_BOUCLE_READY);
	}
}

/**
 *
 */
void notifications_task(void * p_context)
{
    for(;;)
    {
		notifications_tasks();

		backlighting_tasks();

		task_events_wait(TASK_EVENT_SENSORS_READY);
    }
}


/**
 *
 */
#ifdef _DEBUG_TWI
void sensors_task(void * p_context)
{
	for(;;)
	{
		stc.refresh(nullptr);
		task_yield();
		veml.refresh(nullptr);
		task_yield();
		fxos_tasks(nullptr);
		task_yield();
		baro.refresh(nullptr);
		task_yield();

		model_dispatch_sensors_update();

		//Trigger context switch after any interrupt
		task_events_wait(TASK_EVENT_SENSORS_READY);
	}
}
#endif

