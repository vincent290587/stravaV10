/*
 * AltiBaro.h
 *
 *  Created on: 23 sept. 2018
 *      Author: Vincent
 */

#ifndef LIBRARIES_ALTIBARO_ALTIBARO_H_
#define LIBRARIES_ALTIBARO_ALTIBARO_H_

#include <stdint.h>

class AltiBaro {
public:
	AltiBaro();
	bool computeAlti(float& alti_);
	bool computeVA(float& va_);

    void seaLevelForAltitude(float altitude);
	void setCorrection(float cor_) {correction = cor_;}

	bool hasSeaLevelRef(void) {return m_is_init;}

	void runFilter(void);

	bool isUpdated(void);
	bool isDataReady(void);
	void sensorRefresh(void);
	void sensorRead(void);
	void sensorInit(void);

#ifdef TDD
	float getAlti() const {
		return m_alti;
	}

	void setAlti(float alti) {
		m_alti = alti;
	}

	void setPressure(float press) {
		sea_level_pressure = press;
	}
#endif

private:
	bool m_is_init;
	float sea_level_pressure;
	float correction;
	float m_alti_f;
	float m_va_f;
	uint32_t nb_filtering;

#ifdef TDD
	float m_alti;
#endif

	float pressureToAltitude(float atmospheric);
};


#endif /* LIBRARIES_ALTIBARO_ALTIBARO_H_ */
