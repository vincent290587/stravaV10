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
#include "app_timer.h"
#include "Model.h"

static uint32_t m_last_polled_time = 0;

// Buffer for data read from sensors.
static uint8_t m_veml_buffer[10];
static tSTC31000Data m_stc_buffer;
static ms5637_handle_t m_ms5637_handle;
static fxos_handle_t m_fxos_handle;


static void read_ms(void);

APP_TIMER_DEF(m_timer);


/**
 *
 */
static void _i2c_scheduling_sensors_post_init(void) {

	ms5637.setCx(&m_ms5637_handle);

	// start gathering pressure data
	read_ms();

}


/**
 *
 */
static void _i2c_scheduling_sensors_init() {

	m_last_polled_time = 1;

	// Reset sensors
	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND stc_config[2] = {REG_CONTROL, STC_RESET};

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND veml_config[2];
	veml_config[0] = VEML6075_REG_CONF;
	veml_config[1] = veml.config;

	static nrf_twi_mngr_transfer_t const sensors_init_transfers[] =
	{
	    NRF_TWI_MNGR_WRITE(STC3100_ADDRESS, stc_config, sizeof(stc_config), 0),
		NRF_TWI_MNGR_WRITE(VEML6075_ADDR, veml_config, sizeof(veml_config), 0),
		MS5637_RESET()
	};
	i2c_perform(NULL, sensors_init_transfers, sizeof(sensors_init_transfers) / sizeof(sensors_init_transfers[0]), NULL);

	// init sensors configuration
	fxos_init();

	veml.init();

	// init configuration
	stc.init(STC3100_CUR_SENS_RES_MO);

	// Init sensors
	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND veml_config2[2];
	veml_config2[0] = VEML6075_REG_CONF;
	veml_config2[1] = veml.config;

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND stc_config2[2];
	stc_config2[0] = REG_MODE;
	stc_config2[1] = stc._stc3100Mode;

	static nrf_twi_mngr_transfer_t const sensors_init_transfers2[] =
	{
			NRF_TWI_MNGR_WRITE(STC3100_ADDRESS, stc_config2, sizeof(stc_config2), 0),
			NRF_TWI_MNGR_WRITE(VEML6075_ADDR,  veml_config2, sizeof(veml_config2), 0),
			MS5637_INIT(m_ms5637_handle.cx_data),
			MS5637_CMD_TEMP()
	};
	i2c_perform(NULL, sensors_init_transfers2, sizeof(sensors_init_transfers2) / sizeof(sensors_init_transfers2[0]), NULL);

	// post-init steps
	_i2c_scheduling_sensors_post_init();
}

/**
 *
 * @param result
 * @param p_user_data
 */
static void read_all_cb(ret_code_t result, void * p_user_data) {

	if (m_last_polled_time == 0) {
		_i2c_scheduling_sensors_init();
	}

	LOG_DEBUG("Refreshing sensors t=%u ms\r\n", millis());

	fxos_tasks(&m_fxos_handle);

	stc.refresh(&m_stc_buffer);

	veml.refresh(m_veml_buffer);

	// dispatch to model
	m_last_polled_time = millis();

	model_dispatch_sensors_update();

}

/**
 *
 */
static void read_all(void)
{
    // [these structures have to be "static" - they cannot be placed on stack
    //  since the transaction is scheduled and these structures most likely
    //  will be referred after this function returns]
    static nrf_twi_mngr_transfer_t const transfers[] =
    {
    	STC3100_READ_ALL (&m_stc_buffer.array[0]),
		VEML6075_READ_ALL(&m_veml_buffer[0]),
		FXOS_READ_ALL    (&m_fxos_handle.mag_buffer[0], &m_fxos_handle.acc_buffer[0])
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
 * @param result
 * @param p_user_data
 */
static void read_ms_cb(ret_code_t result, void * p_user_data) {

	ASSERT(p_user_data);

	// state machine of the measurement
    switch (m_ms5637_handle.meas_type) {
    case eMS5637MeasCmdTemp:
    {
    	m_ms5637_handle.meas_type = eMS5637MeasCmdPress;

    	// temp has just been commanded, meaning we just got a pressure
    	//  measurement
    	// --> refresh device
    	ms5637_handle_t *_handle = (ms5637_handle_t*)p_user_data;
    	ms5637.refresh(_handle);
    }
    	break;
    case eMS5637MeasCmdPress:
    {
    	m_ms5637_handle.meas_type = eMS5637MeasCmdTemp;
    }
    	break;
    default:
    	break;
    }

}

/**
 *
 */
static void read_ms(void) {

	// [these structures have to be "static" - they cannot be placed on stack
	//  since the transaction is scheduled and these structures most likely
	//  will be referred after this function returns]
	static nrf_twi_mngr_transfer_t const transfers_temp[] =
	{
			MS5637_READ_TEMP  (&m_ms5637_handle.temp_adc[0], OSR_8192)
	};
	static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction_temp =
	{
			.callback            = read_ms_cb,
			.p_user_data         = (void*)&m_ms5637_handle,
			.p_transfers         = transfers_temp,
			.number_of_transfers = sizeof(transfers_temp) / sizeof(transfers_temp[0])
	};

	static nrf_twi_mngr_transfer_t const transfers_press[] =
	{
			MS5637_READ_PRESS  (&m_ms5637_handle.press_adc[0], OSR_8192)
	};
	static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction_press =
	{
			.callback            = read_ms_cb,
			.p_user_data         = (void*)&m_ms5637_handle,
			.p_transfers         = transfers_press,
			.number_of_transfers = sizeof(transfers_press) / sizeof(transfers_press[0])
	};

	// state machine of the measurement
	switch (m_ms5637_handle.meas_type) {

	case eMS5637MeasCmdTemp:
	{
		i2c_schedule(&transaction_temp);
	}
	break;

	case eMS5637MeasCmdPress:
	default:
	{
		i2c_schedule(&transaction_press);
	}
	break;
	}
}

/**
 *
 * @param p_context
 */
static void timer_handler(void * p_context)
{
	m_last_polled_time++;

    read_ms();

    if (m_last_polled_time >= 1000 / (MS5637_REFRESH_PER_MS * SENSORS_REFRESH_FREQ)) {
    	m_last_polled_time = 1;

    	read_all();
    }
}

/**
 *
 */
void i2c_scheduling_init(void) {

	m_last_polled_time = 0;

	_i2c_scheduling_sensors_init();

	nrf_delay_ms(3);

    ret_code_t err_code;

    err_code = app_timer_create(&m_timer, APP_TIMER_MODE_REPEATED, timer_handler);
    APP_ERROR_CHECK(err_code);

    err_code = app_timer_start(m_timer, APP_TIMER_TICKS(MS5637_REFRESH_PER_MS), NULL);
    APP_ERROR_CHECK(err_code);
}
