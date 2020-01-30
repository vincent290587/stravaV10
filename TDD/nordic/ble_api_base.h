/*
 * ble_api_base.h
 *
 *  Created on: 9 janv. 2020
 *      Author: vgol
 */

#ifndef TDD_NORDIC_BLE_API_BASE_H_
#define TDD_NORDIC_BLE_API_BASE_H_


#include "g_structs.h"

typedef enum {
	eBleEventTypeStartXfer,
} eBleEventType;

#ifdef __cplusplus
extern "C" {
#endif


void ble_init(void);

void ble_nus_tasks(void);

void ble_start_evt(eBleEventType evt);

void ble_get_navigation(sKomootNavigation *nav);

void fec_init(void);

void roller_manager_tasks(void);

#ifdef __cplusplus
}
#endif


#endif /* TDD_NORDIC_BLE_API_BASE_H_ */
