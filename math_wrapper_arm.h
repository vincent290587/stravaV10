/*
 * math_wrapper_arm.h
 *
 *  Created on: 19 nov. 2018
 *      Author: Vincent
 */

#ifndef MATH_WRAPPER_ARM_H_
#define MATH_WRAPPER_ARM_H_


#include "app_util_platform.h"
#include "arm_math.h"

#ifndef M_PI
#define M_PI       ((float)PI)
#endif

static inline float my_sqrtf(float val) {
	float t_val;
	APP_ERROR_CHECK(arm_sqrt_f32(val, &t_val));
	return t_val;
}

static inline float my_cos(float val) {
	return arm_cos_f32(val);
}

static inline float my_sin(float val) {
	return arm_sin_f32(val);
}



#endif /* MATH_WRAPPER_ARM_H_ */
