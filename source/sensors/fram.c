/*
 * fram.c
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#include <string.h>
#include "fram.h"
#include "utils.h"
#include "nordic_common.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

#if defined(FRAM_PRESENT)

#include "i2c.h"
#include "nrf_twi_mngr.h"

#define FRAM_TWI_ADDRESS       0b10100000

#define I2C_READ_REG(addr, p_reg_addr, p_buffer, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, NRF_TWI_MNGR_NO_STOP), \
		NRF_TWI_MNGR_READ (addr, p_buffer, byte_cnt, 0)

#define I2C_READ_REG_REP_STOP(addr, p_reg_addr, p_buffer, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, 0), \
		NRF_TWI_MNGR_READ (addr, p_buffer, byte_cnt, 0)

#define I2C_WRITE(addr, p_data, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_data, byte_cnt, 0)


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void fram_init_sensor() {

	LOG_WARNING("FRAM init done");

}

bool fram_read_block(uint16_t block_addr, uint8_t *readout, uint16_t length) {

	uint8_t twi_address = FRAM_TWI_ADDRESS;
	twi_address |= (block_addr & 0x100) >> 7;

	uint8_t address = block_addr & 0x0FF;

	{
		nrf_twi_mngr_transfer_t const fram_xfer[] =
		{
				I2C_READ_REG_REP_STOP(twi_address, &address, readout, length)
		};

		i2c_perform(NULL, fram_xfer, sizeof(fram_xfer) / sizeof(fram_xfer[0]), NULL);
	}

	return true;
}

bool fram_write_block(uint16_t block_addr, uint8_t *writeout, uint16_t length) {

	uint8_t twi_address = FRAM_TWI_ADDRESS;
	twi_address |= (block_addr & 0x100) >> 7;

	if (length > 254) return false;

	uint8_t p_data[256] = {0};
	p_data[0] = block_addr & 0x0FF;

	memcpy(&p_data[1], writeout, length);
	length++;

	{
		nrf_twi_mngr_transfer_t const fram_xfer[] =
		{
				I2C_WRITE(twi_address, p_data, length)
		};

		i2c_perform(NULL, fram_xfer, sizeof(fram_xfer) / sizeof(fram_xfer[0]), NULL);
	}

	return true;
}

#else

#include "g_structs.h"

static sUserParameters m_params;


void fram_init_sensor() {

	m_params.hrm_devid = 0x0D22;
	m_params.bsc_devid = 0xB02B;
	m_params.fec_devid = 2766U;
	m_params.version = 1U;

	m_params.crc = calculate_crc(&m_params.flat_user_params, sizeof(sUserParameters) - 1);

	LOG_WARNING("FRAM init done");

}

bool fram_read_block(uint16_t block_addr, uint8_t *readout, uint16_t length) {

	uint16_t res_len = MIN(sizeof(m_params), length);

	LOG_INFO("FRAM reading %u bytes", res_len);

	// copy as much as we can
	memcpy(readout, &m_params.flat_user_params, res_len);

	return true;
}

bool fram_write_block(uint16_t block_addr, uint8_t *writeout, uint16_t length) {

	uint16_t res_len = MIN(sizeof(m_params), length);

	LOG_INFO("FRAM writing %u bytes", res_len);

	// copy as much as we can
	memcpy(&m_params.flat_user_params, writeout, res_len);

	return true;
}

#endif
