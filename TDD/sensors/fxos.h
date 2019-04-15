/*
 * fxos.h
 *
 *  Created on: 5 mrt. 2019
 *      Author: v.golle
 */

#ifndef TDD_SENSORS_FXOS_H_
#define TDD_SENSORS_FXOS_H_

#include <stdbool.h>


void fxos_calibration_start(void);

bool fxos_get_yaw(float &yaw_rad);

void fxos_set_yaw(float yaw_rad);

#endif /* TDD_SENSORS_FXOS_H_ */
