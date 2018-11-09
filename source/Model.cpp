/*
 * Global.cpp
 *
 *  Created on: 17 oct. 2017
 *      Author: Vincent
 */

#include "Model.h"
#include "nrf_pwr_mgmt.h"
#include "sdk_config.h"
#include "neopixel.h"
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

sTasksIDs     m_tasks_id;

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
			// TODO il fait tout noir: TG
			backlight.state = 0;
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
 * @param p_context
 */
void idle_task(void * p_context)
{
    for(;;)
    {
    	while (NRF_LOG_PROCESS()) { }

    	//No more logs to process, go to sleep
    	nrf_pwr_mgmt_run();

    	task_yield();
    	W_SYSVIEW_OnIdle();
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

		// BSP tasks
		bsp_tasks();

    	if (!NRF_LOG_PROCESS()) {
        	task_yield();
    	}
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
		LOG_INFO("\r\nTask %u", millis());

#ifdef ANT_STACK_SUPPORT_REQD
		roller_manager_tasks();
#endif

		boucle.run();

		if (!millis()) NRF_LOG_WARNING("No millis");
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
		wdt_reload();

		events_wait(TASK_EVENT_LS027_TRIGGER);

		// check screen update & unlock task
		vue.writeWhole();
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
#ifdef _DEBUG_TWI
		static uint32_t _counter = 0;

		if (++_counter >= 1000 / (SENSORS_REFRESH_FREQ * APP_TIMEOUT_DELAY_MS)) {
			W_SYSVIEW_OnTaskStartExec(I2C_TASK);
			_counter = 0;
			stc.refresh(nullptr);
			veml.refresh(nullptr);
			fxos_tasks(nullptr);
			baro.refresh(nullptr);
			W_SYSVIEW_OnTaskStopExec(I2C_TASK);
		}

		model_dispatch_sensors_update();
#endif

#ifndef BLE_STACK_SUPPORT_REQD
		CRITICAL_REGION_ENTER();
		neopixel_radio_callback_handler(false);
		CRITICAL_REGION_EXIT();
#endif
		// check screen update & unlock task
		if (millis() - vue.getLastRefreshed() > LS027_TIMEOUT_DELAY_MS) {
			vue.refresh();
		}

		// update date
		SDate dat;
		locator.getDate(dat);
		attitude.addNewDate(&dat);

		notifications_tasks();

		backlighting_tasks();

		events_wait(TASK_EVENT_PERIPH_TRIGGER);
	}
}


