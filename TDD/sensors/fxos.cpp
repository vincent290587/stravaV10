/*
 * fxos.cpp
 *
 *  Created on: 18 mrt. 2019
 *      Author: v.golle
 */


#include "fxos.h"
#include "Model.h"
#include "UserSettings.h"

static float m_yaw = 0.0f;
static volatile bool m_is_updated = false;

bool is_fxos_updated(void) {
	return m_is_updated;
}

void fxos_readChip(void) {

}

void fxos_calibration_start(void) {

}

bool fxos_get_yaw(float &yaw_rad) {
	yaw_rad = m_yaw;
	return true;
}

void fxos_set_yaw(float yaw_rad) {
	m_yaw = yaw_rad;
}

bool fxos_init(void) {

	return true;
}

void fxos_tasks()
{
	if (!m_is_updated) return;
	m_is_updated = false;


	if (u_settings.isConfigValid()) {

		sMagCal &mag_cal = u_settings.getMagCal();
		// check if we have a previous calibration
		if (mag_cal.is_present) {

			LOG_INFO("Magnetometer calibration found");

			//vue.addNotif("Event", "Magnetometer calibration found", 4, eNotificationTypeComplete);

		}

	}


}
