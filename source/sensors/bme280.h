/*
 * bme280.h
 *
 *  Created on: 27 janv. 2019
 *      Author: Vincent
 */

#ifndef BME280_H_
#define BME280_H_

#include <stdint.h>
#include <stdbool.h>

#define BME280_DATA_T_SIZE                6u

#define BME280_TEMP_PRESS_CALIB_DATA_LEN  26u
#define BME280_HUMIDITY_CALIB_DATA_LEN    7u

//Register names:
#define BME280_DIG_T1_LSB_REG			0x88
#define BME280_DIG_T1_MSB_REG			0x89
#define BME280_DIG_T2_LSB_REG			0x8A
#define BME280_DIG_T2_MSB_REG			0x8B
#define BME280_DIG_T3_LSB_REG			0x8C
#define BME280_DIG_T3_MSB_REG			0x8D
#define BME280_DIG_P1_LSB_REG			0x8E
#define BME280_DIG_P1_MSB_REG			0x8F
#define BME280_DIG_P2_LSB_REG			0x90
#define BME280_DIG_P2_MSB_REG			0x91
#define BME280_DIG_P3_LSB_REG			0x92
#define BME280_DIG_P3_MSB_REG			0x93
#define BME280_DIG_P4_LSB_REG			0x94
#define BME280_DIG_P4_MSB_REG			0x95
#define BME280_DIG_P5_LSB_REG			0x96
#define BME280_DIG_P5_MSB_REG			0x97
#define BME280_DIG_P6_LSB_REG			0x98
#define BME280_DIG_P6_MSB_REG			0x99
#define BME280_DIG_P7_LSB_REG			0x9A
#define BME280_DIG_P7_MSB_REG			0x9B
#define BME280_DIG_P8_LSB_REG			0x9C
#define BME280_DIG_P8_MSB_REG			0x9D
#define BME280_DIG_P9_LSB_REG			0x9E
#define BME280_DIG_P9_MSB_REG			0x9F
#define BME280_DIG_H1_REG				0xA1
#define BME280_CHIP_ID_REG				0xD0 //Chip ID
#define BME280_RST_REG					0xE0 //Softreset Reg
#define BME280_DIG_H2_LSB_REG			0xE1
#define BME280_DIG_H2_MSB_REG			0xE2
#define BME280_DIG_H3_REG				0xE3
#define BME280_DIG_H4_MSB_REG			0xE4
#define BME280_DIG_H4_LSB_REG			0xE5
#define BME280_DIG_H5_MSB_REG			0xE6
#define BME280_DIG_H6_REG				0xE7
#define BME280_CTRL_HUMIDITY_REG		0xF2 //Ctrl Humidity Reg
#define BME280_STAT_REG					0xF3 //Status Reg
#define BME280_CTRL_MEAS_REG			0xF4 //Ctrl Measure Reg
#define BME280_CONFIG_REG				0xF5 //Configuration Reg
#define BME280_PRESSURE_MSB_REG			0xF7 //Pressure MSB
#define BME280_PRESSURE_LSB_REG			0xF8 //Pressure LSB
#define BME280_PRESSURE_XLSB_REG		0xF9 //Pressure XLSB
#define BME280_TEMPERATURE_MSB_REG		0xFA //Temperature MSB
#define BME280_TEMPERATURE_LSB_REG		0xFB //Temperature LSB
#define BME280_TEMPERATURE_XLSB_REG		0xFC //Temperature XLSB
#define BME280_HUMIDITY_MSB_REG			0xFD //Humidity MSB
#define BME280_HUMIDITY_LSB_REG			0xFE //Humidity LSB

enum sensor_sampling {
	SAMPLING_NONE = 0b000,
	SAMPLING_X1   = 0b001,
	SAMPLING_X2   = 0b010,
	SAMPLING_X4   = 0b011,
	SAMPLING_X8   = 0b100,
	SAMPLING_X16  = 0b101
};

/**************************************************************************/
/*!
            @brief  power modes
 */
/**************************************************************************/
enum sensor_mode {
	MODE_SLEEP  = 0b00,
	MODE_FORCED = 0b01,
	MODE_NORMAL = 0b11
};

/**************************************************************************/
/*!
            @brief  filter values
 */
/**************************************************************************/
enum sensor_filter {
	FILTER_OFF = 0b000,
	FILTER_X2  = 0b001,
	FILTER_X4  = 0b010,
	FILTER_X8  = 0b011,
	FILTER_X16 = 0b100
};

/**************************************************************************/
/*!
            @brief  standby duration in ms
 */
/**************************************************************************/
enum standby_duration {
	STANDBY_MS_0_5  = 0b000,
	STANDBY_MS_10   = 0b110,
	STANDBY_MS_20   = 0b111,
	STANDBY_MS_62_5 = 0b001,
	STANDBY_MS_125  = 0b010,
	STANDBY_MS_250  = 0b011,
	STANDBY_MS_500  = 0b100,
	STANDBY_MS_1000 = 0b101
};

typedef struct
{
	uint16_t dig_T1; ///< temperature compensation value
	int16_t  dig_T2; ///< temperature compensation value
	int16_t  dig_T3; ///< temperature compensation value

	uint16_t dig_P1; ///< pressure compensation value
	int16_t  dig_P2; ///< pressure compensation value
	int16_t  dig_P3; ///< pressure compensation value
	int16_t  dig_P4; ///< pressure compensation value
	int16_t  dig_P5; ///< pressure compensation value
	int16_t  dig_P6; ///< pressure compensation value
	int16_t  dig_P7; ///< pressure compensation value
	int16_t  dig_P8; ///< pressure compensation value
	int16_t  dig_P9; ///< pressure compensation value

	uint8_t  dig_H1; ///< humidity compensation value
	int16_t  dig_H2; ///< humidity compensation value
	uint8_t  dig_H3; ///< humidity compensation value
	int16_t  dig_H4; ///< humidity compensation value
	int16_t  dig_H5; ///< humidity compensation value
	int8_t   dig_H6; ///< humidity compensation value
	int32_t  t_fine;

} bme280_calib_data;

// The config register
typedef struct  {
	// inactive duration (standby time) in normal mode
	// 000 = 0.5 ms
	// 001 = 62.5 ms
	// 010 = 125 ms
	// 011 = 250 ms
	// 100 = 500 ms
	// 101 = 1000 ms
	// 110 = 10 ms
	// 111 = 20 ms
	unsigned int t_sb : 3;

	// filter settings
	// 000 = filter off
	// 001 = 2x filter
	// 010 = 4x filter
	// 011 = 8x filter
	// 100 and above = 16x filter
	unsigned int filter : 3;

	// unused - don't set
	unsigned int none : 1;
	unsigned int spi3w_en : 1;

} sConfig;

// The ctrl_meas register
typedef struct ctrl_meas {
	// temperature oversampling
	// 000 = skipped
	// 001 = x1
	// 010 = x2
	// 011 = x4
	// 100 = x8
	// 101 and above = x16
	unsigned int osrs_t : 3;

	// pressure oversampling
	// 000 = skipped
	// 001 = x1
	// 010 = x2
	// 011 = x4
	// 100 = x8
	// 101 and above = x16
	unsigned int osrs_p : 3;

	// device mode
	// 00       = sleep
	// 01 or 10 = forced
	// 11       = normal
	unsigned int mode : 2;

} sCtrlMeas;

// The ctrl_hum register
typedef struct  {
	// unused - don't set
	unsigned int none : 5;

	// pressure oversampling
	// 000 = skipped
	// 001 = x1
	// 010 = x2
	// 011 = x4
	// 100 = x8
	// 101 and above = x16
	unsigned int osrs_h : 3;

} sCtrlHum;

typedef struct  {
	/* Compensated values. */
	int32_t comp_temp;
	uint32_t comp_press;
	uint32_t comp_humidity;

	/* Carryover between temperature and pressure/humidity compensation. */
	int32_t t_fine;
	bool is_updated;
} bme280_data;


#ifdef __cplusplus
extern "C" {
#endif

void bme280_init_sensor(void);

void bme280_sleep(void);

void bme280_read_sensor(void);

void bme280_refresh(void);

float bme280_get_pressure(void);

float bme280_get_temp(void);

void bme280_clear_flags(void);

bool bme280_is_data_ready(void);

bool is_bme280_updated(void);

#ifdef __cplusplus
}
#endif

#endif /* BME280_H_ */
