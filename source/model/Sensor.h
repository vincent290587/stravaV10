/*
 * Sensor.h
 *
 *  Created on: 19 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_SENSOR_H_
#define SOURCE_MODEL_SENSOR_H_

#include <string.h>
#include <stdint.h>
#include "millis.h"

/**
 *
 */
template <typename T>
class Sensor {
public:
	Sensor() {
		m_last_updated = 0;
		m_new_data     = false;
		memset(&data, 0, sizeof(T));
	}

	Sensor(Sensor const& copy) : Sensor() {
		memcpy(&data, &copy, sizeof(T));
	}

	void setIsUpdated(void) {
		m_last_updated = millis();
		m_new_data     = true;
	}

	void clearIsUpdated(void) {
		m_new_data     = false;
	}

	bool isUpdated(void) {
		return m_new_data;
	}

	uint32_t getAge() {
		if (!m_last_updated) return UINT32_MAX;
		return (millis() - m_last_updated);
	}

	uint32_t getLastUpdateTime() {
		return m_last_updated;
	}

	T getData() {
		m_new_data = false;
		return data;
	}

	Sensor& operator=(const T& data_) {
		memcpy(&data, &data_, sizeof(T));
		this->setIsUpdated();
		return *this;
	}

	T data;

private:
	uint32_t m_last_updated;
	bool     m_new_data;
};


#endif /* SOURCE_MODEL_SENSOR_H_ */
