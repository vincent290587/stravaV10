/*
 * STC3100.cpp
 *
 *  Created on: 1 mars 2017
 *      Author: Vincent
 */


#include "millis.h"
#include "helper.h"
#include "parameters.h"
#include "segger_wrapper.h"
#include "STC3100.h"
#include "utils.h"

#include "i2c.h"
#include "nrf_twi_mngr.h"

#define I2C_READ_REG(addr, p_reg_addr, p_buffer, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, NRF_TWI_MNGR_NO_STOP), \
		NRF_TWI_MNGR_READ (addr, p_buffer, byte_cnt, 0)

#define I2C_READ_REG_REP_STOP(addr, p_reg_addr, p_buffer, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, 0), \
		NRF_TWI_MNGR_READ (addr, p_buffer, byte_cnt, 0)

#define I2C_WRITE(addr, p_data, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_data, byte_cnt, 0)

static tSTC31000Data m_stc_buffer;
static volatile bool m_is_updated = false;

/***************************************************************************
 C FUNCTIONS
 ***************************************************************************/

static void _stc_read_cb(ret_code_t result, void * p_user_data) {

	APP_ERROR_CHECK(result);
	if (result) return;

	m_is_updated = true;

}

bool is_stc_updated(void) {
	return m_is_updated;
}

/***************************************************************************
 CONSTRUCTOR
 ***************************************************************************/

STC3100::STC3100(int32_t sensorID) {
	_sensorID = sensorID;
	_charge = 0;
	_deviceID = 0;
	_r_sens = 0;
	_counter = 0;
	_temperature = 0;
	_current = 0;
	_voltage = 0;
	m_start_av = 0;
	_stc3100Mode = STC3100_MODE_ULTRAHIGHRES;
	m_charge_offset = 0;
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

	m_charge_offset = 0;

	// Reset sensor
	this->reset();

	/* Set the mode indicator */
	_stc3100Mode = 0;
	_stc3100Mode |= MODE_RUN;
	_stc3100Mode |= res;

	delay_ms(1);

	// read device ID
#ifdef _DEBUG_TWI

	i2c_read_reg_8(STC3100_ADDRESS, REG_DEVICE_ID, &_deviceID);
	LOG_INFO("Device ID: %x\r\n", _deviceID);

	// set mode
	this->writeCommand(REG_MODE, _stc3100Mode);
#else
	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND stc_config3[1];
	stc_config3[0] = REG_DEVICE_ID;

	static uint8_t raw_data_stc[1];

	static nrf_twi_mngr_transfer_t const sensors_init_transfers3[] =
	{
			I2C_READ_REG        (STC3100_ADDRESS, stc_config3, raw_data_stc, 1)
	};

	i2c_perform(NULL, sensors_init_transfers3, sizeof(sensors_init_transfers3) / sizeof(sensors_init_transfers3[0]), NULL);

	LOG_INFO("Device ID: %x\r\n", raw_data_stc[0]);

#endif

	// set mode
	this->writeCommand(REG_MODE, _stc3100Mode);

	return true;
}

void STC3100::checkDevID(uint8_t dev_id) {

	LOG_INFO("STC device ID: %x\r\n", dev_id);

}


void STC3100::reset(void) {

	// Reset sensor, no open-drain
	this->writeCommand(REG_CONTROL, STC_RESET);

}


void STC3100::shutdown(void) {

	// reset
//	this->reset();
//	delay_ms(1);

	/* Set the mode indicator */
	_stc3100Mode  = 0;
	//_stc3100Mode |= MODE_RUN;
	//_stc3100Mode |= STC3100_MODE_HIGHRES;

	// set mode OFF
	this->writeCommand(REG_MODE, _stc3100Mode);

	// Activate open-drain
	this->writeCommand(REG_CONTROL, STC_IO_OD);

}

/**************************************************************************/
/*!
    @brief  Reads the sensor
 */
/**************************************************************************/
bool STC3100::refresh(void)
{
	memcpy(&_stc_data, &m_stc_buffer, sizeof(_stc_data));

	if (!m_is_updated) return false;
	m_is_updated = false;

#ifdef _DEBUG_TWI
	this->readChip();
#endif

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

	LOG_DEBUG("Voltage %dmV", (int) (_voltage*1000));

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
	_voltage = (float)  val * 0.00244f;

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
	_charge = (val * 6.7f / _r_sens);

}

void STC3100::computeCurrent()
{
	uint8_t tl=_stc_data.CurrentLow, th=_stc_data.CurrentHigh;
	float val;

	LOG_DEBUG("Current L=0x%x H=0x%x\r\n", tl, th);

	val = compute2Complement(th, tl);
	// LSB = 11.77uV
	_current = (val * 11.77f / _r_sens);

}


void STC3100::computeTemp()
{
	uint8_t tl=_stc_data.TemperatureLow, th=_stc_data.TemperatureHigh;
	uint32_t t;
	int val;

	t = th;
	t <<= 8;
	val = (t & 0xFF00) | tl;

	_temperature = ((float) val * 0.125f);

}


float STC3100::getCorrectedVoltage(float int_res) {

	float res = _voltage + int_res * _current / 1000.f;
	return res;

}

/**
 *
 * @return The average current consumption in ma
 */
float STC3100::getAverageCurrent(void) {

	if (!(millis() - m_start_av)) return 0.;

	float res = (_charge - m_charge_offset) * 3600 * 1000 / (millis() - m_start_av);
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
#ifdef _DEBUG_TWI
	i2c_write_reg_8(STC3100_ADDRESS, reg, value);
#else
	uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND stc_config[2] = {reg, value};

	nrf_twi_mngr_transfer_t const sensors_init_transfers[] =
	{
		I2C_WRITE     (STC3100_ADDRESS, stc_config, sizeof(stc_config))
	};

	i2c_perform(NULL, sensors_init_transfers, sizeof(sensors_init_transfers) / sizeof(sensors_init_transfers[0]), NULL);

#endif
}

/**
 *
 */
void STC3100::readChip()
{
#ifdef _DEBUG_TWI
	i2c_read_reg_n(STC3100_ADDRESS, REG_CHARGE_LOW, _stc_data.array, 10);
#else
    // [these structures have to be "static" - they cannot be placed on stack
    //  since the transaction is scheduled and these structures most likely
    //  will be referred after this function returns]

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND stc_regs[1]  = STC_READ_ALL_REGS;

    static nrf_twi_mngr_transfer_t const transfers[] =
    {
    	STC3100_READ_ALL (&stc_regs[0] , &m_stc_buffer.array[0])
    };
    static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
    {
        .callback            = _stc_read_cb,
        .p_user_data         = NULL,
        .p_transfers         = transfers,
        .number_of_transfers = sizeof(transfers) / sizeof(transfers[0])
    };

    i2c_schedule(&transaction);

#endif
}
