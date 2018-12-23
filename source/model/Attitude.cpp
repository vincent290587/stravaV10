/*
 * Attitude.cpp
 *
 *  Created on: 29 déc. 2017
 *      Author: Vincent
 */

#include <math.h>
#include <Attitude.h>
#include "utils.h"
#include "parameters.h"
#include "Model.h"
#include "sd_functions.h"
#include "segger_wrapper.h"

#ifdef USE_JSCOPE
#include "JScope.h"
JScope jscope;
#endif


Attitude::Attitude() {
	m_last_save_dist = 0.;
	m_last_stored_ele = 0.;
	m_cur_ele = 0.;
	m_climb = 0.;
	m_vit_asc = 0.;

	m_is_init = false;
	m_is_acc_init = false;
	m_is_alt_init = false;

	m_st_buffer_nb_elem = 0;

#ifdef USE_JSCOPE
		jscope.init();
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


float Attitude::filterElevation(void) {

	float ele = 0.;

	if (!baro.computeAlti(&ele)) return 0.;

	m_cur_ele = ele;

	if (!m_is_alt_init) {
		m_is_alt_init = true;
		m_last_stored_ele = ele;
		return 0.;
	}

	if (ele > m_last_stored_ele + 1.) {
		// mise a jour de la montee totale
		m_climb += ele - m_last_stored_ele;
		m_last_stored_ele = ele;
	}
	else if (ele + 1. < m_last_stored_ele) {
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
			!baro.hasSeaLevelRef()) {

		// init sea level pressure
		LOG_INFO("Init sea level pressure...");
		baro.seaLevelForAltitude(loc_.alt, baro.m_pressure);

		// treat elevation
		this->filterElevation();

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
			baro.hasSeaLevelRef()) {

		res = att.climb = this->filterElevation();

		// overwrite GPS/NRF's elevation
		loc_.alt = m_cur_ele;

	}

	return res;
}


void Attitude::computeDistance(SLoc& loc_, SDate &date_, eLocationSource source_) {

	float tmp_dist = distance_between(att.loc.lat, att.loc.lon, loc_.lat, loc_.lon);

	char buffer[100];
	snprintf(buffer, sizeof(buffer), "Weird pos: %f %f %f %f\r\n", att.loc.lat, att.loc.lon, loc_.lat, loc_.lon);
	if (tmp_dist > 400000) LOG_WARNING(buffer);

	att.dist += tmp_dist;

	// save attitude to a buffer for later saving to memory
	if (m_is_acc_init &&
			att.dist > m_last_save_dist + 15.) {

		sysview_task_void_enter(SaveUserPosition);

		m_last_save_dist = att.dist;

		// save position on queue
		if (m_st_buffer_nb_elem >= ATT_BUFFER_NB_ELEM) {

			// save on SD
			sd_save_pos_buffer(m_st_buffer, ATT_BUFFER_NB_ELEM);

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

		sysview_task_void_exit(SaveUserPosition);

	} else if (att.dist > m_last_save_dist + 25.) {
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
		float cur_time = (float)date_.secj + ((millis() - date_.timestamp) / 1000.);

		// add this point to our historic
		mes_points.ajouteFinIso(loc_.lat, loc_.lon, loc_.alt, cur_time, HISTO_POINT_SIZE);
		if (loc_.speed > 7.) att.nbsec_act++;
		att.nbpts++;

		// calculate elevation shifts
		this->computeElevation(loc_, source_);

		// update the computed power
		att.pwr = this->filterPower(loc_.speed);

		this->computeDistance(loc_, date_, source_);

	}

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
float Attitude::filterPower(float speed_) {

	float res = 0.;
	float power = 0.;
	float fSpeed = -1.;
	Point P1, P2, Pc;
	float dTime;
	uint8_t i;

	float _y[FILTRE_NB+1];
	float _x[FILTRE_NB+1];
	float _lrCoef[2];

	if (mes_points.size() <= FILTRE_NB + 1) return res;

	P1 = mes_points.getFirstPoint();
	P2 = mes_points.getPointAt(FILTRE_NB);

	dTime = P1._rtime - P2._rtime;

	if (fabsf(dTime) > 1.5 && fabsf(dTime) < 25) {

		// calcul de la vitesse ascentionnelle par regression lineaire
		for (i = 0; i <= FILTRE_NB; i++) {

			Pc = mes_points.getPointAt(i);
			_x[i] = Pc._rtime - P2._rtime;
			_y[i] = Pc._alt;

			LOG_DEBUG("%d ms %d mm", (int)(_x[i]*1000), (int)(_y[i]*1000));
		}

		_lrCoef[1] = _lrCoef[0] = 0;

		// regression lineaire
		float corrsq = simpLinReg(_x, _y, _lrCoef, FILTRE_NB + 1);

#ifdef USE_JSCOPE
		{
			// output some results to Segger JSCOPE
			jscope.inputData(P1._alt, 0);
			jscope.inputData(_lrCoef[0], 4);
			jscope.inputData(corrsq, 8);
		}

		jscope.flush();
#endif

		// STEP 1 : on filtre altitude et vitesse
		if (corrsq > 0.8) {
			m_vit_asc = _lrCoef[0];

			LOG_INFO("Vit. vert.= %d mm/s (corr= %f)",
					(int)(m_vit_asc*1000), corrsq);
		} else {
			m_vit_asc = 0;

			LOG_INFO("Vit. vert.= %d mm/s (corr= %f)",
					(int)(m_vit_asc*1000), corrsq);
		}


		// horizontal speed (m/s)
		fSpeed = speed_ / 3.6;

		// STEP 2 : Calcul
		power = 9.81 * MASSE * m_vit_asc; // grav
		power += 0.004 * 9.81 * MASSE * fSpeed; // sol + meca
		power += 0.204 * fSpeed * fSpeed * fSpeed; // air
		power *= 1.025; // transmission (rendement velo)
		res = power;

	} else {
		LOG_INFO("dTime= %d ms", (int)(dTime*1000));
	}

	res = power;

	return res;
}



/**
 *
 */

void Attitude::addNewFECPoint(sFecInfo& fec_) {

	// TODO

}

