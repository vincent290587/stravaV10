/*
 * STC3100.cpp
 *
 *  Created on: 1 mars 2017
 *      Author: Vincent
 */

#define USE_RTT 0


#include "millis.h"
#include "segger_wrapper.h"
#include "STC3100.h"
#include "utils.h"




/***************************************************************************
 CONSTRUCTOR
 ***************************************************************************/

STC3100::STC3100(int32_t sensorID) {
	_sensorID = sensorID;
	_charge = 0;
	_deviceID = 0;
	_r_sens = 0;
	_stc3100Mode = STC3100_MODE_ULTRAHIGHRES;

	m_soft_reset = false;
}

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/

/**************************************************************************/
/*!
    @brief  Setups the HW
 */
/**************************************************************************/

/**
 *
 * @param r_sens Sense resistor in milliohms
 * @param res Sampling mode
 * @return
 */
bool STC3100::init(uint32_t r_sens, stc3100_res_t res) {

	_r_sens = r_sens;

    // reset
	this->reset();
	delay_ms(1);

	// TODO read device ID
	//i2c0_read_reg(STC3100_ADDRESS, REG_DEVICE_ID, &_deviceID, 1);

//	LOG_INFO("Device ID: %x\r\n", _deviceID);

	/* Set the mode indicator */
	_stc3100Mode = 0;
	_stc3100Mode |= MODE_RUN;
	_stc3100Mode |= res;

	// set mode
	this->writeCommand(REG_MODE, _stc3100Mode);

	return true;
}


void STC3100::reset(void) {

	// reset
	this->writeCommand(REG_CONTROL, STC_RESET);

}


void STC3100::shutdown(void) {

	// reset
	this->writeCommand(REG_CONTROL, STC_RESET);
	delay_ms(1);

	/* Set the mode indicator */
	_stc3100Mode  = 0;
	//_stc3100Mode |= MODE_RUN;
	//_stc3100Mode |= STC3100_MODE_HIGHRES;

	// set mode
	this->writeCommand(REG_MODE, _stc3100Mode);

}

/**************************************************************************/
/*!
    @brief  Reads the sensor
 */
/**************************************************************************/
bool STC3100::refresh(tSTC31000Data *_data)
{
	memcpy(&_stc_data, _data, sizeof(_stc_data));

	this->computeVoltage ();
	this->computeCharge  ();
	this->computeCurrent ();
	this->computeTemp    ();
	this->computeCounter ();

	if (!m_soft_reset &&
			millis() > 5000) {

		this->reset();
		m_soft_reset = true;
	}

	return true;
}


void STC3100::computeCounter()
{
	uint8_t tl=_stc_data.CounterLow, th=_stc_data.CounterHigh;
	uint32_t t;
	int val;

	t = th;
	t <<= 8;
	val = (t & 0xFF00) | tl;
	_counter = (float) val;
}

void STC3100::computeVoltage()
{

	uint8_t tl=_stc_data.VoltageLow, th=_stc_data.VoltageHigh;
	uint32_t t;
	int val;

	t = th;
	t <<= 8;
	val = (t & 0xFF00) | tl;
	// LSB is 2.44mV
	_voltage = (float)  val * 0.00244;

}

void STC3100::computeCharge()
{

	uint8_t tl=_stc_data.ChargeLow, th=_stc_data.ChargeHigh;
	float val;

	LOG_DEBUG("Charge L=0x%x H=0x%x\r\n", tl, th);

	val = compute2Complement(th, tl);
	//
	// charge is in mA.h
	//
	// LSB = 6.7 uV.h
	_charge = (val * 6.7 / _r_sens);

}

void STC3100::computeCurrent()
{
	uint8_t tl=_stc_data.CurrentLow, th=_stc_data.CurrentHigh;
	float val;

	LOG_DEBUG("Current L=0x%x H=0x%x\r\n", tl, th);

	val = compute2Complement(th, tl);
	// LSB = 11.77uV
	_current = (val * 11.77 / _r_sens);

}


void STC3100::computeTemp()
{
	uint8_t tl=_stc_data.TemperatureLow, th=_stc_data.TemperatureHigh;
	uint32_t t;
	int val;

	t = th;
	t <<= 8;
	val = (t & 0xFF00) | tl;

	_temperature = ((float) val * 0.125);

}


float STC3100::getCorrectedVoltage(float int_res) {

	float res = _voltage + int_res * _current / 1000.;
	return res;

}

/**
 *
 * @return The average current consumption in ma
 */
float STC3100::getAverageCurrent(void) {

	if (!millis()) return 0.;

	float res = _charge * 3600 * 1000 / millis();
	return res;
}

/***************************************************************************
 PRIVATE FUNCTIONS
 ***************************************************************************/

/**
 *
 * @param reg
 * @param value
 */
void STC3100::writeCommand(uint8_t reg, uint8_t value)
{
//	uint8_t val_ = value;

//	if (kStatus_Success != i2c0_write_reg(STC3100_ADDRESS, reg, &val_, 1)) {
//		LOG_INFO("i2c error\r\n");
//	}
}

/**
 *
 */
void STC3100::readChip()
{
//	if (kStatus_Success != i2c0_read_reg(STC3100_ADDRESS, REG_CHARGE_LOW, _stc_data.array, 10)) {
//		LOG_INFO("i2c error\r\n");
//	}
}
