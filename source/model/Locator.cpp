/*
 * Locator.cpp
 *
 *  Created on: 19 oct. 2017
 *      Author: Vincent
 */

#include "segger_wrapper.h"
#include "parameters.h"
#include "nordic_common.h"
#include "utils.h"
#include "TinyGPS++.h"
#include <Model.h>
#include <stdlib.h>
#include <Locator.h>


static bool m_is_gps_updated = false;

TinyGPSPlus   gps;
TinyGPSCustom hdop(gps, "GPGSA", 16);       // $GPGSA sentence, 16th element
TinyGPSCustom vdop(gps, "GPGSA", 17);       // $GPGSA sentence, 17th element

TinyGPSCustom totalGPGSVMessages(gps, "GPGSV", 1); // $GPGSV sentence, first element
TinyGPSCustom messageNumber(gps, "GPGSV", 2);      // $GPGSV sentence, second element

TinyGPSCustom satsInView(gps, "GPGSV", 3);  // $GPGSV sentence, third element

TinyGPSCustom satsInUse(gps, "GNGGA", 7);  // $GPGSV sentence, 7th element

TinyGPSCustom satNumber[4]; // to be initialized later
TinyGPSCustom elevation[4];
TinyGPSCustom azimuth[4];
TinyGPSCustom snr[4];

sSatellite sats[MAX_SATELLITES];

/**
 *
 * @param c Character to encode
 * @return
 */
uint32_t locator_encode_char(char c) {

	LOG_DEBUG("%c", c);

	if (gps.encode(c)) {

		if (GPS_SENTENCE_GPRMC == gps.curSentenceType) {
			m_is_gps_updated = true;
		}

	}

	return 0;
}

/**
 *
 */
void locator_dispatch_lns_update(sLnsInfo *lns_info) {

	// convert data
	locator.nrf_loc.data.alt = (float)(lns_info->ele / 100.f);
	locator.nrf_loc.data.lat = (float)(lns_info->lat / 10000000.f);
	locator.nrf_loc.data.lon = (float)(lns_info->lon / 10000000.f);
	locator.nrf_loc.data.speed = (float)(lns_info->speed / 10.f);

	locator.nrf_loc.data.course = (float)(lns_info->heading);
	locator.nrf_loc.data.utc_time = lns_info->secj;
	locator.nrf_loc.data.date = lns_info->date;

	locator.nrf_loc.setIsUpdated();

	// notify task
	w_task_events_set(m_tasks_id.boucle_id, TASK_EVENT_LOCATION);
}


Locator::Locator() {
	m_is_gps_updated = false;

	anyChanges   = false;

	m_nb_nrf_pos = 0;

	// Initialize all the uninitialized TinyGPSCustom objects
	for (int i = 0; i < 4; ++i)
	{
	    satNumber[i].begin(gps, "GPGSV", 4 + 4 * i); // offsets 4, 8, 12, 16
	    elevation[i].begin(gps, "GPGSV", 5 + 4 * i); // offsets 5, 9, 13, 17
	    azimuth[i].begin(  gps, "GPGSV", 6 + 4 * i); // offsets 6, 10, 14, 18
	    snr[i].begin(      gps, "GPGSV", 7 + 4 * i); // offsets 7, 11, 15, 19
	}

}

/**
 *
 * @return
 */
eLocationSource Locator::getUpdateSource() {

	this->tasks();

	if (sim_loc.isUpdated()) {
		return eLocationSourceSimu;
	} else if (sim_loc.getAge() < 2000) {
		return eLocationSourceNone;
	}

	if (gps_loc.isUpdated()) {
		return eLocationSourceGPS;
	} else if (gps_loc.getAge() < 1500) {
		return eLocationSourceNone;
	}

	if (nrf_loc.isUpdated() && !gps_mgmt.isFix()) {
		return eLocationSourceNRF;
	} else if (nrf_loc.isUpdated()) {
		LOG_INFO("LNS data refused: GPS data valid");
	}

	return eLocationSourceNone;
}

/**
 *
 * @return
 */
bool Locator::isUpdated()      {

	eLocationSource source = this->getUpdateSource();

	if (eLocationSourceNone != source) {
		LOG_DEBUG("Locator source: %u", source);
		return true;
	}
	return false;
}



/**
 *
 * @return The last update age in milliseconds
 */
uint32_t Locator::getLastUpdateAge() {

	uint32_t last_update_age;

	last_update_age = MIN(sim_loc.getAge(), gps_loc.getAge());
	last_update_age = MIN(last_update_age , nrf_loc.getAge());

	return last_update_age;
}

/**
 *
 * @param lat
 * @param lon
 * @param sec_
 * @return
 */
eLocationSource Locator::getPosition(SLoc& loc_, SDate& date_) {

	eLocationSource res = this->getUpdateSource();

	LOG_INFO("Locator update source: %u", (uint8_t)res);

	switch (res) {
	case eLocationSourceSimu:
	{
		loc_.lat = sim_loc.data.lat;
		loc_.lon = sim_loc.data.lon;
		loc_.alt = sim_loc.data.alt / 100.f;
		loc_.speed = 20.f;
		loc_.course = -1.f;
		date_.secj = sim_loc.data.utc_time;
		date_.date = 291217;
		date_.timestamp = millis();
		sim_loc.clearIsUpdated();
	}
	break;
	case eLocationSourceNRF:
	{
		loc_.lat = nrf_loc.data.lat;
		loc_.lon = nrf_loc.data.lon;
		loc_.alt = nrf_loc.data.alt;
		loc_.speed = nrf_loc.data.speed;
		loc_.course = nrf_loc.data.course;
		date_.secj = nrf_loc.data.utc_time;
		date_.date = nrf_loc.data.date;
		date_.timestamp = nrf_loc.data.utc_timestamp;
		nrf_loc.clearIsUpdated();

		if (5 == ++m_nb_nrf_pos) {

			gps_mgmt.startHostAidingEPO(nrf_loc.data, 500);

		}
	}
	break;
	case eLocationSourceGPS:
	{
		loc_.lat = gps_loc.data.lat;
		loc_.lon = gps_loc.data.lon;
		loc_.alt = gps_loc.data.alt;
		loc_.speed = gps_loc.data.speed;
		loc_.course = gps_loc.data.course;
		date_.secj = gps_loc.data.utc_time;
		date_.date = gps_loc.data.date;
		date_.timestamp = gps_loc.data.utc_timestamp;
		gps_loc.clearIsUpdated();

		m_nb_nrf_pos = 0;
	}
	break;
	case eLocationSourceNone:
		date_.secj = gps_loc.data.utc_time;
		date_.date = gps_loc.data.date;
		date_.timestamp = gps_loc.data.utc_timestamp;
		break;
	default:
		break;
	}

	return res;
}


/**
 *
 * @param lat
 * @param lon
 * @param sec_
 * @return
 */
eLocationSource Locator::getDate(SDate& date_) {

	eLocationSource res = this->getUpdateSource();

	switch (res) {
	case eLocationSourceSimu:
	{
		date_.secj = sim_loc.data.utc_time;
		date_.date = 291217;

		date_.timestamp = millis();
	}
	break;
	case eLocationSourceNRF:
	{
		date_.secj = nrf_loc.data.utc_time;
		date_.date = nrf_loc.data.date;

		date_.timestamp = nrf_loc.data.utc_timestamp;

	}
	break;
	case eLocationSourceGPS:
	{
		date_.secj = gps_loc.data.utc_time;
		date_.date = gps_loc.data.date;

		date_.timestamp = gps_loc.data.utc_timestamp;
	}
	break;
	case eLocationSourceNone:
		date_.secj = gps_loc.data.utc_time;
		date_.date = gps_loc.data.date;

		date_.timestamp = gps_loc.data.utc_timestamp;
		break;
	default:
		break;
	}

	return res;
}

/**
 * Used to get the data from the GPS parsing module
 */
void Locator::tasks() {

	if (m_is_gps_updated) {
		m_is_gps_updated = false;

		if (gps.time.isValid()) {
			gps_loc.data.utc_time = get_sec_jour(gps.time.hour(), gps.time.minute(), gps.time.second());

			gps_loc.data.utc_timestamp = millis();

			gps_loc.data.date = gps.date.year()   % 100;
			gps_loc.data.date += gps.date.day()   * 10000;
			gps_loc.data.date += gps.date.month() * 100;

		}

		if (gps.location.isValid()) {

			// notify task
			w_task_events_set(m_tasks_id.boucle_id, TASK_EVENT_LOCATION);

			gps_loc.data.speed  = (float)gps.speed.kmph();
			gps_loc.data.alt    = (float)gps.altitude.meters();
			gps_loc.data.lat    = (float)gps.location.lat();
			gps_loc.data.lon    = (float)gps.location.lng();

			gps_loc.data.course = (float)gps.course.deg();

			LOG_INFO("GPS location set");

			gps_loc.setIsUpdated();

		} else if (gps.time.isValid()) {
			LOG_DEBUG("GPS location invalid !");
			// trick to force taking LNS data
			gps_loc.data.utc_time -= (LNS_OVER_GPS_DTIME_S + 3);
		}

		if (totalGPGSVMessages.isUpdated()) {

			for (int i=0; i<4; ++i) {

				int no = atoi(satNumber[i].value());
				// Serial.print(F("SatNumber is ")); Serial.println(no);
				if (no >= 1 && no <= MAX_SATELLITES)
				{
					sats[no-1].elevation = atoi(elevation[i].value());
					sats[no-1].azimuth   = atoi(azimuth[i].value());
					sats[no-1].snr       = atoi(snr[i].value());
					sats[no-1].active    = ACTIVE_VAL;
				}
			}
		}

	}
}

/**
 * Returns the GPS date
 *
 * @param iYr
 * @param iMo
 * @param iDay
 * @param iHr
 */
bool Locator::getGPSDate(int& iYr, int& iMo, int& iDay, int& iHr) {

	iYr = gps.date.year();
	iMo = gps.date.month();
	iDay = gps.date.day();
	iHr = gps.time.hour();

	return (gps.date.isValid() && gps.time.isValid());
}


/**
 *
 */
void Locator::displayGPS2(void) {

	sysview_task_void_enter(Ls027Print);

//	int totalMessages = atoi(totalGPGSVMessages.value());
//	int currentMessage = atoi(messageNumber.value());

	vue.setCursor(20,20);
	vue.setTextSize(2);

	vue.print(satsInUse.value());
	vue.print(F(" used of "));
	vue.println(satsInView.value());
	vue.println("");

	if (gps.location.isValid()) {
		vue.println("Loc valid");
	} else {
		vue.println("Loc pb");
	}

	String line = "Loc age: ";
	line += String((int)gps.location.age());
	vue.println(line);

	if (gps_mgmt.isFix()) {
		vue.println("FIX pin high");
	} else {
		vue.println("No fix");
	}

	line = "Time age: ";
	line += String((int)gps.time.age());
	vue.println(line);

	vue.println("  ------");

	sysview_task_void_exit(Ls027Print);

}

