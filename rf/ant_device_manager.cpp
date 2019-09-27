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

	// init channels
	ant_stack_init();
	ant_timers_init();
	ant_setup_init();

	// check registered devices
	if (u_settings.isConfigValid()) {
		ant_setup_start(u_settings.getHRMdevID(), u_settings.getBSCdevID(), u_settings.getFECdevID());
	} else {
		LOG_ERROR("Impossible to start ANT channels: wrong configuration");
	}
}

void ant_device_manager_search_start(eAntPairingSensorType dev_type) {

	// prepare ANT+ search list
	m_sensors_list.nb_sensors = 0;

#if defined(ANT_STACK_SUPPORT_REQD)
	// start search channel
	ant_search_start(dev_type);
	m_search_type = dev_type;
#endif

	LOG_WARNING("Starting ANT+ search...");
}

void ant_device_manager_search_validate(int var) {
#if defined(ANT_STACK_SUPPORT_REQD)
	if (eAntPairingSensorTypeNone == m_search_type) return;
	if (var < 0 || var >= m_sensors_list.nb_sensors) return;
	if (m_sensors_list.nb_sensors == 0) return;
	ant_search_end(m_search_type, m_sensors_list.sensors[var].dev_id);

	sUserParameters *settings = user_settings_get();

	switch (m_search_type) {
	case eAntPairingSensorTypeHRM:
	{
		// Set the new device ID.
		settings->hrm_devid = m_sensors_list.sensors[var].dev_id;
	} break;
	case eAntPairingSensorTypeBSC:
	{
		// Set the new device ID.
		settings->bsc_devid = m_sensors_list.sensors[var].dev_id;
	} break;
	case eAntPairingSensorTypeFEC:
	{
		// Set the new device ID.
		settings->fec_devid = m_sensors_list.sensors[var].dev_id;
	} break;
	default:
		break;
	}

	u_settings.writeConfig();

	vue.addNotif("ANT", "New device added", 4, eNotificationTypeComplete);

	m_search_type = eAntPairingSensorTypeNone;
#endif
	LOG_WARNING("ANT+ search ended");
}

void ant_device_manager_search_cancel(void) {

#if defined(ANT_STACK_SUPPORT_REQD)
	// start normal channel
	ant_search_end(m_search_type, 0x0000);

	m_search_type = eAntPairingSensorTypeNone;
#endif

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
