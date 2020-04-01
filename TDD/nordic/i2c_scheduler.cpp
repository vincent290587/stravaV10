/*
 * i2c_scheduler.c
 *
 *  Created on: 10 déc. 2017
 *      Author: Vincent
 */

#include <i2c_scheduler.h>
#include "segger_wrapper.h"
#include "parameters.h"
#include "millis.h"
#include "fxos.h"
#include "fram.h"
#include "Model.h"

#define I2C_SCHEDULING_PERIOD_MS      (BARO_REFRESH_PER_MS)

static uint32_t m_fxos_updated = 0;
static uint32_t m_last_polled_index = 0;

void bsp_tasks() {

}


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

#ifdef VEML_PRESENT
	veml.init();
#endif

	// init configuration
	stc.init(STC3100_CUR_SENS_RES_MO);

#ifdef FRAM_PRESENT
	fram_init_sensor();
#endif

	baro.sensorInit();

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

	if (boucle__get_mode() != eBoucleGlobalModesFEC) baro.sensorRead();

	if (++m_last_polled_index >= SENSORS_REFRESH_PER_MS / I2C_SCHEDULING_PERIOD_MS) {
		m_last_polled_index = 0;

		stc.readChip();

#ifdef VEML_PRESENT
		veml.readChip();
#endif

		if (boucle__get_mode() != eBoucleGlobalModesFEC) fxos_readChip();
	}

    W_SYSVIEW_RecordExitISR();
}


/**
 *
 */
void i2c_scheduling_init(void) {

	_i2c_scheduling_sensors_init();
	_i2c_scheduling_sensors_post_init();

}

void i2c_scheduling_tasks(void) {
#ifndef _DEBUG_TWI
	if (baro.isUpdated()) {
		sysview_task_void_enter(I2cMgmtReadMs);
		baro.sensorRefresh();
		sysview_task_void_exit(I2cMgmtReadMs);

		if (m_fxos_updated) {
			m_fxos_updated = 0;

			attitude.computeFusion();
		}
	}
	if (is_fxos_updated()) {
		sysview_task_void_enter(I2cMgmtRead1);
		fxos_tasks();
		m_fxos_updated = 1;
		sysview_task_void_exit(I2cMgmtRead1);
	}
#ifdef VEML_PRESENT
	if (is_veml_updated()) {
		sysview_task_void_enter(I2cMgmtRead2);
		veml.refresh();
		sysview_task_void_exit(I2cMgmtRead2);
	}
#endif
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
#ifdef VEML_PRESENT
		veml.refresh(nullptr);
#endif
		if (boucle__get_mode() != eBoucleGlobalModesFEC) fxos_tasks(nullptr);
		if (boucle__get_mode() != eBoucleGlobalModesFEC) baro.refresh(nullptr);
	}
#endif

	// dispatch to model
	model_dispatch_sensors_update();
}
