/*
 * MS5637.cpp
 *
 *  Created on: 26 févr. 2017
 *      Author: Vincent
 */

#include "i2c.h"
#include "utils.h"
#include "Model.h"
#include "MS5637.h"
#include "millis.h"
#include "parameters.h"
#include "pgmspace.h"
#include "app_timer.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

/* error codes */
#define NO_ERR          0
#define ERR_NOREPLY     -1
#define ERR_BAD_READLEN -2
#define ERR_NEEDS_BEGIN -3

APP_TIMER_DEF(m_ms5637_update);


/* delay to wait for sampling to complete, on each OSR level */
static const uint8_t SamplingDelayMs[6] PROGMEM = { 2, 4, 6, 10, 18, 34 };

#define NAN (-1.)

#ifdef _DEBUG_TWI
static void ms5637_timer_callback(void * p_context) {

	sTasksIDs *_tasks_ids = (sTasksIDs *) p_context;

	if (_tasks_ids->peripherals_id != TASK_ID_INVALID) {
		events_set(_tasks_ids->peripherals_id, TASK_EVENT_PERIPH_MS_WAIT);
	}

}
#endif

MS5637::MS5637 () {

	 initialised = false;
	 m_err = ERR_NEEDS_BEGIN;
	 m_temperature = 20.;
	 m_pressure = 999.;

}

void MS5637::init(void) {

	initialised = false;

#ifdef _DEBUG_TWI
	this->setCx(nullptr);

	// Create timer.
	uint32_t err_code = app_timer_create(&m_ms5637_update, APP_TIMER_MODE_SINGLE_SHOT, ms5637_timer_callback);
	APP_ERROR_CHECK(err_code);
#endif

	return;
}


/**
 * \brief CRC check
 *
 * \param[in] uint16_t *: List of EEPROM coefficients
 * \param[in] uint8_t : crc to compare with
 *
 * \return bool : TRUE if CRC is OK, FALSE if KO
 */
bool MS5637::crc_check(uint16_t *n_prom, uint8_t crc) {
	uint8_t cnt, n_bit;
	uint16_t n_rem, crc_read;

	n_rem = 0x00;
	crc_read = n_prom[0];
	n_prom[MS5637_COEFFICIENT_COUNT] = 0;
	n_prom[0] = (0x0FFF & (n_prom[0])); // Clear the CRC byte

	for (cnt = 0; cnt < (MS5637_COEFFICIENT_COUNT + 1) * 2; cnt++) {

		// Get next byte
		if (cnt % 2 == 1)
			n_rem ^= n_prom[cnt >> 1] & 0x00FF;
		else
			n_rem ^= n_prom[cnt >> 1] >> 8;

		for (n_bit = 8; n_bit > 0; n_bit--) {

			if (n_rem & 0x8000)
				n_rem = (n_rem << 1) ^ 0x3000;
			else
				n_rem <<= 1;
		}
	}
	n_rem >>= 12;
	n_prom[0] = crc_read;

	return (n_rem == crc);
}

/**
 *
 * @param _handle
 * @return
 */
bool MS5637::setCx(ms5637_handle_t *_handle) {

	if (this->reset()) {
		LOG_ERROR("MS5637 reset failure");
		return false;
	}


	uint16_t prom[MS5637_COEFFICIENT_COUNT+1];

#ifdef _DEBUG_TWI
	uint8_t prom_val[3];

	for (int i = 0; i < MS5637_COEFFICIENT_COUNT; i++) {

		if (wireReadDataBlock(CMD_PROM_READ(i), prom_val, 2) != 2) {
			m_err = ERR_BAD_READLEN;
			return false;
		}

		prom[i] = ((uint16_t) prom_val[0]) << 8;
		prom[i] |= prom_val[1];
		NRF_LOG_DEBUG("Prom %u: 0x%X", i, prom[i]);
	}
#else
	for (int i = 0; i < MS5637_COEFFICIENT_COUNT; i++) {

		prom[i] = ((uint16_t) _handle->cx_data[2*i]) << 8;
		prom[i] |= _handle->cx_data[2*i+1];
		NRF_LOG_DEBUG("Prom %u: 0x%X", i, prom[i]);
	}
#endif

	// Verify CRC4 in top 4 bits of prom[0] (follows AN520 but not directly...)
	if (!this->crc_check(prom, (prom[MS5637_CRC_INDEX] & 0xF000) >> 12)) {
		LOG_ERROR("MS5637 Cx CRC fail");
		return false;
	}

	m_err = NO_ERR;

	c1 = prom[1];
	c2 = prom[2];
	c3 = prom[3];
	c4 = prom[4];
	c5 = prom[5];
	c6 = prom[6];

	initialised = true;

	LOG_INFO("MS5637 init success");

	return false;
}

uint32_t MS5637::reset() {

	if (!this->wireWriteByte(CMD_RESET)) {
		return 1;
	} else {
		return 0;
	}

}

#ifdef _DEBUG_TWI
void MS5637::refresh(ms5637_handle_t *_handle) {

	int32_t d2 = takeReading(CMD_START_D2(BARO_LEVEL), BARO_LEVEL);
	if (d2 == 0) {
		LOG_ERROR("d2 issue");
		//return;
	}

	int32_t d1 = takeReading(CMD_START_D1(BARO_LEVEL), BARO_LEVEL);
	if (d1 == 0) {
		LOG_ERROR("d1 issue");
		//return;
	}

	this->computeTempAndPressure(d1, d2);
}
#else
void MS5637::refresh(ms5637_handle_t *_handle) {

	ASSERT(_handle);

	uint32_t temp = (uint32_t) _handle->temp_adc[0] << 16;
	temp |= (uint32_t) _handle->temp_adc[1] << 8;
	temp |= _handle->temp_adc[2];

	int32_t d1 = temp;

	LOG_DEBUG("Calculating d1: %d", d1);

	temp = (uint32_t) _handle->press_adc[0] << 16;
	temp |= (uint32_t) _handle->press_adc[1] << 8;
	temp |= _handle->press_adc[2];

	int32_t d2 = temp;

	LOG_DEBUG("Calculating d2: %d", d2);

	this->computeTempAndPressure(d1, d2);
}
#endif

uint32_t MS5637::takeReading(uint8_t trigger_cmd, BaroOversampleLevel oversample_level) {

	if (!this->wireWriteByte(trigger_cmd)) {
		return 0;
	}

	uint8_t sampling_delay = pgm_read_byte(SamplingDelayMs + (int )oversample_level);

	// arm the timer to later unblock the task
	uint32_t timeout_ticks = APP_TIMER_TICKS(sampling_delay);
	uint32_t err_code = app_timer_start(m_ms5637_update, timeout_ticks, &m_tasks_id);
	APP_ERROR_CHECK(err_code);
	// we block the task while performing the measurement
	if (!err_code) {
		events_wait(TASK_EVENT_PERIPH_MS_WAIT);
	} else {
		delay_ms(sampling_delay);
	}

	uint8_t adc[3] = {0};
	if (!this->wireReadDataBlock(CMD_READ_ADC, adc, sizeof(adc))) {
		return 0;
	}

	// Sometimes first read fails...?

	uint32_t result = (uint32_t) adc[0] << 16;
	result |= (uint32_t) adc[1] << 8;
	result |= adc[2];
	return result;
}

bool MS5637::computeTempAndPressure(int32_t d1, int32_t d2) {

	if (m_err || !initialised)
		return false;

	if (d1 == 0 || d2 == 0) {
		return false;
	}

	int64_t dt = d2 - c5 * (1L << 8);

	int32_t temp = 2000 + (dt * c6) / (1L << 23);

	/* Second order temperature compensation */
	int64_t t2;
	if (temp >= 2000) {
		/* High temperature */
		t2 = 5 * (dt * dt) / (1LL << 38);
	} else {
		/* Low temperature */
		t2 = 3 * (dt * dt) / (1LL << 33);
	}

	m_temperature =  ((float)temp - (float)t2) / 100.;

	int64_t off = c2 * (1LL << 17) + (c4 * dt) / (1LL << 6);
	int64_t sens = c1 * (1LL << 16) + (c3 * dt) / (1LL << 7);

	/* Second order temperature compensation for pressure */
	if (temp < 2000) {
		/* Low temperature */
		int32_t tx = temp - 2000;
		tx *= tx;
		int32_t off2 = 61 * tx / (1 << 4);
		int32_t sens2 = 29 * tx / (1 << 4);
		if (temp < -1500) {
			/* Very low temperature */
			tx = temp + 1500;
			tx *= tx;
			off2 += 17 * tx;
			sens2 += 9 * tx;
		}
		off -= off2;
		sens -= sens2;
	}

	int32_t p = ((int64_t) d1 * sens / (1LL << 21) - off) / (1LL << 15);
	m_pressure =  (float)p / 100.;

	LOG_INFO("Pressure: %d mbar", (int) (m_pressure));
	LOG_INFO("Temperature: %d°", (int) (m_temperature));

	return true;
}

/*******************************************************************************
 * Raw I2C Reads and Writes
 ******************************************************************************/

/**
 * @brief Writes a single byte to the I2C device (no register)
 *
 * @param[in] val the 1-byte value to write to the I2C device
 * @return True if successful write operation. False otherwise.
 */
bool MS5637::wireWriteByte(uint8_t val) {
#ifdef _DEBUG_TWI
	return i2c_write8(MS5637_ADDR, val);
#else
	return true;
#endif
}

/**
 * @brief Reads a block (array) of bytes from the I2C device and register
 *
 * @param[in] reg the register to read from
 * @param[out] val pointer to the beginning of the data
 * @param[in] len number of bytes to read
 * @return Number of bytes read. -1 on read error.
 */
int MS5637::wireReadDataBlock(uint8_t reg, uint8_t *val, unsigned int len) {
#ifdef _DEBUG_TWI
	i2c_write8(MS5637_ADDR, reg);
	// repeated stop
	// repeated start
	if (!i2c_read_n(MS5637_ADDR, val, len)) {
		return 0;
	}
#endif
	return len;
}
