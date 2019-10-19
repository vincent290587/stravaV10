/*
 * fxos.h
 *
 *  Created on: 5 mrt. 2019
 *      Author: v.golle
 */

#ifndef TDD_SENSORS_FXOS_H_
#define TDD_SENSORS_FXOS_H_

#include <stdbool.h>
#include "VueCommon.h"


void fxos_calibration_start(void);

bool fxos_get_yaw(float &);

void fxos_set_yaw(float );

bool fxos_get_pitch(float &);

void fxos_set_pitch(float );

tHistoValue fxos_histo_read(uint16_t ind_);

uint16_t fxos_histo_size(void);

#endif /* TDD_SENSORS_FXOS_H_ */
