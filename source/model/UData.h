/*
 * Sensor.h
 *
 *  Created on: 19 oct. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_MODEL_UDATA_H_
#define SOURCE_MODEL_UDATA_H_

#include <string.h>
#include <stdint.h>
#include "millis.h"

/**
 *
 */
template <typename T>
class UData {
public:
	UData() {
		m_last_updated = 0;
		memset(&m_data, 0, sizeof(T));
	}

	UData(UData const& copy) {
		memcpy(&m_data, &copy, sizeof(T));
	}

	void setUpdateTime(void) {
		m_last_updated = millis();
	}

	T& getData() {
		return m_data;
	}

	uint32_t getAge() {
		if (!m_last_updated) return UINT32_MAX;
		return (millis() - m_last_updated);
	}

	uint32_t getLastUpdateTime() {
		return m_last_updated;
	}

	bool equals(UData const& a) {
		if (memcmp(&m_data, &a.m_data, sizeof(T))) {
			return true;
		}
		return false;
	}

	UData& operator=(const T& data) {
		m_data = data;
		m_last_updated = millis();
		return *this;
	}

	UData& operator=(const UData& data) {
		m_data = data.m_data;
		m_last_updated = millis();
		return *this;
	}

	UData& operator!(void) {
		m_data = !m_data;
		return *this;
	}

	bool operator==(T const& a) {
		return (m_data == a);
	}

	bool operator==(UData const& a) {
		return this->equals(a);
	}

	bool operator!=(T const& a) {
		return !(m_data == a);
	}

	bool operator!=(UData const& a) {
		return !(this->equals(a));
	}

	T m_data;

protected:
	uint32_t m_last_updated;

};

/**
 *
 */
template <typename T>
class NData {
public:
	NData() {
		m_new_data = false;
		memset(&m_data, 0, sizeof(T));
	}

	NData(NData const& copy) {
		memcpy(&m_data, &copy, sizeof(T));
	}

	void setIsUpdated(void) {
		m_new_data = true;
	}

	T& getData() {
		m_new_data = false;
		return m_data;
	}

	bool hasNewData() {
		return m_new_data;
	}

	bool equals(NData const& a) {
		if (memcmp(&m_data, &a.m_data, sizeof(T))) {
			return true;
		}
		return false;
	}

	NData& operator=(const T& data) {
		if (m_data != data) {
			m_data = data;
			m_new_data = true;
		}
		return *this;
	}

	NData& operator=(const NData& data) {
		if (!this->equals(data)) {
			memcpy(&m_data, &data.m_data, sizeof(T));
			m_new_data = true;
		}
		return *this;
	}

	bool operator==(NData const& a) {
		return this->equals(a);
	}

	bool operator!=(NData const& a) {
	    return !(this->equals(a)); //On utilise l'opérateur == qu'on a défini précédemment !
	}

	T m_data;

protected:
	bool m_new_data;

};

#endif /* SOURCE_MODEL_UDATA_H_ */
