/*
 * fram.c
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#include <string.h>
#include "fram.h"
#include "utils.h"
#include "millis.h"
#include "parameters.h"
#include "nordic_common.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

#if defined(FRAM_PRESENT)

#include "i2c.h"
#include "nrf_twi_mngr.h"

#define FRAM_TWI_ADDRESS       0b1010000

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
	twi_address |= (block_addr & 0x700) >> 8;

	uint8_t address = block_addr & 0x0FF;

	{
		nrf_twi_mngr_transfer_t const fram_xfer[] =
		{
				I2C_READ_REG(twi_address, &address, readout, length)
		};

		i2c_perform(NULL, fram_xfer, sizeof(fram_xfer) / sizeof(fram_xfer[0]), NULL);
	}

	return true;
}

bool fram_write_block(uint16_t block_addr, uint8_t *writeout, uint16_t length) {

	uint8_t twi_address = FRAM_TWI_ADDRESS;
	twi_address |= (block_addr & 0x700) >> 8;

	uint8_t p_data[128] = {0};
	if (length > sizeof(p_data) - 1) return false;

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

#elif defined( FDS_PRESENT ) && !defined(TDD)


#include "fds.h"
#include "Model.h"
#include "UserSettings.h"

/* File ID and Key used for the configuration record. */

#define CONFIG_FILE     (0xF010)
#define CONFIG_REC_KEY  (0x7010)

/* Flag to check fds initialization. */
static bool volatile m_fds_initialized;
static bool volatile m_fds_dl_pending;
static bool volatile m_fds_wr_pending;
static bool volatile m_fds_gc_pending;


static void fds_evt_handler(fds_evt_t const * p_evt)
{
	switch (p_evt->id)
	{
	case FDS_EVT_INIT:
		if (p_evt->result == FDS_SUCCESS)
		{
			m_fds_initialized = true;
		}
		break;

	case FDS_EVT_GC:
	{
		if (p_evt->result == FDS_SUCCESS)
		{
			LOG_INFO("FDS GC success");
		} else {
			LOG_ERROR("FDS GC error");
		}
		m_fds_gc_pending = false;
	} break;

	case FDS_EVT_WRITE:
	{
		if (p_evt->result != FDS_SUCCESS)
		{
			LOG_ERROR("Record write error");
		}
		m_fds_wr_pending = false;
	} break;

	case FDS_EVT_UPDATE:
	{
		if (p_evt->result != FDS_SUCCESS)
		{
			LOG_ERROR("Record update error");
		} else {

			APP_ERROR_CHECK(fds_gc());

		}
		m_fds_wr_pending = false;
	} break;

	case FDS_EVT_DEL_RECORD:
	{
		if (p_evt->result != FDS_SUCCESS)
		{
			LOG_ERROR("Record delete error");
		}
		m_fds_dl_pending = false;
	} break;


	default:
		break;
	}
}

void fram_init_sensor() {

	ret_code_t rc;

	/* Register first to receive an event when initialization is complete. */
	rc = fds_register(fds_evt_handler);
	APP_ERROR_CHECK(rc);

	rc = fds_init();
	APP_ERROR_CHECK(rc);

	LOG_WARNING("FRAM init pending...");

	while (!m_fds_initialized)
	{
		perform_system_tasks_light();
	}

	LOG_WARNING("FRAM init done");

}

bool fram_read_block(uint16_t block_addr, uint8_t *readout, uint16_t length) {

	fds_record_desc_t desc = {0};
	fds_find_token_t  tok  = {0};

	ASSERT(m_fds_initialized);

	LOG_DEBUG("Reading config file...");

	ret_code_t rc = fds_record_find(CONFIG_FILE, CONFIG_REC_KEY, &desc, &tok);

	if (rc == FDS_SUCCESS)
	{
		/* A config file is in flash. Let's update it. */
		fds_flash_record_t config = {0};

		/* Open the record and read its contents. */
		rc = fds_record_open(&desc, &config);
		APP_ERROR_CHECK(rc);

		if (rc) return false;

		/* Copy the configuration from flash into readout. */
		length = MIN(sizeof(sUserParameters), length);
		memcpy(readout, config.p_data, length);

		/* Close the record when done reading. */
		rc = fds_record_close(&desc);
		APP_ERROR_CHECK(rc);

		return true;
	}

	LOG_ERROR("FDS record not found");

	return true;
}

bool fram_write_block(uint16_t block_addr, uint8_t *writeout, uint16_t length) {

	ret_code_t rc = FDS_SUCCESS;
	fds_record_desc_t desc = {0};
	fds_find_token_t  tok  = {0};

	ASSERT(m_fds_initialized);

	LOG_DEBUG("Writing config file...");

	fds_record_t _record =
	{
			.file_id           = CONFIG_FILE,
			.key               = CONFIG_REC_KEY,
			.data.p_data       = writeout,
			/* The length of a record is always expressed in 4-byte units (words). */
			.data.length_words = (length + 3) / sizeof(uint32_t),
	};

	rc = fds_record_find(CONFIG_FILE, CONFIG_REC_KEY, &desc, &tok);

	if (rc == FDS_SUCCESS) {

		rc = fds_record_update(&desc, &_record);
		APP_ERROR_CHECK(rc);

	} else {

		rc = fds_record_write(&desc, &_record);
		APP_ERROR_CHECK(rc);

	}

	// return if write failed
	if (rc) return false;

	m_fds_wr_pending = true;

	uint32_t time_prev = millis();
	while (m_fds_wr_pending) {
		perform_system_tasks_light();

		// timeout
		if (millis() - time_prev > 50) {
			APP_ERROR_CHECK(0x2);
			return false;
		}
	}

	LOG_DEBUG("Written !");

	return true;
}

#else

#include "g_structs.h"
#include "UserSettings.h"

static sUserParameters m_params;


void fram_init_sensor() {

	m_params.hrm_devid = 0x0D22;
	m_params.bsc_devid = 0xB02B;
	m_params.fec_devid = 2846U;
	m_params.version = 1U;
	m_params.FTP = USER_FTP;
	m_params.weight = USER_WEIGHT;

	m_params.crc = calculate_crc(&m_params.flat_user_params, sizeof(sUserParameters) - 1);

	LOG_WARNING("FRAM init done");

}

bool fram_read_block(uint16_t block_addr, uint8_t *readout, uint16_t length) {

	uint16_t res_len = MIN(sizeof(sUserParameters), length);

	LOG_INFO("FRAM reading %u bytes", res_len);

	// copy as much as we can
	memcpy(readout, &m_params.flat_user_params, res_len);

	return true;
}

bool fram_write_block(uint16_t block_addr, uint8_t *writeout, uint16_t length) {

	uint16_t res_len = MIN(sizeof(sUserParameters), length);

	LOG_INFO("FRAM writing %u bytes", res_len);

	// copy as much as we can
	memcpy(&m_params.flat_user_params, writeout, res_len);

	return true;
}

#endif
