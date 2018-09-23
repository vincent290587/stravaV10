/*
 * AltiBaro.cpp
 *
 *  Created on: 23 sept. 2018
 *      Author: Vincent
 */

#include <math.h>
#include "assert_wrapper.h"
#include "segger_wrapper.h"
#include "AltiBaro.h"

AltiBaro::AltiBaro() {
	m_alti = 0.;
	sea_level_pressure = 1015.;
	correction = 0.;
	m_is_init = false;
}

/**
 *
 * @param alti_ Pointer to store altitude
 * @return True on success
 */
bool AltiBaro::computeAlti(float *alti_) {

	assert(alti_);
#ifdef TDD
	*alti_ = this->getAlti();

	LOG_INFO("Alti: %f", *alti_);
#else
	if (!m_is_init) return false;

	// m_temperature, m_pressure;
	*alti_ = this->pressureToAltitude(this->m_pressure);
#endif

	return true;
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

  float res;

  res = 44330.0 * (1.0 - pow(atmospheric / sea_level_pressure, 0.1903));
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

  sea_level_pressure = atmospheric / pow(1.0 - (altitude/44330.0), 5.255);

  m_is_init = true;

  return;
}
