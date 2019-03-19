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
#include "ant_device_manager.h"
#include "MenuObjects.h"
#include "Menuable.h"

static void _page1_mode_ant_list(int var);

static MenuPageItems m_main_page(vue, nullptr);
static MenuPageItems prc_sel(vue, &m_main_page);
static MenuPageItems page_set(vue, &m_main_page);

static MenuPageSetting page_value(vue, &page_set);

static MenuPagePairing page_pair(vue, &page_set, _page1_mode_ant_list);


static eFuncMenuAction _page0_mode_crs(int var) {

	vue.setCurrentMode(eVueGlobalScreenCRS);
	boucle.changeMode(eBoucleGlobalModesCRS);
	return eFuncMenuActionEndMenu;
}

static eFuncMenuAction _page0_mode_prc_go(int var) {

	LOG_INFO("parcoursSelect %d", var);

	vue.setCurrentMode(eVueGlobalScreenPRC);
	boucle.changeMode(eBoucleGlobalModesPRC);

	boucle_crs.parcoursSelect(var);

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

static eFuncMenuAction _page1_erase(int var) {

	if (sd_erase_pos()) {
		vue.addNotif("GPX", "Erasing... ", 4, eNotificationTypeComplete);
	} else {
		vue.addNotif("GPX", "Erase failed", 4, eNotificationTypeComplete);
	}

	return eFuncMenuActionEndMenu;
}

static eFuncMenuAction _page1_format(int var) {

	format_memory();

	vue.addNotif("", "Formatting... ", 4, eNotificationTypePartial);

	return eFuncMenuActionEndMenu;
}

static eFuncMenuAction _page0_settings(int var) {

	return eFuncMenuActionNone;
}

static eFuncMenuAction _page1_pair_hrm(int var) {

	ant_device_manager_search_start(eAntPairingSensorTypeHRM);

	return eFuncMenuActionNone;
}

static eFuncMenuAction _page1_pair_bsc(int var) {

	ant_device_manager_search_start(eAntPairingSensorTypeBSC);

	return eFuncMenuActionNone;
}

static eFuncMenuAction _page1_pair_fec(int var) {

	ant_device_manager_search_start(eAntPairingSensorTypeFEC);

	return eFuncMenuActionNone;
}

static eFuncMenuAction _page0_mode_ant_stop(int var) {

	// search cancel
	ant_device_manager_search_cancel();

	return eFuncMenuActionEndMenu;
}

static eFuncMenuAction _page0_mode_ant_go(int var) {

	if (var <= 0) {
		// stop search
		ant_device_manager_search_cancel();

		vue.addNotif("ANT", "Search cancelled", 2, eNotificationTypeComplete);
	} else {

		// sensor selection
		ant_device_manager_search_validate(var);

	}

	return eFuncMenuActionEndMenu;
}

static void _page1_mode_ant_list(int var) {

	page_pair.removeAllItems();

	// populate list: map sensor ID to item
	sAntPairingSensorList* s_list = ant_device_manager_get_sensors_list();

	MenuItem item_can(page_pair, "Cancel", _page0_mode_ant_stop);
	page_pair.addItem(item_can);

	String name;
	for (int i=0; i < s_list->nb_sensors; i++) {

		name = "ANT+";
		name += "   ";
		name += String(s_list->sensors[i].dev_id);
		//name += "   ";
		//name += String(s_list->sensors[i].ssid);

		MenuItem item_sns(page_pair, name.c_str(), _page0_mode_ant_go);

		page_pair.addItem(item_sns);
	}
}

static int get_cur_ftp(int var) {
	return u_settings.getFTP();
}

static int set_cur_ftp(int var) {

	sUserParameters *settings = user_settings_get();
	settings->FTP = (uint16_t) var;

	u_settings.writeConfig();

	return 0;
}

static eFuncMenuAction _page1_set_ftp(int var) {

	sSettingsCallbacks callbacks = {
			get_cur_ftp,
			set_cur_ftp};

	page_value.setCallbacks(callbacks);
	page_value.setName("FTP:");

	return eFuncMenuActionNone;
}

static int get_cur_weight(int var) {
	return u_settings.getWeight();
}

static int set_cur_weight(int var) {

	sUserParameters *settings = user_settings_get();
	settings->weight = (uint16_t) var;

	u_settings.writeConfig();

	return 0;
}

static eFuncMenuAction _page1_set_weight(int var) {

	sSettingsCallbacks callbacks = {
			get_cur_weight,
			set_cur_weight};

	page_value.setCallbacks(callbacks);
	page_value.setName("Weight:");

	return eFuncMenuActionNone;
}

static eFuncMenuAction _page1_start_cal(int var) {

	fxos_calibration_start();

	return eFuncMenuActionEndMenu;
}

Menuable::Menuable() {
	m_is_menu_selected = false;
	p_cur_page = nullptr;
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
	MenuItem item_set(m_main_page, "Settings", _page0_settings, &page_set);
	MenuItem item_shu(m_main_page, "Shutdown", _page0_shutdown);

	m_main_page.addItem(item_fec);
	m_main_page.addItem(item_crs);
	m_main_page.addItem(item_prc);
	m_main_page.addItem(item_deb);
	m_main_page.addItem(item_set);
	m_main_page.addItem(item_shu);


	MenuItem item_prm(page_set, "Pair HRM", _page1_pair_hrm, &page_pair);
	MenuItem item_psc(page_set, "Pair BSC", _page1_pair_bsc, &page_pair);
	MenuItem item_pec(page_set, "Pair FEC", _page1_pair_fec, &page_pair);
	MenuItem item_ftpf(page_set, "Set FTP", _page1_set_ftp, &page_value);
	MenuItem item_weif(page_set, "Set Weight", _page1_set_weight, &page_value);
	MenuItem item_cal(page_set, "Cal. mag", _page1_start_cal);
	MenuItem item_era(page_set, "Erase GPX", _page1_erase);
	MenuItem item_for(page_set, "! Format !", _page1_format);

	page_set.addItem(item_prm);
	page_set.addItem(item_psc);
	page_set.addItem(item_pec);
	page_set.addItem(item_ftpf);
	page_set.addItem(item_weif);
	page_set.addItem(item_cal);
	page_set.addItem(item_era);
	page_set.addItem(item_for);


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
	} else if (m_is_menu_selected) {
		p_cur_page->propagateEvent(event);
	}

	if (!m_is_menu_selected) return;

	this->tasksMenu();

	this->refreshMenu();
}

void Menuable::tasksMenu(void) {

	p_cur_page->render();

}
