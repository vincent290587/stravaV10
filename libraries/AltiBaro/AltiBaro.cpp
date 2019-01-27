/*
 * AltiBaro.cpp
 *
 *  Created on: 23 sept. 2018
 *      Author: Vincent
 */

#include "math_wrapper.h"
#include "assert_wrapper.h"
#include "segger_wrapper.h"
#include "AltiBaro.h"
#include "parameters.h"
#include "utils.h"
#include "Model.h"

#ifdef USE_JSCOPE
#include "JScope.h"
JScope jscope;
#endif

#define ATT_VIT_ASC_COEFF0_MULT     0.

#define MOV_AV_NB_VAL               18

AltiBaro::AltiBaro() {
#ifdef TDD
	m_alti = 0.;
#endif
	sea_level_pressure = 1015.;
	correction = 0.;
	nb_filtering = 0;
	m_is_init = false;
	m_alti_f = 0.;
	m_va_f = 0.;

}

/**
 *
 * @param alti_ Pointer to store altitude
 * @return True on success
 */
bool AltiBaro::computeAlti(float& alti_) {

	if (!m_is_init) return false;

	// m_temperature, m_pressure;
	if (nb_filtering <= FILTRE_NB) {
		alti_ = this->pressureToAltitude(this->m_pressure);
	} else {
		alti_ = m_alti_f;
	}

	return true;
}

/**
 *
 * @param alti_ Pointer to store altitude
 * @return True on success
 */
bool AltiBaro::computeVA(float& va_) {

	if (!m_is_init) return false;

	// m_temperature, m_pressure;
	if (nb_filtering <= FILTRE_NB) {
		va_ = 0.;
	} else {
		va_ = m_va_f;
	}

	return true;
}

void AltiBaro::runFilter(void) {

	if (!m_is_init) return;

#ifdef TDD
	float input = this->getAlti();
#else
	float input = this->pressureToAltitude(this->m_pressure);
#endif

	static float xk1[MOV_AV_NB_VAL] = {0};
	static uint16_t ind = 0;
	xk1[ind++] = input;
	ind = ind % MOV_AV_NB_VAL;
	float input2 = 0.;
	for (int i=0; i< MOV_AV_NB_VAL; i++) {
		input2 += xk1[i] / MOV_AV_NB_VAL;
	}

	float _y[FILTRE_NB];
	float _x[FILTRE_NB];
	float _lrCoef[2];

	static float xk[FILTRE_NB] = {0};
	static uint16_t ind2 = 0;
	xk[ind2++] = input2;
	ind2 = ind2 % FILTRE_NB;

	// calcul de la vitesse ascentionnelle par regression lineaire
	uint16_t ind_tmp = ind2;
	for (int i = 0; i < FILTRE_NB; i++) {

		_x[i] = - i * MS5637_REFRESH_PER_MS / 1000.;

		ind_tmp += FILTRE_NB - 1;
		ind_tmp = ind_tmp % FILTRE_NB;

		_y[i] = xk[ind_tmp] + ATT_VIT_ASC_COEFF0_MULT * _x[i];

		LOG_DEBUG("%d ms %d mm", (int)(_x[i]*1000.), (int)(_y[i]*1000.));
	}

	_lrCoef[1] = _lrCoef[0] = 0.;

	// regression lineaire
	float corrsq = simpLinReg(_x, _y, _lrCoef, FILTRE_NB);

	m_va_f = _lrCoef[0] - ATT_VIT_ASC_COEFF0_MULT;
	m_alti_f = _lrCoef[1];

#ifdef USE_JSCOPE
	{
		// output some results to Segger JSCOPE
		jscope.inputData(input, 0);
		jscope.inputData(m_alti_f, 4);
		jscope.inputData(input2, 8);
		jscope.inputData(m_va_f, 12);
	}

	jscope.flush();
#endif

	// STEP 1 : on filtre altitude et vitesse
	if (corrsq > 0.8) {
	}

	LOG_INFO("#Vit. vert.= %d cm/s %d cm (corr= %f)",
			(int)(m_va_f*100), (int)(m_alti_f*100), corrsq);


	nb_filtering++;
}

/**************************************************************************/
/*!
    Calculates the altitude (in meters) from the specified atmospheric
    pressure (in hPa), and sea-level pressure (in hPa).

    @param  seaLevel      Sea-level pressure in hPa
    @param  atmospheric   Atmospheric pressure in hPa
 */
/**************************************************************************/
float AltiBaro::pressureToAltitude(float atmospheric)
{
	// Equation taken from BMP180 datasheet (page 16):
	//  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

	// Note that using the equation from wikipedia can give bad results
	// at high altitude.  See this thread for more information:
	//  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

	float res = 0.;

	ASSERT(sea_level_pressure != 0.);
	ASSERT(atmospheric / sea_level_pressure > 0.);

	res = 44330.0 * (1.0 - powf(atmospheric / sea_level_pressure, 0.1903));
	res -= correction;

	return res;
}


/**************************************************************************/
/*!
    Calculates the pressure at sea level (in hPa) from the specified altitude
    (in meters), and atmospheric pressure (in hPa).

    @param  altitude      Altitude in meters
    @param  atmospheric   Atmospheric pressure in hPa
 */
/**************************************************************************/
void AltiBaro::seaLevelForAltitude(float altitude, float atmospheric)
{
	// Equation taken from BMP180 datasheet (page 17):
	//  http://www.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf

	// Note that using the equation from wikipedia can give bad results
	// at high altitude.  See this thread for more information:
	//  http://forums.adafruit.com/viewtopic.php?f=22&t=58064

	ASSERT(atmospheric != 0.);

	sea_level_pressure = atmospheric / powf(1.0 - (altitude/44330.0), 5.255);

	ASSERT(sea_level_pressure != 0.);

	m_is_init = true;


#ifdef USE_JSCOPE
	jscope.init();
#endif

	return;
}
