/*
 * ant_device_manager.c
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#include "ant.h"
#include "ant_interface.h"
#include "Model.h"
#include "ant_device_manager.h"

static sAntPairingSensorList m_sensors_list;

static eAntPairingSensorType m_search_type = eAntPairingSensorTypeNone;


sAntPairingSensorList* ant_device_manager_get_sensors_list(void) {
	return &m_sensors_list;
}

void ant_device_manager_init(void) {

	// check registered devices

	// can we start ANT+ ?

}

void ant_device_manager_connect(void) {

	// end ANT+ search

	// open ANT+ channels

}

void ant_device_manager_search_start(eAntPairingSensorType dev_type) {

	ret_code_t err_code;

	// TODO close ANT+ channels

	// prepare ANT+ search
	m_sensors_list.nb_sensors = 0;
	m_search_type = dev_type;

	// TODO start search channel
	ant_search_start(dev_type);
}

void ant_device_manager_search_validate(int var) {

	if (eAntPairingSensorTypeNone == m_search_type) return;
	if (var < 0 || var >= m_sensors_list.nb_sensors) return;
	if (m_sensors_list.nb_sensors == 0) return;

	// TODO
	switch (m_search_type) {
	case eAntPairingSensorTypeHRM:
		break;
	case eAntPairingSensorTypeBSC:
		break;
	case eAntPairingSensorTypeFEC:
		break;
	default:
		break;
	}

	m_search_type = eAntPairingSensorTypeNone;
}

void ant_device_manager_search_cancel(void) {

	m_search_type = eAntPairingSensorTypeNone;

	// TODO close search channel

	// start normal channel
	ant_search_end(0x0000);
}

void ant_device_manager_search_add(uint16_t sensor_id, int8_t ssid) {

	// sensor list full
	if (m_sensors_list.nb_sensors >= ANT_DEVICE_MANAGER_MAX_SENSORS_NB - 1) return;

	for (int i=0; i < m_sensors_list.nb_sensors; i++) {
		if (sensor_id == m_sensors_list.sensors[i].dev_id) {

			// update properties
			m_sensors_list.sensors[i].ssid = ssid;

			return;
		}
	}

	m_sensors_list.sensors[m_sensors_list.nb_sensors++].dev_id = sensor_id;
}
