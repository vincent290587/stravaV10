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



#ifndef _DEBUG_TWI

static uint32_t m_last_polled_index = 0;

// Buffer for data read from sensors.
static uint8_t m_veml_buffer[10];
static tSTC31000Data m_stc_buffer;
static fxos_handle_t m_fxos_handle;
static ms5637_handle_t m_ms5637_handle;

static void read_ms(void);

APP_TIMER_DEF(m_timer);


/**
 *
 */
static void _i2c_scheduling_sensors_post_init(void) {

	ms5637.setCx(&m_ms5637_handle);

	LOG_INFO("Sensors initialized");
}


/**
 *
 */
static void _i2c_scheduling_sensors_init() {

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
		I2C_WRITE     (MS5637_ADDR    , ms_config, sizeof(ms_config))
//		I2C_WRITE     (STC3100_ADDRESS, stc_config, sizeof(stc_config))
	};
	i2c_perform(NULL, sensors_init_transfers, sizeof(sensors_init_transfers) / sizeof(sensors_init_transfers[0]), NULL);

	// Init sensors configuration
	fxos_init();

	veml.on();

	// init configuration
	stc.init(STC3100_CUR_SENS_RES_MO);

	nrf_delay_ms(1);

	// Init sensors
	static uint8_t raw_data[2];

//	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND stc_config2[2];
//	stc_config2[0] = REG_MODE;
//	stc_config2[1] = stc._stc3100Mode;
//
//	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND stc_config3[1];
//	stc_config3[0] = REG_DEVICE_ID;
//	static uint8_t raw_data_stc[1];


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

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND ms_config2[7] = MS5637_INIT_REGS;

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND ms_config3[1] = MS5637_CMD_TEMP_REG;


	static nrf_twi_mngr_transfer_t const sensors_init_transfers3[] =
	{
			MS5637_INIT         (ms_config2, m_ms5637_handle.cx_data),
			MS5637_CMD_TEMP     (ms_config3)
//			I2C_READ_REG_NO_STOP(STC3100_ADDRESS, stc_config3, raw_data_stc, 1),
//			I2C_WRITE           (STC3100_ADDRESS, stc_config2, sizeof(stc_config2))
	};
	i2c_perform(NULL, sensors_init_transfers3, sizeof(sensors_init_transfers3) / sizeof(sensors_init_transfers3[0]), NULL);

	uint16_t res = raw_data[1] << 8;
	res |= raw_data[0];
	veml.init(res);

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

	if (m_last_polled_index == 0) {
		_i2c_scheduling_sensors_init();
	}

	LOG_DEBUG("Refreshing sensors t=%u ms\r\n", millis());

	fxos_tasks(&m_fxos_handle);

//	stc.refresh(&m_stc_buffer);

	veml.refresh(m_veml_buffer);

	// dispatch to model
	m_last_polled_index++;

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

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND veml_regs[5] = VEML_READ_ALL_REGS;

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND fxos_regs[2] = FXOS_READ_ALL_REGS;

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND stc_regs[1]  = STC_READ_ALL_REGS;

    static nrf_twi_mngr_transfer_t const transfers[] =
    {
//    	STC3100_READ_ALL (&stc_regs[0] , &m_stc_buffer.array[0]),
		VEML6075_READ_ALL(&veml_regs[0], &m_veml_buffer[0]),
		FXOS_READ_ALL    (&fxos_regs[0], &m_fxos_handle.mag_buffer[0], &m_fxos_handle.acc_buffer[0])
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

	APP_ERROR_CHECK(result);

	ASSERT(p_user_data);

	ms5637_handle_t *_handle = (ms5637_handle_t*)p_user_data;

	// state machine of the measurement
    switch (_handle->meas_type) {
    case eMS5637MeasCmdTemp:
    {
    	_handle->meas_type = eMS5637MeasCmdPress;

    	// temp has just been commanded, meaning we just got a pressure
    	//  measurement
    	// --> refresh device
    	ms5637.refresh(_handle);
    }
    	break;
    case eMS5637MeasCmdPress:
    {
    	_handle->meas_type = eMS5637MeasCmdTemp;
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
	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND ms_config[2] = MS5637_READ_TEMP_REG;

	static nrf_twi_mngr_transfer_t const transfers_temp[] =
	{
			MS5637_READ_TEMP(&m_ms5637_handle.temp_adc[0], ms_config)
	};
	static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction_temp =
	{
			.callback            = read_ms_cb,
			.p_user_data         = (void*)&m_ms5637_handle,
			.p_transfers         = transfers_temp,
			.number_of_transfers = sizeof(transfers_temp) / sizeof(transfers_temp[0])
	};

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND ms_config2[2] = MS5637_READ_PRESS_REG;

	static nrf_twi_mngr_transfer_t const transfers_press[] =
	{
			MS5637_READ_PRESS  (&m_ms5637_handle.press_adc[0], ms_config2)
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
	W_SYSVIEW_RecordEnterISR();

	m_last_polled_index++;

    read_ms();

    if (m_last_polled_index >= 1000 / (MS5637_REFRESH_PER_MS * SENSORS_REFRESH_FREQ)) {
    	m_last_polled_index = 1;

    	read_all();
    }

    W_SYSVIEW_RecordEnterISR();
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

    if (!m_last_polled_index) {
    	err_code = app_timer_create(&m_timer, APP_TIMER_MODE_REPEATED, timer_handler);
    	APP_ERROR_CHECK(err_code);

    	err_code = app_timer_start(m_timer, APP_TIMER_TICKS(MS5637_REFRESH_PER_MS), NULL);
    	APP_ERROR_CHECK(err_code);

    	m_last_polled_index = 1;
    }
#else
    stc.init(100);
    veml.init();
    ms5637.init();

    if (fxos_init()) LOG_ERROR("FXOS init fail");
#endif
}
