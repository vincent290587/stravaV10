/*
 * MS5637.h
 *
 *  Created on: 26 févr. 2017
 *      Author: Vincent
 */

#ifndef LIBRARIES_MS5637_H_
#define LIBRARIES_MS5637_H_


/* Module supports a range of lower oversampling levels, for faster
   less accurate results.

   Default is maximum accuracy.
 */
enum BaroOversampleLevel {
	OSR_256, OSR_512, OSR_1024, OSR_2048, OSR_4096, OSR_8192 };

typedef enum {
	CELSIUS,
	FAHRENHEIT
} TempUnit;

typedef enum {
	eMS5637MeasCmdTemp,
	eMS5637MeasCmdPress,
} eMS5637MeasType;

typedef struct _ms5637_handle
{
	uint8_t press_adc[3];
	uint8_t temp_adc[3];
	eMS5637MeasType meas_type;
} ms5637_handle_t;

#define BARO_LEVEL    OSR_8192

/*=========================================================================*/

class MS5637 {
public:
	MS5637();
	void init(void);
	bool setCx(ms5637_handle_t *_handle);

	/* Update both temperature and pressure together. This takes less
     time than calling each function separately (as pressure result
     depends on temperature.) Returns true for success, false on an
     error */

	void refresh(ms5637_handle_t *_handle);
	bool computeTempAndPressure(int32_t d1, int32_t d2);

	uint32_t reset();

	inline bool isOK() { return initialised && m_err == 0; }

	float getTemperature() const {
		return m_temperature;
	}

	void setTemperature(float temperature) {
		m_temperature = temperature;
	}

	float m_temperature;
	float m_pressure;

private:
	bool initialised;
	int8_t m_err;
	uint16_t c1,c2,c3,c4,c5,c6; // Calibration constants used in producing results

	bool crc_check(uint16_t *n_prom, uint8_t crc);
	uint32_t takeReading(uint8_t trigger_cmd, BaroOversampleLevel oversample_level=BARO_LEVEL);

	bool wireWriteByte(uint8_t val);
	int wireReadDataBlock(uint8_t reg, uint8_t *val, unsigned int len);
};


#endif /* LIBRARIES_MS5637_H_ */
