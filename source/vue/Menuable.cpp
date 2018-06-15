/*
 * Menuable.cpp
 *
 *  Created on: 13 déc. 2017
 *      Author: Vincent
 */

#include "millis.h"
#include "Model.h"
#include "nrf_ASSERT.h"
#include <vue/Menuable.h>

static void _retour_menu(int var) {

	if (!vue.m_is_menu_selected) return;

	if (vue.m_menus.cur_page == 0) {
		vue.m_is_menu_selected = false;
	} else {
		vue.m_menus.cur_page = 0;
	}

}

static void _page0_mode_crs(int var) {

	vue.setCurrentMode(eVueGlobalScreenCRS);
	boucle.changeMode(eBoucleGlobalModesCRS);
	_retour_menu(0);
}

static void _page0_mode_prc(int var) {

	vue.setCurrentMode(eVueGlobalScreenPRC);
	boucle.changeMode(eBoucleGlobalModesPRC);
	_retour_menu(0);
}

static void _page0_mode_fec(int var) {

	vue.setCurrentMode(eVueGlobalScreenFEC);
	boucle.changeMode(eBoucleGlobalModesFEC);
	_retour_menu(0);
}

static void _page0_shutdown(int var) {

	// TODO shutdown
	_retour_menu(0);
}

void menu_init_page(sMenuPage *page) {
	ASSERT(page);

	page->nb_elem = 1;
	page->item[0].name   = "Retour";
	page->item[0].p_func = _retour_menu;
}

void menu_add_item(sMenuPage *page, const char *name, f_menu_callback callback) {
	ASSERT(page);

	page->item[page->nb_elem].name   = name;
	if (callback) {
		page->item[page->nb_elem].p_func = callback;
	} else {
		page->item[page->nb_elem].p_func = 0;
	}

	page->nb_elem += 1;
}

Menuable::Menuable() : Adafruit_GFX(0, 0) {
	m_is_menu_selected = false;
	m_ind_selec        = 0;
	m_menus.cur_page   = 0;
}

Menuable::~Menuable() {
	// Auto-generated destructor stub
}

void Menuable::initMenu(void) {

	for (uint16_t i=0; i < MENU_MAX_ITEMS_NB; i++) {
		menu_init_page(&m_menus.menu_page[i]);
	}

	// page 0
	menu_add_item(&m_menus.menu_page[0], "Mode FEC", _page0_mode_fec);
	menu_add_item(&m_menus.menu_page[0], "Mode CRS", _page0_mode_crs);
	menu_add_item(&m_menus.menu_page[0], "Mode PRC", _page0_mode_prc);

	menu_add_item(&m_menus.menu_page[0], "Shutdown", _page0_shutdown);

}

void Menuable::refreshMenu(void) {

	this->refresh();

}

void Menuable::propagateEvent(eButtonsEvent event) {

	if (millis() < 5000) return;

	switch (event) {
	case eButtonsEventLeft:
	{
		if (m_is_menu_selected) {
			m_ind_selec += m_menus.menu_page[m_menus.cur_page].nb_elem;
			m_ind_selec = (m_ind_selec - 1) % m_menus.menu_page[m_menus.cur_page].nb_elem;
			this->refreshMenu();
		}
	}
	break;
	case eButtonsEventRight:
	{
		if (m_is_menu_selected) {
			m_ind_selec = (m_ind_selec + 1) % m_menus.menu_page[m_menus.cur_page].nb_elem;
			this->refreshMenu();
		}
	}
	break;
	case eButtonsEventCenter:
	{
		if (!m_is_menu_selected) {
			m_is_menu_selected = true;
			m_ind_selec = 0;
			m_menus.cur_page   = 0;
		} else {
			//perform the item's action
			ASSERT(m_ind_selec < m_menus.menu_page[m_menus.cur_page].nb_elem);

			if (m_menus.menu_page[m_menus.cur_page].item[m_ind_selec].p_func)
				(m_menus.menu_page[m_menus.cur_page].item[m_ind_selec].p_func)(0);
		}
		this->refreshMenu();
	}
	break;

	default:
		break;
	}

}

void Menuable::tasksMenu(void) {

	this->setCursor(0, 20);

	if (m_is_menu_selected) {
		for (uint16_t i=0; i < MENU_MAX_ITEMS_NB; i++) {

			this->setTextSize(3);
			this->println();
			this->print(" ");
			this->print(m_menus.menu_page[m_menus.cur_page].item[i].name);

			this->setTextSize(2);
			if (i==m_ind_selec) {
				this->print(" <<--");
			}
		}
	}
}
