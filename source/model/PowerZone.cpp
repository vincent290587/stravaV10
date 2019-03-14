/*
 * PowerZone.cpp
 *
 *  Created on: 14 mrt. 2019
 *      Author: v.golle
 */

#include <string.h>
#include "Model.h"
#include "PowerZone.h"


static const float pw_lims[PW_ZONES_NB+1] = {
		-100.0f,
		0.55f,
		0.75f,
		0.90f,
		1.05f,
		1.20f,
		1.50f,
		100.0f
};


PowerZone::PowerZone(void) : BinnedData() {
	m_tot_time = 0;
	m_last_timestamp = 0;

	memset(m_pw_bins, 0, sizeof(m_pw_bins));
}

void PowerZone::addPowerData(int pw_meas) {

	uint32_t timestamp = millis();

	// first call
	if (m_last_timestamp == 0) {
		m_last_timestamp = timestamp;
		return;
	}

	// do not record if outside the admissible window
	if (pw_meas < 50.0f || pw_meas > 1950.0f) {
		m_last_timestamp = timestamp;
		return;
	}

	// calculate elapsed time
	float time_integ = ((float)timestamp - (float)m_last_timestamp) / 1000.;

	// retrieve FTP from settings
	const uint16_t ftp = u_settings.getFTP();

	// find the zone
	for (int i=0; i < PW_ZONES_NB; i++) {

		if (pw_meas > pw_lims[i] * ftp &&
				pw_meas < pw_lims[i+1] * ftp) {

			m_pw_bins[i] += time_integ;

			// update state
			m_last_timestamp = timestamp;

			return;
		}

	}

	// should never be here
	LOG_ERROR("Wrong bin power");
}

uint32_t PowerZone::getTimeTotal(void) {
	float tot_time = 0;
	for (int i=0; i< PW_ZONES_NB; i++) tot_time += m_pw_bins[i];
	return (uint32_t)tot_time;
}
