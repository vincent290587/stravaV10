/*
 * fxos.cpp
 *
 *  Created on: 18 mrt. 2019
 *      Author: v.golle
 */


#include "fxos.h"
#include "Model.h"
#include "parameters.h"
#include "RingBuffer.h"
#include "UserSettings.h"

static tHistoValue m_pi_buffer[PITCH_BUFFER_SIZE];
RingBuffer<tHistoValue> m_pitch_buffer(PITCH_BUFFER_SIZE, m_pi_buffer);

static float m_yaw = 0.0f;
static float m_pitch = 0.0f;
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

	LOG_DEBUG("FXOS Updated");

	static int is_init = 0;
	if (!is_init && u_settings.isConfigValid()) {

		sMagCal &mag_cal = u_settings.getMagCal();
		// check if we have a previous calibration
		if (mag_cal.is_present) {

			LOG_INFO("Magnetometer calibration found");

			//vue.addNotif("Event", "Magnetometer calibration found", 4, eNotificationTypeComplete);
		}

		is_init = 1;

	}


}


bool fxos_get_pitch(float &pitch_rad) {
	pitch_rad = m_pitch;
	return true;
}

void fxos_set_pitch(float pitch_rad) {

	int16_t integ_pitch = (int16_t)((pitch_rad + 1.57) * 100.);
	uint16_t u_integ_pitch = (uint16_t)integ_pitch;

	if (m_pitch_buffer.isFull()) {
		m_pitch_buffer.popLast();
	}
	m_pitch_buffer.add(&u_integ_pitch);

	m_pitch = pitch_rad;
	m_is_updated = true;
}

tHistoValue fxos_histo_read(uint16_t ind_) {

	tHistoValue *p_ret_val = m_pitch_buffer.get(ind_);

	ASSERT(p_ret_val);

	tHistoValue ret_val = p_ret_val[0];

	return ret_val;
}

uint16_t fxos_histo_size(void) {

	return m_pitch_buffer.size();
}
