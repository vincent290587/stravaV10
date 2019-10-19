/*
 * MS5637.cpp
 *
 *  Created on: 26 févr. 2017
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdbool.h>
#include "i2c.h"
#include "nrf_twi_mngr.h"
#include "ms5637.h"
#include "millis.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

#define MS5637_TWI_ADDRESS              0x76

#define MS5637_READOUT_SIZE             3

#define I2C_READ_REG(addr, p_reg_addr, p_buffer, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, NRF_TWI_MNGR_NO_STOP), \
		NRF_TWI_MNGR_READ (addr, p_buffer, byte_cnt, 0)

#define I2C_READ_REG_REP_STOP(addr, p_reg_addr, p_buffer, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_reg_addr, 1, 0), \
		NRF_TWI_MNGR_READ (addr, p_buffer, byte_cnt, 0)

#define I2C_WRITE(addr, p_data, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_data, byte_cnt, 0)

#define I2C_WRITE_CONT(addr, p_data, byte_cnt) \
		NRF_TWI_MNGR_WRITE(addr, p_data, byte_cnt, NRF_TWI_MNGR_NO_STOP)

#define MS5637_READ_ALL(p_cmd, p_mag_buffer, p_buffer) \
		I2C_READ_REG(MS5637_TWI_ADDRESS, p_cmd, p_buffer, MS5637_READOUT_SIZE)


/* Module supports a range of lower oversampling levels, for faster
   less accurate results.
 */
enum BaroOversampleLevel {
  OSR_256, OSR_512, OSR_1024, OSR_2048, OSR_4096, OSR_8192 };

#define BARO_LEVEL    OSR_8192

typedef enum {
  eMS5637MeasCmdD1,
  eMS5637MeasCmdD2,
} eMS5637MeasType;

typedef struct {
	uint8_t cx_data[MS5637_COEFFICIENT_COUNT*2];
	float press, temp;
	int32_t d1, d2;
	eMS5637MeasType meas_type;
	bool has_started;
	bool is_updated;
} ms5637_data_t;

/* delay to wait for sampling to complete, on each OSR level */
//static const uint8_t SamplingDelayMs[6] PROGMEM = { 2, 4, 6, 10, 18, 34 };

float m_temperature, m_pressure;

static bool m_is_updated;

static bool initialised;
static uint16_t c1,c2,c3,c4,c5,c6; // Calibration constants used in producing results

static ms5637_data_t m_data;


/**
 * \brief CRC check
 *
 * \param[in] uint16_t *: List of EEPROM coefficients
 * \param[in] uint8_t : crc to compare with
 *
 * \return bool : TRUE if CRC is OK, FALSE if KO
 */
static bool ms5637_crc_check(uint16_t *n_prom, uint8_t crc) {
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

static void ms5637_get_cx(int i, uint8_t *prom) {

	uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND config[1] = { CMD_PROM_READ(i) };

	nrf_twi_mngr_transfer_t cx_transfer[] =
	{
			I2C_READ_REG_REP_STOP(MS5637_TWI_ADDRESS, config, prom, 2)
	};

	i2c_perform(NULL, cx_transfer, sizeof(cx_transfer) / sizeof(cx_transfer[0]), NULL);

}

static void ms5637_send_cmd(uint8_t cmd) {

	uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND config[1] = { cmd };

	nrf_twi_mngr_transfer_t _transfer[] =
	{
			I2C_WRITE(MS5637_TWI_ADDRESS, config, 1)
	};

	i2c_perform(NULL, _transfer, sizeof(_transfer) / sizeof(_transfer[0]), NULL);

}

static void ms5637_order_d1(void) {

	static uint8_t p_cmd[1] = {CMD_START_D1(BARO_LEVEL)};

	static nrf_twi_mngr_transfer_t const _readout_transfer[] =
	{
			I2C_WRITE(MS5637_TWI_ADDRESS, p_cmd, 1)
	};

	static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
	{
			.callback            = NULL,
			.p_user_data         = NULL,
			.p_transfers         = _readout_transfer,
			.number_of_transfers = sizeof(_readout_transfer) / sizeof(_readout_transfer[0])
	};

	i2c_schedule(&transaction);

}

static void ms5637_order_d2(void) {

	static uint8_t p_cmd[1] = {CMD_START_D2(BARO_LEVEL)};

	static nrf_twi_mngr_transfer_t const _readout_transfer[] =
	{
			I2C_WRITE(MS5637_TWI_ADDRESS, p_cmd, 1)
	};

	static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
	{
			.callback            = NULL,
			.p_user_data         = NULL,
			.p_transfers         = _readout_transfer,
			.number_of_transfers = sizeof(_readout_transfer) / sizeof(_readout_transfer[0])
	};

	i2c_schedule(&transaction);

}

static void ms5637_readout_cb(ret_code_t result, void * p_user_data) {

	uint8_t *p_buffer = (uint8_t*)p_user_data;

	// handle D1
	uint32_t temp = (uint32_t) p_buffer[0] << 16;
	temp |= (uint32_t) p_buffer[1] << 8;
	temp |= p_buffer[2];

	if (result) {
		LOG_WARNING("BME read error");
		return;
	}

	switch (m_data.meas_type) {
	case eMS5637MeasCmdD1:
	{
		// handle d1
		m_data.d1 = temp;

		// set next state
		m_data.meas_type = eMS5637MeasCmdD2;
	}
		break;
	case eMS5637MeasCmdD2:
	{
		// handle d2
		m_data.d2 = temp;

		// set data process ready
		m_is_updated = true;

		// set next state
		m_data.meas_type = eMS5637MeasCmdD1;
	}
		break;
	default:
		break;
	}

	LOG_DEBUG("MS5637 read");
}

/**
 *
 * @param _handle
 * @return
 */
static bool ms5637_setCx(void) {

	uint8_t prom_val[3] = {0};
	uint16_t prom[MS5637_COEFFICIENT_COUNT+1] = {0};

	for (int i = 0; i < MS5637_COEFFICIENT_COUNT; i++) {

		ms5637_get_cx(i, prom_val);

		prom[i] = ((uint16_t) prom_val[0]) << 8;
		prom[i] |= prom_val[1];
		NRF_LOG_DEBUG("Prom %u: 0x%X", i, prom[i]);
	}

	// Verify CRC4 in top 4 bits of prom[0] (follows AN520 but not directly...)
	if (!ms5637_crc_check(prom, (prom[MS5637_CRC_INDEX] & 0xF000) >> 12)) {
		LOG_ERROR("MS5637 Cx CRC fail");
		return false;
	}

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

static bool ms5637_compute_t_p(int32_t d1, int32_t d2) {

	if (!initialised)
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

	m_data.temp =  ((float)temp - (float)t2) / 100.f;

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
	m_data.press =  (float)p / 100.f;

	LOG_INFO("Pressure: %d mbar", (int) (m_pressure));
	LOG_INFO("Temperature: %d°", (int) (m_temperature));

	return true;
}

static void ms5637_reset() {

	ms5637_send_cmd(CMD_RESET);

}

void ms5637_init_sensor(void) {

	initialised = false;
	m_temperature = 20.;
	m_pressure = 999.;

	ms5637_reset();

	delay_ms(3);

	if (ms5637_setCx()) {
		LOG_ERROR("MS5637 CX failure");
		return;
	}

}

void ms5637_refresh(void) {

	if (!m_is_updated) return;
	m_is_updated = false;

	if (ms5637_compute_t_p(m_data.d1, m_data.d2))
		m_data.is_updated = true;
}

void ms5637_read_sensor(void) {

	if (m_data.has_started) {

		static uint8_t p_ans_buffer[MS5637_READOUT_SIZE] = {0};

		static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND readout_reg[] = {CMD_READ_ADC};

		static nrf_twi_mngr_transfer_t const _readout_transfer[] =
		{
				I2C_READ_REG(MS5637_TWI_ADDRESS, readout_reg, p_ans_buffer ,sizeof(p_ans_buffer))
		};

		static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
		{
				.callback            = ms5637_readout_cb,
				.p_user_data         = p_ans_buffer,
				.p_transfers         = _readout_transfer,
				.number_of_transfers = sizeof(_readout_transfer) / sizeof(_readout_transfer[0])
		};

		i2c_schedule(&transaction);

	} else {

		/// the first time, we need to trigger the measurement before reading the ADC
		m_data.has_started = true;
		m_data.meas_type = eMS5637MeasCmdD1;

	}

	switch (m_data.meas_type) {
	case eMS5637MeasCmdD1:
	{
		// now order d1
		ms5637_order_d1();
	} break;
	case eMS5637MeasCmdD2:
	{
		// now order d2
		ms5637_order_d2();
	} break;
	default:
		break;
	}

}

float ms5637_get_pressure(void) {
	return m_data.press;
}

float ms5637_get_temp(void) {
	return m_data.temp;
}

bool ms5637_is_data_ready(void) {
	return m_data.is_updated == true;
}

bool is_ms5637_updated(void) {
	return m_is_updated == 1;
}

void ms5637_clear_flags(void) {
	m_data.is_updated = 0;
}
