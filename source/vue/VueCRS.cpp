/*
 * VueCRS.cpp
 *
 *  Created on: 12 déc. 2017
 *      Author: Vincent
 */

#include "assert_wrapper.h"
#include "ant.h"
#include "Model.h"
#include "Screenutils.h"
#include "segger_wrapper.h"
#include <vue/VueCRS.h>
#include "komoot_nav.h"

#define VUE_CRS_NB_LINES         7


VueCRS::VueCRS() : Adafruit_GFX(0, 0) {
	m_crs_screen_mode = eVueCRSScreenInit;
	m_screen_page = eVueCRSScreenPage1;
}

eVueCRSScreenModes VueCRS::tasksCRS() {

	LOG_DEBUG("Last update age: %lu", locator.getLastUpdateAge());

	if (locator.getLastUpdateAge() > LOCATOR_MAX_DATA_AGE_MS) {

		m_crs_screen_mode =  eVueCRSScreenInit;
	} else {

		m_crs_screen_mode =  eVueCRSScreenDataFull;
	}

	switch (m_crs_screen_mode) {
	case eVueCRSScreenInit:
	{
		// display GPS page
		vue.displayGPS();
	} break;

	default:
		if (m_screen_page == eVueCRSScreenPage1) {
			this->afficheScreen1();
		} else if (m_screen_page == eVueCRSScreenPage2) {
			this->afficheScreen2();
		} else {
			this->afficheSensors();
		}
		break;

	}

	return m_crs_screen_mode;
}



/**
 *
 * @param event
 * @return
 */
bool VueCRS::propagateEventsCRS(eButtonsEvent event) {

	switch (event) {
		case eButtonsEventLeft:
		{
			if (m_screen_page==eVueCRSScreenPage1) m_screen_page = eVueCRSScreenPage3;
			else if (m_screen_page==eVueCRSScreenPage2) m_screen_page = eVueCRSScreenPage1;
			else m_screen_page = eVueCRSScreenPage2;
			break;
		}

		case eButtonsEventRight:
		{
			if (m_screen_page==eVueCRSScreenPage1) m_screen_page = eVueCRSScreenPage2;
			else if (m_screen_page==eVueCRSScreenPage2) m_screen_page = eVueCRSScreenPage3;
			else m_screen_page = eVueCRSScreenPage1;
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


void VueCRS::afficheScreen1(void) {

	if (m_crs_screen_mode != eVueCRSScreenInit) {
		switch (segMngr.getNbSegs()) {
		case 0:
			m_crs_screen_mode = eVueCRSScreenDataFull;
			break;
		case 1:
			m_crs_screen_mode = eVueCRSScreenDataSS;
			break;
		case 2:
			// no break
		default:
			m_crs_screen_mode = eVueCRSScreenDataDS;
			break;
		}
	}

	LOG_INFO("Displaying %u segments", segMngr.getNbSegs());

	switch (m_crs_screen_mode) {
	case eVueCRSScreenDataFull:
	{
		this->cadran(1, VUE_CRS_NB_LINES, 1, "Dist", _fmkstr(att.dist / 1000.f, 1U), "km");
		this->cadran(1, VUE_CRS_NB_LINES, 2, "Pwr", _imkstr(att.pwr), "W");

		this->cadran(2, VUE_CRS_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
		this->cadran(2, VUE_CRS_NB_LINES, 2, "Climb", _imkstr((int)att.climb), "m");

		this->cadran(3, VUE_CRS_NB_LINES, 1, "CAD", _imkstr(bsc_info.cadence), "rpm");
		this->cadran(3, VUE_CRS_NB_LINES, 2, "HRM", _imkstr(hrm_info.bpm), "bpm");

		this->cadran(4, VUE_CRS_NB_LINES, 1, "SL", _imkstr(att.slope), "%");
		this->cadran(4, VUE_CRS_NB_LINES, 2, "VA", _fmkstr(att.vit_asc, 2U), "m/s");

		this->cadran(5, VUE_CRS_NB_LINES, 1, "Next", _imkstr(att.next), "m");
		this->cadran(5, VUE_CRS_NB_LINES, 2, "Alt" , _fmkstr(att.loc.alt, 1U), "m");

		float avg_speed = 0.;
		if (att.nbsec_act) {
			avg_speed = att.dist * 3.6f / att.nbsec_act;
		}
		this->cadran(6, VUE_CRS_NB_LINES, 1, "Avg"  , _fmkstr(avg_speed, 2U), "km/h");
		this->cadran(6, VUE_CRS_NB_LINES, 2, "Score", _fmkstr(suffer_score.getScore(), 1U), NULL);

		this->cadran(7, VUE_CRS_NB_LINES, 1, "STC", _imkstr((int)stc.getCurrent()), "mA");
		this->cadran(7, VUE_CRS_NB_LINES, 2, "SOC", _imkstr((int)percentageBatt(stc.getVoltage(), stc.getCurrent())), "%");

	}  break;

	case eVueCRSScreenDataSS:
	{
		this->cadran(1, VUE_CRS_NB_LINES, 1, "Dist", _fmkstr(att.dist / 1000, 1U), "km");
		this->cadran(1, VUE_CRS_NB_LINES, 2, "Pwr", _imkstr(att.pwr), "W");

		this->cadran(2, VUE_CRS_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
		this->cadran(2, VUE_CRS_NB_LINES, 2, "Climb", _fmkstr(att.climb, 1U), "m");

		this->cadran(3, VUE_CRS_NB_LINES, 1, "CAD", _imkstr(bsc_info.cadence), "rpm");
		this->cadran(3, VUE_CRS_NB_LINES, 2, "HRM", _imkstr(hrm_info.bpm), "bpm");

		this->cadran(4, VUE_CRS_NB_LINES, 1, "SL", _imkstr(att.slope), "%");
		this->cadran(4, VUE_CRS_NB_LINES, 2, "VA", _fmkstr(att.vit_asc, 2U), "m/s");

		ASSERT(segMngr.getSeg(0));

		this->afficheSegment(5, segMngr.getSeg(0)->p_seg);

		if (SEG_OFF == segMngr.getSeg(0)->p_seg->getStatus()) {

			this->cadranH(7, VUE_CRS_NB_LINES, "Next", _imkstr(att.next), "m");

		} else {
			this->partner(7, segMngr.getSeg(0)->p_seg);
		}

	}  break;

	case eVueCRSScreenDataDS:
	{
		this->cadran(1, VUE_CRS_NB_LINES, 1, "VA", _fmkstr(att.vit_asc, 2U), "m/s");
		this->cadran(1, VUE_CRS_NB_LINES, 2, "HRM", _imkstr(hrm_info.bpm), "bpm");

		ASSERT(segMngr.getSeg(0));
		ASSERT(segMngr.getSeg(1));

		if (SEG_OFF == segMngr.getSeg(0)->p_seg->getStatus() &&
				SEG_OFF == segMngr.getSeg(1)->p_seg->getStatus()) {

			// all segments are OFF
			this->cadran(2, VUE_CRS_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
			this->cadran(2, VUE_CRS_NB_LINES, 2, "Pwr"  , _imkstr(att.pwr), "W");

			this->cadran(3, VUE_CRS_NB_LINES, 1, "CAD"  , _imkstr(bsc_info.cadence), "rpm");
			this->cadran(3, VUE_CRS_NB_LINES, 2, "Climb", _fmkstr(att.climb, 1U), "m");

			this->cadran(4, VUE_CRS_NB_LINES, 1, "Dist", _fmkstr(att.dist / 1000.f, 1U), "km");
			this->cadran(4, VUE_CRS_NB_LINES, 2, "SL"  , _imkstr(att.slope), "%");

			this->afficheSegment(VUE_CRS_NB_LINES - 2, segMngr.getSeg(0)->p_seg);
			this->afficheSegment(VUE_CRS_NB_LINES - 2, segMngr.getSeg(1)->p_seg);

			this->cadranH(VUE_CRS_NB_LINES, VUE_CRS_NB_LINES, "Next", _imkstr(att.next), "m");

		} else if (SEG_OFF == segMngr.getSeg(0)->p_seg->getStatus()) {

			// only one segment is OFF
			this->cadran(2, VUE_CRS_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
			this->cadran(2, VUE_CRS_NB_LINES, 2, "Pwr", _imkstr(att.pwr), "W");

			this->afficheSegment(VUE_CRS_NB_LINES - 4, segMngr.getSeg(0)->p_seg);

			this->afficheSegment(VUE_CRS_NB_LINES - 2, segMngr.getSeg(1)->p_seg);
			this->partner(VUE_CRS_NB_LINES, segMngr.getSeg(1)->p_seg);

		} else if (SEG_OFF == segMngr.getSeg(1)->p_seg->getStatus()) {

			// only one segment is OFF
			this->cadran(2, VUE_CRS_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
			this->cadran(2, VUE_CRS_NB_LINES, 2, "Pwr", _imkstr(att.pwr), "W");

			this->afficheSegment(VUE_CRS_NB_LINES - 4, segMngr.getSeg(0)->p_seg);
			this->partner(VUE_CRS_NB_LINES - 2, segMngr.getSeg(0)->p_seg);

			this->afficheSegment(VUE_CRS_NB_LINES - 1, segMngr.getSeg(1)->p_seg);

		} else {

			// no segment is OFF
			this->afficheSegment(VUE_CRS_NB_LINES - 5, segMngr.getSeg(0)->p_seg);
			this->partner(VUE_CRS_NB_LINES - 3, segMngr.getSeg(0)->p_seg);

			this->afficheSegment(VUE_CRS_NB_LINES - 2, segMngr.getSeg(1)->p_seg);
			this->partner(VUE_CRS_NB_LINES, segMngr.getSeg(1)->p_seg);
		}

	}  break;

	default:
		break;
	}

}

void VueCRS::afficheScreen2(void) {

	this->cadran(1, VUE_CRS_NB_LINES, 1, "Dist", _fmkstr(att.dist / 1000.f, 1U), "km");
	this->cadran(1, VUE_CRS_NB_LINES, 2, "Pwr", _imkstr(att.pwr), "W");

	this->cadran(2, VUE_CRS_NB_LINES, 1, "Speed", _fmkstr(att.loc.speed, 1U), "km/h");
	this->cadran(2, VUE_CRS_NB_LINES, 2, "Climb", _imkstr((int)att.climb), "m");

	this->cadran(3, VUE_CRS_NB_LINES, 1, "CAD", _imkstr(bsc_info.cadence), "rpm");
	this->cadran(3, VUE_CRS_NB_LINES, 2, "HRM", _imkstr(hrm_info.bpm), "bpm");

	this->cadran(4, VUE_CRS_NB_LINES, 1, "PR", _imkstr(att.pr), 0);
	this->cadran(4, VUE_CRS_NB_LINES, 2, "VA", _fmkstr(att.vit_asc, 2U), "m/s");

	static sKomootNavigation navi;
	model_get_navigation(&navi);

	this->cadran(5, VUE_CRS_NB_LINES, 1, "Next turn", _imkstr(navi.distance), "m");
	this->cadranRR(5, VUE_CRS_NB_LINES, 2, "RR", rrZones);

	const uint8_t* bitmap = komoot_nav_get_icon(navi.direction);
	if (bitmap) {
		this->drawBitmap(_width / 2 - KOMOOT_ICON_SIZE_W / 2, 289,
				bitmap,
				KOMOOT_ICON_SIZE_W, KOMOOT_ICON_SIZE_H, 0, 1);
	}

}

void VueCRS::afficheSensors(void) {

	// get values from FXOS
	float yaw_rad;
	float pitch_rad;
	(void)fxos_get_yaw(yaw_rad);
	(void)fxos_get_pitch(pitch_rad);

	// pitch
	const uint16_t bar_len = 140;
	float val = regFenLim(pitch_rad, -0.24, 0.24, -bar_len/2, bar_len/2);
	if (val > 0.) {
		this->fillRect(this->width()/2, 53, val, 8, LS027_PIXEL_BLACK);
	} else {
		this->fillRect(this->width()/2 + val, 53, -val, 8, LS027_PIXEL_BLACK);
	}
	this->drawRect(this->width()/2-bar_len/2, 50, bar_len, 14, LS027_PIXEL_BLACK);

	this->setTextSize(2);
	this->setCursor(this->width()/2-bar_len/2, 35);
	this->print("Pitch");

	// pitch #2
	sVueHistoConfiguration h_config;
	h_config.cur_elem_nb = fxos_histo_size();
	h_config.ref_value   = (tHistoValue)40;
	h_config.max_value   = (tHistoValue)80;
	h_config.nb_elem_tot = PITCH_BUFFER_SIZE;
	h_config.p_f_read    = fxos_histo_read;

	this->HistoH(3, 6, h_config);

	// mag heading
	const uint16_t radius = 50;
	this->drawCircle(this->width()/2, 300, radius, LS027_PIXEL_BLACK);

	int16_t x2, y2;
	rotate_point(yaw_rad * 180. / 3.1415, this->width()/2, 300,
			this->width()/2, 300 - radius, x2, y2);
	this->drawLine(this->width()/2, 300, x2, y2, LS027_PIXEL_BLACK, 3);

	// Text info
	float roughness[4];
	fxos_get_roughness(roughness);
	roughness[3] = baro.getRoughness();

	this->setTextSize(2);
	for (int i=0; i < 4; i++) {
		this->setCursor(10, 300 + 15 * i);
		this->print(_fmkstr(roughness[i], 1));
	}
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

	ASSERT(p_seg);

	if (p_seg->longueur() < 4) {
		LOG_ERROR("Segment %s not loaded properly\r\n", p_seg->getName());
		return;
	}

	sysview_task_void_enter(ComputeZoom);

	uint16_t debut_cadran = _height / VUE_CRS_NB_LINES * (ligne - 1);
	uint16_t fin_cadran   = _height / VUE_CRS_NB_LINES * (ligne + 1);

	// print delimiters
	if (ligne > 1) drawFastHLine(0, debut_cadran, _width, LS027_PIXEL_BLACK);
	if (ligne < VUE_CRS_NB_LINES) drawFastHLine(0, fin_cadran, _width, LS027_PIXEL_BLACK);

	// on cherche la taille de fenetre
	liste = p_seg->getListePoints();

	ASSERT(liste);

	// compute convertion between deg and meters
	float deglon_to_m = 1000.f * distance_between(att.loc.lat, att.loc.lon, att.loc.lat, att.loc.lon + 0.001f);
	float deglat_to_m = 1000.f * distance_between(att.loc.lat, att.loc.lon, att.loc.lat + 0.001f, att.loc.lon);

	if (p_seg->getStatus() == SEG_OFF) {

		// our pos is at the center
		maxLat = minLat = att.loc.lat;
		maxLon = minLon = att.loc.lon;
		maxAlt = minAlt = att.loc.alt;

		LOG_DEBUG("Printing SEG OFF\r\n");

		// zoom level
		minLat -= 300.f / deglat_to_m;
		minLon -= 300.f / deglon_to_m;
		minAlt -= 15.f;
		maxLat += 300.f / deglat_to_m;
		maxLon += 300.f / deglon_to_m;
		maxAlt += 15.f;

	} else {

		Vecteur& delta = liste->getDeltaListe();
		Point2D& centre = liste->getCenterListe();

		//LOG_DEBUG("VueCRS centre lon: %ld\r\n", (int)((centre._lon - att.lon) * deglon_to_m));
		//LOG_DEBUG("VueCRS centre lat: %ld\r\n", (int)((centre._lat - att.lat) * deglat_to_m));

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
		minAlt -= 15.f;
		maxLat += 100 / deglat_to_m;
		maxLon += 100 / deglon_to_m;
		maxAlt += 15.f;

		LOG_DEBUG("Printing SEG status=%d\r\n", p_seg->getStatus());

		LOG_DEBUG("VueCRS delta2 lon: %ld\r\n", (int)((maxLon - minLon) * deglon_to_m));
		LOG_DEBUG("VueCRS delta2 lat: %ld\r\n", (int)((maxLat - minLat) * deglat_to_m));

	}

	sysview_task_void_exit(ComputeZoom);
	sysview_task_void_enter(DisplayPoints);

	// on affiche
	points_nb = 0;
	uint16_t pourc = 0;
	bool pourc_found = 0;
	for (auto& pPt : *liste->getLPTS()) {

		pSuivant = pPt;

		if (!pourc_found) {
			if (pPt._rtime == liste->m_P1._rtime) {
				pourc_found = true;
				pourc = (100 * points_nb) / liste->size();
			}
		}

		if (points_nb &&
				(((pCourant._lon > minLon && pCourant._lon < maxLon) &&
						(pCourant._lat > minLat && pCourant._lat < maxLat)) ||
						((pSuivant._lon > minLon && pSuivant._lon < maxLon) &&
								(pSuivant._lat > minLat && pSuivant._lat < maxLat)))) {

			if (!pSuivant.isValid() || !pCourant.isValid()) break;

			drawLine(regFenLim(pCourant._lon, minLon, maxLon, 0.f, _width),
					regFenLim(pCourant._lat, minLat, maxLat, fin_cadran, debut_cadran),
					regFenLim(pSuivant._lon, minLon, maxLon, 0.f, _width),
					regFenLim(pSuivant._lat, minLat, maxLat, fin_cadran, debut_cadran), LS027_PIXEL_BLACK);
		}

		pCourant = pSuivant;
		points_nb++;
	}
	LOG_DEBUG("VueCRS %u points printed\r\n", points_nb);

	if (p_seg->getStatus() < SEG_OFF) {

		// draw a rect at the end of the segment (done)
		drawRect(regFenLim(pSuivant._lon, minLon, maxLon, 0.f, _width) - 5,
				regFenLim(pSuivant._lat, minLat, maxLat, fin_cadran, debut_cadran) - 5, 10, 10, LS027_PIXEL_BLACK);

	} else if (p_seg->getStatus() != SEG_OFF) {

		drawRect(regFenLim(pSuivant._lon, minLon, maxLon, 0.f, _width) - 5,
				regFenLim(pSuivant._lat, minLat, maxLat, fin_cadran, debut_cadran) - 5, 10, 10, LS027_PIXEL_BLACK);
		fillRect(regFenLim(pSuivant._lon, minLon, maxLon, 0.f, _width) - 5,
				regFenLim(pSuivant._lat, minLat, maxLat, fin_cadran, debut_cadran) - 5, 5, 5, LS027_PIXEL_BLACK);
		fillRect(regFenLim(pSuivant._lon, minLon, maxLon, 0.f, _width),
				regFenLim(pSuivant._lat, minLat, maxLat, fin_cadran, debut_cadran), 5, 5, LS027_PIXEL_BLACK);

	} else {

		// draw a circle at the start of the segment
		maPos = liste->getFirstPoint();
		drawCircle(regFenLim(maPos->_lon, minLon, maxLon, 0.f, _width),
				regFenLim(maPos->_lat, minLat, maxLat, fin_cadran, debut_cadran), 5.f, LS027_PIXEL_BLACK);
	}

	float _lat, _lon;
	if (p_seg->getStatus() < SEG_OFF) {
		// limit position when segment is finished
		_lon = pCourant._lon;
		_lat = pCourant._lat;
	} else {
		_lon = att.loc.lon;
		_lat = att.loc.lat;
	}

	sysview_task_void_exit(DisplayPoints);
	sysview_task_void_enter(DisplayMyself);

	// ma position
	maDpex = regFenLim(_lon, minLon, maxLon, 0, _width);
	maDpey = regFenLim(_lat, minLat, maxLat, fin_cadran, debut_cadran);
	if (att.loc.course > 0) {
		int16_t x0f, x0 = (int16_t)maDpex;
		int16_t y0f, y0 = (int16_t)maDpey - 15;

		int16_t x1f, x1 = (int16_t)maDpex + 5;
		int16_t y1f, y1 = (int16_t)maDpey + 5;

		int16_t x2f, x2 = (int16_t)maDpex - 5;
		int16_t y2f, y2 = (int16_t)maDpey + 5;

		rotate_point(att.loc.course, (int16_t)maDpex, (int16_t)maDpey, x0, y0, x0f, y0f);
		rotate_point(att.loc.course, (int16_t)maDpex, (int16_t)maDpey, x1, y1, x1f, y1f);
		rotate_point(att.loc.course, (int16_t)maDpex, (int16_t)maDpey, x2, y2, x2f, y2f);

		drawTriangle(x0f, y0f, x1f, y1f, x2f, y2f, LS027_PIXEL_BLACK);
	} else {
		fillCircle((int16_t)maDpex, (int16_t)maDpey, 4, LS027_PIXEL_BLACK);
	}

	// return before printing text
	if (p_seg->getStatus() == SEG_OFF) {
		return;
	}

	if (maDpey > fin_cadran - 30) {
		setCursor(maDpex > _width - 70 ? _width - 70 : (int16_t)maDpex, (int16_t)maDpey - 20);
	} else {
		setCursor(maDpex > _width - 70 ? _width - 70 : (int16_t)maDpex, (int16_t)maDpey + 15);
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

	sysview_task_void_exit(DisplayMyself);
}


/**
 * Indicateur du pourcentage de perf en forme d'avion
 */
void VueCRS::partner(uint8_t ligne, Segment *p_seg) {

	int hl, ol, dixP;
	float indice;
	static int centre = _width / 2;
	float rtime, curtime;

	ASSERT(p_seg);

	rtime   = p_seg->getAvance();
	curtime = p_seg->getCur();

	if (curtime < 5.) {
		indice = rtime / 5.f;
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
	centre = regFenLim(indice, -0.25f, 0.25f, ol, _width - ol);
	if (centre < ol) {
		centre = ol;
	} else if (centre > (int)_width - ol) {
		centre = _width - ol;
	}

	dixP = (_width - 2 * ol) * 10 / 50;
	drawFastVLine(_width / 2 - dixP, hl + 2, 7, LS027_PIXEL_BLACK);
	drawFastVLine(_width / 2 + dixP, hl + 2, 7, LS027_PIXEL_BLACK);

	fillTriangle(centre - 7, hl + 7, centre, hl - 7, centre + 7, hl + 7, LS027_PIXEL_BLACK);
	setCursor(centre - 15, hl + 12);
	setTextSize(VUE_CRS_NB_LINES > 7 ? 1 : 2);
	print(_imkstr((int)(indice * 100.f)));

	// marques
	drawFastVLine(_width / 2, hl - 12, 12, LS027_PIXEL_BLACK);

}
