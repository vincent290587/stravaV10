/*
 * Attitude.cpp
 *
 *  Created on: 29 déc. 2017
 *      Author: Vincent
 */

#include <Attitude.h>
#include "Model.h"
#include "parameters.h"


Attitude::Attitude() {
	m_last_save_dist = 0.;

	m_is_init = false;
}


/**
 *
 * @param loc_
 */
void Attitude::addNewDate(SDate &date_) {

	// update date
	memcpy(&att.date, &date_, sizeof(SDate));
}

/**
 *
 * @param loc_
 */
void Attitude::addNewLocation(SLoc& loc_, SDate &date_) {

	// small correction to allow time with millisecond precision
	float cur_time = (float)date_.secj + ((millis() - date_.timestamp) / 1000.);

	// add this point to our historic
	mes_points.ajouteFinIso(loc_.lat, loc_.lon, loc_.alt, cur_time, HISTO_POINT_SIZE);

	float tmp_dist = distance_between(att.loc.lat, att.loc.lon, loc_.lat, loc_.lon);

	att.dist += tmp_dist;

	if (m_is_init &&
			att.dist > m_last_save_dist + 15) {

		m_last_save_dist = att.dist;

		// TODO save position on SD card

	} else if (att.dist > m_last_save_dist + 25) {
		// first position
		att.dist = 0;
		m_is_init = true;
	}

	// update global location
	memcpy(&att.loc , &loc_ , sizeof(SLoc));
	// update date
	memcpy(&att.date, &date_, sizeof(SDate));
}

/**
 *
 */
void Attitude::addNewFECPoint(sFecInfo& fec_) {

	// TODO

}
