/*
 * MS5637.cpp
 *
 *  Created on: 26 févr. 2017
 *      Author: Vincent
 */

#include "i2c.h"
#include "utils.h"
#include "MS5637.h"
#include "millis.h"
#include "parameters.h"
#include "segger_wrapper.h"
#include "pgmspace.h"


/* error codes */
#define ERR_NOREPLY -1
#define ERR_BAD_READLEN -2
#define ERR_NEEDS_BEGIN -3


/* delay to wait for sampling to complete, on each OSR level */
static const uint8_t SamplingDelayMs[6] PROGMEM = { 2, 4, 6, 10, 18, 34 };

#define NAN (-1.)

MS5637::MS5637 () {

	 initialised = false;
	 err = ERR_NEEDS_BEGIN;

}

void MS5637::init(void) {

	initialised = false;

#ifdef _DEBUG_TWI
	this->setCx(nullptr);
#endif

	return;
}

bool MS5637::setCx(ms5637_handle_t *_handle) {

	if (this->reset() != 0)
		return false;

	uint16_t prom[7];
#ifdef _DEBUG_TWI
	uint8_t prom_val[3];

	for (int i = 0; i < 7; i++) {

		if (wireReadDataBlock(CMD_PROM_READ(i), prom_val, 2) != 2) {
			err = ERR_BAD_READLEN;
			return false;
		}

		prom[i] = ((uint16_t) prom_val[0]) << 8;
		prom[i] |= prom_val[1];
	}
#else
	for (int i = 0; i < 7; i++) {

		prom[i] = decode_uint16 (_handle->cx_data + 2*i);

	}
#endif

	// TODO verify CRC4 in top 4 bits of prom[0] (follows AN520 but not directly...)
	c1 = prom[1];
	c2 = prom[2];
	c3 = prom[3];
	c4 = prom[4];
	c5 = prom[5];
	c6 = prom[6];

	initialised = true;

	return false;
}

uint32_t MS5637::reset() {

	if (!this->wireWriteByte(CMD_RESET)) {
		return 1;
	} else {
		return 0;
	}

}

void MS5637::refresh(ms5637_handle_t *_handle) {

	uint32_t temp = (uint32_t) _handle->temp_adc[0] << 16;
	temp |= (uint32_t) _handle->temp_adc[1] << 8;
	temp |= _handle->temp_adc[2];

	int32_t d1 = temp;

	NRF_LOG_INFO("Calculating d1: %d", d1);

	temp = (uint32_t) _handle->press_adc[0] << 16;
	temp |= (uint32_t) _handle->press_adc[1] << 8;
	temp |= _handle->press_adc[2];

	int32_t d2 = temp;

	NRF_LOG_INFO("Calculating d2: %d", d2);

	NRF_LOG_DEBUG("Calculating pressure...");

	this->getTempAndPressure(d1, d2, &temperature, &pressure);
}

uint32_t MS5637::takeReading(uint8_t trigger_cmd, BaroOversampleLevel oversample_level) {

	if (this->wireWriteByte(trigger_cmd) != 0) {
		return 0;
	}

	uint8_t sampling_delay = pgm_read_byte(SamplingDelayMs + (int )oversample_level);

	delay_ms(sampling_delay);

	uint8_t adc[4];
	if (this->wireReadDataBlock(CMD_READ_ADC, adc, 3) != 0) {
		return 0;
	}

	// Sometimes first read fails...?

	uint32_t result = (uint32_t) adc[0] << 16;
	result |= (uint32_t) adc[1] << 8;
	result |= adc[2];
	return result;
}

bool MS5637::getTempAndPressure(int32_t d1, int32_t d2, float *temperature, float *pressure) {

	if (err || !initialised)
		return false;

	if (d2 == 0) {
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

	if (temperature != NULL) {
		*temperature = (float) (temp - t2) / 100;
	}

	if (pressure != NULL) {

		if (d1 == 0) {
			return false;
		}

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
		*pressure = (float) p / 100;
	}
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
	if (!i2c_read_reg_n(MS5637_ADDR, reg, val, len)) {
		return 0;
	}
#endif
	return len;
}
