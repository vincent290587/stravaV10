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

#include "order1_filter.h"

float lp2_filter_coefficients[5] =
{
		// Scaled for floating point: 0.25.fs
		0.5010471990823734, 0.5010471990823734, 0, -0.0020943981647468628, 0// b0, b1, b2, a1, a2
};

static order1_filterType m_lp_filt;

AltiBaro::AltiBaro() {
#ifdef TDD
	m_alti = 0.;
#endif
	sea_level_pressure = 1015.;
	correction = 0.;
	nb_filtering = 0;
	m_is_init = false;
}

/**
 *
 * @param alti_ Pointer to store altitude
 * @return True on success
 */
bool AltiBaro::computeAlti(float& alti_) {

	if (!m_is_init) return false;

	// m_temperature, m_pressure;
	if (nb_filtering < 10) {
		alti_ = this->pressureToAltitude(this->m_pressure);
	} else {
		alti_ = order1_filter_readOutput(&m_lp_filt);
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
	order1_filter_writeInput(&m_lp_filt, &input);
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

	order1_filter_init(&m_lp_filt, lp2_filter_coefficients);

	m_is_init = true;

	return;
}
