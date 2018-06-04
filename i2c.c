/*
 * I2C.c
 *
 *  Created on: 26 févr. 2017
 *      Author: Vincent
 */

#include "app_util_platform.h"
#include "app_error.h"
#include "nrf_drv_twi.h"
#include "nrf_gpio.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "i2c.h"

/* TWI instance ID. */
#define TWI_INSTANCE_ID     1

/**
 * @brief TWI master instance.
 *
 * Instance of TWI master driver that will be used for communication with simulated
 * EEPROM memory.
 */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);


// TWI (with transaction manager) initialization.
void i2c_init(void) {
    uint32_t err_code;

    nrf_drv_twi_config_t const twi_config = {
       .scl                = 9,
       .sda                = 10,
       .frequency          = NRF_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_LOW,
       .clear_bus_init     = true
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);

}

static const nrf_drv_twi_t* i2c_get_ref() {
	return &m_twi;
}

void i2c_scan() {

	uint8_t sample_data;
	uint32_t err_code;

	for (uint8_t address = 1; address <= 127; address++)
	{
		err_code = nrf_drv_twi_rx(i2c_get_ref(), address, &sample_data, sizeof(sample_data));
		if (err_code == NRF_SUCCESS)
		{
			NRF_LOG_INFO("TWI device detected at address 0x%x.\r\n", address);
		}
		NRF_LOG_FLUSH();
	}

}


/*******************************************************************************
 * Raw I2C Reads and Writes
 ******************************************************************************/

bool i2c_device_present(uint8_t address) {
	uint8_t val = 0;

	uint32_t err_code = nrf_drv_twi_tx(i2c_get_ref(), address, &val, 1, true);
	if (err_code != NRF_SUCCESS) {
		NRF_LOG_ERROR("TWI device not present @ 0x%x.\r\n", address);
		NRF_LOG_FLUSH();
		return false;
	} else {
		NRF_LOG_INFO("TWI device present @ 0x%x.\r\n", address);
		NRF_LOG_FLUSH();
	}

	return true;
}



bool i2c_read8(uint8_t address, uint8_t *val) {

	uint32_t err_code = nrf_drv_twi_rx(i2c_get_ref(), address, val, 1);
	if (err_code != NRF_SUCCESS) {
		NRF_LOG_INFO("TWI read8 problem at address 0x%x.\r\n", address);
		NRF_LOG_FLUSH();
		return false;
	}

	return true;
}


bool i2c_read_n(uint8_t address, uint8_t *val, unsigned int len) {

	uint32_t err_code = nrf_drv_twi_rx(i2c_get_ref(), address, val, len);
	if (err_code != NRF_SUCCESS) {
		NRF_LOG_INFO("TWI readn error 0x%X at address 0x%x.\r\n", err_code, address);
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
		NRF_LOG_INFO("TWI write8 error 0x%X at address 0x%x value= 0x%x.\r\n", err_code, address, val);
		NRF_LOG_FLUSH();
		return false;
	}

	return true;
}

bool i2c_write8_cont(uint8_t address, uint8_t val) {

	uint8_t sample_data = val;

	uint32_t err_code = nrf_drv_twi_tx(i2c_get_ref(), address, &sample_data, 1, false);
	if (err_code != NRF_SUCCESS) {
		NRF_LOG_INFO("TWI write8_c problem at address 0x%x.\r\n", address);
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
		NRF_LOG_INFO("TWI i2c_write_n problem at address 0x%x.\r\n", address);
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
		NRF_LOG_INFO("TWI write_reg_8 error 0x%X @ 0x%x reg=0x%x val=0x%x.\r\n", err_code, address, reg, val);
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
		NRF_LOG_INFO("TWI read_reg_8 problem at address 0x%x.\r\n", address);
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
		NRF_LOG_INFO("TWI read_reg_n error 0x%X at address 0x%x.\r\n", err_code, address);
		NRF_LOG_FLUSH();
		return false;
	}

	return true;
}

