/*
 * VueFEC.cpp
 *
 *  Created on: 12 dec. 2017
 *      Author: Vincent
 */

#include "Model.h"
#include "assert_wrapper.h"
#include "ant.h"
#include "fec.h"
#include <vue/VueFEC.h>
#include <vue/Screenutils.h>
#include "segger_wrapper.h"

#define VUE_FEC_NB_LINES            6

VueFEC::VueFEC() : Adafruit_GFX(0, 0) {

	m_fec_screen_mode = eVueFECScreenInit;
}

static tHistoValue _vue_fec_pw_rb_read(uint16_t ind_) {

	tHistoValue *p_ret_val = boucle_fec.m_pw_buffer.get(ind_);

	ASSERT(p_ret_val);

	tHistoValue ret_val = p_ret_val[0];

	return ret_val;
}

eVueFECScreenModes VueFEC::tasksFEC() {

	eVueFECScreenModes res = m_fec_screen_mode;

	if (m_fec_screen_mode == eVueFECScreenInit) {

		// Init with welcome text
		this->setCursor(10,50);
		this->setTextSize(3);
		this->print(String("Connecting"));

		LOG_INFO("VueFEC waiting for sensors");

		if (fec_info.el_time) {
			// FEC just became active
			m_fec_screen_mode = eVueFECScreenDataFull;
			return m_fec_screen_mode;
		}

		vue.addNotif("FEC", "Connecting...", 5, eNotificationTypeComplete);

	} else if (m_fec_screen_mode == eVueFECScreenDataFull) {

		LOG_INFO("VueFEC update full data");

		this->cadranH(1, VUE_FEC_NB_LINES, "Time", _secjmkstr(fec_info.el_time, ':'), NULL);

		this->cadran(2, VUE_FEC_NB_LINES, 1, "CAD", _imkstr(bsc_info.cadence), "rpm");
		this->cadran(2, VUE_FEC_NB_LINES, 2, "HRM", _imkstr(hrm_info.bpm), "bpm");

		this->cadran(3, VUE_FEC_NB_LINES, 1, "Score", _fmkstr(suffer_score.getScore(), 1U), NULL);
		this->cadranZones(3, VUE_FEC_NB_LINES, 2, "PZone", zPower);

		this->cadran(4, VUE_FEC_NB_LINES, 1, "Pwr", _imkstr(fec_info.power), "W");
		this->cadranRR(4, VUE_FEC_NB_LINES, 2, "RR", rrZones);

#if 0
		sVueHistoConfiguration h_config;
		h_config.cur_elem_nb = boucle_fec.m_pw_buffer.size();
		h_config.ref_value   = (tHistoValue)210;
		h_config.max_value   = (tHistoValue)500;
		h_config.nb_elem_tot = FEC_PW_BUFFER_NB_ELEM;
		h_config.p_f_read    = _vue_fec_pw_rb_read;

		this->HistoH(5, VUE_FEC_NB_LINES, h_config);
#else
		this->cadranPowerVector(5, VUE_FEC_NB_LINES, NULL, powerVector);
#endif
	}

#if 0
	this->cadran(7, VUE_FEC_NB_LINES, 1, "Cur", _imkstr((int)stc.getCurrent()), "mA");
	this->cadran(7, VUE_FEC_NB_LINES, 2, "SOC", _imkstr((int)percentageBatt(stc.getVoltage(), stc.getCurrent())), "%");
#endif

	return res;
}

// Unit is in newton/meter with a resolution of 1/32
#define SCALE_TORQUE(X, Y)         (((X) * 80) / (Y))     /* 4 LSB is one pixel */

void VueFEC::cadranPowerVector(uint8_t p_lig, uint8_t nb_lig, const char *champ, sPowerVector &vector) {

	// center coordinates
	const int16_t xc = _width / 2;
	const int16_t yc = _height / nb_lig * p_lig;

	// calculate max torque for scale
	int16_t max_torque = 100;
	for (int i=0; i < vector.array_size; i++) {

		if (vector.inst_torque_mag_array[i] > max_torque) {
			max_torque = vector.inst_torque_mag_array[i];
		}
	}

	// empty array
	if (!vector.array_size) {
		return;
	}

	// first point coordinates
	int16_t x1;
	int16_t y1;
	int16_t ppower = vector.inst_torque_mag_array[0];
	float first_angle = (float)vector.first_crank_angle;
	rotate_point(first_angle,
			xc, yc,
			xc, yc - SCALE_TORQUE(ppower, max_torque),
			x1, y1);

	int16_t xp, yp;
	int16_t x2=x1, y2=y1;
	for (int i=1; i < vector.array_size; i++) {

		int16_t ppower = vector.inst_torque_mag_array[i];
		int16_t y2 = yc - SCALE_TORQUE(ppower, max_torque);
		float angle = (first_angle + i * 360.f / vector.array_size);
		rotate_point(angle,
				xc, yc,
				xc, y2,
				xp, yp);

		this->drawLine(x1, y1, xp, yp, LS027_PIXEL_BLACK, 3);

		x1 = xp;
		y1 = yp;
	}

	// Print the data
	const int y = _height / nb_lig * (p_lig - 1);
	this->setCursor(0, y - 20 + (_height / (nb_lig*2)));
	this->setTextSize(2);
	this->println(max_torque);
	this->println(vector.first_crank_angle);

	// close the figure
	this->drawLine(xp, yp, x2, y2, LS027_PIXEL_BLACK, 3);

}

void VueFEC::cadranZones(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, BinnedData &data) {

	const int x = _width / 2 * p_col;
	const int y = _height / nb_lig * (p_lig - 1);

	if (champ) {
		setCursor(x + 5 - _width / 2, y + 8);
		setTextSize(1);
		print(champ);
	}

	// Print the data
	setCursor(x - 55 - 20, y - 10 + (_height / (nb_lig*2)));
	setTextSize(3);

	uint32_t tot = data.getTimeMax();
	uint32_t cur_zone = data.getCurBin();
	LOG_INFO("PZ time %u", tot);

	// loop over bins
	for (uint32_t i=0; i< data.getNbBins(); i++) {

		int16_t width = regFenLim((float)data.getTimeZX(i), 0.f, tot, 2.f, _width / 2.f - 35.f);
		this->fillRect(x - _width / 2 + 20, y + 20 + i*6, width, 4, 1);

		if (i == cur_zone) {
			setCursor(x - _width / 2 + 7, y + 15 + i*6);
			setTextSize(2);
			print((char)('>'));
		}
	}


	// print delimiters
	drawFastVLine(_width / 2, _height / nb_lig * (p_lig - 1), _height / nb_lig, 1);

	if (p_lig > 1)      drawFastHLine(_width * (p_col - 1) / 2, _height / nb_lig * (p_lig - 1), _width / 2, 1);
	if (p_lig < nb_lig) drawFastHLine(_width * (p_col - 1) / 2, _height / nb_lig * p_lig, _width / 2, 1);

}
