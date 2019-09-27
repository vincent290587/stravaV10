/*
 * Attitude.cpp
 *
 *  Created on: 29 déc. 2017
 *      Author: Vincent
 */

#include <math.h>
#include <Attitude.h>
#include "utils.h"
#include "fxos.h"
#include "parameters.h"
#include "Model.h"
#include "sd_functions.h"
#include "segger_wrapper.h"

#include "order1_filter.h"


static float lp1_filter_coefficients[5] =
{
// Scaled for floating point: 0.008.fs
    0.024521609249465722f, 0.024521609249465722f, 0.f, 0.9509567815010685f, 0.f// b0, b1, b2, a1, a2
};

static order1_filterType m_lp_filt;

static order1_filterType m_lp_alpha;


Attitude::Attitude(AltiBaro &_baro) : m_baro(_baro) {
	m_last_save_dist = 0.;
	m_last_stored_ele = 0.;
	m_cur_ele = 0.;
	m_climb = 0.;

	m_is_init = false;
	m_is_acc_init = false;
	m_is_alt_init = false;
	m_is_pw_init = false;

	m_st_buffer_nb_elem = 0;

}

/**
 *
 * @param loc_
 */
void Attitude::addNewDate(SDate *date_) {

	// update date
	memcpy(&att.date, date_, sizeof(SDate));
}


float Attitude::filterElevation(SLoc& loc_) {

	float ele = 0.;

	// get current elevation
	if (!m_baro.computeAlti(ele)) return 0.;

	m_cur_ele = ele;

	if (!m_is_alt_init) {
		m_is_alt_init = true;
		m_last_stored_ele = ele;

		order1_filter_init(&m_lp_filt, lp1_filter_coefficients);

		return 0.;
	}

	LOG_INFO("Filtering elevation");

	// filter with a high time-constant the difference between
	// GPS altitude and barometer altitude to remove drifts
	float input = ele - loc_.alt;
	order1_filter_writeInput(&m_lp_filt, &input);

	float alt_div = order1_filter_readOutput(&m_lp_filt);
	// set barometer correction
	m_baro.setCorrection(alt_div);

	// compute accumulated climb on the corrected filtered barometer altitude
	if (ele > m_last_stored_ele + 4.f) {
		// mise a jour de la montee totale
		m_climb += ele - m_last_stored_ele;
		m_last_stored_ele = ele;
	}
	else if (ele + 4.f < m_last_stored_ele) {
		// on descend, donc on garde la derniere alti
		// la plus basse
		m_last_stored_ele = ele;
	}

	return m_climb;
}


float Attitude::computeElevation(SLoc& loc_, eLocationSource source_) {

	float res = 0.;

	// init the altitude model
	if ((eLocationSourceGPS == source_ ||
			eLocationSourceNRF == source_) &&
			m_baro.isDataReady() &&
			!m_baro.hasSeaLevelRef()) {

		// init sea level pressure
		LOG_WARNING("Init sea level pressure...");
		m_baro.seaLevelForAltitude(loc_.alt);

		// treat elevation
		this->filterElevation(loc_);

		// reset accumulated data
		m_climb = 0.;
		m_last_stored_ele = m_cur_ele;

		if (m_app_error.saved_data.crc == SYSTEM_DESCR_POS_CRC) {

			// restoring position and accumulated climb
			att.climb = m_app_error.saved_data.att.climb;
			m_climb = att.climb;

			att.dist = m_app_error.saved_data.att.dist;
			m_last_save_dist = att.dist;

			att.pr = m_app_error.saved_data.att.pr;
			att.nbsec_act = m_app_error.saved_data.att.nbsec_act;

			LOG_WARNING("Last stored date: %u", m_app_error.saved_data.att.date.date);
			LOG_WARNING("Last stored dist: %f", att.dist);

		}

		m_app_error.saved_data.crc = 0x00;

	} else if ((eLocationSourceGPS == source_ ||
			eLocationSourceNRF == source_) &&
			m_baro.hasSeaLevelRef()) {

		res = att.climb = this->filterElevation(loc_);

		// overwrite GPS/NRF's elevation
		loc_.alt = m_cur_ele;

	}

	return res;
}


void Attitude::computeDistance(SLoc& loc_, SDate &date_, eLocationSource source_) {

	float tmp_dist = distance_between(att.loc.lat, att.loc.lon, loc_.lat, loc_.lon);

	char buffer[100];
	snprintf(buffer, sizeof(buffer), "Weird pos: %f %f %f %f\r\n", att.loc.lat, att.loc.lon, loc_.lat, loc_.lon);
	if (tmp_dist > 400000.f) LOG_WARNING(buffer);

	att.dist += tmp_dist;

	// save attitude to a buffer for later saving to memory
	if (m_is_acc_init &&
			att.dist > m_last_save_dist + 15.f) {

		m_last_save_dist = att.dist;

		// save position on queue
		if (m_st_buffer_nb_elem >= ATT_BUFFER_NB_ELEM) {

			// save on SD
			sysview_task_void_enter(SaveUserPosition);
			sd_save_pos_buffer(m_st_buffer, ATT_BUFFER_NB_ELEM);
			sysview_task_void_exit(SaveUserPosition);

			m_st_buffer_nb_elem = 0;

		}

		// save on buffer
		memcpy(&m_st_buffer[m_st_buffer_nb_elem].loc , &loc_ , sizeof(SLoc));
		memcpy(&m_st_buffer[m_st_buffer_nb_elem].date, &date_, sizeof(SDate));
		m_st_buffer[m_st_buffer_nb_elem].pwr = att.pwr;

		m_st_buffer_nb_elem++;

		// save position and stuff in case of crash
		memcpy(&m_app_error.saved_data.att, &att, sizeof(SAtt));
		m_app_error.saved_data.crc = SYSTEM_DESCR_POS_CRC;


	} else if (att.dist > m_last_save_dist + 25.f) {
		// first position
		att.dist = m_last_save_dist;
		m_is_acc_init = true;
	}

}



/**
 *
 * @param loc_
 */
void Attitude::addNewLocation(SLoc& loc_, SDate &date_, eLocationSource source_) {

	static float cur_time_prev;

	// small correction to allow time with millisecond precision
	float cur_time = (float)date_.secj + ((millis() - date_.timestamp) / 1000);

	if (m_is_init) {

		// add this point to our historic
		mes_points.ajouteFinIso(loc_.lat, loc_.lon, loc_.alt, cur_time, HISTO_POINT_SIZE);
		if (loc_.speed > 7.f) att.nbsec_act++;
		att.nbpts++;

		// calculate elevation shifts
		this->computeElevation(loc_, source_);

		// update the computed power
		att.pwr = this->computePower(loc_.speed, cur_time - cur_time_prev);

		this->computeDistance(loc_, date_, source_);
	}

	cur_time_prev = cur_time;

	m_is_init = true;

	// update global location
	memcpy(&att.loc , &loc_ , sizeof(SLoc));
	// update date
	memcpy(&att.date, &date_, sizeof(SDate));
}


/**
 *
 * @param speed_
 */
float Attitude::computePower(float speed_, float dt) {

	float power = 0.0f;
	const float speed_ms = speed_ / 3.6f;

	float alti = 0.0f;
	static float alti_prev;
	static float alpha_bar;

	// init alpha_0 filter
	if (!m_is_pw_init) {
		m_is_pw_init = true;

		order1_filter_init(&m_lp_alpha, lp1_filter_coefficients);

		return power;
	}

	// proceed to fusing measurements
	if (m_is_pw_init && m_baro.computeAlti(alti)) {

		float yaw_rad;

		fxos_get_yaw(yaw_rad);

		// 3 seconds time constant with 1000ms measurement period
		const float tau = 3 / (3 + SENSORS_REFRESH_PER_MS / 1000.);
		float innov = yaw_rad;

		alpha_bar = tau * alpha_bar + (1 - tau) * (innov);

		// work on alpha zero
		if (speed_ms < 1.5f || dt < 0.01f) {
			LOG_DEBUG("Power skipped %f", speed_ms);
			return power;
		}

		float new_alpha_z = yaw_rad - atan2f((alti - alti_prev) , (speed_ms * dt));
		order1_filter_writeInput(&m_lp_alpha, &new_alpha_z);
		float alpha_zero = order1_filter_readOutput(&m_lp_alpha);

		// update vertical speed
		att.vit_asc = tanf(alpha_bar - alpha_zero) * speed_ms;

		LOG_INFO("Vit. vert.: %f / alpha: %f / alpha0: %f", att.vit_asc,
				180.f*(alpha_bar-alpha_zero)/3.1415f,
				180.f*alpha_zero/3.1415f);

		const float weight = (float)u_settings.getWeight();

		// STEP 2 : Calculate power
		power += 9.81f * weight * att.vit_asc; // gravity
		power += 0.004f * 9.81f * weight * speed_ms; // sol + meca
		power += 0.204f * speed_ms * speed_ms * speed_ms; // air
		power *= 1.025f; // transmission (rendement velo)

		alti_prev = alti;
	}

	return power;
}



/**
 *
 */

void Attitude::addNewFECPoint(sFecInfo& fec_) {

	// TODO

}
