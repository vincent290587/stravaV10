/*
 * Menuable.cpp
 *
 *  Created on: 13 dÃ©c. 2017
 *      Author: Vincent
 */

#include "millis.h"
#include "Model.h"
#include "gpio.h"
#include "sd_hal.h"
#include "assert_wrapper.h"
#include "segger_wrapper.h"
#include "sd_functions.h"
#include "MenuObjects.h"
#include "Menuable.h"


static MenuPage m_main_page(vue, nullptr);
static MenuPage prc_sel(vue, &m_main_page);
static MenuPage page_set(vue, &m_main_page);


static eFuncMenuAction _page0_mode_crs(int var) {

	vue.setCurrentMode(eVueGlobalScreenCRS);
	boucle.changeMode(eBoucleGlobalModesCRS);
	return eFuncMenuActionEndMenu;
}

static eFuncMenuAction _page0_mode_prc_go(int var) {

	LOG_INFO("parcoursSelect %d", var);
	vue.parcoursSelect(var);

	vue.setCurrentMode(eVueGlobalScreenPRC);
	boucle.changeMode(eBoucleGlobalModesPRC);

	return eFuncMenuActionEndMenu;
}

static eFuncMenuAction _page0_mode_prc_list(int var) {

	if (!mes_parcours.size()) {
		vue.addNotif("Error", "No PRC in memory", 5, eNotificationTypeComplete);
		LOG_ERROR("No PRC in memory");
		return eFuncMenuActionEndMenu;
	}

	if (prc_sel.nbItems() == 1) {
		// Populate PRC list
		for (auto& object : mes_parcours._parcs) {

			LOG_INFO("Added PRC to list");
			MenuItem item_prc_list(prc_sel, object.getName(), _page0_mode_prc_go);
			prc_sel.addItem(item_prc_list);

		}
	}

	return eFuncMenuActionNone;
}

static eFuncMenuAction _page0_mode_fec(int var) {

	vue.setCurrentMode(eVueGlobalScreenFEC);
	boucle.changeMode(eBoucleGlobalModesFEC);
	return eFuncMenuActionEndMenu;
}

static eFuncMenuAction _page0_mode_debug(int var) {

	vue.setCurrentMode(eVueGlobalScreenDEBUG);
	boucle.changeMode(eBoucleGlobalModesCRS);
	return eFuncMenuActionEndMenu;
}

static eFuncMenuAction _page0_shutdown(int var) {

	// shutdown
	gpio_set(KILL_PIN);

	return eFuncMenuActionEndMenu;
}

static eFuncMenuAction _page0_erase(int var) {

	if (sd_erase_pos()) {
		vue.addNotif("Erasing... ", "", 4, eNotificationTypePartial);
	} else {
		vue.addNotif("Erase failed", "", 4, eNotificationTypePartial);
	}

	return eFuncMenuActionEndMenu;
}

static eFuncMenuAction _page0_format(int var) {

	format_memory();

	vue.addNotif("Formatting... ", "", 4, eNotificationTypePartial);

	return eFuncMenuActionEndMenu;
}

static eFuncMenuAction _page0_settings(int var) {

	return eFuncMenuActionEndMenu;
}

Menuable::Menuable() {
	m_is_menu_selected = false;
}

Menuable::~Menuable() {
	// Auto-generated destructor stub
}

void Menuable::initMenu(void) {

	// Main page
	MenuItem item_fec(m_main_page, "Mode FEC", _page0_mode_fec);
	MenuItem item_crs(m_main_page, "Mode CRS", _page0_mode_crs);
	MenuItem item_prc(m_main_page, "Mode PRC", _page0_mode_prc_list, &prc_sel);
	MenuItem item_deb(m_main_page, "Mode DBG", _page0_mode_debug);
	MenuItem item_era(m_main_page, "Erase GPX", _page0_erase);
	MenuItem item_for(m_main_page, "! Format !", _page0_format);
	MenuItem item_set(m_main_page, "Settings", _page0_settings, &page_set);
	MenuItem item_shu(m_main_page, "Shutdown", _page0_shutdown);

	m_main_page.addItem(item_fec);
	m_main_page.addItem(item_crs);
	m_main_page.addItem(item_prc);
	m_main_page.addItem(item_deb);
	m_main_page.addItem(item_era);
	m_main_page.addItem(item_for);
	m_main_page.addItem(item_set);
	m_main_page.addItem(item_shu);

	// init variables
	this->closeMenu();
}

void Menuable::closeMenu() {
	m_is_menu_selected = false;
	p_cur_page = &m_main_page;
}

void Menuable::goToParentPage(void) {
	if (p_cur_page->getParent()) {
		p_cur_page = p_cur_page->getParent();
	} else {
		// page has no parent
		this->closeMenu();
	}
}

void Menuable::goToChildPage(MenuPage *page) {
	ASSERT(page);
	p_cur_page = page;
}

void Menuable::refreshMenu(void) {

	this->refresh();

}

void Menuable::propagateEvent(eButtonsEvent event) {

	if (millis() < 5000) return;

	LOG_INFO("Menu event %u", event);

	if (!m_is_menu_selected &&
			eButtonsEventCenter == event) {
		m_is_menu_selected = true;
	} else {
		p_cur_page->propagateEvent(event);
	}

	if (!m_is_menu_selected) return;

	this->tasksMenu();

	this->refreshMenu();
}

void Menuable::tasksMenu(void) {

	p_cur_page->render();

}
