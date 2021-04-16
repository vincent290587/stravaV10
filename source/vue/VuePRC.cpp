/*
 * VuePRC.cpp
 *
 *  Created on: 12 d�c. 2017
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
}

eVuePRCScreenModes VuePRC::tasksPRC() {

	LOG_DEBUG("Last update age: %lu\r\n", locator.getLastUpdateAge());

	// switch back and forth to GPS information page
	if (eVuePRCScreenInit != m_prc_screen_mode &&
			locator.getLastUpdateAge() > LOCATOR_MAX_DATA_AGE_MS) {

		m_prc_screen_mode = eVuePRCScreenGps;
	} else {

		m_prc_screen_mode = eVuePRCScreenDataFull;
	}

	Parcours *p_parcours = boucle_crs.m_s_parcours;

	switch (m_prc_screen_mode) {
	case eVuePRCScreenInit:
	{
		this->displayLoading();
		// boucle has finished init
		if (!boucle_crs.needsInit()) {
			this->displayGPS();
			m_prc_screen_mode = eVuePRCScreenGps;
		}
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
		if (p_parcours) {

			LOG_INFO("Printing PRC\r\n");

			this->cadran(1, VUE_PRC_NB_LINES, 1, "Dist", _fmkstr(att.dist / 1000.f, 1U), "km");
			this->cadran(1, VUE_PRC_NB_LINES, 2, "Pwr", _imkstr(att.pwr), "W");

			this->cadran(2, VUE_PRC_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
			this->cadran(2, VUE_PRC_NB_LINES, 2, "Climb", _imkstr((int)att.climb), "m");

			this->cadran(3, VUE_PRC_NB_LINES, 1, "CAD", _imkstr(bsc_info.cadence), "rpm");
			this->cadran(3, VUE_PRC_NB_LINES, 2, "HRM", _imkstr(hrm_info.bpm), "bpm");

			this->cadran(4, VUE_PRC_NB_LINES, 1, "SL", _imkstr(att.slope), "%");
			this->cadran(4, VUE_PRC_NB_LINES, 2, "VA", _fmkstr(att.vit_asc, 2U), "m/s");

			// display parcours
			this->afficheParcours(5, p_parcours->getListePoints());

			// display the segments
			for (int j=0; j < segMngr.getNbSegs() && j < NB_SEG_ON_DISPLAY - 1; j++) {

				ASSERT(segMngr.getSeg(j)->p_seg);

				this->afficheSegment(5, segMngr.getSeg(j)->p_seg);
			}

		} else {
			LOG_INFO("No PRC in memory");
		}

		this->cadran(7, VUE_PRC_NB_LINES, 1, "Avg", _imkstr((int)stc.getAverageCurrent()), "mA");
		this->cadran(7, VUE_PRC_NB_LINES, 2, "SOC", _imkstr((int)percentageBatt(stc.getVoltage(), stc.getCurrent())), "%");

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

	switch (event) {
		case eButtonsEventLeft:
		{
			this->decreaseZoom();
			break;
		}

		case eButtonsEventRight:
		{
			this->increaseZoom();
			break;
		}

		case eButtonsEventCenter:
		default:
		{
			break;
		}
	}

	return true;
}

/**
 *
 * @param ligne
 * @param p_liste
 */
void VuePRC::afficheParcours(uint8_t ligne, ListePoints2D *p_liste) {

	uint16_t points_nb = 0;
	Point2D pCourant, pSuivant;

	sysview_task_void_enter(ComputeZoom);

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

	m_distance_prc = 0; // unused argument

	this->computeZoom(att.loc.lat, m_distance_prc, dZoom_h, dZoom_v);

	sysview_task_void_exit(ComputeZoom);
	sysview_task_void_enter(DisplayPoints);

	// on affiche
	points_nb = 0;
	Location center(att.loc.lat, att.loc.lon);
	for (auto& pPt : *p_liste->getLPTS()) {

		pSuivant = pPt;

		// print only points in the current zoom
		if (points_nb) {

			if (!pSuivant.isValid() || !pCourant.isValid()) break;

			// handle very distant points with line clipped in window
			PixelLine line_rep;
			int res = this->intersects(center, _width, fin_cadran - debut_cadran, pCourant, pSuivant, line_rep);

			if (res > 0) {

				// shift from center of rectangle to screen coordinates
				line_rep.shift(_width/2, debut_cadran + _height / VUE_PRC_NB_LINES);

				int16_t thickness = 1;
				if (points_nb >= p_liste->idx_P1 &&
						points_nb < p_liste->idx_P1 + 15) {
					// show which path to take
					thickness = 2;
				}

				// draw the line
				drawLine(line_rep.x0, line_rep.y0,
						line_rep.x1, line_rep.y1,
						LS027_PIXEL_BLACK, thickness);
			}
		}

		pCourant = pPt;
		points_nb++;
	}

	sysview_task_void_exit(DisplayPoints);
	sysview_task_void_enter(DisplayMyself);

	LOG_INFO("VuePRC %u points in total\r\n", points_nb);

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

	sysview_task_void_exit(DisplayMyself);
}

/**
 *
 * @param ligne
 * @param p_seg
 */
void VuePRC::afficheSegment(uint8_t ligne, Segment *p_seg) {

	uint16_t points_nb = 0;
	ListePoints *liste = nullptr;
	Point pCourant, pSuivant;
	Point *maPos = nullptr;

	ASSERT(p_seg);

	if (p_seg->longueur() < 4) {
		LOG_ERROR("Segment %s not loaded properly\r\n", p_seg->getName());
		return;
	}

	sysview_task_void_enter(ComputeZoom);

	uint16_t debut_cadran = _height / VUE_PRC_NB_LINES * (ligne - 1);
	uint16_t fin_cadran   = _height / VUE_PRC_NB_LINES * (ligne + 1);

	// print delimiters
	if (ligne > 1)                drawFastHLine(0, debut_cadran, _width, LS027_PIXEL_BLACK);
	if (ligne < VUE_PRC_NB_LINES) drawFastHLine(0, fin_cadran, _width, LS027_PIXEL_BLACK);

	// on cherche la taille de fenetre
	liste = p_seg->getListePoints();

	ASSERT(liste);

	LOG_DEBUG("Printing PRC\r\n");

	sysview_task_void_exit(ComputeZoom);
	sysview_task_void_enter(DisplayPoints);

	// on affiche
	points_nb = 0;
	uint16_t pourc = 0;
	bool pourc_found = 0;
	Location center(att.loc.lat, att.loc.lon);
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

		// print only points in the current zoom
		if (points_nb) {

			if (!pSuivant.isValid() || !pCourant.isValid()) break;

			// handle very distant points with line clipped in window
			PixelLine line_rep;
			int res = this->intersects(center, _width, fin_cadran - debut_cadran, pCourant, pSuivant, line_rep);

			if (res > 0) {

				// shift from center of rectangle to screen coordinates
				line_rep.shift(_width/2, debut_cadran + _height / VUE_PRC_NB_LINES);

				// draw the line
				drawLine(line_rep.x0, line_rep.y0,
						line_rep.x1, line_rep.y1,
						LS027_PIXEL_BLACK);
			}
		}

		pCourant = pPt;
		points_nb++;
	}

	PixelPoint point;
	if (p_seg->getStatus() < SEG_OFF) {

		// draw a rectangle at the end of the segment (done)
		if (this->includes(center, pSuivant, _width, fin_cadran - debut_cadran, point)) {

			// shift from center of rectangle to screen coordinates
			point.shift(_width/2, debut_cadran + _height / VUE_PRC_NB_LINES);

			drawRect(point.x - 5, point.y - 5, 10, 10, LS027_PIXEL_BLACK);
		}
	} else if (p_seg->getStatus() != SEG_OFF) {

		// draw a flag at the end of the segment (not done yet)
		if (this->includes(center, pSuivant, _width, fin_cadran - debut_cadran, point)) {

			// shift from center of rectangle to screen coordinates
			point.shift(_width/2, debut_cadran + _height / VUE_PRC_NB_LINES);

			drawRect(point.x - 5, point.y - 5, 10, 10, LS027_PIXEL_BLACK);

			fillRect(point.x - 5, point.y - 5, 5, 5, LS027_PIXEL_BLACK);

			fillRect(point.x, point.y, 5, 5, LS027_PIXEL_BLACK);
		}

	} else {

		// draw a circle at the start of the segment
		maPos = liste->getFirstPoint();
		if (this->includes(center, *maPos, _width, fin_cadran - debut_cadran, point)) {

			// shift from center of rectangle to screen coordinates
			point.shift(_width/2, debut_cadran + _height / VUE_PRC_NB_LINES);

			drawCircle(point.x, point.y, 5.f, LS027_PIXEL_BLACK);
		}
	}

	// return before printing text
	if (p_seg->getStatus() == SEG_OFF) {
		return;
	}

	setTextSize(2);
	setCursor(_width / 2 + 10, 0.5f * (debut_cadran + fin_cadran) + 10.f);

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

	sysview_task_void_exit(DisplayPoints);
}

void VuePRC::displayLoading() {

	static uint8_t nb_dots = 0;

	vue.setCursor(0, 20);
	vue.setTextSize(3);

	vue.print("Loading");
	for (int i=0; i < nb_dots; i++) vue.print(".");
	vue.println(".");

	nb_dots++;
	nb_dots = nb_dots % 10;
}
