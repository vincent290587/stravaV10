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
 * @param angle
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

	float angle_rad = angle * PI / 180.;

	// coordonnees dans (cx, cy)
	tmp1 = x1 - cx;
	tmp2 = y1 - cy;

	// rotation dans (cx, cy)
	tmp3 = tmp1 * cos(angle_rad) - tmp2 * sin(angle_rad);
	tmp4 = tmp1 * sin(angle_rad) + tmp2 * cos(angle_rad);

	// coordonnees dans (0, 0)
	x2 = tmp3 + cx;
	y2 = tmp4 + cy;

	//NRF_LOG_INFO("Nouveaux points: %u %u\r\n", x2, y2);
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
  dlon *= PI / 180.;
  lat1 *= PI / 180.;
  lat2 *= PI / 180.;
  float a1 = sin(dlon) * cos(lat2);
  float a2 = sin(lat1) * cos(lat2) * cos(dlon);
  a2 = cos(lat1) * sin(lat2) - a2;
  a2 = atan2(a1, a2);
  if (a2 < 0.0)
  {
    a2 += TWO_PI;
  }
  a2 *= 180 / PI;
  return a2;
}


String _fmkstr(float value, unsigned int nb_digits) {

	String res;

	if (fabs(value) > 100000) {
		res = "---";
		return res;
	}

	int ent_val = (int) value;
	res = String(ent_val);

	if (nb_digits > 0) {
		res += ".";
		uint16_t dec_val = fabs(value - (float)ent_val) * pow(10, nb_digits);
		res += dec_val;
	}

	return res;
}

String _imkstr(int value) {

	String res = String(value);

	return res;
}

String _secjmkstr(uint32_t value, char sep) {

	char time_char[10];

	memset(time_char, 0, sizeof(time_char));

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

	uint32_t addition = ((millis() - date_.timestamp) / 1000.);
	uint32_t value = date_.secj;

	LOG_INFO("Addition to time: %lu\r\n", addition);

	return _secjmkstr(value + addition, sep);
}
