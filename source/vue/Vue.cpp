/*
 * Vue.cpp
 *
 *  Created on: 12 déc. 2017
 *      Author: Vincent
 */

#include <vue/Vue.h>
#include "nordic_common.h"
#include "assert_wrapper.h"
#include "WString.h"
#include "segger_wrapper.h"
#include "Org_01.h"

Vue::Vue() : Adafruit_GFX(LS027_HW_WIDTH, LS027_HW_HEIGHT) {

	m_global_mode = VUE_DEFAULT_MODE;

}

void Vue::init(void) {
	this->setRotation(3);
	this->initMenu();
	this->setTextWrap(false);
	this->setFont(&Org_01);
	LS027_Init();
}

void Vue::tasks(eButtonsEvent event) {

	switch (m_global_mode) {
	case eVueGlobalScreenCRS:
	{
		// propagate to the inner menu
		this->propagateEvent(event);
		break;
	}
	case eVueGlobalScreenFEC:
	{
		// propagate to the inner menu
		this->propagateEvent(event);
		break;
	}
	case eVueGlobalScreenPRC:
	{
		if (this->propagateEventsPRC(event)) {
			// propagate to the inner menu
			this->propagateEvent(event);
		}
		break;
	}
	case eVueGlobalScreenDEBUG:
	{
		// propagate to the inner menu
		this->propagateEvent(event);
		break;
	}
	default:
		break;
	}

}

void Vue::setCurrentMode(eVueGlobalScreenModes mode_) {

	m_global_mode = mode_;

}

void Vue::refresh(void) {

	if (m_is_menu_selected) {
		this->tasksMenu();
	} else {
		switch (m_global_mode) {
		case eVueGlobalScreenCRS:
			this->tasksCRS();
			break;
		case eVueGlobalScreenFEC:
			this->tasksFEC();
			break;
		case eVueGlobalScreenPRC:
			this->tasksPRC();
			break;
		case eVueGlobalScreenDEBUG:
			this->displayDebug();
			break;

		default:
			break;
		}
	}

	// Notifications tasks
	if (m_notifs.size()) {

		Notif& notif = m_notifs.front();

		this->fillRect(0, 0, _width, 60, textbgcolor);
		this->drawFastHLine(0, 60, _width, textcolor);

		this->setCursor(5, 5);
		this->setTextSize(2);

		if (eNotificationTypeComplete == notif.m_type) {
			this->print(notif.m_title);
			this->println(":");
		}
		this->print(notif.m_msg);

		notif.m_persist--;
		if (notif.m_persist == 0) {
			m_notifs.pop_front();
		}
	}

	this->writeWhole();
}

void Vue::drawPixel(int16_t x, int16_t y, uint16_t color) {

	if((x < 0) || (x >= _width) || (y < 0) || (y >= _height)) return;

	switch(rotation) {
	case 1:
		adagfxswap(x, y);
		x = WIDTH  - 1 - x;
		break;
	case 2:
		x = WIDTH  - 1 - x;
		y = HEIGHT - 1 - y;
		break;
	case 3:
		adagfxswap(x, y);
		y = HEIGHT - 1 - y;
		break;
	}

	LS027_drawPixel(x, y, color);
}

void Vue::cadranH(uint8_t p_lig, uint8_t nb_lig, const char *champ, String  affi, const char *p_unite) {

	int decal = 0;
	int x = _width / 2 * 0.5;
	int y = _height / nb_lig * (p_lig - 1);

	setCursor(5, y + 8);
	setTextSize(1);

	if (champ) print(champ);

	if (affi.length() > 9) {
		affi = "-----";
	} else {
		decal = (4 - affi.length()) * 6;
	}

	setCursor(x + 20 + decal, y - 10 + (_height / (nb_lig*2)));
	setTextSize(3);
	print(affi);

	setTextSize(1);
	x = _width / 2;
	setCursor(x + 105, y + 8);// y + 42

	if (p_unite) print(p_unite);

	// print delimiters
	if (p_lig > 1) drawFastHLine(0, _height / nb_lig * (p_lig - 1), _width, 1);
	if (p_lig < nb_lig) drawFastHLine(0, _height / nb_lig * (p_lig), _width, 1);
}


void Vue::cadran(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, const char *champ, String  affi, const char *p_unite) {

	int decal = 0;
	int x = _width / 2 * (p_col - 1);
	int y = _height / nb_lig * (p_lig - 1);

	setCursor(x + 5, y + 8);
	setTextSize(1);

	if (champ) print(champ);

	if (affi.length() <= 6) {
		decal = (4 - affi.length()) * 14;
	} else {
		affi = "---";
	}
	setCursor(x + 25 + decal, y - 10 + (_height / (nb_lig*2)));
	setTextSize(3);

	print(affi);

	setTextSize(1);
	setCursor(x + 95, y + 8); // y + 42

	if (p_unite) print(p_unite);

	// print delimiters
	drawFastVLine(_width / 2, _height / nb_lig * (p_lig - 1), _height / nb_lig, 1);

	if (p_lig > 1)      drawFastHLine(_width * (p_col - 1) / 2, _height / nb_lig * (p_lig - 1), _width / 2, 1);
	if (p_lig < nb_lig) drawFastHLine(_width * (p_col - 1) / 2, _height / nb_lig * p_lig, _width / 2, 1);
}

void Vue::HistoH(uint8_t p_lig, uint8_t nb_lig, sVueHistoConfiguration& h_config_) {

	int y_base = _height / nb_lig * (p_lig);

	if (h_config_.cur_elem_nb) {

		ASSERT(h_config_.p_f_read);
		ASSERT(h_config_.nb_elem_tot);

		uint16_t dx = _width / h_config_.nb_elem_tot;

		// iterate in buffer
		for (int i=0; i < h_config_.cur_elem_nb; i++) {

			int x_base0 = i * dx;

			tHistoValue elem   = (h_config_.p_f_read)(i);
			tHistoValue height = elem * _height / (nb_lig * h_config_.max_value);

			uint16_t dy = 10;

			if (h_config_.ref_value) {

				if (elem <= h_config_.ref_value) height = h_config_.ref_value * _height / (nb_lig * h_config_.max_value);
				else                             height -= 1;

				dy = abs(elem - h_config_.ref_value) * _height / (nb_lig * h_config_.max_value);

				dy = MAX(dy, 1);

				drawFastPHLine(0, y_base - h_config_.ref_value * _height / (nb_lig * h_config_.max_value), _width, 1);
			}

			// rectangle complet
			this->fillRect(x_base0, y_base-height, dx, dy, 1);
		}
	}

	// print delimiters
	if (p_lig > 1)      drawFastHLine(0, _height / nb_lig * (p_lig - 1), _width, 1);
	if (p_lig < nb_lig) drawFastHLine(0, _height / nb_lig * (p_lig), _width, 1);
}

void Vue::Histo(uint8_t p_lig, uint8_t nb_lig, uint8_t p_col, sVueHistoConfiguration& h_config_) {

	int x_base = _width / 2 * (p_col - 1);
	int y_base = _height / nb_lig * (p_lig);

	if (h_config_.cur_elem_nb) {

		ASSERT(h_config_.p_f_read);
		ASSERT(h_config_.nb_elem_tot);

		int dx = _width / (2 * h_config_.nb_elem_tot);

		// iterate in buffer
		for (int i=0; i < h_config_.cur_elem_nb; i++) {

			int x_base0 = x_base + i * dx;

			tHistoValue elem   = (h_config_.p_f_read)(i);
			tHistoValue height = elem * _height / (nb_lig * h_config_.max_value);

//			// rectangle complet
//			this->fillRect(x_base0, y_base-height, dx, height, 2);
			// petite barre
			this->fillRect(x_base0, y_base-height, dx, 4, 1);
		}
	}

	// print delimiters
	drawFastVLine(_width / 2, _height / nb_lig * (p_lig - 1), _height / nb_lig, 1);

	if (p_lig > 1)      drawFastHLine(_width * (p_col - 1) / 2, _height / nb_lig * (p_lig - 1), _width / 2, 1);
	if (p_lig < nb_lig) drawFastHLine(_width * (p_col - 1) / 2, _height / nb_lig * p_lig, _width / 2, 1);
}
