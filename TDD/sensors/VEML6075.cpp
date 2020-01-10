/*
 * VEML6075.cpp
 *
 *  Created on: 28 f�vr. 2017
 *      Author: Vincent
 */

#include "millis.h"
#include "parameters.h"
#include "segger_wrapper.h"
#include "pgmspace.h"
#include "utils.h"
#include "VEML6075.h"

#define VEML_NOMINAL_CONF     (VEML6075_CONF_PW_ON | VEML6075_CONF_HD_NORM | VEML6075_CONF_IT_200MS)

static uint8_t m_veml_buffer[10];
static volatile bool m_is_updated = false;

/***************************************************************************
 C FUNCTIONS
 ***************************************************************************/

bool is_veml_updated(void) {
	return m_is_updated;
}

VEML6075::VEML6075() {

	raw_uva = 0;
	raw_uvb = 0;
	raw_dark = 0;
	raw_vis = 0;
	raw_ir = 0;

	// Despite the datasheet saying this isn't the default on startup, it appears
	// like it is. So tell the thing to actually start gathering data.
	this->config = VEML_NOMINAL_CONF;

}

void VEML6075::reset(void) {

	LOG_INFO("VEML reset");

	// Despite the datasheet saying this isn't the default on startup, it appears
	// like it is. So tell the thing to actually start gathering data.
	this->config = VEML_NOMINAL_CONF;

	this->init();
}

bool VEML6075::init(void) {

	uint16_t dev_id;

	this->off();

	delay_ms(5);

	this->on();

	delay_ms(1);

	dev_id = this->getDevID();

	return true;
}

void VEML6075::on() {
	// Write config to make sure device is enabled
	this->config &= 0b11111110;
	this->write16(VEML6075_REG_CONF, this->config);
}

void VEML6075::off() {
	// Write config to make sure device is disabled
	this->config |= VEML6075_CONF_PW_OFF;
	this->write16(VEML6075_REG_CONF, this->config);
}

void VEML6075::readChip(void) {


}

// Poll sensor for latest values and cache them
void VEML6075::refresh(void) {

	this->config = this->read16(VEML6075_REG_CONF);
	LOG_DEBUG("Config: %u", this->config);
	if (!this->config) {
		this->reset();
		return;
	}

	this->raw_uva  = this->read16(VEML6075_REG_UVA);
	this->raw_uvb  = this->read16(VEML6075_REG_UVB);
	this->raw_dark = this->read16(VEML6075_REG_DUMMY);
	this->raw_vis  = this->read16(VEML6075_REG_UVCOMP1);
	this->raw_ir   = this->read16(VEML6075_REG_UVCOMP2);

	LOG_DEBUG ("Raw IR : %u", this->raw_ir);
	LOG_DEBUG ("Raw VIS: %u", this->raw_vis);
	LOG_DEBUG ("Raw UVA: %u", this->raw_uva);
	LOG_DEBUG ("UV index: %d", (int)this->getUVIndex());
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
	return (uva_weighted + uvb_weighted) / 2.0f;
}

/////// I2C functions  ////////

uint16_t VEML6075::read16(uint8_t reg) {
	uint16_t res = 0;

	return res;
}

uint16_t VEML6075::read16_raw(uint8_t reg) {
	uint16_t res = 0;

	return res;
}

void VEML6075::write16(uint8_t reg, uint16_t raw_data) {


}
