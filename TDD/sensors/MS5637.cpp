/*
 * MS5637.cpp
 *
 *  Created on: 26 févr. 2017
 *      Author: Vincent
 */

#include "utils.h"
#include "MS5637.h"
#include "millis.h"
#include "parameters.h"
#include "segger_wrapper.h"
#include "pgmspace.h"


/* error codes */
#define NO_ERR          0
#define ERR_NOREPLY     -1
#define ERR_BAD_READLEN -2
#define ERR_NEEDS_BEGIN -3


/* delay to wait for sampling to complete, on each OSR level */
static const uint8_t SamplingDelayMs[6] PROGMEM = { 2, 4, 6, 10, 18, 34 };

#define NAN (-1.)

MS5637::MS5637 () {

	initialised = false;
	m_err = ERR_NEEDS_BEGIN;

	m_temperature = 22.;
	m_pressure = 1015.;

}

void MS5637::init(void) {

	initialised = false;
	this->setCx(nullptr);

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

	return true;
}

/**
 *
 * @param _handle
 * @return
 */
bool MS5637::setCx(ms5637_handle_t *_handle) {

	LOG_INFO("MS5637 init success");

	initialised = true;

	return false;
}

uint32_t MS5637::reset() {

	return 0;
}

void MS5637::refresh(ms5637_handle_t *_handle) {

}

uint32_t MS5637::takeReading(uint8_t trigger_cmd, BaroOversampleLevel oversample_level) {

	return 0;
}

bool MS5637::computeTempAndPressure(int32_t d1, int32_t d2) {

	return true;
}
