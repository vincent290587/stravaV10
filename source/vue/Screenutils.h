/*
 * Screenutils.h
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#ifndef SOURCE_VUE_SCREENUTILS_H_
#define SOURCE_VUE_SCREENUTILS_H_

#include "Attitude.h"

#define PI 3.14159265
#define TWO_PI (2.*PI)

#define CLIP(X,Y,Z) (MIN(MAX(X,Y),Z))

void rotate_point(float angle, int16_t cx, int16_t cy,
		int16_t x1, int16_t y1, int16_t &x2, int16_t &y2);

float course_to (float lat1, float long1, float lat2, float long2);

String _imkstr(int value);

String _fmkstr(float value, unsigned int nb_digits);

String _secjmkstr(uint32_t value, char sep);

String _timemkstr(SDate& date_, char sep);

#endif /* SOURCE_VUE_SCREENUTILS_H_ */
