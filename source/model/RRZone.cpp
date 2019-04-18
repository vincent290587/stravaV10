/*
 * RRZone.cpp
 *
 *  Created on: 14 mrt. 2019
 *      Author: v.golle
 */

#include <string.h>
#include "Model.h"
#include "RRZone.h"

#define VAR_NB_ELEM    20


static float rr_lims[RR_ZONES_NB+1] = {
		-1000.0f,
		70.0f,
		108.0f,
		143.0f,
		161.0f,
		178.0f,
		1000.0f
};


RRZone::RRZone(void) : BinnedData() {
	m_tot_time = 0;
	m_last_bin = 0;
	m_last_timestamp = 0;

	memset(m_rr_bins, 0, sizeof(m_rr_bins));
	memset(m_tm_bins, 0, sizeof(m_tm_bins));
}

RRZone::~RRZone(void) {

}

void RRZone::addRRData(sHrmInfo &hrm_info) {

	if (!hrm_info.timestamp) return;

	// first call
	if (m_last_timestamp == 0) {
		m_last_timestamp = hrm_info.timestamp;
		return;
	}

	static float tab_meas[VAR_NB_ELEM];
	static uint16_t tab_meas_nb = 0;

	// save data
	tab_meas[tab_meas_nb++] = hrm_info.rr;

	// check if we should process it
	if (tab_meas_nb < VAR_NB_ELEM) return;

	// array is full
	// calculate RMSSD
	float var_meas = 0.0F;
//	float mea_meas = 0.0F;
	float sum = tab_meas[0];
	float sum_sq = 0.0F;
	for (int i = 1; i < VAR_NB_ELEM; i++) {

		sum += tab_meas[i];

		float val = tab_meas[i] - tab_meas[i-1];
		sum_sq += val * val;

	}

//	mea_meas = sum / VAR_NB_ELEM;
	var_meas = my_sqrtf(sum_sq / (VAR_NB_ELEM - 1));

	LOG_INFO("Measured RR RMSSD: %d", (int)var_meas);

	// reset index and timestamp
	tab_meas_nb = 0;

	// update state
	m_last_timestamp = hrm_info.timestamp;
	hrm_info.timestamp = 0;

	// find the zone
	for (int i=0; i < RR_ZONES_NB; i++) {

		if (hrm_info.bpm >= rr_lims[i] &&
				hrm_info.bpm < rr_lims[i+1]) {

			m_rr_bins[i] += var_meas;
			m_tm_bins[i] += 1.;

			m_last_bin = i;

			LOG_DEBUG("Logging RR in bin %d", i);

			return;
		}

	}

	// should never be here
	LOG_ERROR("Wrong bin RR: rr_var_meas: %d", (int)var_meas);
}

uint32_t RRZone::getTimeMax(void) {
	uint32_t max_time = m_tm_bins[0];
	for (int i=1; i< RR_ZONES_NB; i++) if (m_tm_bins[i] > max_time) max_time = m_tm_bins[i];
	return max_time;
}

uint32_t RRZone::getValMax(void) {
	uint32_t max_val = getValZX(0);
	for (int i=1; i< RR_ZONES_NB; i++) if (getValZX(i) > max_val) max_val = getValZX(i);
	return max_val;
}

uint32_t RRZone::getTimeTotal(void) {
	float tot_time = 0;
	for (int i=0; i< RR_ZONES_NB; i++) tot_time += m_tm_bins[i];
	return (uint32_t)tot_time;
}

uint32_t RRZone::getTimeZX(uint16_t i) {
	return (uint32_t)m_tm_bins[i];
}

uint32_t RRZone::getValZX(uint16_t i) {

	float val = m_rr_bins[i];
	if (m_tm_bins[i] > 0)
		val /= (float)m_tm_bins[i];
	else {
		val = 0;
	}

	return (uint32_t)val;
}

uint32_t RRZone::getNbBins(void) {
	return RR_ZONES_NB;
}

uint32_t RRZone::getCurBin(void) {
	return m_last_bin;
}
