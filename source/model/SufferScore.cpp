/*
 * SufferScore.cpp
 *
 *  Created on: 18 déc. 2018
 *      Author: Vincent
 */

#include <string.h>
#include "SufferScore.h"

SufferScore::SufferScore() {
	m_score = 0.;

	memset(m_hrm_bins, 0, sizeof(m_hrm_bins));
}

void SufferScore::updateScore(void) {

	m_score  = (float)m_hrm_bins[0] * HRM_Z1_PTS_PER_HOUR;
	m_score += (float)m_hrm_bins[1] * HRM_Z2_PTS_PER_HOUR;
	m_score += (float)m_hrm_bins[2] * HRM_Z3_PTS_PER_HOUR;
	m_score += (float)m_hrm_bins[3] * HRM_Z4_PTS_PER_HOUR;
	m_score += (float)m_hrm_bins[4] * HRM_Z5_PTS_PER_HOUR;

}

void SufferScore::addHrmData(int hrm_meas) {

	if (hrm_meas > HRM_Z1_LIM &&
			hrm_meas < HRM_Z2_LIM) {

		m_hrm_bins[0] += 1;

	} else if (hrm_meas > HRM_Z2_LIM &&
			hrm_meas < HRM_Z3_LIM) {

		m_hrm_bins[1] += 1;

	} else if (hrm_meas > HRM_Z3_LIM &&
			hrm_meas < HRM_Z4_LIM) {

		m_hrm_bins[2] += 1;

	} else if (hrm_meas > HRM_Z4_LIM &&
			hrm_meas < HRM_Z5_LIM) {

		m_hrm_bins[3] += 1;

	} else if (hrm_meas > HRM_Z5_LIM) {

		m_hrm_bins[4] += 1;

	}

	this->updateScore();
}
