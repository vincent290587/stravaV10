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
	m_power = 0.;

	m_is_init = false;
	m_is_alt_init = false;

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


void Attitude::computeElevation(void) {

	float ele = 0.;

	if (!baro.computeAlti(&ele)) return;

	m_cur_ele = ele;

	if (!m_is_alt_init) {
		m_is_alt_init = true;
		m_last_stored_ele = ele;
		return;
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
}


/**
 *
 * @param loc_
 */
void Attitude::addNewLocation(SLoc& loc_, SDate &date_, eLocationSource source_) {

	// small correction to allow time with millisecond precision
	float cur_time = (float)date_.secj + ((millis() - date_.timestamp) / 1000.);

	// init the altitude model
	if ((eLocationSourceGPS == source_ ||
			eLocationSourceNRF == source_) &&
			!baro.hasSeaLevelRef()) {

		// init sea level pressure
		LOG_INFO("Init sea level pressure...");
		baro.seaLevelForAltitude(loc_.alt, baro.m_pressure);

		// treat elevation
		this->computeElevation();

		// reset accumulated data
		m_climb = 0.;
		m_last_stored_ele = m_cur_ele;

	} else if ((eLocationSourceGPS == source_ ||
			eLocationSourceNRF == source_) &&
			baro.hasSeaLevelRef()) {

		// treat elevation
		this->computeElevation();

		att.climb = m_climb;

		// overwrite GPS/NRF's elevation
		loc_.alt = m_cur_ele;

	}

	// update the computed power
	this->majPower(loc_.speed);

	att.pwr = m_power;

	// add this point to our historic
	mes_points.ajouteFinIso(loc_.lat, loc_.lon, loc_.alt, cur_time, HISTO_POINT_SIZE);

	float tmp_dist = distance_between(att.loc.lat, att.loc.lon, loc_.lat, loc_.lon);

	att.dist += tmp_dist;

	// save attitude to a buffer for later saving to memory
	if (m_is_init &&
			att.dist > m_last_save_dist + 15) {

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

	} else if (att.dist > m_last_save_dist + 25) {
		// first position
		att.dist = 0;
		m_is_init = true;
	}

	// update global location
	memcpy(&att.loc , &loc_ , sizeof(SLoc));
	// update date
	memcpy(&att.date, &date_, sizeof(SDate));
	// save position and stuff
	memcpy(&m_app_error.saved_att, &att, sizeof(SAtt));
}


/**
 *
 * @param speed_
 */
void Attitude::majPower(float speed_) {

	float fSpeed = -1.;
	Point P1, P2, Pc;
	float dTime;
	uint8_t i;

	float _y[FILTRE_NB+1];
	float _x[FILTRE_NB+1];
	float _lrCoef[2];

	if (mes_points.size() <= FILTRE_NB + 1) return;

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
		m_power = 9.81 * MASSE * m_vit_asc; // grav
		m_power += 0.004 * 9.81 * MASSE * fSpeed; // sol + meca
		m_power += 0.204 * fSpeed * fSpeed * fSpeed; // air
		m_power *= 1.025; // transmission (rendement velo)

	} else {
		LOG_INFO("dTime= %d ms", (int)(dTime*1000));
	}

	return;
}



/**
 *
 */

void Attitude::addNewFECPoint(sFecInfo& fec_) {

	// TODO

}

