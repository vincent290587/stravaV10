/*
 * ble_api6.c
 *
 *  Created on: 9 janv. 2020
 *      Author: vgol
 */

#include "ble_api_base.h"


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

}

void roller_manager_tasks(void) {

}
