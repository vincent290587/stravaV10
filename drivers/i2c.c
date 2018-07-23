/*
 * I2C.c
 *
 *  Created on: 26 févr. 2017
 *      Author: Vincent
 */

#include "boards.h"
#include "parameters.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "app_timer.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "Model.h"
#include "segger_wrapper.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "i2c.h"

/* TWI instance ID. */
#define TWI_INSTANCE_ID     1


#define MAX_PENDING_TRANSACTIONS    40

NRF_TWI_MNGR_DEF(m_nrf_twi_mngr, MAX_PENDING_TRANSACTIONS, TWI_INSTANCE_ID);

#ifdef _DEBUG_TWI
/* Number of possible TWI addresses. */
#define TWI_ADDRESSES      127

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
#endif

/**
 *
 */
void i2c_init(void) {

	ret_code_t err_code;

    nrf_drv_twi_config_t const config = {
       .scl                = SCL_PIN_NUMBER,
       .sda                = SDA_PIN_NUMBER,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_LOWEST,
       .clear_bus_init     = true,
	   .hold_bus_uninit    = true
    };

#ifndef _DEBUG_TWI
    err_code = nrf_twi_mngr_init(&m_nrf_twi_mngr, &config);
    APP_ERROR_CHECK(err_code);
#else
    err_code = nrf_drv_twi_init(&m_twi, &config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);

    nrf_delay_ms(10);

#endif
}

void i2c_scan(void) {

#ifdef _DEBUG_TWI
	ret_code_t err_code;

    uint8_t sample_data = 0x1;

    err_code = nrf_drv_twi_rx(&m_twi, 0x76, &sample_data, sizeof(sample_data));
    if (err_code == NRF_SUCCESS)
    {
    	LOG_INFO("TWI device detected at address 0x%X", 0x76);
    }
    NRF_LOG_FLUSH();

    err_code = nrf_drv_twi_rx(&m_twi, 0x10, &sample_data, sizeof(sample_data));
    if (err_code == NRF_SUCCESS)
    {
    	LOG_INFO("TWI device detected at address 0x%X", 0x10);
    }
    NRF_LOG_FLUSH();

    err_code = nrf_drv_twi_rx(&m_twi, 0x1E, &sample_data, sizeof(sample_data));
    if (err_code == NRF_SUCCESS)
    {
    	LOG_INFO("TWI device detected at address 0x%X", 0x1E);
    }
    NRF_LOG_FLUSH();
#endif
}

/**
 *
 * @param p_transaction
 */
void i2c_schedule(nrf_twi_mngr_transaction_t const * p_transaction) {

	/* Start master transfer */
	ret_code_t ret_val = nrf_twi_mngr_schedule(&m_nrf_twi_mngr, p_transaction);
	APP_ERROR_CHECK(ret_val);
}


/**
 *
 * @param p_transaction
 */
void i2c_perform(nrf_drv_twi_config_t const *    p_config,
        nrf_twi_mngr_transfer_t const * p_transfers,
        uint8_t                         number_of_transfers,
        void                            (* user_function)(void)) {

	/* Start master transfer */
	ret_code_t ret_val = nrf_twi_mngr_perform(&m_nrf_twi_mngr,
			p_config,
			p_transfers,
			number_of_transfers,
			user_function);
	APP_ERROR_CHECK(ret_val);
}


#ifdef _DEBUG_TWI

/*******************************************************************************
 * Raw I2C Reads and Writes
 ******************************************************************************/

static const nrf_drv_twi_t* i2c_get_ref() {
	return &m_twi;
}


bool i2c_read8(uint8_t address, uint8_t *val) {

	uint32_t err_code = nrf_drv_twi_rx(i2c_get_ref(), address, val, 1);
	if (err_code != NRF_SUCCESS) {
		LOG_INFO("TWI read8 problem at address 0x%x.", address);
		NRF_LOG_FLUSH();
		return false;
	}

	return true;
}


bool i2c_read_n(uint8_t address, uint8_t *val, unsigned int len) {

	uint32_t err_code = nrf_drv_twi_rx(i2c_get_ref(), address, val, len);
	if (err_code != NRF_SUCCESS) {
		LOG_INFO("TWI readn error 0x%X at address 0x%x.", err_code, address);
		NRF_LOG_FLUSH();
		return false;
	}

	return true;
}

/**
 * @brief Writes a single byte to the I2C device (no register)
 *
 * @param[in] val the 1-byte value to write to the I2C device
 * @return True if successful write operation. False otherwise.
 */
bool i2c_write8(uint8_t address, uint8_t val) {

	uint8_t sample_data = val;

	uint32_t err_code = nrf_drv_twi_tx(i2c_get_ref(), address, &sample_data, 1, true);
	if (err_code != NRF_SUCCESS) {
		LOG_INFO("TWI write8 error 0x%X at address 0x%x value= 0x%x.", err_code, address, val);
		NRF_LOG_FLUSH();
		return false;
	}

	return true;
}

bool i2c_write8_cont(uint8_t address, uint8_t val) {

	uint8_t sample_data = val;

	uint32_t err_code = nrf_drv_twi_tx(i2c_get_ref(), address, &sample_data, 1, false);
	if (err_code != NRF_SUCCESS) {
		LOG_INFO("TWI write8_c problem at address 0x%x.", address);
		NRF_LOG_FLUSH();
		return false;
	}

	return true;
}


/**
 * @brief Writes a block (array) of bytes from the I2C device and register
 *
 * @param[in] reg the register to read from
 * @param[out] val pointer to the beginning of the data
 * @param[in] len number of bytes to read
 * @return Number of bytes read. -1 on read error.
 */
bool i2c_write_n(uint8_t address, uint8_t *val, unsigned int len) {

	/* Indicate which register we want to write from */
	ret_code_t err_code = nrf_drv_twi_tx(i2c_get_ref(), address, val, len, true);

	if (err_code != NRF_SUCCESS) {
		LOG_INFO("TWI i2c_write_n problem at address 0x%x.", address);
		NRF_LOG_FLUSH();
		return false;
	}

	return true;
}


/**
 * @brief Writes a single byte to the I2C device and specified register
 *
 * @param[in] reg the register in the I2C device to write to
 * @param[in] val the 1-byte value to write to the I2C device
 * @return True if successful write operation. False otherwise.
 */
bool i2c_write_reg_8(uint8_t address, uint8_t reg, uint8_t val) {

	uint8_t sample_data[2] = {reg, val};

	uint32_t err_code = nrf_drv_twi_tx(i2c_get_ref(), address, sample_data, 2, true);
	if (err_code != NRF_SUCCESS) {
		LOG_INFO("TWI write_reg_8 error 0x%X @ 0x%x reg=0x%x val=0x%x.", err_code, address, reg, val);
		NRF_LOG_FLUSH();
		return false;
	}

	return true;
}

/**
 * @brief Writes a single byte to the I2C device and specified register
 *
 * @param[in] reg the register in the I2C device to write to
 * @param[in] val the 1-byte value to write to the I2C device
 * @return True if successful write operation. False otherwise.
 */
bool i2c_read_reg_8(uint8_t address, uint8_t reg, uint8_t *val) {

	/* Indicate which register we want to read from */
	ret_code_t err_code = nrf_drv_twi_tx(i2c_get_ref(), address, &reg, 1, false);

	// read from I2C
	err_code |= nrf_drv_twi_rx(i2c_get_ref(), address, val, 1);
	if (err_code != NRF_SUCCESS) {
		LOG_INFO("TWI read_reg_8 problem at address 0x%x.", address);
		NRF_LOG_FLUSH();
		return false;
	}

	return true;
}

/**
 * @brief Reads a single byte from the I2C device and specified register
 *
 * @param[in] reg the register to read from
 * @param[out] the value returned from the register
 * @return True if successful read operation. False otherwise.
 */
bool i2c_read_reg_n(uint8_t address, uint8_t reg, uint8_t *val, unsigned int len) {

	/* Indicate which register we want to read from */
	ret_code_t err_code = nrf_drv_twi_tx(i2c_get_ref(), address, &reg, 1, false);

	// read from I2C
	err_code |= nrf_drv_twi_rx(i2c_get_ref(), address, val, len);
	if (err_code != NRF_SUCCESS) {
		LOG_INFO("TWI read_reg_n error 0x%X at address 0x%x.", err_code, address);
		NRF_LOG_FLUSH();
		return false;
	}

	return true;
}

#endif
