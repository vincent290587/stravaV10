/*
 * Attitude.cpp
 *
 *  Created on: 29 dec. 2017
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

#define USE_KALMAN        1

#ifdef TDD
#include "tdd_logger.h"
#endif

#ifdef USE_JSCOPE
#include "JScope.h"
JScope jscope;
#endif

#define CLIMB_ELEVATION_HYSTERESIS_M           2.f

#if USE_KALMAN
#include "kalman_ext.h"
static sKalmanDescr m_k_lin;
#endif


Attitude::Attitude(AltiBaro &_baro) : m_baro(_baro) {

	dv = 0.f;
	m_last_save_dist = 0.;
	m_last_stored_ele = 0.;
	m_cur_ele = 0.;
	m_climb = 0.;
	m_speed_ms = 0.;

	m_is_init = false;
	m_is_acc_init = false;
	m_is_alt_init = false;

	m_st_buffer_nb_elem = 0;


#ifdef USE_JSCOPE
	jscope.init();
#endif
}

void Attitude::reset(void) {

	m_last_save_dist = 0.;
	m_last_stored_ele = 0.;
	m_cur_ele = 0.;
	m_climb = 0.;
	m_speed_ms = 0.;

	m_is_init = false;
	m_is_acc_init = false;
	m_is_alt_init = false;

	m_st_buffer_nb_elem = 0;

	// reset global location
	memset(&att.loc , 0 , sizeof(SLoc));
	// reset date
	memset(&att.date, 0 , sizeof(SDate));

	// remove saved points
	mes_points.removeAll();

#if USE_KALMAN
	m_k_lin.is_init = 0;
#endif
}

/**
 *
 * @param loc_
 */
void Attitude::addNewDate(SDate *date_) {

	// update date
	memcpy(&att.date, date_, sizeof(SDate));
}

///
/// m_cur_ele
/// pitch_rad
/// alpha_zero
///
/// d(ele)\dt = speed * tan(slope_rad)
/// slope_rad = pitch_rad - alpha_zero
///
///  d(m_cur_ele) = (pitch_rad - alpha_zero) * dl
///  dl = speed * dt

/**
 *       | 1  dl -dl |
 * Phi = | 0   1   0 |
 *       | 0   0   1 |
 *
 */
void Attitude::computeFusion(void) {

	// get current elevation
	float ele = att.loc.alt;

#ifndef KALMAN_USE_GPS_ALTITUDE
	if (!m_baro.computeAlti(ele)) {
		LOG_WARNING("Altitude error or sea level missing");
		return;
	}
#endif

#if USE_KALMAN

	static sKalmanExtFeed feed;
	static uint32_t m_update_time = 0;
	float alpha_zero = 0;
	float alpha_bar = 0;

	if (!m_k_lin.is_init) {

		// init kalman
		m_k_lin.ker.ker_dim = 3;
		m_k_lin.ker.obs_dim = 2;

		kalman_lin_init(&m_k_lin);

		m_k_lin.ker.matA.unity();
		m_k_lin.ker.matA.print();

		m_k_lin.ker.matB.zeros();
		m_k_lin.ker.matB.print();

		// set Q: model noise
		m_k_lin.ker.matQ.set(0, 0, 0.03f);
		m_k_lin.ker.matQ.set(1, 1, 0.20f);
		m_k_lin.ker.matQ.set(2, 2, 0.0002f);

		// set P
		m_k_lin.ker.matP.ones(900);

		// set R: observations noise
		m_k_lin.ker.matR.unity(1);
		m_k_lin.ker.matR.set(0, 0, 1000.f); // 400/400 is a good setting for acc_n = 1600, 800
		m_k_lin.ker.matR.set(1, 1, 600.f);  // 1000/600 is for oversampling limited to 1sec. of data

		// init X
		m_k_lin.ker.matX.zeros();
		m_k_lin.ker.matX.set(0, 0, ele);
		m_update_time = millis();

		LOG_INFO("Kalman lin. init !");
	}

	if (m_speed_ms < 1.5f) {
		m_update_time = millis();
		return;
	}

	float pitch_rad;
	fxos_get_pitch(pitch_rad);

	feed.dt = m_speed_ms * 0.001f * (float)(millis() - m_update_time); // in seconds

	// set measures: Z
	feed.matZ.resize(m_k_lin.ker.obs_dim, 1);
	feed.matZ.zeros();
	feed.matZ.set(0, 0, ele);
	feed.matZ.set(1, 0, pitch_rad);

	// measures mapping (Z = C.X)
	m_k_lin.ker.matC.zeros();
	m_k_lin.ker.matC.set(0, 0, 1);
	m_k_lin.ker.matC.set(1, 1, 1);

	// command mapping
	m_k_lin.ker.matB.zeros();

	// set command: U
	feed.matU.resize(m_k_lin.ker.ker_dim, 1);
	feed.matU.zeros();

	// set core
	m_k_lin.ker.matA.unity(1);
	m_k_lin.ker.matA.set(0, 1,  feed.dt);
	m_k_lin.ker.matA.set(0, 2, -feed.dt);

	kalman_lin_feed(&m_k_lin, &feed);

	m_cur_ele  = m_k_lin.ker.matX.get(0, 0);
	alpha_bar  = m_k_lin.ker.matX.get(1, 0);
	alpha_zero = m_k_lin.ker.matX.get(2, 0);

#else

	static uint32_t m_update_time = 0;

	// 7 seconds time constant
	const float taub = 40 / (40 + 1.f);
	m_cur_ele = taub * m_cur_ele + (1 - taub) * (ele);

	static float alti_prev = m_cur_ele;

	float pitch_rad;
	fxos_get_pitch(pitch_rad);

	// 7 seconds time constant
	static float alpha_bar;
	const float tau = 40 / (40 + 1.f);
	float innov = tanf(pitch_rad);
	alpha_bar = tau * alpha_bar + (1 - tau) * (innov);

	float dl = m_speed_ms * 0.001f * (float)(millis() - m_update_time); // in seconds

	if (dl < 1.5f) {
		return;
	}
	m_update_time = millis();

	// work on alpha zero
	// about 40 seconds time constant
	float new_alpha_z = alpha_bar - (m_cur_ele - alti_prev) / dl;
	static float alpha_zero = 0;
	alpha_zero = new_alpha_z * 0.003f + 0.997f * alpha_zero;

	alti_prev = m_cur_ele;

#endif

	// update vertical speed after 3 mins
	if (att.nbsec_act > 3.f * 60.f) {
		const float slope = (alpha_bar - alpha_zero);
		att.slope = (int8_t)(100.f * slope);
		att.vit_asc = slope * m_speed_ms;
	}

	m_update_time = millis();

	LOG_DEBUG("Vit. vert.: %f / alpha: %f / alpha0: %f",
			att.vit_asc,
			180.f*(alpha_bar-alpha_zero)/3.1415f,
			180.f*alpha_zero/3.1415f);

	if (m_st_buffer_nb_elem < ATT_BUFFER_NB_ELEM) {

		m_st_buffer[m_st_buffer_nb_elem].alti.baro_ele   = ele;
		m_st_buffer[m_st_buffer_nb_elem].alti.alpha_zero = alpha_zero;
		m_st_buffer[m_st_buffer_nb_elem].alti.alpha_bar  = alpha_bar;

		// log fxos roughness
		fxos_get_roughness(m_st_buffer[m_st_buffer_nb_elem].alti.rough);
		// log baro roughness
		m_st_buffer[m_st_buffer_nb_elem].alti.b_rough = baro.getRoughness();
	}

#ifdef TDD
		tdd_logger_log_float(TDD_LOGGING_HP    , att.vit_asc);
		tdd_logger_log_float(TDD_LOGGING_ALPHA , 180.f * alpha_bar /3.1415f);
		tdd_logger_log_float(TDD_LOGGING_ALPHA0, 180.f * alpha_zero / 3.1415f);
		tdd_logger_log_float(TDD_LOGGING_EST_SLOPE, 100.0f * (alpha_bar - alpha_zero));
		tdd_logger_log_float(TDD_LOGGING_ALT_EST, m_cur_ele);
#endif

#ifdef USE_JSCOPE
	{
		// output some results to Segger JSCOPE
		jscope.inputData(att.vit_asc,								0);
		jscope.inputData(180.f * alpha_bar / 3.1415f,				4);
		jscope.inputData(180.f * alpha_zero / 3.1415f,				8);
		jscope.inputData(100.0f * (alpha_bar - alpha_zero),			12);
	}

	jscope.flush();
#endif
}


float Attitude::filterElevation(SLoc& loc_, eLocationSource source_) {

	if (!m_is_alt_init) {

		m_last_stored_ele = m_cur_ele = loc_.alt;
		m_climb = 0.;

		m_is_alt_init = true;

#if USE_KALMAN
		m_k_lin.is_init = 0;
#endif

	}

	// high-pass on the difference baro/GPS to remove air pressure drifts
	if (source_ == eLocationSourceGPS ||
			source_ == eLocationSourceNRF) {

		LOG_INFO("Filtering GPS/Baro correction");

		// get barometer elevation
		float ele = 0.;
		if (!m_baro.computeAlti(ele)) {
			LOG_WARNING("Altitude error or sea level missing 2");
			return 0.f;
		}

		// filter with a high time-constant the difference between
		// GPS altitude and barometer altitude to remove drifts
		float input = ele - loc_.alt;
		static float alt_div = input;

		const float taub = 400.f / (400.f + 1.f);
		alt_div = taub * alt_div + (1 - taub) * (input);

		// set barometer correction
		m_baro.setCorrection(alt_div);

		// store correction
		if (m_st_buffer_nb_elem < ATT_BUFFER_NB_ELEM) {
			m_st_buffer[m_st_buffer_nb_elem].alti.baro_corr  = alt_div;
		}

#ifdef TDD
	tdd_logger_log_float(TDD_LOGGING_BARO_DIFF, alt_div);
#endif

	}

	// compute accumulated climb on the corrected filtered barometer altitude
	if (m_cur_ele > m_last_stored_ele + CLIMB_ELEVATION_HYSTERESIS_M) {
		// mise a jour de la montee totale
		m_climb += m_cur_ele - m_last_stored_ele;
		m_last_stored_ele = m_cur_ele;
	}
	else if (m_cur_ele + CLIMB_ELEVATION_HYSTERESIS_M < m_last_stored_ele) {
		// on descend, donc on garde la derniere alti
		// la plus basse
		m_last_stored_ele = m_cur_ele;
	}

	if (m_climb < 0) {
		// reset it
		m_is_alt_init = 0;
	}

#ifdef TDD
	tdd_logger_log_float(TDD_LOGGING_TOT_CLIMB, m_climb);
#endif

	return m_climb;
}


float Attitude::computeElevation(SLoc& loc_, eLocationSource source_) {

	float res = 0.;

	if (eLocationSourceSIM == source_) {

		// compute baro/GPS altitude corrections and total climb
		res = this->filterElevation(loc_, source_);
		return res;
	}

	// init the altitude model
	if (eLocationSourceNone != source_ &&
			m_baro.isDataReady() &&
			!m_baro.hasSeaLevelRef() &&
			att.nbpts > 15) {

		// init sea level pressure
		m_baro.seaLevelForAltitude(loc_.alt);

		LOG_WARNING("Init sea level pressure... gps: %dm bme:%dm", (int)loc_.alt, (int)m_cur_ele);

		// compute baro/GPS altitude corrections and total climb
		// force reset
		m_is_alt_init = false;
		this->filterElevation(loc_, source_);

		if (m_app_error.saved_data.crc == calculate_crc((uint8_t*)&m_app_error.saved_data.att, sizeof(m_app_error.saved_data.att))) {

			// compare dates
			if (att.date.date == m_app_error.saved_data.att.date.date) {

				// restoring position and accumulated climb
				att.climb = m_app_error.saved_data.att.climb;
				m_climb = att.climb;

				att.dist = m_app_error.saved_data.att.dist;
				m_last_save_dist = att.dist;

				att.pr = m_app_error.saved_data.att.pr;
				att.nbsec_act = m_app_error.saved_data.att.nbsec_act;

				LOG_WARNING("Last stored dist: %f", att.dist);

				vue.addNotif("FDIR", "Attitude restored", 6, eNotificationTypeComplete);
			} else {
				// that block comes from another date ..?? or is invalid
				LOG_ERROR("Last stored date: %u", m_app_error.saved_data.att.date.date);
			}
		}

		m_app_error.saved_data.crc = 0x00;

	} else if (eLocationSourceNone != source_ &&
			m_baro.hasSeaLevelRef()) {

		// compute baro/GPS altitude corrections and total climb
		res = this->filterElevation(loc_, source_);

	}

	return res;
}


void Attitude::computeDistance(SLoc& loc_, SDate &date_) {

	float tmp_dist = distance_between(att.loc.lat, att.loc.lon, loc_.lat, loc_.lon);

#if defined( DEBUG_NRF_USER ) || defined( TDD )
	if (tmp_dist > 400000.f) {
		char buffer[100];
		snprintf(buffer, sizeof(buffer), "Weird pos: %f %f %f %f (%f)\r\n", att.loc.lat, att.loc.lon, loc_.lat, loc_.lon, tmp_dist);
		LOG_WARNING(buffer);
	}
#endif

	att.dist += tmp_dist;

	// save attitude to a buffer for later saving to memory
	if (m_is_acc_init &&
			att.dist > m_last_save_dist + 15.f) {

		m_last_save_dist = att.dist;

		// save on buffer
		memcpy(&m_st_buffer[m_st_buffer_nb_elem].loc , &loc_ , sizeof(SLoc));
		memcpy(&m_st_buffer[m_st_buffer_nb_elem].date, &date_, sizeof(SDate));

		m_st_buffer[m_st_buffer_nb_elem].sensors.pwr   = att.pwr;

		m_st_buffer[m_st_buffer_nb_elem].alti.filt_ele = m_cur_ele;
		m_st_buffer[m_st_buffer_nb_elem].alti.vit_asc  = att.vit_asc;
		m_st_buffer[m_st_buffer_nb_elem].alti.climb    = att.climb;
		m_st_buffer[m_st_buffer_nb_elem].alti.gps_ele  = att.loc.alt;

		m_st_buffer[m_st_buffer_nb_elem].sensors.bpm     = hrm_info.bpm;
		m_st_buffer[m_st_buffer_nb_elem].sensors.cadence = bsc_info.cadence;

		m_st_buffer_nb_elem++;

		// save position on queue when buffer is full
		if (m_st_buffer_nb_elem >= ATT_BUFFER_NB_ELEM) {

			// save to SD
			sysview_task_void_enter(SaveUserPosition);
			sd_save_pos_buffer(m_st_buffer, ATT_BUFFER_NB_ELEM);
			sysview_task_void_exit(SaveUserPosition);

			m_st_buffer_nb_elem = 0;

		}

		// save position and stuff in case of crash
		memcpy(&m_app_error.saved_data.att, &att, sizeof(SAtt));
		m_app_error.saved_data.crc = calculate_crc((uint8_t*)&m_app_error.saved_data.att, sizeof(m_app_error.saved_data.att));


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
		att.climb = this->computeElevation(loc_, source_);

		// update the computed power
		att.pwr = this->computePower(loc_.speed);

		this->computeDistance(loc_, date_);
	}

	// update speed and acceleration
	const float n_speed = loc_.speed / 3.6f;
	dv = n_speed - m_speed_ms;
	m_speed_ms = n_speed;

#ifdef TDD
	tdd_logger_log_float(TDD_LOGGING_CUR_SPEED, loc_.speed);
#endif

	m_is_init = true;

	// update global location
	memcpy(&att.loc , &loc_ , sizeof(SLoc));
	// update date
	memcpy(&att.date, &date_, sizeof(SDate));
}

void Attitude::addNewSIMPoint(SLoc& loc_, SDate& date_) {

	LOG_INFO("LNS point added (%lu %lu)", date_.secj, att.date.secj);

	// calculate vertical speed
	att.vit_asc = loc_.alt - m_cur_ele;

	// current filtered elevation is supposed exact: #NoFilter
	m_cur_ele = loc_.alt;

	this->addNewLocation(loc_, date_, eLocationSourceSIM);

}

/**
 *
 * @param speed_
 */
float Attitude::computePower(float speed_) {

	float power = 0.0f;

	const float weight = (float)u_settings.getWeight();

	// STEP 2 : Calculate power
	power += 9.81f * weight * att.vit_asc; // gravity
	power += 0.004f * 9.81f * weight * m_speed_ms; // sol + meca
	power += 0.204f * m_speed_ms * m_speed_ms * m_speed_ms; // air
	//power += weight * m_speed_ms * dv; // acceleration ---> too noisy
	power *= 1.025f; // transmission (rendement velo)

#ifdef TDD
	tdd_logger_log_float(TDD_LOGGING_CUR_POWER, power);
#endif


	return power;
}

/**
 *
 */

void Attitude::addNewFECPoint(sFecInfo& fec_) {

	// TODO

}
