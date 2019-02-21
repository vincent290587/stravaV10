/*
 * komoot_nav.h
 *
 *  Created on: 22 déc. 2018
 *      Author: Vincent
 */

#ifndef LIBRARIES_BLE_KOMOOT_C_KOMOOT_NAV_H_
#define LIBRARIES_BLE_KOMOOT_C_KOMOOT_NAV_H_

#define KOMOOT_ICON_SIZE_W    110
#define KOMOOT_ICON_SIZE_H    110

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


const uint8_t* komoot_nav_get_icon(uint8_t direction);


#ifdef __cplusplus
}
#endif


#endif /* LIBRARIES_BLE_KOMOOT_C_KOMOOT_NAV_H_ */
