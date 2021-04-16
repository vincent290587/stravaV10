/* 
 * File:   utils.h
 * Author: vincent
 *
 * Created on October 27, 2015, 10:55 AM
 */

#ifndef UTILS_H
#define	UTILS_H

#include <stdint.h>
#include "math_wrapper.h"


#ifdef	__cplusplus
extern "C" {
#endif


float regFen(float val_, float b1_i, float b1_f, float b2_i, float b2_f) __attribute__ ((pure));

float regFenLim(float val_, float b1_i, float b1_f, float b2_i, float b2_f) __attribute__ ((pure));

static inline float toRadians(float angle) __attribute__ ((pure));
static inline float toRadians(float angle) {
  return ((float)M_PI * angle / 180.0f);
}
	
/**
 * distance_between5: 24ms
 * distance_between2: 24ms
 * distance_between3: 21ms
 * distance_between4: 40ms
 * distance_between: ?
 */
static inline float distance_between(float lat1, float lon1, float lat2, float lon2) __attribute__ ((pure));
static inline float distance_between(float lat1, float lon1, float lat2, float lon2) {
    const float two_r = 2.0f * 6371008.0f; // meters
//    const float sdlat = my_sin(toRadians(lat2 - lat1) / 2);
//    const float sdlon = my_sin(toRadians(lon2 - lon1) / 2);
    const float sdlat = (toRadians(lat2 - lat1) / 2);
    const float sdlon = (toRadians(lon2 - lon1) / 2);
//    const float q = sdlat * sdlat + my_cos(toRadians(lat1)) * my_cos(toRadians(lat2)) * sdlon * sdlon;
    const float q = sdlat * sdlat + 0.5f * (1.0f + my_cos(toRadians(lat1 + lat2))) * sdlon * sdlon;
    const float d = two_r * my_sqrtf(q);

    return d;
}

void calculePos (const char *nom, float *lat, float *lon);

long unsigned int toBase10 (char *entree);

extern void loggerMsg(const char *msg_);

float radians(float value) __attribute__ ((pure));

float degrees(float value) __attribute__ ((pure));

float sq(float value) __attribute__ ((pure));

uint32_t get_sec_jour(uint8_t hour_, uint8_t min_, uint8_t sec_) __attribute__ ((pure));

float compute2Complement(uint8_t msb, uint8_t lsb) __attribute__ ((pure));

float percentageBatt(float tensionValue, float current) __attribute__ ((pure));

void encode_uint16 (uint8_t* dest, uint16_t input);

void encode_uint32 (uint8_t* dest, uint32_t input);

uint16_t decode_uint16 (uint8_t* dest);

uint32_t decode_uint32 (uint8_t* dest);

void const_char_to_buffer(const char *str_, uint8_t *buff_, uint16_t max_size);

float simpLinReg(float* x, float* y, float* lrCoef, int n);

uint8_t calculate_crc(uint8_t *, uint16_t);

int floorSqrt(int x);

uint32_t date_to_timestamp(uint32_t sec_j, uint8_t day, uint8_t month, uint16_t year);

#ifdef	__cplusplus
}
#endif

#endif	/* UTILS_H */

