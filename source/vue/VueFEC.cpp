/*
 * VueFEC.cpp
 *
 *  Created on: 12 déc. 2017
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
	m_el_time = 0;
	m_fec_screen_mode = eVueFECScreenInit;
}

static tHistoValue _vue_fec_pw_rb_read(uint16_t ind_) {

	tHistoValue *p_ret_val = boucle_fec.m_pw_buffer.get(ind_);

	ASSERT(p_ret_val);

	tHistoValue ret_val = p_ret_val[0];

	return ret_val;
}

eVueFECScreenModes VueFEC::tasksFEC() {

	static uint8_t el_time_prev = 0;

	eVueFECScreenModes res = m_fec_screen_mode;

	if (m_fec_screen_mode == eVueFECScreenInit) {

		// Init with welcome text
		this->setCursor(10,50);
		this->setTextSize(3);
		this->print(String("Connecting"));

		LOG_INFO("VueFEC waiting for sensors");

		if (!m_el_time) vue.addNotif("FEC", "Connecting...", 5, eNotificationTypeComplete);

		m_el_time = 1;

		if (fec_info.el_time) {
			// FEC just became active
			m_fec_screen_mode = eVueFECScreenDataFull;
			el_time_prev = fec_info.el_time;
		}

	} else if (m_fec_screen_mode == eVueFECScreenDataFull) {

		LOG_INFO("VueFEC update full data");

		// treat rollover at 256
		uint8_t rollof = fec_info.el_time;
		rollof -= el_time_prev;
		rollof &= 0x3F;
		if (rollof) {
			m_el_time += rollof;
			el_time_prev = fec_info.el_time;
		}

		this->cadranH(1, VUE_FEC_NB_LINES, "Time", _secjmkstr(m_el_time, ':'), NULL);

		this->cadran(2, VUE_FEC_NB_LINES, 1, "CAD", _imkstr(bsc_info.cadence), "rpm");
		this->cadran(2, VUE_FEC_NB_LINES, 2, "HRM", _imkstr(hrm_info.bpm), "bpm");

		this->cadran(3, VUE_FEC_NB_LINES, 1, "Score", _fmkstr(suffer_score.getScore(), 1U), NULL);
		this->cadranZones(3, VUE_FEC_NB_LINES, 2, "PZone", zPower);

		this->cadran(4, VUE_FEC_NB_LINES, 1, "Pwr", _imkstr(fec_info.power), "W");
		this->cadran(4, VUE_FEC_NB_LINES, 2, "Speed", _fmkstr((float)fec_info.speed / 10., 1U), "km/h");

		sVueHistoConfiguration h_config;
		h_config.cur_elem_nb = boucle_fec.m_pw_buffer.size();
		h_config.ref_value   = (tHistoValue)210;
		h_config.max_value   = (tHistoValue)500;
		h_config.nb_elem_tot = FEC_PW_BUFFER_NB_ELEM;
		h_config.p_f_read    = _vue_fec_pw_rb_read;

		this->HistoH(5, VUE_FEC_NB_LINES, h_config);
	}

	this->cadran(6, VUE_FEC_NB_LINES, 1, "Cur", _imkstr((int)stc.getCurrent()), "mA");
	this->cadran(6, VUE_FEC_NB_LINES, 2, "SOC", _imkstr(percentageBatt(stc.getVoltage(), stc.getCurrent())), "%");

	return res;
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

		int16_t width = regFenLim((float)data.getTimeZX(i), 0, tot, 2, _width / 2 - 30);
		this->fillRect(x - _width / 2 + 10, y + 20 + i*6, width, 4, 1);

		if (i == cur_zone) {
			setCursor(x - _width / 2 + 20 + width, y + 14 + i*6);
			setTextSize(2);
			print((char)('<'));
		}
	}


	// print delimiters
	drawFastVLine(_width / 2, _height / nb_lig * (p_lig - 1), _height / nb_lig, 1);

	if (p_lig > 1)      drawFastHLine(_width * (p_col - 1) / 2, _height / nb_lig * (p_lig - 1), _width / 2, 1);
	if (p_lig < nb_lig) drawFastHLine(_width * (p_col - 1) / 2, _height / nb_lig * p_lig, _width / 2, 1);

}
