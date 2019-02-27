/*
 * bme280.c
 *
 *  Created on: 27 janv. 2019
 *      Author: Vincent
 */

#include <stdbool.h>
#include "i2c.h"
#include "nrf_twi_mngr.h"
#include "bme280.h"
#include "nrf_delay.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

#define BME280_TWI_ADDRESS              0x76

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

#define BME280_READ_ALL(p_cmd, p_mag_buffer, p_buffer) \
		I2C_READ_REG(BME280_TWI_ADDRESS, p_cmd, p_buffer, 6)


static bme280_calib_data m_calib;

static sCtrlHum  m_hum_config;
static sConfig   m_cfg_config;
static sCtrlMeas m_meas_config;

static bme280_data m_data;

static bool bme280_is_busy(void) {

	static uint8_t p_ans_buffer[2] = {0};

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND busy_reg1[] = {BME280_STAT_REG};

	static nrf_twi_mngr_transfer_t const bme280_busy_xfer[] =
	{
			I2C_READ_REG(BME280_TWI_ADDRESS, busy_reg1, p_ans_buffer , 1)
	};

	i2c_perform(NULL, bme280_busy_xfer, sizeof(bme280_busy_xfer) / sizeof(bme280_busy_xfer[0]), NULL);

	return (p_ans_buffer[0] & (1 << 0)) != 0;
}

static void bme280_hum_config(void) {

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND hum_config[2] = { BME280_CTRL_HUMIDITY_REG, 0x00 };

	hum_config[1] = m_hum_config.osrs_h;

	static nrf_twi_mngr_transfer_t const bme280_hum_transfer[] =
	{
			I2C_WRITE(BME280_TWI_ADDRESS, hum_config, 2)
	};

	i2c_perform(NULL, bme280_hum_transfer, sizeof(bme280_hum_transfer) / sizeof(bme280_hum_transfer[0]), NULL);

	LOG_INFO("BME humidity configured");

}

static void bme280_cfg_config(void) {

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND cfg_config[2] = { BME280_CONFIG_REG, 0x00 };

	cfg_config[1] = (m_cfg_config.t_sb << 5) | (m_cfg_config.filter << 3) | m_cfg_config.spi3w_en;

	static nrf_twi_mngr_transfer_t const bme280_cfg_transfer[] =
	{
			I2C_WRITE(BME280_TWI_ADDRESS, cfg_config, 2)
	};

	i2c_perform(NULL, bme280_cfg_transfer, sizeof(bme280_cfg_transfer) / sizeof(bme280_cfg_transfer[0]), NULL);

	LOG_INFO("BME CFG configured");

}

static void bme280_meas_config(void) {

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND meas_config[2] = { BME280_CTRL_MEAS_REG, 0x00 };

	meas_config[1] = (m_meas_config.osrs_t << 5) | (m_meas_config.osrs_p << 3) | m_meas_config.mode;

	static nrf_twi_mngr_transfer_t const bme280_meas_transfer[] =
	{
			I2C_WRITE(BME280_TWI_ADDRESS, meas_config, 2)
	};

	i2c_perform(NULL, bme280_meas_transfer, sizeof(bme280_meas_transfer) / sizeof(bme280_meas_transfer[0]), NULL);

	LOG_INFO("BME measurement configured");

}

/*
 * Compensation code taken from BME280 datasheet, Section 4.2.3
 * "Compensation formula".
 */
static void bme280_compensate_temp(bme280_data *data, int32_t adc_temp)
{
	int32_t var1, var2;

	var1 = (((adc_temp >> 3) - ((int32_t)m_calib.dig_T1 << 1)) *
			((int32_t)m_calib.dig_T2)) >> 11;
	var2 = (((((adc_temp >> 4) - ((int32_t)m_calib.dig_T1)) *
			((adc_temp >> 4) - ((int32_t)m_calib.dig_T1))) >> 12) *
			((int32_t)m_calib.dig_T3)) >> 14;

	data->t_fine = var1 + var2;
	data->comp_temp = (data->t_fine * 5 + 128) >> 8;
}

static void bme280_compensate_press(bme280_data *data, int32_t adc_press)
{
	int64_t var1, var2, p;

	var1 = ((int64_t)data->t_fine) - 128000;
	var2 = var1 * var1 * (int64_t)m_calib.dig_P6;
	var2 = var2 + ((var1 * (int64_t)m_calib.dig_P5) << 17);
	var2 = var2 + (((int64_t)m_calib.dig_P4) << 35);
	var1 = ((var1 * var1 * (int64_t)m_calib.dig_P3) >> 8) +
			((var1 * (int64_t)m_calib.dig_P2) << 12);
	var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)m_calib.dig_P1) >> 33;

	/* Avoid exception caused by division by zero. */
	if (var1 == 0) {
		data->comp_press = 0U;
		return;
	}

	p = 1048576 - adc_press;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)m_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)m_calib.dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)m_calib.dig_P7) << 4);

	data->comp_press = (uint32_t)p;
}

static void bme280_readout_cb(ret_code_t result, void * p_user_data) {

	int32_t adc_press, adc_temp;
	uint8_t *buf = p_user_data;

	if (result) {
		LOG_INFO("BME read error");
		return;
	}

	LOG_INFO("BME read");

	adc_press = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
	adc_temp = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);

	bme280_compensate_temp(&m_data, adc_temp);
	bme280_compensate_press(&m_data, adc_press);

	m_data.is_updated = true;

	LOG_INFO("BME temp: %d", m_data.comp_temp);
	LOG_INFO("BME press: %d", m_data.comp_press / 256);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void bme280_init_sensor() {

	static uint8_t p_ans_buffer[2] = {0};

#ifndef _DEBUG_TWI

	{
		static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND cal_reg1[] = {BME280_CHIP_ID_REG};

		static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND bme_config[2] = { BME280_RST_REG, 0xB6 };

		static nrf_twi_mngr_transfer_t const bme280_init_transfers2[] =
		{
				I2C_READ_REG(BME280_TWI_ADDRESS, cal_reg1, p_ans_buffer , 1),
				I2C_WRITE(BME280_TWI_ADDRESS, bme_config, 2)
		};

		i2c_perform(NULL, bme280_init_transfers2, sizeof(bme280_init_transfers2) / sizeof(bme280_init_transfers2[0]), NULL);
	}

	LOG_INFO("BME iD: %u", p_ans_buffer[0]);
	if (p_ans_buffer[0] != 0x60) {
		LOG_ERROR("BME wrong iD: 0x%02X != 0x60", p_ans_buffer[0]);
		return;
	}

	LOG_WARNING("BME iD: %u", p_ans_buffer[0]);

	nrf_delay_ms(500);

	{
		static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND wai_reg1[] = {BME280_CHIP_ID_REG,
				BME280_DIG_T1_LSB_REG,
				BME280_DIG_P1_LSB_REG,
				BME280_DIG_H1_REG,
				BME280_DIG_H2_LSB_REG};

		static nrf_twi_mngr_transfer_t const bme280_init_transfers1[] =
		{
				I2C_READ_REG(BME280_TWI_ADDRESS, wai_reg1, p_ans_buffer , 1),
				I2C_READ_REG(BME280_TWI_ADDRESS, wai_reg1+1, &m_calib.dig_T1 , BME280_CALIB_T_SIZE),
				I2C_READ_REG(BME280_TWI_ADDRESS, wai_reg1+2, &m_calib.dig_P1 , BME280_CALIB_P_SIZE),
				I2C_READ_REG(BME280_TWI_ADDRESS, wai_reg1+3, &m_calib.dig_H1 , BME280_CALIB_H1_SIZE),
				I2C_READ_REG(BME280_TWI_ADDRESS, wai_reg1+4, &m_calib.dig_H2 , BME280_CALIB_H2_SIZE)
		};

		i2c_perform(NULL, bme280_init_transfers1, sizeof(bme280_init_transfers1) / sizeof(bme280_init_transfers1[0]), NULL);
	}

#else

	BME280FXOS_ReadReg(nullptr, BME280_CHIP_ID_REG, p_ans_buffer, 1);

#endif

	m_hum_config.osrs_h = SAMPLING_NONE;

	m_cfg_config.filter = FILTER_X16;
	m_cfg_config.t_sb = STANDBY_MS_250;

	m_meas_config.mode = MODE_NORMAL;
	m_meas_config.osrs_p = SAMPLING_X16;
	m_meas_config.osrs_t = SAMPLING_X1;

	bme280_hum_config();
	bme280_cfg_config();
	bme280_meas_config();

	LOG_WARNING("BME init done");

}

void bme280_read_sensor(void) {

	static uint8_t p_ans_buffer[6] = {0};

	static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND readout_reg[] = {BME280_PRESSURE_MSB_REG};

	static nrf_twi_mngr_transfer_t const bme280_readout_transfer[] =
	{
			I2C_READ_REG(BME280_TWI_ADDRESS, readout_reg, p_ans_buffer ,sizeof(p_ans_buffer))
	};

	static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
	{
			.callback            = bme280_readout_cb,
			.p_user_data         = p_ans_buffer,
			.p_transfers         = bme280_readout_transfer,
			.number_of_transfers = sizeof(bme280_readout_transfer) / sizeof(bme280_readout_transfer[0])
	};

	i2c_schedule(&transaction);

}

bme280_data *bme280_get_data_handle(void) {
	return &m_data;
}

bool bme280_is_updated(void) {
	return m_data.is_updated == 1;
}
