/*
 * ant_device_manager.c
 *
 *  Created on: 22 feb. 2019
 *      Author: v.golle
 */

#include "ant.h"
#include "Model.h"
#include "ant_device_manager.h"
#include "segger_wrapper.h"

static sAntPairingSensorList m_sensors_list;

static eAntPairingSensorType m_search_type = eAntPairingSensorTypeNone;


sAntPairingSensorList* ant_device_manager_get_sensors_list(void) {
	return &m_sensors_list;
}

void ant_device_manager_init(void) {

	// check registered devices

}

void ant_device_manager_connect(void) {

	// end ANT+ search

	// open ANT+ channels

}

void ant_device_manager_search_start(eAntPairingSensorType dev_type) {

	// prepare ANT+ search list
	m_sensors_list.nb_sensors = 0;

	// start search channel
	ant_search_start(dev_type);
	m_search_type = dev_type;

	LOG_WARNING("Starting ANT+ search...");
}

void ant_device_manager_search_validate(int var) {

	if (eAntPairingSensorTypeNone == m_search_type) return;
	if (var < 0 || var >= m_sensors_list.nb_sensors) return;
	if (m_sensors_list.nb_sensors == 0) return;

	ant_search_end(m_search_type, m_sensors_list.sensors[var].dev_id);

	m_search_type = eAntPairingSensorTypeNone;

	LOG_WARNING("ANT+ search ended");
}

void ant_device_manager_search_cancel(void) {

	// start normal channel
	ant_search_end(m_search_type, 0x0000);

	m_search_type = eAntPairingSensorTypeNone;

	LOG_WARNING("ANT+ search cancelled");
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

	LOG_WARNING("Found ANT+ sensor %u", sensor_id);
}
