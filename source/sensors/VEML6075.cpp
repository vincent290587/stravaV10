/*
 * VEML6075.cpp
 *
 *  Created on: 28 févr. 2017
 *      Author: Vincent
 */

#include "millis.h"
#include "nrf_delay.h"
#include "parameters.h"
#include "segger_wrapper.h"
#include "pgmspace.h"
#include "utils.h"
#include "VEML6075.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define VEML_NOMINAL_CONF     (VEML6075_CONF_PW_ON | VEML6075_CONF_HD_HIGH | VEML6075_CONF_IT_100MS)

VEML6075::VEML6075() {

	// Despite the datasheet saying this isn't the default on startup, it appears
	// like it is. So tell the thing to actually start gathering data.
	this->config = VEML_NOMINAL_CONF;

}

void VEML6075::reset(void) {

	NRF_LOG_WARNING("VEML reset");

	// Despite the datasheet saying this isn't the default on startup, it appears
	// like it is. So tell the thing to actually start gathering data.
	this->config = VEML_NOMINAL_CONF;

	this->init();
}

bool VEML6075::init(uint16_t dev_id) {

	this->off();

	nrf_delay_ms(5);

	this->on();

#ifdef _DEBUG_TWI
	nrf_delay_ms(1);
	dev_id = this->getDevID();
#endif

	if (dev_id != VEML6075_DEVID) {
		LOG_ERROR("VEML wrong device ID: %u", dev_id);
		return false;
	} else {
		LOG_INFO("VEML device ID: 0x%X", dev_id);
	}

	return true;
}

void VEML6075::on() {
#ifdef _DEBUG_TWI
	// Write config to make sure device is enabled
	this->write16(VEML6075_REG_CONF, this->config & 0b11111110);
	nrf_delay_ms(1);
#endif
}

void VEML6075::off() {
#ifdef _DEBUG_TWI
	// Write config to make sure device is disabled
	this->write16(VEML6075_REG_CONF, this->config | VEML6075_CONF_PW_OFF);
#endif
}

// Poll sensor for latest values and cache them
void VEML6075::refresh(uint8_t *_data) {
#ifndef _DEBUG_TWI
	this->raw_uva  = decode_uint16(_data);
	this->raw_uvb  = decode_uint16(_data+2);
	this->raw_dark = decode_uint16(_data+4);
	this->raw_vis  = decode_uint16(_data+6);
	this->raw_ir   = decode_uint16(_data+8);
#else

	this->config = this->read16(VEML6075_REG_CONF);
	LOG_INFO("Config: %u", this->config);
	if (!this->config) {
		this->reset();
		return;
	}

	this->raw_uva  = this->read16(VEML6075_REG_UVA);
	this->raw_uvb  = this->read16(VEML6075_REG_UVB);
	this->raw_dark = this->read16(VEML6075_REG_DUMMY);
	this->raw_vis  = this->read16(VEML6075_REG_UVCOMP1);
	this->raw_ir   = this->read16(VEML6075_REG_UVCOMP2);

#endif

	LOG_INFO("Raw IR : %u", this->raw_ir);
	LOG_INFO("Raw VIS: %u", this->raw_vis);
}

uint16_t VEML6075::getRawUVA() {
	return this->raw_uva;
}

uint16_t VEML6075::getRawUVB() {
	return this->raw_uvb;
}

uint16_t VEML6075::getRawDark() {
	return this->raw_dark;
}

uint16_t VEML6075::getRawVisComp() {
	return this->raw_vis;
}

uint16_t VEML6075::getRawIRComp() {
	return this->raw_ir;
}

uint16_t VEML6075::getDevID() {
	return this->read16(VEML6075_REG_DEVID);
}

uint16_t VEML6075::getConf() {
	return this->read16(VEML6075_REG_CONF);
}

float VEML6075::getUVA() {
	float comp_vis = this->raw_vis - this->raw_dark;
	float comp_ir = this->raw_ir - this->raw_dark;
	float comp_uva = this->raw_uva - this->raw_dark;

	comp_uva -= VEML6075_UVI_UVA_VIS_COEFF * comp_vis;
	comp_uva -= VEML6075_UVI_UVA_IR_COEFF * comp_ir;

	return comp_uva;
}

float VEML6075::getUVB() {
	float comp_vis = this->raw_vis - this->raw_dark;
	float comp_ir = this->raw_ir - this->raw_dark;
	float comp_uvb = this->raw_uvb - this->raw_dark;

	comp_uvb -= VEML6075_UVI_UVB_VIS_COEFF * comp_vis;
	comp_uvb -= VEML6075_UVI_UVB_IR_COEFF * comp_ir;

	return comp_uvb;
}

float VEML6075::getUVIndex() {
	float uva_weighted = this->getUVA() * VEML6075_UVI_UVA_RESPONSE;
	float uvb_weighted = this->getUVB() * VEML6075_UVI_UVB_RESPONSE;
	return (uva_weighted + uvb_weighted) / 2.0;
}

/////// I2C functions  ////////

uint16_t VEML6075::read16(uint8_t reg) {
	uint16_t res = 0;
#ifdef _DEBUG_TWI
	uint8_t retries = 3;

	while (retries--) {
		res = this->read16_raw(reg);
		if (res != 0xFFFF) {
			// success
			return res;
		} else {
			NRF_LOG_WARNING("VEML retry");
		}
	}

	if (!retries) {
		LOG_ERROR("VEML error");
		return 0xFFFF;
	}
#endif
	return res;
}

uint16_t VEML6075::read16_raw(uint8_t reg) {
	uint16_t res = 0;
#ifdef _DEBUG_TWI
	if (!i2c_write8(VEML6075_ADDR, reg)) {
		return 0xFFFF;
	}

	// read from I2C
	uint8_t raw_data[2];
	if (!i2c_read_n(VEML6075_ADDR, raw_data, 2)) {
		return 0xFFFF;
	}

	res = raw_data[1] << 8;
	res |= raw_data[0];
#endif
	return res;
}

void VEML6075::write16(uint8_t reg, uint16_t raw_data) {
#ifdef _DEBUG_TWI
	uint8_t retries = 3;

	// reg LSB MSB
	uint8_t data[3] = {reg, (uint8_t)(0xFF & raw_data), (uint8_t)((0xFF00 & raw_data) >> 8)};

	while (retries--) {
		if (i2c_write_n(VEML6075_ADDR, data, 3)) {
			// success
			return;
		}
	}

	if (!retries) LOG_ERROR("VEML no retry left");
#endif
}
