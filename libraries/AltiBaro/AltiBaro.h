/*
 * AltiBaro.h
 *
 *  Created on: 23 sept. 2018
 *      Author: Vincent
 */

#ifndef LIBRARIES_ALTIBARO_ALTIBARO_H_
#define LIBRARIES_ALTIBARO_ALTIBARO_H_

#include <stdint.h>
#include "MS5637.h"

class AltiBaro : public MS5637 {
public:
	AltiBaro();
	bool computeAlti(float *alti_);

    void seaLevelForAltitude(float altitude, float atmospheric);
	void setCorrection(float cor_) {correction = cor_;}

	float getAlti() const {
		return m_alti;
	}

	void setAlti(float pressure) {
		m_alti = pressure;
	}


private:
	bool m_is_init;
	float m_alti;
	float sea_level_pressure;
	float correction;

	float pressureToAltitude(float atmospheric);
};


#endif /* LIBRARIES_ALTIBARO_ALTIBARO_H_ */
