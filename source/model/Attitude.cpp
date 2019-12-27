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

#ifdef TDD
#include "tdd_logger.h"
#endif


static float lp1_filter_coefficients[5] =
{
// Scaled for floating point: 0.008.fs
    0.024521609249465722f, 0.024521609249465722f, 0.f, 0.9509567815010685f, 0.f// b0, b1, b2, a1, a2
};

static order1_filterType m_lp_filt;


Attitude::Attitude(AltiBaro &_baro) : m_baro(_baro) {
	m_last_save_dist = 0.;
	m_last_stored_ele = 0.;
	m_cur_ele = 0.;
	m_climb = 0.;
	m_speed_ms = 0.;

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


void Attitude::computeFusion(void) {

	// get current elevation
	float ele = 0.;
	if (!m_baro.computeAlti(ele)) {
		LOG_ERROR("Baro error");
		return;
	}

	// 2 seconds time constant
	const float taub = 2 / (2 + BARO_REFRESH_PER_MS / 1000.);
	m_cur_ele = taub * m_cur_ele + (1 - taub) * (ele);

	float alti = ele;
	static float alti_prev = ele;

	static float cur_time_prev;
	float dt = ((float)millis() - cur_time_prev) / 1000.0f;

	float yaw_rad;
	fxos_get_yaw(yaw_rad);

	// 3 seconds time constant
	static float alpha_bar;
	const float tau = 3 / (3 + SENSORS_REFRESH_PER_MS / 1000.);
	float innov = yaw_rad;
	alpha_bar = tau * alpha_bar + (1 - tau) * (innov);

	// work on alpha zero
	if (m_speed_ms < 1.5f || dt < 0.01f || m_speed_ms * dt < 0.1) {
		return;
	}

	// about 40 seconds time constant
	float new_alpha_z = yaw_rad - atan2f((alti - alti_prev) , (m_speed_ms * dt));
	static float alpha_zero = 0;
	alpha_zero = new_alpha_z * 0.003 + 0.997 * alpha_zero;

	// update vertical speed
	att.vit_asc = tanf(alpha_bar - alpha_zero) * m_speed_ms;

	LOG_DEBUG("Vit. vert.: %f / alpha: %f / alpha0: %f", att.vit_asc,
			180.f*(alpha_bar-alpha_zero)/3.1415f,
			180.f*alpha_zero/3.1415f);

	cur_time_prev = millis();
	alti_prev = alti;

#ifdef TDD
		tdd_logger_log_float(TDD_LOGGING_HP    , att.vit_asc);
		tdd_logger_log_float(TDD_LOGGING_ALPHA , 180.f * (alpha_bar - alpha_zero)/3.1415f);
		tdd_logger_log_float(TDD_LOGGING_ALPHA0, 180.f * alpha_zero / 3.1415f);
		tdd_logger_log_float(TDD_LOGGING_EST_SLOPE, 100.0f * tanf(alpha_bar - alpha_zero));
		tdd_logger_log_float(TDD_LOGGING_ALT_EST, m_cur_ele);
#endif
}


float Attitude::filterElevation(SLoc& loc_) {

	if (!m_is_alt_init) {
		m_is_alt_init = true;
		m_last_stored_ele = m_cur_ele;

		order1_filter_init(&m_lp_filt, lp1_filter_coefficients);

		return 0.;
	}

	LOG_INFO("Filtering climb");

	// filter with a high time-constant the difference between
	// GPS altitude and barometer altitude to remove drifts
	float input = m_cur_ele - loc_.alt;
	order1_filter_writeInput(&m_lp_filt, &input);

	float alt_div = order1_filter_readOutput(&m_lp_filt);
	// set barometer correction
	m_baro.setCorrection(alt_div);

	// compute accumulated climb on the corrected filtered barometer altitude
	if (m_cur_ele > m_last_stored_ele + 2.f) {
		// mise a jour de la montee totale
		m_climb += m_cur_ele - m_last_stored_ele;
		m_last_stored_ele = m_cur_ele;
	}
	else if (m_cur_ele + 2.f < m_last_stored_ele) {
		// on descend, donc on garde la derniere alti
		// la plus basse
		m_last_stored_ele = m_cur_ele;
	}

#ifdef TDD
	tdd_logger_log_float(TDD_LOGGING_TOT_CLIMB, m_climb);
#endif

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
		m_baro.seaLevelForAltitude(loc_.alt);

		// get current elevation
		m_baro.computeAlti(m_cur_ele);

		LOG_WARNING("Init sea level pressure... gps: %f bme:%f", loc_.alt, m_cur_ele);

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

	if (m_is_init) {

		// small correction to allow time with millisecond precision
		float cur_time = (float)date_.secj + ((millis() - date_.timestamp) / 1000);

		LOG_INFO("Adding location: %f", cur_time);

		// add this point to our historic
		mes_points.ajouteFinIso(loc_.lat, loc_.lon, loc_.alt, cur_time, HISTO_POINT_SIZE);
		if (loc_.speed > 7.f) att.nbsec_act++;
		att.nbpts++;

		// calculate elevation shifts
		this->computeElevation(loc_, source_);

		// update the computed power
		att.pwr = this->computePower(loc_.speed);

		this->computeDistance(loc_, date_, source_);
	}

	m_speed_ms = loc_.speed / 3.6f;

#ifdef TDD
	tdd_logger_log_float(TDD_LOGGING_CUR_SPEED, loc_.speed);
#endif

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
float Attitude::computePower(float speed_) {

	float power = 0.0f;

	// init alpha_0 filter
	if (!m_is_pw_init) {

		m_is_pw_init = true;

		return power;
	}

	// proceed to fusing measurements
	if (m_is_pw_init) {

		const float weight = (float)u_settings.getWeight();

		// STEP 2 : Calculate power
		power += 9.81f * weight * att.vit_asc; // gravity
		power += 0.004f * 9.81f * weight * m_speed_ms; // sol + meca
		power += 0.204f * m_speed_ms * m_speed_ms * m_speed_ms; // air
		power *= 1.025f; // transmission (rendement velo)

#ifdef TDD
		tdd_logger_log_float(TDD_LOGGING_CUR_POWER, power);
#endif
	}

	return power;
}



/**
 *
 */

void Attitude::addNewFECPoint(sFecInfo& fec_) {

	// TODO

}
