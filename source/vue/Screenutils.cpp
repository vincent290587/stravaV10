/*
 * Screenutils.cpp
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#include <stdint.h>
#include <stdio.h>
#include <math.h>
#include <millis.h>
#include "WString.h"
#include "segger_wrapper.h"
#include <Screenutils.h>


/**
 * rotation d'angle angle et de centre (cx, cy)
 *
 * @param angle in degrees
 * @param cx
 * @param cy
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 */
void rotate_point(float angle, int16_t cx, int16_t cy,
		int16_t x1, int16_t y1, int16_t &x2, int16_t &y2) {

	float tmp1, tmp2, tmp3, tmp4;

	float angle_rad = angle * (float)M_PI / 180.f;

	// coordonnees dans (cx, cy)
	tmp1 = x1 - cx;
	tmp2 = y1 - cy;

	// rotation dans (cx, cy)
	tmp3 = tmp1 * cosf(angle_rad) - tmp2 * sinf(angle_rad);
	tmp4 = tmp1 * sinf(angle_rad) + tmp2 * cosf(angle_rad);

	// coordonnees dans (0, 0)
	x2 = (int16_t)(tmp3 + cx);
	y2 = (int16_t)(tmp4 + cy);

	//LOG_INFO("Nouveaux points: %u %u\r\n", x2, y2);
}

/**
 * returns course in degrees (North=0, West=270) from position 1 to position 2,
 * both specified as signed decimal-degrees latitude and longitude
 *
 * @param lat1
 * @param long1
 * @param lat2
 * @param long2
 * @return
 */
float course_to (float lat1, float long1, float lat2, float long2)
{
  // returns course in degrees (North=0, West=270) from position 1 to position 2,
  // both specified as signed decimal-degrees latitude and longitude.
  // Because Earth is no exact sphere, calculated course may be off by a tiny fraction.
  // Courtesy of Maarten Lamers
  float dlon = long2-long1;
  dlon *= (float)M_PI / 180.f;
  lat1 *= (float)M_PI / 180.f;
  lat2 *= (float)M_PI / 180.f;
  float a1 = sinf(dlon) * cosf(lat2);
  float a2 = sinf(lat1) * cosf(lat2) * cosf(dlon);
  a2 = cosf(lat1) * sinf(lat2) - a2;
  a2 = atan2f(a1, a2);
  if (a2 < 0.0f)
  {
    a2 += (float)TWO_PI;
  }
  a2 *= 180.f / (float)M_PI;
  return a2;
}


String _fmkstr(float value, unsigned int nb_digits) {

	String res;

	if (fabsf(value) > 100000) {
		res = "---";
		return res;
	}

	int ent_val = (int) value;

	if (ent_val == 0 && value < 0.0F) res = "-";

	res += String(ent_val);

	if (nb_digits > 0) {
		res += ".";

		for (uint16_t i=0; i < nb_digits; i++) {
			value = fabsf(value - (float)ent_val);
			value *= 10;
			ent_val = (int) value;

			uint32_t dec_val = uint32_t(value);
			res += dec_val;
		}

	}

	return res;
}

String _imkstr(int value) {

	String res = String(value);

	return res;
}

String _secjmkstr(uint32_t value, char sep) {

	char time_char[16];

	memset(time_char, 0, sizeof(time_char));

	if (value >= 86400) {

		String res = " --:--:--";
		return res;
	}

	uint8_t hours   = (uint8_t) (value / 3600);
	value -= hours * 3600;
	uint8_t minutes = (uint8_t) (value / 60);
	value -= minutes * 60;
	uint8_t seconds = (uint8_t) (value % 60);

	(void)snprintf(time_char, sizeof(time_char), "%02u%c%02u%c%02u",
			hours, sep, minutes, sep, seconds);

	String res = time_char;

	return res;
}

String _timemkstr(SDate& date_, char sep) {

	uint32_t addition = ((millis() - date_.timestamp) / 1000);
	uint32_t value = date_.secj;

	LOG_DEBUG("Addition to time: %lu", addition);

	return _secjmkstr(value + addition, sep);
}
