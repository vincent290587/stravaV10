/*
 * i2c_scheduler.c
 *
 *  Created on: 10 déc. 2017
 *      Author: Vincent
 */

#include "i2c.h"
#include <i2c_scheduler.h>
#include "segger_wrapper.h"
#include "parameters.h"
#include "millis.h"
#include "millis.h"
#include "fxos.h"
#include "fram.h"
#include "bme280.h"
#include "app_timer.h"
#include "Model.h"



#ifndef _DEBUG_TWI

#define I2C_SCHEDULING_PERIOD_MS      (BARO_REFRESH_PER_MS)

static uint32_t m_last_polled_index = 0;

APP_TIMER_DEF(m_timer);


/**
 *
 */
static void _i2c_scheduling_sensors_post_init(void) {

	LOG_WARNING("Sensors initialized");
}


/**
 *
 */
static void _i2c_scheduling_sensors_init() {

	// Init sensors configuration
	fxos_init();

	veml.init();

	// init configuration
	stc.init(STC3100_CUR_SENS_RES_MO);

#ifdef FRAM_PRESENT
	fram_init_sensor();
#endif
  
	bme280_init_sensor();

	// post-init steps
	_i2c_scheduling_sensors_post_init();
}

/**
 *
 * @param p_context
 */
static void timer_handler(void * p_context)
{
	W_SYSVIEW_RecordEnterISR();

	if (boucle.getGlobalMode() != eBoucleGlobalModesFEC)
		bme280_read_sensor();

	if (++m_last_polled_index >= SENSORS_REFRESH_PER_MS / I2C_SCHEDULING_PERIOD_MS) {
		m_last_polled_index = 0;

		stc.readChip();

		veml.readChip();

		if (boucle.getGlobalMode() != eBoucleGlobalModesFEC)
			fxos_readChip();
	}

    W_SYSVIEW_RecordExitISR();
}

#endif

/**
 *
 */
void i2c_scheduling_init(void) {
#ifndef _DEBUG_TWI

	_i2c_scheduling_sensors_init();

	delay_ms(3);

	ret_code_t err_code;
	err_code = app_timer_create(&m_timer, APP_TIMER_MODE_REPEATED, timer_handler);
	APP_ERROR_CHECK(err_code);

	err_code = app_timer_start(m_timer, APP_TIMER_TICKS(I2C_SCHEDULING_PERIOD_MS), NULL);
	APP_ERROR_CHECK(err_code);

#else
    stc.init(100);
    veml.init();
    baro.init();

    if (fxos_init()) LOG_ERROR("FXOS init fail");
#endif
}

void i2c_scheduling_tasks(void) {
#ifndef _DEBUG_TWI
	if (is_bme280_updated()) {
		sysview_task_void_enter(I2cMgmtReadMs);
		bme280_refresh();
		sysview_task_void_exit(I2cMgmtReadMs);
	}
	if (is_fxos_updated()) {
		sysview_task_void_enter(I2cMgmtRead1);
		fxos_tasks();
		sysview_task_void_exit(I2cMgmtRead1);
	}
	if (is_veml_updated()) {
		sysview_task_void_enter(I2cMgmtRead2);
		veml.refresh();
		sysview_task_void_exit(I2cMgmtRead2);
	}
	if (is_stc_updated()) {
		sysview_task_void_enter(I2cMgmtRead2);
		stc.refresh();
		sysview_task_void_exit(I2cMgmtRead2);
	}
#else
	static uint32_t _counter = 0;

	if (++_counter >= SENSORS_REFRESH_PER_MS / APP_TIMEOUT_DELAY_MS) {
		_counter = 0;
		stc.refresh(nullptr);
		veml.refresh(nullptr);
		if (boucle.getGlobalMode() != eBoucleGlobalModesFEC) fxos_tasks(nullptr);
		if (boucle.getGlobalMode() != eBoucleGlobalModesFEC) baro.refresh(nullptr);
	}
#endif

	// dispatch to model
	model_dispatch_sensors_update();
}
