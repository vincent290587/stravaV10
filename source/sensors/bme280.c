/*
 * bme280.c
 *
 *  Created on: 27 janv. 2019
 *      Author: Vincent
 */

#include "i2c.h"
#include "nrf_twi_mngr.h"
#include "bme280.h"
#include "millis.h"
#include "segger_wrapper.h"
#include "task_manager_wrapper.h"

#define BME280_TWI_ADDRESS              0x76

/**\name Macro to combine two 8 bit data's to form a 16 bit data */
#define BME280_CONCAT_BYTES(msb, lsb)            (((uint16_t)msb << 8) | (uint16_t)lsb)

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
static volatile bool m_is_updated = false;

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

	static nrf_twi_mngr_transaction_t NRF_TWI_MNGR_BUFFER_LOC_IND transaction =
	{
			.callback            = NULL,
			.p_user_data         = NULL,
			.p_transfers         = bme280_meas_transfer,
			.number_of_transfers = sizeof(bme280_meas_transfer) / sizeof(bme280_meas_transfer[0])
	};

	i2c_schedule(&transaction);

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

static bool bme280_compensate_press(bme280_data *data, int32_t adc_press)
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
		return false;
	}

	p = 1048576 - adc_press;
	p = (((p << 31) - var2) * 3125) / var1;
	var1 = (((int64_t)m_calib.dig_P9) * (p >> 13) * (p >> 13)) >> 25;
	var2 = (((int64_t)m_calib.dig_P8) * p) >> 19;
	p = ((p + var1 + var2) >> 8) + (((int64_t)m_calib.dig_P7) << 4);

	data->comp_press = (uint32_t)p;
	return true;
}

/*!
 *  @brief This internal API is used to parse the temperature and
 *  pressure calibration data and store it in device structure.
 */
static void parse_temp_press_calib_data(const uint8_t *reg_data)
{
	bme280_calib_data *calib_data = &m_calib;

    calib_data->dig_T1 = BME280_CONCAT_BYTES(reg_data[1], reg_data[0]);
    calib_data->dig_T2 = (int16_t)BME280_CONCAT_BYTES(reg_data[3], reg_data[2]);
    calib_data->dig_T3 = (int16_t)BME280_CONCAT_BYTES(reg_data[5], reg_data[4]);
    calib_data->dig_P1 = BME280_CONCAT_BYTES(reg_data[7], reg_data[6]);
    calib_data->dig_P2 = (int16_t)BME280_CONCAT_BYTES(reg_data[9], reg_data[8]);
    calib_data->dig_P3 = (int16_t)BME280_CONCAT_BYTES(reg_data[11], reg_data[10]);
    calib_data->dig_P4 = (int16_t)BME280_CONCAT_BYTES(reg_data[13], reg_data[12]);
    calib_data->dig_P5 = (int16_t)BME280_CONCAT_BYTES(reg_data[15], reg_data[14]);
    calib_data->dig_P6 = (int16_t)BME280_CONCAT_BYTES(reg_data[17], reg_data[16]);
    calib_data->dig_P7 = (int16_t)BME280_CONCAT_BYTES(reg_data[19], reg_data[18]);
    calib_data->dig_P8 = (int16_t)BME280_CONCAT_BYTES(reg_data[21], reg_data[20]);
    calib_data->dig_P9 = (int16_t)BME280_CONCAT_BYTES(reg_data[23], reg_data[22]);
    calib_data->dig_H1 = reg_data[25];
}

/*!
 *  @brief This internal API is used to parse the humidity calibration data
 *  and store it in device structure.
 */
static void parse_humidity_calib_data(const uint8_t *reg_data)
{
	bme280_calib_data *calib_data = &m_calib;
    int16_t dig_H4_lsb;
    int16_t dig_H4_msb;
    int16_t dig_H5_lsb;
    int16_t dig_H5_msb;

    calib_data->dig_H2 = (int16_t)BME280_CONCAT_BYTES(reg_data[1], reg_data[0]);
    calib_data->dig_H3 = reg_data[2];
    dig_H4_msb = (int16_t)(int8_t)reg_data[3] * 16;
    dig_H4_lsb = (int16_t)(reg_data[4] & 0x0F);
    calib_data->dig_H4 = dig_H4_msb | dig_H4_lsb;
    dig_H5_msb = (int16_t)(int8_t)reg_data[5] * 16;
    dig_H5_lsb = (int16_t)(reg_data[4] >> 4);
    calib_data->dig_H5 = dig_H5_msb | dig_H5_lsb;
    calib_data->dig_H6 = (int8_t)reg_data[6];
}

static void bme280_readout_cb(ret_code_t result, void * p_user_data) {

	bme280_data *buf = p_user_data;

	if (result) {
		LOG_WARNING("BME read error");
		return;
	}

	m_is_updated = true;

	memcpy(&m_data, buf, BME280_DATA_T_SIZE);

	LOG_DEBUG("BME read");

}

void bme280_refresh(void) {

	if (!m_is_updated) return;
	m_is_updated = false;

	uint8_t *buf = (uint8_t*) &m_data;

	int32_t adc_press, adc_temp;

	adc_press = (buf[0] << 12) | (buf[1] << 4) | (buf[2] >> 4);
	adc_temp = (buf[3] << 12) | (buf[4] << 4) | (buf[5] >> 4);

	bme280_compensate_temp(&m_data, adc_temp);
	LOG_DEBUG("BME temp: %d", m_data.comp_temp);

	if (!bme280_compensate_press(&m_data, adc_press)) {
		LOG_WARNING("BME press error");
		return;
	}

	m_data.is_updated = true;

	LOG_DEBUG("BME press: %d", m_data.comp_press / 256);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void bme280_init_sensor() {

	static uint8_t p_ans_buffer[2] = {0};

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

	delay_ms(50);

	{
		static uint8_t buffer1[BME280_TEMP_PRESS_CALIB_DATA_LEN + 2];
		static uint8_t buffer2[BME280_HUMIDITY_CALIB_DATA_LEN + 2];

		static uint8_t NRF_TWI_MNGR_BUFFER_LOC_IND wai_reg1[] = {
				BME280_DIG_T1_LSB_REG,
				BME280_DIG_H2_LSB_REG};

		static nrf_twi_mngr_transfer_t const bme280_init_transfers1[] =
		{
				I2C_READ_REG(BME280_TWI_ADDRESS, &wai_reg1[0], buffer1 , BME280_TEMP_PRESS_CALIB_DATA_LEN),
				I2C_READ_REG(BME280_TWI_ADDRESS, &wai_reg1[1], buffer2 , BME280_HUMIDITY_CALIB_DATA_LEN)
		};

		i2c_perform(NULL, bme280_init_transfers1, sizeof(bme280_init_transfers1) / sizeof(bme280_init_transfers1[0]), NULL);

		// process calibration
		parse_temp_press_calib_data(buffer1);
		parse_humidity_calib_data  (buffer2);
	}

	m_meas_config.osrs_p = SAMPLING_X16;
	m_meas_config.osrs_t = SAMPLING_X1;

	// set to sleep
	m_meas_config.mode = MODE_SLEEP;
	bme280_meas_config();

	// configure
	m_hum_config.osrs_h = SAMPLING_NONE;

	bme280_hum_config();

	m_cfg_config.filter = FILTER_X16;
	m_cfg_config.t_sb = STANDBY_MS_250;

	bme280_cfg_config();

	// go do measures !
	m_meas_config.mode = MODE_NORMAL;
	bme280_meas_config();

	LOG_INFO("BME init done");

}

void bme280_sleep(void) {

	m_meas_config.osrs_p = SAMPLING_NONE;
	m_meas_config.osrs_t = SAMPLING_NONE;

	// set to sleep
	m_meas_config.mode = MODE_SLEEP;
	bme280_meas_config();
}

void bme280_read_sensor(void) {

	static uint8_t p_ans_buffer[BME280_DATA_T_SIZE] = {0};

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

float bme280_get_pressure(void) {
	return m_data.comp_press / (256.0F * 100.0F);
}

float bme280_get_temp(void) {
	return m_data.comp_temp / (100.0F);
}

bool bme280_is_data_ready(void) {
	return m_data.is_updated == 1;
}

bool is_bme280_updated(void) {
	return m_is_updated == 1;
}

void bme280_clear_flags(void) {
	m_data.is_updated = 0;
}


