/*
 * ant_device_manager.h
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#ifndef RF_ANT_DEVICE_MANAGER_H_
#define RF_ANT_DEVICE_MANAGER_H_

#include "ant.h"
#include <stdint.h>

#define ANT_DEVICE_MANAGER_MAX_SENSORS_NB    8

typedef struct {
	uint16_t dev_id;
	int8_t ssid;
} sAntPairingSensor;

typedef struct {
	uint16_t nb_sensors;
	sAntPairingSensor sensors[ANT_DEVICE_MANAGER_MAX_SENSORS_NB];
} sAntPairingSensorList;




void ant_device_manager_search_start(eAntPairingSensorType dev_type);

void ant_device_manager_search_validate(int var);

void ant_device_manager_search_cancel(void);

void ant_device_manager_search_add(uint16_t sensor_id, int8_t ssid);

sAntPairingSensorList* ant_device_manager_get_sensors_list(void);



#endif /* RF_ANT_DEVICE_MANAGER_H_ */
