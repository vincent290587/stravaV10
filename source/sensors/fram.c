/*
 * fram.c
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */


#include "fram.h"
#include "i2c.h"
#include "nrf_twi_mngr.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

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

bool fram_read_block(uint16_t block_addr, uint8_t *readout, size_t length) {

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

bool fram_write_block(uint16_t block_addr, uint8_t *writeout, size_t length) {

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
