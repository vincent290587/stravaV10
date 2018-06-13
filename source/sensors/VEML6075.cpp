/*
 * VEML6075.cpp
 *
 *  Created on: 28 févr. 2017
 *      Author: Vincent
 */

#include "millis.h"
#include "segger_wrapper.h"
#include "pgmspace.h"
#include "utils.h"
#include "VEML6075.h"



VEML6075::VEML6075() {

	// Despite the datasheet saying this isn't the default on startup, it appears
	// like it is. So tell the thing to actually start gathering data.
	this->config = VEML6075_CONF_PW_OFF;
}

bool VEML6075::init() {

	this->config |= VEML6075_CONF_HD_HIGH;

	// App note only provided math for this one...
	this->config |= VEML6075_CONF_IT_100MS;

	this->on();

//	uint16_t dev_id = this->getDevID();
//	if (dev_id != VEML6075_DEVID) {
//		LOG_ERROR("Wrong device ID: %u\r\n", dev_id);
//		return false;
//	}

	return true;
}

void VEML6075::on() {

	this->config &= ~VEML6075_CONF_PW_OFF;

}

void VEML6075::off() {

	this->config |= VEML6075_CONF_PW_OFF;

}

// Poll sensor for latest values and cache them
void VEML6075::refresh(uint8_t *_data) {

	this->raw_uva  = decode_uint16(_data);
	this->raw_uvb  = decode_uint16(_data+2);
	this->raw_dark = decode_uint16(_data+4);
	this->raw_vis  = decode_uint16(_data+6);
	this->raw_ir   = decode_uint16(_data+8);

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

//	uint16_t res = 0xFFFF;
//	uint8_t retries = 3;
//
//	while (retries--) {
//		res = this->read16_raw(reg);
//		if (res != 0xFFFF) {
//			// success
//			return res;
//		} else {
//			LOG_INFO("VEML6075 read fail");
//		}
//	}
//
//	if (!retries) {
//		return 0xFFFF;
//	}

	return 0;
}

uint16_t VEML6075::read16_raw(uint8_t reg) {

//	// read from I2C
//	uint8_t raw_data[2];
//
//	if (kStatus_Success != i2c0_read_reg(VEML6075_ADDR, reg, raw_data, 2)) {
//		return 0xFFFF;
//	}
//
//	uint16_t res = raw_data[1] << 8;
//	res |= raw_data[0];
//
	return 0;
}

void VEML6075::write16(uint8_t reg, uint16_t raw_data) {

//	uint8_t retries = 3;
//
//	// reg LSB MSB
//	uint8_t data[2] = {(uint8_t)(0xFF & raw_data), (uint8_t)((0xFF00 & raw_data) >> 8)};
//
//	while (retries--) {
//		if (kStatus_Success == i2c0_write_reg(VEML6075_ADDR, reg, data, 2)) {
//			// success
//			return;
//		}
//	}

}
