/*
 * ble_api.h
 *
 *  Created on: 21 d�c. 2018
 *      Author: Vincent
 */

#ifndef RF_BLE_API_BASE_H_
#define RF_BLE_API_BASE_H_

#include "g_structs.h"

typedef enum {
	eBleEventTypeStartXfer,
} eBleEventType;

#ifdef __cplusplus
extern "C" {
#endif


void ble_init(void);
uint16_t ble_get_mtu(void);

void app_ble_central_init(void);

static inline void ble_nus_tasks(void) {

}

static inline void ble_start_evt(eBleEventType evt) {

}

static inline void ble_get_navigation(sKomootNavigation *nav) {

	nav->isUpdated = false;
}


#ifdef __cplusplus
}
#endif

#endif /* RF_BLE_API_BASE_H_ */
