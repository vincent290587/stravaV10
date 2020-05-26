/* 
 * File:   utils.c
 * Author: vincent
 *
 * Created on October 27, 2015, 10:55 AM
 */


#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "utils.h"
#include "segger_wrapper.h"


#define BATT_INT_RES                   0.273f

#define FACTOR 100000.0f

static const float R1 = 6356752.;
static const float R2 = 6378137.;


float min(float val1, float val2) {
  if (val1 <= val2) return val1;
  else return val2;
}

float max(float val1, float val2) {
  if (val1 <= val2) return val2;
  else return val1;
}

float radians(float value) {
	return value * M_PI / 180.f;
}

float degrees(float value) {
	return value * 180.f / M_PI;
}

float sq(float value) {
	return value * value;
}

float regFen(float val_, float b1_i, float b1_f, float b2_i, float b2_f) {

  if (b1_f == b1_i) return b2_i;
  
  float x, res;
  // calcul x
  x = (val_ - b1_i) / (b1_f - b1_i);
  
  // calcul valeur: x commun
  res = x * (b2_f - b2_i) + b2_i;
  return res;
}

float regFenLim(float val_, float b1_i, float b1_f, float b2_i, float b2_f) {

  if (b1_f == b1_i) return b2_i;

  float x, res;
  // calcul x
  x = (val_ - b1_i) / (b1_f - b1_i);
  
  // calcul valeur: x commun
  res = x * (b2_f - b2_i) + b2_i;
  if (res < min(b2_i,b2_f)) res = min(b2_i,b2_f);
  if (res > max(b2_i,b2_f)) res = max(b2_i,b2_f);
  return res;
}

/**
 *
 * @param lat1 En degres
 * @param long1 En degres
 * @param lat2 En degres
 * @param long2 En degres
 * @return
 */
float distance_between2(float lat1, float long1, float lat2, float long2) {
  float delta = 3.141592f * (long1 - long2) / 180.f;
  float sdlong = sinf(delta);
  float cdlong = cosf(delta);
  lat1 = 3.141592f * (lat1) / 180.f;
  lat2 = 3.141592f * (lat2) / 180.f;
  float slat1 = sinf(lat1);
  float clat1 = cosf(lat1);
  float slat2 = sinf(lat2);
  float clat2 = cosf(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = delta*delta;
  delta += clat2 * sdlong * clat2 * sdlong;
  delta = sqrtf(delta);
  float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2f(delta, denom);
  return delta * 6369933.f;
}

/**
 *
 * @param lat1 En degres
 * @param long1 En degres
 * @param lat2 En degres
 * @param long2 En degres
 * @return
 */
float distance_between5(float lat1, float long1, float lat2, float long2) {
  float delta = 3.141592f * (long1 - long2) / 180.f;
  float sdlong = my_sin(delta);
  float cdlong = my_cos(delta);
  lat1 = 3.141592f * (lat1) / 180.f;
  lat2 = 3.141592f * (lat2) / 180.f;
  float slat1 = my_sin(lat1);
  float clat1 = my_cos(lat1);
  float slat2 = my_sin(lat2);
  float clat2 = my_cos(lat2);
  delta = (clat1 * slat2) - (slat1 * clat2 * cdlong);
  delta = delta*delta;
  delta += clat2 * sdlong * clat2 * sdlong;
  delta = my_sqrtf(delta);
  float denom = (slat1 * slat2) + (clat1 * clat2 * cdlong);
  delta = atan2f(delta, denom);
  return delta * 6369933.f;
}

/**
 * Approximation petits angles sur une Terre ellipsoidale
 *
 * @param lat1 En degres
 * @param long1 En degres
 * @param lat2 En degres
 * @param long2 En degres
 * @return
 */
float distance_between3(float lat1, float long1, float lat2, float long2) {

  static float Rm = 6356752.;
  static float latRm = 0.;

  float lat1rad = M_PI * lat1 / 180.f;

  float cos2lat1 = my_cos(lat1rad);
  cos2lat1 *= cos2lat1;

  if (fabsf(latRm - lat1rad) > 0.008f) {
	  latRm = lat1rad;
	  Rm = my_sqrtf(R1*R1*(1-cos2lat1) + R2*R2*cos2lat1);
  }

  // petits angles: tan = Id
  float deltalat = M_PI * (lat2 -lat1) / 180.f;
  float deltalon = M_PI * (long2-long1) / 180.f;

  float dhori  = deltalon * R2 * my_cos(lat1rad);
  float dverti = deltalat * Rm;

  // projection plane et pythagore
  float res = dhori*dhori + dverti*dverti;

  res = my_sqrtf(res);

  return res;
}

/**
 * Approximation petits angles sur une Terre ellipsoidale
 *
 * @param lat1 En degres
 * @param long1 En degres
 * @param lat2 En degres
 * @param long2 En degres
 * @return
 */
float distance_between4(float lat1, float long1, float lat2, float long2) {

  static float Rm = 6356752.;
  static float latRm = 0.;

  float lat1rad = 3.141592f * lat1 / 180.f;

  float cos2lat1 = powf(cosf(lat1rad), 2.f);

  if (fabsf(latRm - lat1rad) > 0.008f) {
	  latRm = lat1rad;
	  Rm = sqrtf(powf(R1,2.f)*(1-cos2lat1) + powf(R2,2.f)*cos2lat1);
  }

  // petits angles: tan = Id
  float deltalat = 3.141592f * (lat2 -lat1) / 180.f;
  float deltalon = 3.141592f * (long2-long1) / 180.f;

  float dhori  = deltalon * R2 * cosf(lat1rad);
  float dverti = deltalat * Rm;

  // projection plane et pythagore
  return sqrtf(dhori*dhori + dverti*dverti);
}

void calculePos (const char *nom, float *lat, float *lon) {

    char tab[15];
    int iLat;
    int iLon;
    
    if (!nom) {
      return;
    }

    strncpy(tab, nom, 5);
    tab[5] = '\0';
    iLat = toBase10(tab);
    if (lat) {
        *lat = (float) iLat / FACTOR - 90.f;
    }

    strncpy(tab, nom + 6, 2);
    strncpy(tab + 2, nom + 9, 3);
    tab[5] = '\0';
    iLon = toBase10(tab);
    if (lon) {
        *lon = (float) iLon / FACTOR - 180.f;
    }

    return;
}


long unsigned int toBase10 (char *entree) {

    char tab[15];

    if (!entree) {
        return 0;
    }
    
    if (!strstr(entree, ".")) {
        strncpy(tab, entree, 5);
        tab[5] = '\0';
    } else {
        strncpy(tab, entree, 2);
        strncpy(tab + 2, entree + 3, 3);
        tab[5] = '\0';
    }

    return strtoul(tab, NULL, 36);

}

uint32_t get_sec_jour(uint8_t hour_, uint8_t min_, uint8_t sec_)
{
  unsigned long res = 0;

  res = 3600 * hour_ + 60 * min_ + sec_;

  return res;
}


float compute2Complement(uint8_t msb, uint8_t lsb) {
	uint16_t t;
	uint16_t val;
	uint8_t tl=lsb, th=msb;
	float ret;

	if (th & 0b00100000) {
		t = th << 8;
		val = (t & 0xFF00) | (tl & 0x00FF);
		val -= 1;
		val = ~(val | 0b1110000000000000);
		//LOG_INFO("Raw 2c1: %u\r\n", val);
		ret = (float)val;
	} else {
		t = (th & 0xFF) << 8;
		val = (t & 0xFF00) | (tl & 0x00FF);
		//LOG_INFO("Raw 2c2: %u\r\n", val);
		ret = (float)-val;
	}

	return ret;
}

/**
 *
 * @param tensionValue in Volts
 * @param current
 * @return Percentage between 0 and 100
 */
float percentageBatt(float tensionValue, float current) {

    float fp_ = 0.;

    tensionValue += current * BATT_INT_RES / 1000.f;

    if (tensionValue > 4.2f) {
			fp_ = 100.;
    } else if (tensionValue > 3.78f) {
        fp_ = 536.24f * tensionValue * tensionValue * tensionValue;
		fp_ -= 6723.8f * tensionValue * tensionValue;
        fp_ += 28186.f * tensionValue - 39402.f;

		if (fp_ > 100.f) fp_ = 100.f;

    } else if (tensionValue > 3.2f) {
        fp_ = powf(10.f, -11.4f) * powf(tensionValue, 22.315f);
    } else {
        fp_ = 0.f;
    }

    return fp_;
}


void encode_uint16 (uint8_t* dest, uint16_t input) {
	dest[0] = (uint8_t) (input & 0xFF);
	dest[1] = (uint8_t) ((input & 0xFF00) >> 8);
}

void encode_uint32 (uint8_t* dest, uint32_t input) {
	dest[0] = (uint8_t) (input & 0xFF);
	dest[1] = (uint8_t) ((input & 0xFF00) >> 8);
	dest[2] = (uint8_t) ((input & 0xFF0000) >> 16);
	dest[3] = (uint8_t) ((input & 0xFF000000) >> 24);
}

uint16_t decode_uint16 (uint8_t* dest) {
	uint16_t res = dest[0];
	res |=  dest[1] << 8;
	return res;
}

uint32_t decode_uint32 (uint8_t* dest) {
	uint32_t res = dest[0];
	res |=  dest[1] << 8;
	res |=  dest[2] << 16;
	res |=  dest[3] << 24;
	return res;
}

void const_char_to_buffer(const char *str_, uint8_t *buff_, uint16_t max_size) {

	memset(buff_, 0, max_size);

	for (uint8_t i=0; i < strlen(str_); i++) {

		if (i==max_size) {
			return;
		}

		buff_[i] = str_[i];

	}

}

/**
 * http://en.wikipedia.org/wiki/Simple_linear_regression
 *
 * @param x input arrau horizontal
 * @param y input arrau vertical
 * @param lrCoef slope=lrCoef[0] and intercept=lrCoef[1]
 * @param n length of the x and y arrays.
 *
 * @return Square of the correlation
 */
float simpLinReg(float* x, float* y, float* lrCoef, int n) {
	// pass x and y arrays (pointers), lrCoef pointer, and n.  The lrCoef array is comprised of the slope=lrCoef[0] and intercept=lrCoef[1].  n is length of the x and y arrays.
	// http://en.wikipedia.org/wiki/Simple_linear_regression

	// initialize variables
	float xbar = 0;
	float ybar = 0;
	float xybar = 0;
	float xsqbar = 0;
	float ysqbar = 0;

	// calculations required for linear regression
	for (int i = 0; i < n; i++) {
		xbar = xbar + x[i];
		ybar = ybar + y[i];
		xybar = xybar + x[i] * y[i];
		xsqbar = xsqbar + x[i] * x[i];
		ysqbar = ysqbar + y[i] * y[i];
	}
	xbar = xbar / n;
	ybar = ybar / n;
	xybar = xybar / n;
	xsqbar = xsqbar / n;
	ysqbar = ysqbar / n;

	float corr = xybar - xbar*ybar;
	corr *= corr;
	float corr_denomsq = (xsqbar - xbar*xbar)*(ysqbar - ybar*ybar);

	// simple linear regression algorithm
	lrCoef[0] = (xybar - xbar * ybar) / (xsqbar - xbar * xbar);
	lrCoef[1] = ybar - lrCoef[0] * xbar;

	if (corr_denomsq == 0.0f) {
		corr = 1.0f;
	} else {
		corr /= corr_denomsq;
	}

	return corr;
}

/**
 * Calculates the CRC of an array
 */
uint8_t calculate_crc(uint8_t *addr, uint16_t len) {
	uint8_t crc=0;
	for (uint8_t i=0; i<len;i++) {
		uint8_t inbyte = addr[i];
		for (uint8_t j=0;j<8;j++) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix)
				crc ^= 0x8C;
			inbyte >>= 1;
		}
	}

	return crc;
}


/**
 * Returns floor of square root of x
 */
int floorSqrt(int x)
{
	// Base cases
	if (x == 0 || x == 1)
		return x;

	// Staring from 1, try all numbers until
	// i*i is greater than or equal to x.
	int i = 1, result = 1;
	while (result <= x)
	{
		i++;
		result = i * i;
	}
	return i - 1;
}

uint32_t date_to_timestamp(uint32_t sec_j, uint8_t day, uint8_t month, uint16_t year) {

	static const uint32_t fitSystemTimeOffset = 631065600; // Needed for conversion from UNIX time to FIT time
	static const uint8_t day_per_month[13] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
	uint32_t timestamp = 86400;

	if (day < 1 || day > 31 ||
			month < 1 || month > 12 ||
			year < 1970 || year > 2142 ||
			sec_j > 86400) {
		return 0;
	}

	// iterate on the years
	for (uint16_t curY = 1970; curY < year; curY++) {

		uint8_t is_leap = (curY & 0b11) == 0;

		timestamp += 86400 * 365;

		if (is_leap) {

			timestamp += 86400;
		}
	}

	// current year : completed months
	for (uint8_t curM = 1; curM < sizeof(day_per_month) && curM < month; curM++) {

		timestamp += 86400 * day_per_month[curM];

	}

	// current month: completed days
	if (day > 1) {
		timestamp += 86400 * (day - 1);
	}

	timestamp += sec_j;

	if (fitSystemTimeOffset > timestamp) {
		// we are before garmin creation date
		return 0;
	}

	return timestamp - fitSystemTimeOffset;
}
