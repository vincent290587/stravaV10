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
#include "nrf_assert.h"
#include "nrf_delay.h"
#include "fxos.h"
#include "bme280.h"
#include "app_timer.h"
#include "Model.h"



#ifndef _DEBUG_TWI

#define I2C_SCHEDULING_PERIOD_MS      (MS5637_REFRESH_PER_MS)

static uint32_t m_last_polled_index = 0;

// Buffer for data read from sensors.
static uint8_t m_veml_buffer[10];
static tSTC31000Data m_stc_buffer;
static fxos_handle_t m_fxos_handle;

static volatile bool m_is_bme280_updated = false;
static volatile bool m_is_fxos_updated = false;
static volatile bool m_is_stc_veml_updated = false;

static void read_ms(void);

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

	// Reset sensors
	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND stc_config[2] = STC_RESET_REGS;

	veml.off();

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND veml_config[3];
	veml_config[0] = VEML6075_REG_CONF;
	veml_config[1] = (uint8_t) (veml.config & 0xFF);
	veml_config[2] = (uint8_t)((0xFF00 & veml.config) >> 8);

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND ms_config[1] = {CMD_RESET};

	static nrf_twi_mngr_transfer_t const sensors_init_transfers[] =
	{
		I2C_WRITE     (VEML6075_ADDR  , veml_config, sizeof(veml_config)),
		I2C_WRITE     (MS5637_ADDR    , ms_config, sizeof(ms_config)),
		I2C_WRITE     (STC3100_ADDRESS, stc_config, sizeof(stc_config))
	};
	i2c_perform(NULL, sensors_init_transfers, sizeof(sensors_init_transfers) / sizeof(sensors_init_transfers[0]), NULL);

	veml.on();

	// init configuration
	stc.init(STC3100_CUR_SENS_RES_MO);

	nrf_delay_ms(1);

	// Init sensors
	static uint8_t raw_data[2];

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND stc_config2[2];
	stc_config2[0] = REG_MODE;
	stc_config2[1] = stc._stc3100Mode;

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND stc_config3[1];
	stc_config3[0] = REG_DEVICE_ID;


	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND veml_config2[4];
	veml_config2[0] = VEML6075_REG_CONF;
	veml_config2[1] = (uint8_t) (veml.config & 0xFF);
	veml_config2[2] = (uint8_t)((0xFF00 & veml.config) >> 8);
	veml_config2[3] = VEML6075_REG_DEVID;

	static nrf_twi_mngr_transfer_t const sensors_init_transfers2[] =
	{
			VEML_WRITE_REG(&veml_config2[0]),
			VEML_READ_REG_REP_START(&veml_config2[3], &raw_data[0], 2)
	};
	i2c_perform(NULL, sensors_init_transfers2, sizeof(sensors_init_transfers2) / sizeof(sensors_init_transfers2[0]), NULL);

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND fxos_ctrl1[4];
	fxos_ctrl1[0] = CTRL_REG1;

	static uint8_t raw_data_stc[1];
	static uint8_t raw_data_fxos[1];

	static nrf_twi_mngr_transfer_t const sensors_init_transfers3[] =
	{
			I2C_READ_REG        (STC3100_ADDRESS, stc_config3, raw_data_stc, 1),
			I2C_WRITE           (STC3100_ADDRESS, stc_config2, sizeof(stc_config2)),
			I2C_READ_REG        (FXOS_7BIT_ADDRESS, fxos_ctrl1, raw_data_fxos, 1),
	};
	i2c_perform(NULL, sensors_init_transfers3, sizeof(sensors_init_transfers3) / sizeof(sensors_init_transfers3[0]), NULL);

	LOG_INFO("STC dev ID: 0x%02X", raw_data_stc[0]);
	LOG_INFO("FXOS CTRL1: 0x%02X", raw_data_fxos[0]);

	uint16_t res = raw_data[1] << 8;
	res |= raw_data[0];
	veml.init(res);

	bme280_init_sensor();

	// post-init steps
	_i2c_scheduling_sensors_post_init();
}

/**
 *
 * @param result
 * @param p_user_data
 */
static void read_all_cb(ret_code_t result, void * p_user_data) {

	APP_ERROR_CHECK(result);

	m_is_stc_veml_updated = true;

}

/**
 *
 * @param result
 * @param p_user_data
 */
static void read_fxos_cb(ret_code_t result, void * p_user_data) {

	APP_ERROR_CHECK(result);

	m_is_fxos_updated = true;

}

/**
 *
 */
static void read_all(void)
{
    // [these structures have to be "static" - they cannot be placed on stack
    //  since the transaction is scheduled and these structures most likely
    //  will be referred after this function returns]

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND veml_regs[5] = VEML_READ_ALL_REGS;

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND stc_regs[1]  = STC_READ_ALL_REGS;

    static nrf_twi_mngr_transfer_t const transfers[] =
    {
    	STC3100_READ_ALL (&stc_regs[0] , &m_stc_buffer.array[0]),
		VEML6075_READ_ALL(&veml_regs[0], &m_veml_buffer[0])
    };
    static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
    {
        .callback            = read_all_cb,
        .p_user_data         = NULL,
        .p_transfers         = transfers,
        .number_of_transfers = sizeof(transfers) / sizeof(transfers[0])
    };

    i2c_schedule(&transaction);

}

/**
 *
 */
static void read_fxos(void)
{
    // [these structures have to be "static" - they cannot be placed on stack
    //  since the transaction is scheduled and these structures most likely
    //  will be referred after this function returns]

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND fxos_regs[2] = FXOS_READ_ALL_REGS;

    static nrf_twi_mngr_transfer_t const transfers[] =
    {
		FXOS_READ_ALL    (&fxos_regs[0], &m_fxos_handle.mag_buffer[0], &m_fxos_handle.acc_buffer[0])
    };
    static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
    {
        .callback            = read_fxos_cb,
        .p_user_data         = NULL,
        .p_transfers         = transfers,
        .number_of_transfers = sizeof(transfers) / sizeof(transfers[0])
    };

    i2c_schedule(&transaction);

}

/**
 *
 */
static void read_ms(void) {

	// [these structures have to be "static" - they cannot be placed on stack
	//  since the transaction is scheduled and these structures most likely
	//  will be referred after this function returns]
	bme280_read_sensor();
}

/**
 *
 * @param p_context
 */
static void timer_handler(void * p_context)
{
	W_SYSVIEW_RecordEnterISR();

    if (boucle.getGlobalMode() != eBoucleGlobalModesFEC) read_ms();

    if (++m_last_polled_index >= SENSORS_REFRESH_PER_MS / I2C_SCHEDULING_PERIOD_MS) {
    	m_last_polled_index = 0;

    	read_all();

    	if (boucle.getGlobalMode() != eBoucleGlobalModesFEC) read_fxos();
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

	nrf_delay_ms(3);

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
	if (m_is_bme280_updated) {
		m_is_bme280_updated = false;
		sysview_task_void_enter(I2cMgmtReadMs);
		baro.runFilter();
		sysview_task_void_exit(I2cMgmtReadMs);
	}
	if (m_is_fxos_updated) {
		sysview_task_void_enter(I2cMgmtRead1);
		m_is_fxos_updated = false;
		fxos_tasks(&m_fxos_handle);
		sysview_task_void_exit(I2cMgmtRead1);
	}
	if (m_is_stc_veml_updated) {
		sysview_task_void_enter(I2cMgmtRead2);
		m_is_stc_veml_updated = false;
		stc.refresh(&m_stc_buffer);
		veml.refresh(m_veml_buffer);
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
