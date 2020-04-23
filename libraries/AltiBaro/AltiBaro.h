/*
 * AltiBaro.h
 *
 *  Created on: 23 sept. 2018
 *      Author: Vincent
 */

#ifndef LIBRARIES_ALTIBARO_ALTIBARO_H_
#define LIBRARIES_ALTIBARO_ALTIBARO_H_

#include <stdint.h>
#include "parameters.h"

class AltiBaro {
public:
	AltiBaro();
	bool computeAlti(float& alti_);
	bool computeVA(float& va_);

	void sleep(void);

    void seaLevelForAltitude(float altitude);
	void setCorrection(float cor_) {correction = cor_;}

	bool hasSeaLevelRef(void) {return m_is_init;}


	bool isUpdated(void);
	bool isDataReady(void);
	void sensorRefresh(void);
	void sensorRead(void);
	void sensorInit(void);

private:
	bool m_is_init;
	float sea_level_pressure;
	float correction;
	float m_alti_f;
	float m_va_f;
	uint16_t nb_filtering;

	float m_meas_buff[FILTRE_NB];

	void runFilter(void);
	float pressureToAltitude(float atmospheric);
};


#endif /* LIBRARIES_ALTIBARO_ALTIBARO_H_ */
