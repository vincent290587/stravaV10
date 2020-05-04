/*
 * ble_api6.c
 *
 *  Created on: 9 janv. 2020
 *      Author: vgol
 */

#include <math.h>
#include "ble_api_base.h"


sPowerVector powerVector = {
		.first_crank_angle = 60,
		.array_size = 20,
};

/**
 *
 * @param nav
 */
void ble_get_navigation(sKomootNavigation *nav) {

	nav->isUpdated = true;
	nav->distance = 750;
	nav->direction = ++nav->direction % 33;

}

void ble_start_evt(eBleEventType evt) {


}

void ble_nus_tasks(void) {


}

void ant_tasks(void) {

	for (int i=0; i < powerVector.array_size; i++) {

		powerVector.inst_torque_mag_array[i] = (int16_t)(30.f + fabsf(300.f * cosf(i * 3.1415f / (powerVector.array_size / 2))));
	}
}

void roller_manager_tasks(void) {

}
