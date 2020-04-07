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


bool fxos_init(void);

void fxos_calibration_start(void);

void fxos_readChip(void);

bool is_fxos_updated(void);

void fxos_tasks(void);

bool fxos_get_yaw(float &yaw_rad);

void fxos_set_yaw(float yaw_rad);

bool fxos_get_pitch(float &);

void fxos_set_xyz(float g_Ax_Raw, float g_Ay_Raw, float g_Az_Raw);

tHistoValue fxos_histo_read(uint16_t ind_);

uint16_t fxos_histo_size(void);

#endif /* TDD_SENSORS_FXOS_H_ */
