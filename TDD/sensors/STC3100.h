/*
 * STC3100.h
 *
 *  Created on: 1 mars 2017
 *      Author: Vincent
 */

#ifndef LIBRARIES_STC3100_H_
#define LIBRARIES_STC3100_H_


/*=========================================================================
    I2C 7-bit ADDRESS
    -----------------------------------------------------------------------*/
#define STC3100_ADDRESS                (0x70)

/*=========================================================================
    REGISTERS
    -----------------------------------------------------------------------*/

#define REG_MODE                0
#define REG_CONTROL             1
#define REG_CHARGE_LOW          (2)
#define REG_CHARGE_HIGH         3
#define REG_COUNTER_LOW         4
#define REG_COUNTER_HIGH        5
#define REG_CURRENT_LOW         6
#define REG_CURRENT_HIGH        7
#define REG_VOLTAGE_LOW         8
#define REG_VOLTAGE_HIGH        9
#define REG_TEMPERATURE_LOW     10
#define REG_TEMPERATURE_HIGH    11
#define REG_DEVICE_ID           24

/*=========================================================================
 MODE SETTINGS
 -----------------------------------------------------------------------*/

#define MODE_RUN                0x10
#define STC_RESET               0x02

typedef enum {
	STC3100_MODE_ULTRAHIGHRES = 0u,
	STC3100_MODE_HIGHRES = 2u,
	STC3100_MODE_STANDARD = 4u
} stc3100_res_t;

typedef union {
	/**Used for simpler readings of the chip.*/
	uint8_t array[10];
	struct {
		uint8_t ChargeLow;
		/**Gas gauge charge data, bits 8-15*/
		uint8_t ChargeHigh;
		/**Number of conversions, bits 0-7*/
		uint8_t CounterLow;
		/**Number of conversions, bits 8-15*/
		uint8_t CounterHigh;
		/**Battery current value, bits 0-7*/
		uint8_t CurrentLow;
		/**Battery current value, bits 8-15*/
		uint8_t CurrentHigh;
		/**Battery voltage value, bits 0-7*/
		uint8_t VoltageLow;
		/**Battery voltage value, bits 8-15*/
		uint8_t VoltageHigh;
		/**Temperature value, bits 0-7*/
		uint8_t TemperatureLow;
		/**Temperature value, bits 8-15*/
		uint8_t TemperatureHigh;
	};
} tSTC31000Data;

typedef struct {
	/**Value of the voltage in mV.*/
	float Voltage;
	/**Value of the current in mA*/
	float Current;
	/**Value of the temperature in C*/
	float Temperature;
	/**Value of the current charge of the battery in mAh*/
	float Charge;
} tBatteryData;

/*=========================================================================*/

#define STC_RESET_REGS \
{                                          \
	REG_CONTROL,                           \
	STC_RESET                              \
}

#define STC_READ_ALL_REGS \
{                                          \
	REG_CHARGE_LOW                         \
}

#define STC3100_READ_ALL(p_reg, p_buffer) \
		I2C_READ_REG(STC3100_ADDRESS, p_reg, p_buffer, 10)


/*=========================================================================*/


class STC3100 {
public:
	STC3100(int32_t sensorID = -1);

	bool init(uint32_t r_sens = 100,
			stc3100_res_t res = STC3100_MODE_HIGHRES);
	void checkDevID(uint8_t dev_id);

	void shutdown(void);
	void reset(void);

	bool refresh(tSTC31000Data *_data);

	float getCurrent() const { return _current;}
	float getVoltage() const {return _voltage;}
	float getTemperature() const {return _temperature;}
	float getCorrectedVoltage(float int_res);

	float getAverageCurrent(void);

	float getCharge() const {
		return _charge;
	}
	void resetCharge(void) {
	}

	uint8_t _stc3100Mode;

private:
	int32_t _sensorID;
	uint8_t _deviceID;
	int32_t _r_sens;
	tSTC31000Data _stc_data;

	bool m_soft_reset;

	/**Value of the battery voltage in V*/
	float _voltage = 4.1;
	/**Value of the current in mA*/
	float _current = 15;
	/**Value of the temperature in °C*/
	float _temperature = 21;
	/**Value of the charge which went through the batt in mA.h */
	float _charge = 17;
	// counter
	float _counter = 0.7;

	void writeCommand(uint8_t reg, uint8_t value);
	void readChip();
	void computeVoltage();
	void computeCharge();
	void computeCurrent();
	void computeCounter();
	void computeTemp();

};

#endif /* LIBRARIES_STC3100_H_ */
