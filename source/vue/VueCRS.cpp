/*
 * VueCRS.cpp
 *
 *  Created on: 12 déc. 2017
 *      Author: Vincent
 */

#include "assert.h"
#include "Model.h"
#include "ScreenUtils.h"
#include "segger_wrapper.h"
#include <vue/VueCRS.h>


#define VUE_CRS_NB_LINES         7


VueCRS::VueCRS() : Adafruit_GFX(0, 0) {
	m_crs_screen_mode = eVueCRSScreenInit;
}

eVueCRSScreenModes VueCRS::tasksCRS() {

	LOG_DEBUG("Last update age: %lu\r\n", locator.getLastUpdateAge());

	if (locator.getLastUpdateAge() > LOCATOR_MAX_DATA_AGE_MS) {
		m_crs_screen_mode = eVueCRSScreenInit;
	}
	else if (m_crs_screen_mode == eVueCRSScreenInit) {
		m_crs_screen_mode = eVueCRSScreenDataFull;
	}

	if (m_crs_screen_mode != eVueCRSScreenInit) {
		switch (m_segs.nb_segs) {
		case 0:
			m_crs_screen_mode = eVueCRSScreenDataFull;
			break;
		case 1:
			m_crs_screen_mode = eVueCRSScreenDataSS;
			break;
		case 2:
			m_crs_screen_mode = eVueCRSScreenDataDS;
			break;
		default:
			break;
		}
	}

	switch (m_crs_screen_mode) {
	case eVueCRSScreenInit:

		// display GPS page
		this->displayGPS();

		break;
	case eVueCRSScreenDataFull:
	{
		this->cadran(1, VUE_CRS_NB_LINES, 1, "Dist", _fmkstr(att.dist / 1000., 1U), "km");
		this->cadran(1, VUE_CRS_NB_LINES, 2, "Pwr", _imkstr(att.pwr), "W");

		this->cadran(2, VUE_CRS_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
		this->cadran(2, VUE_CRS_NB_LINES, 2, "Climb", _imkstr((int)att.climb), "m");

		this->cadran(3, VUE_CRS_NB_LINES, 1, "CAD", _imkstr(cad.getData().rpm), "rpm");
		this->cadran(3, VUE_CRS_NB_LINES, 2, "HRM", _imkstr(hrm.getData().bpm), "bpm");

		this->cadran(4, VUE_CRS_NB_LINES, 1, "PR", _imkstr(att.pr), 0);
		this->cadran(4, VUE_CRS_NB_LINES, 2, "VA", _fmkstr(att.vit_asc * 3.600, 1U), "km/h");

		this->cadranH(5, VUE_CRS_NB_LINES, "Next", _imkstr(att.next), "m");

		this->cadran(6, VUE_CRS_NB_LINES, 1, "Avg", _imkstr((int)stc.getAverageCurrent()), "mA");
		this->cadran(6, VUE_CRS_NB_LINES, 2, "Temp", _fmkstr(stc.getTemperature(), 1U), "C");

		this->cadran(7, VUE_CRS_NB_LINES, 1, "STC", _imkstr((int)stc.getCurrent()), "mA");
		this->cadran(7, VUE_CRS_NB_LINES, 2, "SOC", _imkstr(percentageBatt(stc.getVoltage(), stc.getCurrent())), "%");
	}
	break;
	case eVueCRSScreenDataSS:
	{
		this->cadran(1, VUE_CRS_NB_LINES, 1, "Dist", _fmkstr(att.dist / 1000, 1U), "km");
		this->cadran(1, VUE_CRS_NB_LINES, 2, "Pwr", _imkstr(att.pwr), "W");

		this->cadran(2, VUE_CRS_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
		this->cadran(2, VUE_CRS_NB_LINES, 2, "Climb", _fmkstr(att.climb, 1U), "m");

		this->cadran(3, VUE_CRS_NB_LINES, 1, "CAD", _imkstr(cad.getData().rpm), "rpm");
		this->cadran(3, VUE_CRS_NB_LINES, 2, "HRM", _imkstr(hrm.getData().bpm), "bpm");

		this->cadran(4, VUE_CRS_NB_LINES, 1, "PR", _imkstr(att.pr), 0);
		this->cadran(4, VUE_CRS_NB_LINES, 2, "VA", _fmkstr(att.vit_asc * 3.600, 1U), "km/h");

		this->afficheSegment(5, m_segs.s_segs[0].p_seg);

		assert(m_segs.s_segs[0].p_seg);

		if (SEG_OFF == m_segs.s_segs[0].p_seg->getStatus()) {

			this->cadranH(7, VUE_CRS_NB_LINES, "Next", _imkstr(att.next), "m");

		} else {
			this->partner(7, m_segs.s_segs[0].p_seg);
		}

	}
	break;
	case eVueCRSScreenDataDS:
	{
		this->cadran(1, VUE_CRS_NB_LINES, 1, "VA", _fmkstr(att.vit_asc * 3.600, 1U), "km/h");
		this->cadran(1, VUE_CRS_NB_LINES, 2, "HRM", _imkstr(hrm.getData().bpm), "bpm");

		assert(m_segs.s_segs[0].p_seg);
		assert(m_segs.s_segs[1].p_seg);

		if (SEG_OFF == m_segs.s_segs[0].p_seg->getStatus() &&
				SEG_OFF == m_segs.s_segs[1].p_seg->getStatus()) {

			// all segments are OFF
			this->cadran(2, VUE_CRS_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
			this->cadran(2, VUE_CRS_NB_LINES, 2, "Pwr", _imkstr(att.pwr), "W");

			this->cadran(3, VUE_CRS_NB_LINES, 1, "CAD", _imkstr(cad.getData().rpm), "rpm");
			this->cadran(3, VUE_CRS_NB_LINES, 2, "Climb", _fmkstr(att.climb, 1U), "m");

			this->cadran(4, VUE_CRS_NB_LINES, 1, "Dist", _fmkstr(att.dist / 1000., 1U), "km");
			this->cadran(4, VUE_CRS_NB_LINES, 2, "STC", _imkstr((int)stc.getCurrent()), "mA");

			this->afficheSegment(VUE_CRS_NB_LINES - 2, m_segs.s_segs[0].p_seg);
			this->afficheSegment(VUE_CRS_NB_LINES - 2, m_segs.s_segs[1].p_seg);

			this->cadranH(VUE_CRS_NB_LINES, VUE_CRS_NB_LINES, "Next", _imkstr(att.next), "m");

		} else if (SEG_OFF == m_segs.s_segs[0].p_seg->getStatus()) {

			// only one segment is OFF
			this->cadran(2, VUE_CRS_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
			this->cadran(2, VUE_CRS_NB_LINES, 2, "Pwr", _imkstr(att.pwr), "W");

			this->afficheSegment(VUE_CRS_NB_LINES - 4, m_segs.s_segs[0].p_seg);

			this->afficheSegment(VUE_CRS_NB_LINES - 2, m_segs.s_segs[1].p_seg);
			this->partner(VUE_CRS_NB_LINES, m_segs.s_segs[1].p_seg);

		} else if (SEG_OFF == m_segs.s_segs[1].p_seg->getStatus()) {

			// only one segment is OFF
			this->cadran(2, VUE_CRS_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
			this->cadran(2, VUE_CRS_NB_LINES, 2, "Pwr", _imkstr(att.pwr), "W");

			this->afficheSegment(VUE_CRS_NB_LINES - 4, m_segs.s_segs[0].p_seg);
			this->partner(VUE_CRS_NB_LINES - 2, m_segs.s_segs[0].p_seg);

			this->afficheSegment(VUE_CRS_NB_LINES - 1, m_segs.s_segs[1].p_seg);

		} else {

			// no segment is OFF
			this->afficheSegment(VUE_CRS_NB_LINES - 5, m_segs.s_segs[0].p_seg);
			this->partner(VUE_CRS_NB_LINES - 3, m_segs.s_segs[0].p_seg);

			this->afficheSegment(VUE_CRS_NB_LINES - 2, m_segs.s_segs[1].p_seg);
			this->partner(VUE_CRS_NB_LINES, m_segs.s_segs[1].p_seg);
		}


	}
	break;
	}

	memset(&m_segs, 0, sizeof(m_segs));

	return m_crs_screen_mode;
}

void VueCRS::afficheSegment(uint8_t ligne, Segment *p_seg) {

	float minLat = 100.;
	float minLon = 400.;
	float minAlt = 10000.;
	float maxLat = -100.;
	float maxLon = -400.;
	float maxAlt = -100.;
	float maDpex = 0;
	float maDpey = 0;
	uint16_t points_nb = 0;
	ListePoints *liste = nullptr;
	Point pCourant, pSuivant;
	Point *maPos = nullptr;

	assert(p_seg);

	if (p_seg->longueur() < 4) {
		LOG_ERROR("Segment %s not loaded properly\r\n", p_seg->getName());
		return;
	}

	W_SYSVIEW_OnTaskStartExec(DISPLAY_TASK3);

	uint16_t debut_cadran = _height / VUE_CRS_NB_LINES * (ligne - 1);
	uint16_t fin_cadran   = _height / VUE_CRS_NB_LINES * (ligne + 1);

	// print delimiters
	if (ligne > 1) drawFastHLine(0, debut_cadran, _width, LS027_PIXEL_BLACK);
	if (ligne < VUE_CRS_NB_LINES) drawFastHLine(0, fin_cadran, _width, LS027_PIXEL_BLACK);

	// on cherche la taille de fenetre
	liste = p_seg->getListePoints();

	assert(liste);

	// compute convertion between deg and meters
	float deglon_to_m = 1000. * distance_between(att.loc.lat, att.loc.lon, att.loc.lat, att.loc.lon + 0.001);
	float deglat_to_m = 1000. * distance_between(att.loc.lat, att.loc.lon, att.loc.lat + 0.001, att.loc.lon);

	if (p_seg->getStatus() == SEG_OFF) {

		// our pos is at the center
		maxLat = minLat = att.loc.lat;
		maxLon = minLon = att.loc.lon;
		maxAlt = minAlt = att.loc.alt;

		LOG_DEBUG("Printing SEG OFF\r\n");

		// zoom level
		minLat -= 300 / deglat_to_m;
		minLon -= 300 / deglon_to_m;
		minAlt -= 15.;
		maxLat += 300 / deglat_to_m;
		maxLon += 300 / deglon_to_m;
		maxAlt += 15.;

	} else {

		Vecteur& delta = liste->getDeltaListe();
		Point2D& centre = liste->getCenterListe();

		LOG_DEBUG("VueCRS centre lon: %ld\r\n", (int)((centre._lon - att.lon) * deglon_to_m));
		LOG_DEBUG("VueCRS centre lat: %ld\r\n", (int)((centre._lat - att.lat) * deglat_to_m));

		LOG_DEBUG("VueCRS delta1 lon: %ld\r\n", (int)(delta._x * deglon_to_m));
		LOG_DEBUG("VueCRS delta1 lat: %ld\r\n", (int)(delta._y * deglat_to_m));

		minLat = centre._lat - delta._y / 2;
		maxLat = centre._lat + delta._y / 2;
		minLon = centre._lon - delta._x / 2;
		maxLon = centre._lon + delta._x / 2;

		// on essaye de rendre l'image carree: ratio 1:1
		float v_span = distance_between(minLat, minLon, maxLat, minLon);
		float h_span = distance_between(minLat, minLon, minLat, maxLon);

		if (h_span < VH_RATIO * v_span) {
			minLon -= 50 / deglon_to_m;
			maxLon += 50 / deglon_to_m;
		} else if (v_span < VH_RATIO * h_span) {
			minLat -= 50 / deglat_to_m;
			maxLat += 50 / deglat_to_m;
		}

		// marge
		minLat -= 100 / deglat_to_m;
		minLon -= 100 / deglon_to_m;
		minAlt -= 15.;
		maxLat += 100 / deglat_to_m;
		maxLon += 100 / deglon_to_m;
		maxAlt += 15.;

		LOG_DEBUG("Printing SEG status=%d\r\n", p_seg->getStatus());

		LOG_DEBUG("VueCRS delta2 lon: %ld\r\n", (int)((maxLon - minLon) * deglon_to_m));
		LOG_DEBUG("VueCRS delta2 lat: %ld\r\n", (int)((maxLat - minLat) * deglat_to_m));

	}

	W_SYSVIEW_OnTaskStopExec(DISPLAY_TASK3);
	W_SYSVIEW_OnTaskStartExec(DISPLAY_TASK4);

	// on affiche
	points_nb = 0;
	uint16_t pourc = 0;
	bool pourc_found = 0;
	for (auto& pPt : *liste->getLPTS()) {

		if (p_seg->getStatus() == SEG_OFF && points_nb > SEG_OFF_NB_POINTS) {
			break;
		}

		pSuivant = pPt;

		if (!pourc_found) {
			if (pPt._rtime == liste->m_P1._rtime) {
				pourc_found = true;
				pourc = (100 * points_nb) / liste->size();
			}
		}

		if (points_nb) {

			if (!pSuivant.isValid() || !pCourant.isValid()) break;

			drawLine(regFenLim(pCourant._lon, minLon, maxLon, 0, _width),
					regFenLim(pCourant._lat, minLat, maxLat, fin_cadran, debut_cadran),
					regFenLim(pSuivant._lon, minLon, maxLon, 0, _width),
					regFenLim(pSuivant._lat, minLat, maxLat, fin_cadran, debut_cadran), LS027_PIXEL_BLACK);
		}

		pCourant = pSuivant;
		points_nb++;
	}
	LOG_DEBUG("VueCRS %u points printed\r\n", points_nb);

	// draw a circle at the end of the segment
	if (p_seg->getStatus() != SEG_OFF) {
		drawCircle(regFenLim(pSuivant._lon, minLon, maxLon, 0, _width),
				regFenLim(pSuivant._lat, minLat, maxLat, fin_cadran, debut_cadran), 5, LS027_PIXEL_BLACK);
	} else {
		// draw a circle at the start of the segment
		maPos = liste->getFirstPoint();
		drawCircle(regFenLim(maPos->_lon, minLon, maxLon, 0, _width),
				regFenLim(maPos->_lat, minLat, maxLat, fin_cadran, debut_cadran), 5, LS027_PIXEL_BLACK);
	}

	// limit position when segment is finished
	float _lat, _lon;
	if (p_seg->getStatus() < SEG_OFF) {
		_lon = pCourant._lon;
		_lat = pCourant._lat;
	} else {
		_lon = att.loc.lon;
		_lat = att.loc.lat;
	}

	// ma position
	maDpex = regFenLim(_lon, minLon, maxLon, 0, _width);
	maDpey = regFenLim(_lat, minLat, maxLat, fin_cadran, debut_cadran);
	if (att.loc.course > 0) {
		int16_t x0f, x0 = maDpex;
		int16_t y0f, y0 = maDpey - 15;

		int16_t x1f, x1 = maDpex + 5;
		int16_t y1f, y1 = maDpey + 5;

		int16_t x2f, x2 = maDpex - 5;
		int16_t y2f, y2 = maDpey + 5;

		rotate_point(att.loc.course, maDpex, maDpey, x0, y0, x0f, y0f);
		rotate_point(att.loc.course, maDpex, maDpey, x1, y1, x1f, y1f);
		rotate_point(att.loc.course, maDpex, maDpey, x2, y2, x2f, y2f);

		drawTriangle(x0f, y0f, x1f, y1f, x2f, y2f, LS027_PIXEL_BLACK);
	} else {
		fillCircle(maDpex, maDpey, 4, LS027_PIXEL_BLACK);
	}

	// return before printing text
	if (p_seg->getStatus() == SEG_OFF) {
		return;
	}

	if (maDpey > fin_cadran - 30) {
		setCursor(maDpex > _width - 70 ? _width - 70 : maDpex, maDpey - 20);
	} else {
		setCursor(maDpex > _width - 70 ? _width - 70 : maDpex, maDpey + 15);
	}

	setTextSize(2);

	print(_fmkstr(p_seg->getAvance(), 1U));

	// completion
	if (pourc_found) {
		if (p_seg->getStatus() < SEG_OFF) {
			pourc = 100;
		}

		setCursor(10, debut_cadran + 10);
		print(pourc);
		print("%");
	}

	W_SYSVIEW_OnTaskStopExec(DISPLAY_TASK4);
}


/**
 * Indicateur du pourcentage de perf en forme d'avion
 */
void VueCRS::partner(uint8_t ligne, Segment *p_seg) {

	int hl, ol, dixP;
	float indice;
	static int centre = _width / 2;
	float rtime, curtime;

	assert(p_seg);

	rtime   = p_seg->getAvance();
	curtime = p_seg->getCur();

	if (curtime < 5.) {
		indice = rtime / 5.;
	} else {
		indice = rtime / curtime;
	}

	hl = _height / VUE_CRS_NB_LINES * (ligne - 1) + 25;
	ol = 30;

	fillRect(ol, hl, _width - 2 * ol, 3, LS027_PIXEL_BLACK);

	// on fait apparaitre l'indice de performance en %
	// -> si -0.25 centre en ol
	// -> si  0.25 centre en 240-ol
	//centre = ol + (LCDWIDTH - ol - ol) * (0.25 + indice) / (0.5);
	centre = regFenLim(indice, -0.25, 0.25, ol, _width - ol);
	if (centre < ol) {
		centre = ol;
	} else if (centre > (int)_width - ol) {
		centre = _width - ol;
	}

	dixP = (_width - 2.*ol) * 10. / 50.;
	drawFastVLine(_width / 2 - dixP, hl + 2, 7, LS027_PIXEL_BLACK);
	drawFastVLine(_width / 2 + dixP, hl + 2, 7, LS027_PIXEL_BLACK);

	fillTriangle(centre - 7, hl + 7, centre, hl - 7, centre + 7, hl + 7, LS027_PIXEL_BLACK);
	setCursor(centre - 15, hl + 12);
	setTextSize(VUE_CRS_NB_LINES > 7 ? 1 : 2);
	print(_imkstr((int)(indice * 100.)));

	// marques
	drawFastVLine(_width / 2, hl - 12, 12, LS027_PIXEL_BLACK);

}
