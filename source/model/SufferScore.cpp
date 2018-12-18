/*
 * SufferScore.cpp
 *
 *  Created on: 18 déc. 2018
 *      Author: Vincent
 */

#include <string.h>
#include "SufferScore.h"


#define HRM_Z1_LIM			80
#define HRM_Z2_LIM			120
#define HRM_Z3_LIM			144
#define HRM_Z4_LIM			165
#define HRM_Z5_LIM			176


#define HRM_Z1_PTS_PER_HOUR			(16./3600.)
#define HRM_Z2_PTS_PER_HOUR			(33./3600.)
#define HRM_Z3_PTS_PER_HOUR			(72./3600.)
#define HRM_Z4_PTS_PER_HOUR			(85./3600.)
#define HRM_Z5_PTS_PER_HOUR			(95./3600.)


SufferScore::SufferScore() {
	m_score = 0.;
	m_last_timestamp = 0;
	memset(m_hrm_bins, 0, sizeof(m_hrm_bins));
}

void SufferScore::updateScore(void) {

	m_score  = m_hrm_bins[0] * HRM_Z1_PTS_PER_HOUR;
	m_score += m_hrm_bins[1] * HRM_Z2_PTS_PER_HOUR;
	m_score += m_hrm_bins[2] * HRM_Z3_PTS_PER_HOUR;
	m_score += m_hrm_bins[3] * HRM_Z4_PTS_PER_HOUR;
	m_score += m_hrm_bins[4] * HRM_Z5_PTS_PER_HOUR;

}

void SufferScore::addHrmData(int hrm_meas, uint32_t timestamp) {

	float time_integ = ((float)timestamp - (float)m_last_timestamp) / 1000.;

	if (hrm_meas > HRM_Z1_LIM &&
			hrm_meas <= HRM_Z2_LIM) {

		m_hrm_bins[0] += time_integ;

	} else if (hrm_meas > HRM_Z2_LIM &&
			hrm_meas <= HRM_Z3_LIM) {

		m_hrm_bins[1] += time_integ;

	} else if (hrm_meas > HRM_Z3_LIM &&
			hrm_meas <= HRM_Z4_LIM) {

		m_hrm_bins[2] += time_integ;

	} else if (hrm_meas > HRM_Z4_LIM &&
			hrm_meas <= HRM_Z5_LIM) {

		m_hrm_bins[3] += time_integ;

	} else if (hrm_meas > HRM_Z5_LIM) {

		m_hrm_bins[4] += time_integ;

	}

	// update state
	m_last_timestamp = timestamp;

	this->updateScore();
}
