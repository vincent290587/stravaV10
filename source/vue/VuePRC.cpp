/*
 * VuePRC.cpp
 *
 *  Created on: 12 déc. 2017
 *      Author: Vincent
 */

#include "assert_wrapper.h"
#include "ant.h"
#include "Model.h"
#include "segger_wrapper.h"
#include <sd/sd_functions.h>
#include <vue/Screenutils.h>
#include <vue/VuePRC.h>


#define VUE_PRC_NB_LINES         7

Zoom zoom;


VuePRC::VuePRC() : Adafruit_GFX(0, 0) {
	m_prc_screen_mode = eVuePRCScreenInit;

	m_distance_prc = 0.;

	m_parcours_sel = 0;

	m_s_parcours = nullptr;

	m_selec_en   = false;

	m_start_loading = false;
}

eVuePRCScreenModes VuePRC::tasksPRC() {

	LOG_DEBUG("Last update age: %lu\r\n", locator.getLastUpdateAge());

	LOG_INFO("Points2D allocated: %d\r\n",Point2D::getObjectCount()-Point::getObjectCount());
	LOG_INFO("Points   allocated: %d\r\n",Point::getObjectCount());

	// switch back and forth to GPS information page
	if (eVuePRCScreenInit != m_prc_screen_mode &&
			locator.getLastUpdateAge() > LOCATOR_MAX_DATA_AGE_MS) {
		m_prc_screen_mode = eVuePRCScreenGps;
	}
	else if (m_prc_screen_mode == eVuePRCScreenGps &&
			locator.getLastUpdateAge() <= LOCATOR_MAX_DATA_AGE_MS) {
		m_prc_screen_mode = eVuePRCScreenDataFull;
	}

	switch (m_prc_screen_mode) {
	case eVuePRCScreenInit:
	{
		m_selec_en = true;

		// let the user select one parcours
		this->parcoursSelector();

		break;
	}
	case eVuePRCScreenGps:
	{
		// display GPS page
		this->displayGPS();

		break;
	}
	case eVuePRCScreenDataFull:
	{
		if (m_s_parcours) {

			this->cadran(1, VUE_PRC_NB_LINES, 1, "Dist", _fmkstr(att.dist / 1000., 1U), "km");
			this->cadran(1, VUE_PRC_NB_LINES, 2, "Pwr", _imkstr(att.pwr), "W");

			this->cadran(2, VUE_PRC_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
			this->cadran(2, VUE_PRC_NB_LINES, 2, "Climb", _imkstr((int)att.climb), "m");

			this->cadran(3, VUE_PRC_NB_LINES, 1, "CAD", _imkstr(bsc_info.cadence), "rpm");
			this->cadran(3, VUE_PRC_NB_LINES, 2, "HRM", _imkstr(hrm_info.bpm), "bpm");

			this->cadran(4, VUE_PRC_NB_LINES, 1, "PR", _imkstr(att.pr), 0);
			this->cadran(4, VUE_PRC_NB_LINES, 2, "VA", _fmkstr(att.vit_asc * 3.600, 1U), "km/h");

			// display parcours
			this->afficheParcours(5, m_s_parcours->getListePoints());

			// display the segments
			for (int j=0; j < segMngr.getNbSegs() && j < NB_SEG_ON_DISPLAY - 1; j++) {

				ASSERT(segMngr.getSeg(j)->p_seg);

				this->afficheSegment(5, segMngr.getSeg(j)->p_seg);
			}

		} else {
			// let the user select one parcours
			m_prc_screen_mode = eVuePRCScreenInit;
		}

		this->cadran(7, VUE_PRC_NB_LINES, 1, "Avg", _imkstr((int)stc.getAverageCurrent()), "mA");
		this->cadran(7, VUE_PRC_NB_LINES, 2, "SOC", _imkstr(percentageBatt(stc.getVoltage(), stc.getCurrent())), "%");

		break;
	}

	}

	return m_prc_screen_mode;
}

/**
 *
 * @param event
 * @return
 */
bool VuePRC::propagateEventsPRC(eButtonsEvent event) {

	bool pass_event_menu = false;

	switch (event) {
		case eButtonsEventLeft:
		{
			if (m_s_parcours) this->decreaseZoom();

			if (!m_selec_en) pass_event_menu = true;
			else             m_parcours_sel += mes_parcours.size() - 1;
			break;
		}
		case eButtonsEventRight:
		{
			if (m_s_parcours) this->increaseZoom();

			if (!m_selec_en) pass_event_menu = true;
			else             m_parcours_sel++;
			break;
		}
		case eButtonsEventCenter:
		{
			if (m_selec_en && !m_s_parcours) {
				m_start_loading = true;
			} else if (!m_selec_en) {
				pass_event_menu = true;
			}

			break;
		}
		default:
		{
			pass_event_menu = true;
			break;
		}
	}

	return pass_event_menu;
}

/**
 *
 */
void VuePRC::parcoursSelector(void) {

	this->setTextSize(2);
	this->setCursor(0, 20);

	if (!mes_parcours.size()) {

		LOG_ERROR("No PRC in memory\r\n");
		this->println(" No PRC in memory");

		return;
	}

	m_parcours_sel = m_parcours_sel % mes_parcours.size();

	uint8_t i=0;
	for (auto& prc : mes_parcours._parcs) {

		this->print(" ");
		this->print(prc.getName());

		if (i==m_parcours_sel) {
			this->println(" <<--");
		} else {
			this->println();
		}

		i++;
	}

	if (m_selec_en && m_start_loading) {

		m_start_loading = false;

		LOG_INFO("Parcours %u chosen\r\n", m_parcours_sel);

		m_s_parcours = mes_parcours.getParcoursAt(m_parcours_sel);

		ASSERT(m_s_parcours);

		if (load_parcours(m_s_parcours[0]) > 0) {
			m_prc_screen_mode = eVuePRCScreenGps;

			m_selec_en = false;

			vue.addNotif("PRC: ", "Success !", 4, eNotificationTypeComplete);
		} else {
			vue.addNotif("PRC: ", "Loading failed", 4, eNotificationTypeComplete);
		}

	}
}

/**
 *
 * @param ligne
 * @param p_liste
 */
void VuePRC::afficheParcours(uint8_t ligne, ListePoints2D *p_liste) {

	float minLat = 100.;
	float minLon = 400.;
	float maxLat = -100.;
	float maxLon = -400.;
	uint16_t points_nb = 0;
	Point2D pCourant, pSuivant;

	W_SYSVIEW_OnTaskStartExec(DISPLAY_TASK3);

	uint16_t debut_cadran = _height / VUE_PRC_NB_LINES * (ligne - 1);
	uint16_t fin_cadran   = _height / VUE_PRC_NB_LINES * (ligne + 1);

	// print delimiters
	if (ligne > 1)                drawFastHLine(0, debut_cadran, _width, LS027_PIXEL_BLACK);
	if (ligne < VUE_PRC_NB_LINES) drawFastHLine(0, fin_cadran,   _width, LS027_PIXEL_BLACK);

	ASSERT(p_liste);

	LOG_DEBUG("Printing PRC\r\n");

	// init zoom
	this->setSpan(_width, fin_cadran - debut_cadran);

	float dZoom_h;
	float dZoom_v;

	LOG_INFO("PRC size %d\r\n", p_liste->size());

	m_distance_prc = p_liste->dist(att.loc.lat, att.loc.lon);

	this->computeZoom(att.loc.lat, m_distance_prc, dZoom_h, dZoom_v);

	// our pos is at the center
	maxLat = minLat = att.loc.lat;
	maxLon = minLon = att.loc.lon;

	// zoom level
	minLat -= dZoom_v;
	minLon -= dZoom_h;
	maxLat += dZoom_v;
	maxLon += dZoom_h;

	W_SYSVIEW_OnTaskStopExec(DISPLAY_TASK3);
	W_SYSVIEW_OnTaskStartExec(DISPLAY_TASK4);

	// on affiche
	points_nb = 0;
	uint16_t printed_nb = 0;
	for (auto& pPt : *p_liste->getLPTS()) {

		pSuivant = pPt;

		// print only points in the current zoom
		if (points_nb &&
				(((pCourant._lon > minLon && pCourant._lon < maxLon) &&
				(pCourant._lat > minLat && pCourant._lat < maxLat)) ||
				((pSuivant._lon > minLon && pSuivant._lon < maxLon) &&
				(pSuivant._lat > minLat && pSuivant._lat < maxLat)))) {

			if (!pSuivant.isValid() || !pCourant.isValid()) break;

			drawLine(regFenLim(pCourant._lon, minLon, maxLon, 0, _width),
					regFenLim(pCourant._lat, minLat, maxLat, fin_cadran, debut_cadran),
					regFenLim(pSuivant._lon, minLon, maxLon, 0, _width),
					regFenLim(pSuivant._lat, minLat, maxLat, fin_cadran, debut_cadran), LS027_PIXEL_BLACK);

			printed_nb++;
		}

		pCourant = pPt;
		points_nb++;
	}

	LOG_INFO("VuePRC %u / %u points printed\r\n", printed_nb, points_nb);

	// ma position
	if (att.loc.course > 0) {
		int16_t x0f, x0 = _width/2;
		int16_t y0f, y0 = (debut_cadran+fin_cadran)/2 - 15;

		int16_t x1f, x1 = _width/2 + 5;
		int16_t y1f, y1 = (debut_cadran+fin_cadran)/2 + 5;

		int16_t x2f, x2 = _width/2 - 5;
		int16_t y2f, y2 = (debut_cadran+fin_cadran)/2 + 5;

		rotate_point(att.loc.course, _width/2, (debut_cadran+fin_cadran)/2, x0, y0, x0f, y0f);
		rotate_point(att.loc.course, _width/2, (debut_cadran+fin_cadran)/2, x1, y1, x1f, y1f);
		rotate_point(att.loc.course, _width/2, (debut_cadran+fin_cadran)/2, x2, y2, x2f, y2f);

		drawTriangle(x0f, y0f, x1f, y1f, x2f, y2f, LS027_PIXEL_BLACK);
	} else {
		fillCircle(_width/2, (debut_cadran+fin_cadran)/2, 4, LS027_PIXEL_BLACK);
	}

	// print the zoom level in m
	setTextSize(1);
	setCursor(_width - 40, debut_cadran + 10);
	print((int)this->getLastZoom());
	print("m");

	W_SYSVIEW_OnTaskStopExec(DISPLAY_TASK4);
}

/**
 *
 * @param ligne
 * @param p_seg
 */
void VuePRC::afficheSegment(uint8_t ligne, Segment *p_seg) {

	float minLat = 100.;
	float minLon = 400.;
	float maxLat = -100.;
	float maxLon = -400.;
	float maDpex = 0;
	float maDpey = 0;
	uint16_t points_nb = 0;
	ListePoints *liste = nullptr;
	Point pCourant, pSuivant;
	Point *maPos = nullptr;

	ASSERT(p_seg);

	if (p_seg->longueur() < 4) {
		LOG_ERROR("Segment %s not loaded properly\r\n", p_seg->getName());
		return;
	}

	W_SYSVIEW_OnTaskStartExec(DISPLAY_TASK3);

	uint16_t debut_cadran = _height / VUE_PRC_NB_LINES * (ligne - 1);
	uint16_t fin_cadran   = _height / VUE_PRC_NB_LINES * (ligne + 1);

	// print delimiters
	if (ligne > 1)                drawFastHLine(0, debut_cadran, _width, LS027_PIXEL_BLACK);
	if (ligne < VUE_PRC_NB_LINES) drawFastHLine(0, fin_cadran, _width, LS027_PIXEL_BLACK);

	// on cherche la taille de fenetre
	liste = p_seg->getListePoints();

	ASSERT(liste);

	LOG_DEBUG("Printing PRC\r\n");

	// init zoom
	this->setSpan(_width, fin_cadran - debut_cadran);

	float dZoom_h;
	float dZoom_v;

	this->computeZoom(att.loc.lat, m_distance_prc, dZoom_h, dZoom_v);

	// our pos is at the center
	maxLat = minLat = att.loc.lat;
	maxLon = minLon = att.loc.lon;

	// zoom level
	minLat -= dZoom_v;
	minLon -= dZoom_h;
	maxLat += dZoom_v;
	maxLon += dZoom_h;

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
	LOG_DEBUG("VuePRC %u points printed\r\n", points_nb);

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
//	maDpex = regFenLim(_lon, minLon, maxLon, 0, _width);
//	maDpey = regFenLim(_lat, minLat, maxLat, fin_cadran, debut_cadran);
//	fillCircle(maDpex, maDpey, 4, LS027_PIXEL_BLACK);

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
